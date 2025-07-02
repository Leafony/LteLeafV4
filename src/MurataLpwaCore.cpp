/*
 * MurataLpwaCore.cpp
 * LTE-M Leaf V4 library
 *
 * 2021.12.28 kt-nakamura@kddi-tech.com
 */

#include "MurataLpwaCore.h"

// #define UART_DBG
#define COMMAND_DBG

#define HTTP_DBG_LEVEL 0

#define MDM_RXBUFFSIZE 4000
/*
 * TCA6407 I2C GPIO
 */
void writeTca6408(uint8_t data, uint8_t reg)
{
  Wire.beginTransmission(TCA6408_ADDR);
  Wire.write((uint8_t)reg);
  Wire.write((uint8_t)data);
  Wire.endTransmission();
  return;
}

bool readTca6408(uint8_t *data, uint8_t reg)
{
  Wire.beginTransmission(TCA6408_ADDR);
  Wire.write((uint8_t)reg);
  Wire.endTransmission();
  uint8_t timeout = 0;

  Wire.requestFrom(TCA6408_ADDR, (uint8_t)0x01);
  while (Wire.available() < 1)
  {
    timeout++;
    if (timeout > TCA6408_TIMEOUT)
    {
      return (true);
    }
    delay(1);
  }
  *data = Wire.read();
  return (false);
}

MurataLpwaCore::MurataLpwaCore(HardwareSerial &serial, const unsigned long baud)
    : _serial(serial), _baud(baud), status(IDLE), _power(false), _init(false) {}

MurataLpwaCore::~MurataLpwaCore() {}

char mdm_rxbuff[MDM_RXBUFFSIZE]; // 最大受信サイズ1500 x2
int mdm_rxbuff_p;
int mdm_rxfind_p;

#define MAX_RESULT 8 // モデム応答の最大要素数

/**
 * モデム応答を分割する
 * @param instring 入力文字列 処理後に文字列が破壊される(NULLが入る)ので注意
 * @param separator 分割文字
 * @param result 分割文字列を入れるポインタ
 * @param max_size 最大分割数
 * @return 分割された文字列数
 */
int MurataLpwaCore::split_resp(char *instring, const char *separator, char **result, size_t max_size)
{
  // 入力チェック
  if ((instring == NULL) || (separator == NULL) || (result == NULL) || (max_size < 1))
  {
    return -1;
  }

  // 改行以降は判定させないよう削除する
  char *cr_p = NULL;
  cr_p = strchr(instring, '\n');
  if (cr_p != NULL)
  {
    *cr_p = 0x0; // 終端文字を入れる
  }
  cr_p = strchr(instring, '\r');
  if (cr_p != NULL)
  {
    *cr_p = 0x0; // 終端文字を入れる
  }
  // Serial.print("@@@@@ MurataLpwaCore::split_resp() step1: ");
  // Serial.println(instring);

  // レスポンス部のセパレータを探す
  char *start_p = strstr(instring, ":");
  //  char *start_p = strstr(instring, ": ");
  if (start_p == NULL)
  {
    return -1;
  }
  start_p++; // セパレータをスキップ
             // start_p++;

  // Serial.print("@@@@@ MurataLpwaCore::split_resp() step2: ");
  // Serial.println(start_p);

  // 文字列分割
  int ret = 0;
  char *res_p = strtok(start_p, separator);
  if (res_p != NULL)
  {
    while ((res_p != NULL) && (ret < max_size))
    {
      result[ret++] = res_p;
      res_p = strtok(NULL, separator);
    }
  }
  else
  {
    // セパレータが見つからない場合は全部
    if (strlen(start_p) > 0)
    {
      result[ret++] = start_p;
    }
  }
  return ret;
}

/*
 * char配列からchar-hex配列への変換
 * @param chrArray char配列(input)
 * @param chrArray hex配列(output)
 * @param size 変換サイズ
 */
void MurataLpwaCore::atohArray(char *chrArray, char *hexArray, size_t size)
{
  char *write_p = hexArray;
  for (size_t i = 0; i < size; i++)
  {
    sprintf(write_p, "%02x", (char)chrArray[i]);
    write_p++;
    write_p++;
  }
}

/*
 * char-hex配列からchar配列への変換
 * @param chrArray hex配列(input)
 * @param chrArray char配列(output)
 * @param size 変換サイズ
 */
void MurataLpwaCore::htoaArray(char *hexArray, char *chrArray, size_t size)
{
  char temp[3];
  for (size_t i = 0; i < size; i++)
  {
    temp[0] = hexArray[i * 2];
    temp[1] = hexArray[i * 2 + 1];
    temp[2] = '\0';
    chrArray[i] = (char)strtol(temp, NULL, 16);
  }
}

/*
 * "aa.bb.cc.dd"形式のIPアドレスをIPAdress形式に変換
 * @param ipstring IPアドレス文字列
 *
 */
IPAddress MurataLpwaCore::str2Ip(char *ipstring)
{
  IPAddress ip;
  char *ipstart_p = NULL;
  char *ipend_p = NULL;
  ipstart_p = strstr(ipstring, "\"");
  if (ipstart_p == NULL)
    return ip;
  ipstart_p++;
  ipend_p = strstr(ipstart_p, "\"");
  if (ipend_p == NULL)
    return ip;
  *ipend_p = '.';
  for (int i = 0; i < 4; i++)
  {
    ipend_p = strstr(ipstart_p, ".");
    if (ipend_p != NULL)
    {
      *ipend_p = 0x0;
    }
    ip[i] = atoi(ipstart_p);
    ipstart_p = ipend_p + 1;
  }
  return ip;
}
void MurataLpwaCore::power(bool enable)
{
  //  Serial.println("@@@@@ MurataLpwaCore::power() enter");

  if (!_power && enable)
  {
    // LTE-Mモデムの電源オン→ウェイクアップ
    Serial.println("<info> lpwa device power_up");
    writeTca6408(0x0, TCA6408_OUTPUT);                             // clear output register
    writeTca6408(0x0,TCA6408_PULLUD_ENABLE);                       // All pull up/down disable
    writeTca6408(LTE_RST_STS | LTE_SC_SWP, TCA6408_CONFIGURATION); // input bit[7:6}
    writeTca6408(LTE_PWR_ON | LTE_WAKEUP, TCA6408_OUTPUT);      //
    delay(10);
    writeTca6408(LTE_PWR_ON | LTE_SHUTDOWNn | LTE_WAKEUP, TCA6408_OUTPUT); // Write values to IO-expander
    _power = true;
  }

  if (_power && !enable)
  {
    // LTE-Mモデムの電源オフ
    Serial.println("<info> lpwa device power_down");
    writeTca6408(0x0, TCA6408_OUTPUT);                             // clear output register
    writeTca6408(0, TCA6408_CONFIGURATION);                        // set all port to output
    _power = false;
  }
}

void MurataLpwaCore::wakeup(bool enable)
{
  //  Serial.println("@@@@@ MurataLpwaCore::wakeup() enter");
  if (_power)
  {
    if (enable)
    {
      Serial.println("<info> lpwa device wakeup");
      writeTca6408(LTE_PWR_ON | LTE_SHUTDOWNn | LTE_WAKEUP, TCA6408_OUTPUT); // Write values to IO-expander
    }
    else
    {
      //      Serial.println("@@@@@ MurataLpwaCore::wakeup() disable");
      writeTca6408(LTE_PWR_ON | LTE_SHUTDOWNn, TCA6408_OUTPUT); // Write values to IO-expander
    }
  }
}

void MurataLpwaCore::setBatteryManagement(bool enable)
{
  // Serial.println("@@@@@ MurataLpwaCore::setBatteryManagement() enter");
  uint8_t data;
  readTca6408(&data, TCA6408_OUTPUT); // Read values from IO-expander
  if (enable)
  {
    data = data | BM_ON;
  }
  else
  {
    data = data & ~BM_ON;
  }
  //  Serial.print("GPIO:");
  //  Serial.println(data,HEX);
  writeTca6408(data, TCA6408_OUTPUT); // Write values to IO-expander
  return;
}

/**
 * 電源をオンにする
 * @param なし
 * @return 通信状態
 */
NetworkStatus MurataLpwaCore::begin()
{
  //  Serial.println("@@@@@ MurataLpwaCore::begin() enter");
  mdm_rxbuff[0] = 0;
  mdm_rxbuff_p = 0;
  mdm_rxfind_p = 0;
  rcv_length = 0;
  pdp_stat = 0;
  socket_event = 0;
  //  _buffer = 0;
  //  _buffer_p = 0;
  http_resp = 0;

  // GPIO初期化
  pinMode(MDM_USART_CTS, OUTPUT);
  digitalWrite(MDM_USART_CTS, LOW);
  //pinMode(MDM_USART_RTS, INPUT);
  //pinMode(MDM_USART_RXD, INPUT);
  pinMode(MDM_USART_TXD, OUTPUT);
  digitalWrite(MDM_USART_TXD, LOW);

  if (!_init)
  {
    Serial.println("@@@@@ MurataLpwaCore::begin() init 1st time");
    // I2C初期化
    Wire.setSDA(I2C2_SDA);
    Wire.setSCL(I2C2_SCL);
    Wire.begin();
    _init = true;
  }

  // モデム電源オン
  power(true);
  delay(1000);

  // UART初期化
  _serial.begin(_baud);

  // ダミーATコマンドでモデム死活チェック
  int remainChk = 10;
  while (remainChk > 0)
  {
    sendCmd("AT\r");
    if (waitForResponse("OK\r", NULL, 0,CMD_TIMEOUT_DEFAULT,true) == 0)
    {
      remainChk = 0;
    }
    else
    {
      remainChk--;
      if (remainChk < 1)
      {
        Serial.println("<error> modem not respond!");
        return status = LPWA_FAIL; // 初期化失敗
      }
    }
  }

  // ハードウェアハンドシェイク設定
  sendCmd("AT&K3\r");
  waitForResponse("OK\r");
  digitalWrite(MDM_USART_CTS, HIGH);

  sendCmd("ATE0\r");
  waitForResponse("OK\r");

  /** All HTTP events enable **/
  sendCmd("AT%HTTPEV=\"ALL\",1\r");
  waitForResponse("OK\r");

  /** HTTPCFG "FORMAT" for prof1
     param1 = 0 (Data text mode)
     param2 = 1 (Enable Response header presence as a part of <data>)
     param3 = 0 (Disable Request header presence as a part of <data>)
  **/
  sendCmd("AT%HTTPCFG=\"FORMAT\",1,0,1,0\r");
  waitForResponse("OK\r");

  Serial.println("<info> lpwa device ready.");
  return status = LPWA_READY; // 正常終了
}

/**
 * 電源をオフにする
 */
void MurataLpwaCore::end()
{
  //  Serial.println("@@@@@ MurataLpwaCore::end()");
  //  if(_buffer != 0) {
  //    delete[] _buffer;
  //  }
  //  Wire.end();
  _serial.end();

  // モデム電源オフ
  power(false);

  // GPIO,UART無効化
  digitalWrite(MDM_USART_CTS, LOW);
  digitalWrite(MDM_USART_TXD, LOW);

  status = LPWA_OFF;
}

/**
 * ATコマンドを送信する
 * @param command ATコマンド
 */
bool MurataLpwaCore::sendCmd(const char *command)
{
  int len = strlen(command);
  uint8_t *tmp = (uint8_t *)command;
  debugPrint(1, "@@@@@ sendCmd: %s\r\n", command);
  bool retval = sendBin(tmp, len, 5000); // バイナリとして送信

  mdm_rxbuff_p = 0; // 受信ポインタをリセット
  return retval;
}

/**
 * ATコマンドを送信する
 * @param format ATコマンド (printf形式)
 */
bool MurataLpwaCore::sendf(const char *format, ...)
{
  char buffer[MAX_COMMAND_SIZE];
  va_list ap;
  va_start(ap, format);
  vsnprintf(buffer, sizeof(buffer) - 1, format, ap);
  va_end(ap);

  return sendCmd(buffer);
}

/**
 * バイナリデータをモデムへ送信する
 * @param data バイナリデータ
 * @param size 送信データ長
 */
bool MurataLpwaCore::sendBin(uint8_t *data, int size, const unsigned long timeout)
{
  uint8_t *tmp = data;

  for (int i = 0; i < size; i++)
  {
    int remain = timeout / 100;
    while (digitalRead(MDM_USART_RTS) == 1)
    {
      debugPrint(3, "<RTS>");
      if (remain < 0)
      {
        Serial.println("<error> write error!(RTS timeout)");
        return false;
      }
      remain--;
      delay(100);
    }
#ifdef UART_DBG
    Serial.print("[");
    Serial.print(*tmp, HEX);
    Serial.print("]");
#endif // UART_DBG
    _serial.write(*tmp++);
  }
#ifdef UART_DBG
  Serial.println("");
#endif // UART_DBG

  _serial.flush();
  return true;
}

/**
 * モデムのシリアルポートに出力する
 * @param format 出力する文字列 (printf形式)
 */
size_t MurataLpwaCore::printf(const char *format, ...)
{
#if 0
//  char buffer[BUFSIZ];
  char buffer[MAX_COMMAND_SIZE];
  va_list ap;
  va_start(ap, format);
  vsnprintf(buffer, sizeof(buffer) - 1, format, ap);
  va_end(ap);

  DEBUG();
  DEBUG("```printf");
  DEBUG(buffer);
  DEBUG("```");

  return _serial.print(buffer);
#endif
  return -1;
}

char *MurataLpwaCore::readBytes(int size, int chunked)
{
  debugPrint(3, "@ readBytes: %d, %d", size, chunked);
  size += 4;
  digitalWrite(MDM_USART_CTS, LOW);
  memset(mdm_rxbuff, 0, MDM_RXBUFFSIZE);
  mdm_rxbuff_p = 0;
  int count = 0;
  while (size > 0)
  {
    int avail = _serial.available();
    if (avail > 0)
    {
      digitalWrite(MDM_USART_CTS, HIGH);
      char c = _serial.read();
      mdm_rxbuff[mdm_rxbuff_p++] = c;
      if (--size <= 0)
      {
        break;
      }
      digitalWrite(MDM_USART_CTS, LOW);
    }
  }
  mdm_rxbuff[mdm_rxbuff_p - 4] = 0;
  mdm_rxbuff_p = 0;
  digitalWrite(MDM_USART_CTS, HIGH);
  return mdm_rxbuff;
}

/**
 * 繰り返し呼び出して応答を待つ
 * @param expectedVal 期待するモデム応答文字列
 * @param lead true: 行頭判定、 false: 行末判定
 * @return 0: データなし、1: データあり、-1: エラー検出
 */
int MurataLpwaCore::poll(const char *expectedVal)
{
  digitalWrite(MDM_USART_CTS, LOW);
  delay(1);

  while (_serial.available() > 0)
  {
    char c = _serial.read();
    mdm_rxbuff[mdm_rxbuff_p++] = c; // 受信データをバッファに積む
#ifdef UART_DBG
    debugPrint(3, "[%x]", c);
#endif // UART_DBG

#if 1
    if (!(((c >= 0x20) && (c < 0x7f)) || (c == '\r') || (c == '\n')))
    {
      mdm_rxfind_p = mdm_rxbuff_p; // 制御文字が来た場合は検索開始ポインタを進めておく
    }

    if (mdm_rxbuff_p > 2)
    {
      if ((mdm_rxbuff[mdm_rxbuff_p - 2] == '\r') && (mdm_rxbuff[mdm_rxbuff_p - 1] == '\n'))
      {
        digitalWrite(MDM_USART_CTS, HIGH); // 処理中はモデムからの送信を止めさせる
        mdm_rxbuff[mdm_rxbuff_p] = 0x0;    // 終端を追加

        // モデム応答チェック
        bool respValid = false; // 有効な応答があった？
        char *pdest = NULL;
        char *mdm_result[MAX_RESULT];
        char *find_string = mdm_rxbuff + mdm_rxfind_p;
        // %SOCKETEV:<event_id>,<socket_id>
        pdest = strstr(mdm_rxbuff, "%SOCKETEV:");
        if (pdest != NULL)
        {
#ifdef COMMAND_DBG
          debugPrint(3, "@@@@@ recive notif: %s\r\n", pdest);
#endif // COMMAND_DBG
          int cnt = split_resp(pdest, ",", mdm_result, MAX_RESULT);
          if (cnt > 0)
          {
            socket_event = atoi(mdm_result[0]);
            //            Serial.print("@@@@@ %SOCKETEV:=");
            //            Serial.println(socket_event);
          }
          respValid = true;
        }

        // %PDNACT:<sessionID>,<stat>,<APN>,<cid>
        pdest = strstr(mdm_rxbuff, "%PDNACT:");
        if (pdest != NULL)
        {
#ifdef COMMAND_DBG
          debugPrint(3, "@@@@@ recive notif: %s\r\n");
#endif // COMMAND_DBG
          int cnt = split_resp(pdest, ",", mdm_result, MAX_RESULT);
          if (cnt > 0)
          {
            pdp_stat = atoi(mdm_result[1]);
            switch (pdp_stat)
            {
            case 0:
              Serial.println("<info> PDP: idle");
              break;
            case 1:
              Serial.println("<info> PDP: connect");
              break;
            }
          }
          respValid = true;
        }

        // 期待値の応答チェック
        if (expectedVal)
        {
          pdest = strstr(find_string, expectedVal);
          if (pdest != NULL)
          {
            int ret = mdm_rxbuff_p;
#ifdef COMMAND_DBG
            debugPrint(3, "@@@@@ Response [%s]\r\n", mdm_rxbuff);
#endif // COMMAND_DBG
            mdm_rxbuff_p = 0;
            mdm_rxfind_p = 0;
            //            digitalWrite(MDM_USART_CTS, LOW); // 受信を再開
            digitalWrite(MDM_USART_CTS, HIGH);
            return ret;
          }
        }
        // エラー応答チェック１
        pdest = strstr(mdm_rxbuff, "ERROR\r");
        if (pdest != NULL)
        {
          //          _buffer = "ERROR";
          mdm_rxbuff_p = 0;
          mdm_rxfind_p = 0;
#ifdef COMMAND_DBG
          debugPrint(3, "@@@@@ Response ERROR : %s\r\n", mdm_rxbuff);
#endif // COMMAND_DBG
          digitalWrite(MDM_USART_CTS, HIGH);
          return -1;
        }
        if (respValid)
        {
          mdm_rxbuff_p = 0;
          mdm_rxfind_p = 0;
        }
        else
        {
          mdm_rxfind_p = mdm_rxbuff_p; // 検索対象がない場合は検索開始ポインタを進めておく
        }

        digitalWrite(MDM_USART_CTS, LOW); // 受信を再開
      }
    }
#endif
  }

  // 受信バッファが空になった
  digitalWrite(MDM_USART_CTS, HIGH);
  return 0;
}
/**
 * モデム応答を待つ
 * @param expectedVal 期待するモデム応答文字列
 * @param respData 応答文字列を入れるバッファ(NULLの場合は応答待ちのみ)
 * @param respSize 応答文字列の許容サイズ
 * @param timeout 応答タイムアウト時間(ms)
 * @param silent メッセージ抑止
 * @return 0: データなし、1<: データあり、-1: エラー検出
 */
int MurataLpwaCore::waitForResponse(const char *expectedVal, char *respData, int respSize, const unsigned long timeout, bool silent)
{
  const unsigned long end = millis() + timeout;
  int ret_len = 0;
  debugPrint(2, "@ waitForResponse <%s>\r\n", expectedVal);
  while (1)
  {
    int resp = poll(expectedVal);
    if (resp > 0)
    {
      if (respData != NULL)
      { // 戻り値バッファが空の場合は判定だけ行い終了
        if (resp < respSize)
        {
          ret_len = resp;
        }
        else
        {
          ret_len = respSize; // 許容値よりも大きいデータが来たら許容値respSizeをセット
        }
        memcpy(respData, mdm_rxbuff, ret_len);
        respData[ret_len] = 0x0; // 終端コードを追加
        debugPrint(2, "@@@@@ waitForResponse: [%s]\r\n", respData);
      }
      mdm_rxbuff_p = 0;
      return ret_len;
    }
    if (resp < 0)
    {
      // エラー応答
      if (!silent)
        Serial.println("<error> lpwa modem error(error response)");
      return -1;
    }
    if (millis() > end)
    {
      if (!silent)
        Serial.println("<error> lpwa modem error(response timeout)");
      debugPrint(2, "@[%s]\r\n", mdm_rxbuff);
      mdm_rxbuff_p = 0;
      return -1;
    }
  }

  Serial.println("<error> lpwa modem error(unknown)");
  return -1;
}

/**
 * デバッグプリント
 * @param format printf形式
 */
void MurataLpwaCore::debugPrint(int lev, const char *format, ...)
{
#if HTTP_DBG_LEVEL != 0
  char buffer[4000];
  va_list ap;
  va_start(ap, format);
  if (lev <= HTTP_DBG_LEVEL)
  {
    vsnprintf(buffer, sizeof(buffer) - 1, format, ap);
    va_end(ap);
    for (char *p = buffer; *p; ++p)
    {
      Serial.print(*p);
      delay(1);
    }
  }
#endif
}

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_SERIAL)
#ifdef ESP32
// NOTE: with esp32 leaf board
MurataLpwaCore theMurataLpwaCore(MDM_SERIAL, 115200);

#endif                                                   // ESP32
HardwareSerial MDM_SERIAL(MDM_USART_RXD, MDM_USART_TXD); // RX,TX
MurataLpwaCore theMurataLpwaCore(MDM_SERIAL, 115200);

#endif // !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_SERIAL)
