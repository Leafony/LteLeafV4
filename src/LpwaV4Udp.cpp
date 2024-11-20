/*
 * LpwaV4Udp.cpp
 * LTE-M Leaf V4 library
 *
 * 2021.12.28 kt-nakamura@kddi-tech.com
 */
#include "LpwaV4Udp.h"

LpwaV4Udp::LpwaV4Udp()
    : _socketId(0), _ip(0, 0, 0, 0), _host(""), _port(0), _readBuffP(0), _rcvBufferP(0), _sndBufferP(0) {}

LpwaV4Udp::~LpwaV4Udp()
{
}
//// AT%SOCKETCMD=<cmd>[,<param1>[,<param2>[,<param3>...]]]
// AT%SOCKETCMD="ALLOCATE",0,"UDP","OPEN","140.227.119.36",12347,{source}
//%SOCKETCMD:1

int LpwaV4Udp::_beginPacket()
{
  //  Serial.println("@@@@@ LpwaV4Udp::_beginPacket() enter");
  int resp_size = strlen(_host) + 100; // コマンド応答値の長さ(_hostで可変)
  char rcvbuff[resp_size];
  char *start_p = NULL;
  char *end_p = NULL;

  if (_socketId != 0)
  {
    // 既にソケットが空いている場合は削除する
    //   Serial.println("<warn> UDP socket already opened!");
    if (!theMurataLpwaCore.sendf("AT%%SOCKETCMD=\"DELETE\",%d\r", _socketId))
      Serial.println("<warn> UDP socket close failed(send)");
    if (theMurataLpwaCore.waitForResponse("OK\r") < 0)
    {
      Serial.println("<warn> UDP socket close failed(rcv)");
    }
    _socketId = 0;
  }

  // ソケット設定コマンド
  if (!theMurataLpwaCore.sendf("AT%%SOCKETCMD=\"ALLOCATE\",0,\"UDP\",\"OPEN\",\"%s\",%d,0,0,,1\r", _host, _port))
    return 0;
  if (theMurataLpwaCore.waitForResponse("OK\r", rcvbuff, resp_size, CMD_TIMEOUT_NETWORK) < 0)
    return 0;

  // コマンド戻り値からソケット番号を取得
  start_p = strstr(rcvbuff, "%SOCKETCMD:");
  if (start_p == NULL)
  {
    Serial.println("<error> invalid response: ");
    return 0;
  }
  start_p += strlen("%SOCKETCMD:");
  _socketId = atoi(start_p);
  // Serial.print("@@@@@@ LpwaV4Udp::_beginPacket() socket_id=");
  // Serial.println(_socketId);

  // ソケット接続
  if (!theMurataLpwaCore.sendf("AT%%SOCKETCMD=\"ACTIVATE\",%d\r", _socketId))
    return 0;
  if (theMurataLpwaCore.waitForResponse("OK\r", NULL, 0, 5000) < 0)
  {
    Serial.println("<error> UDP socket failed!");
    return 0;
  }

  // 接続成功
  Serial.println("<info> UDP socket connected");
  _sndBufferP = 0;
  _rcvBufferP = 0;
  return 1;
}

uint8_t LpwaV4Udp::begin(uint16_t localPort)
{
  // Note: LPWA では　localPort を指定できない
  return 1;
}

int LpwaV4Udp::beginPacket(IPAddress ip, uint16_t port)
{
  //  Serial.println("@@@@@ LpwaV4Udp::beginPacket(ip)  enter");
  sprintf(_host, "%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);
  _port = port;
  return _beginPacket();
}

int LpwaV4Udp::beginPacket(const char *host, uint16_t port)
{
  //  Serial.println("@@@@@ LpwaV4Udp::beginPacket(host)  enter");
  //  Serial.print("@@@@@ host: ");
  //  Serial.println(host);
  //  Serial.print("@@@@@ port: ");
  //  Serial.println(port);
  sprintf(_host, "%s", host);
  _port = port;
  return _beginPacket();
}

int LpwaV4Udp::endPacket()
{
  //  Serial.println("@@@@@ LpwaV4Udp::endPacket() enter");
  if ((_socketId != 0) && (_sndBufferP > 0))
  {

    // UDPコマンドを送信
    if (!theMurataLpwaCore.sendf("AT%%SOCKETDATA=\"SEND\",%d,%d,\"", _socketId, _sndBufferP))
    {
      Serial.println("@@@@@ LpwaV4Udp::endPacket() error1");
      _sndBufferP = 0;
      _rcvBufferP = 0;
      return 0;
    }

    // データ部を送信
    for (int i = 0; i < _sndBufferP; i++)
    {
      if (!theMurataLpwaCore.sendf("%02x", _sndBuffer[i]))
      {
        Serial.println("@@@@@ LpwaV4Udp::endPacket() error2");
        _sndBufferP = 0;
        _rcvBufferP = 0;
        return 0;
      }
    }

    // 終端コードを送信
    if (!theMurataLpwaCore.sendCmd("\"\r"))
    {
      Serial.println("@@@@@ LpwaV4Udp::endPacket() error3");
      _sndBufferP = 0;
      _rcvBufferP = 0;
      return 0;
    }

    // コマンド終了待ち
    if (theMurataLpwaCore.waitForResponse("OK\r") < 0)
    {
      Serial.println("@@@@@ LpwaV4Udp::endPacket() error4");
      _sndBufferP = 0;
      _rcvBufferP = 0;
      return 0;
    }

    //    Serial.println("@@@@@ LpwaV4Udp::endPacket() exit");
    _sndBufferP = 0;
    _rcvBufferP = 0;
    return 1;
  }
  return 0;
}

size_t LpwaV4Udp::write(const uint8_t c) { return write(&c, 1); }

size_t LpwaV4Udp::write(const uint8_t *buffer, size_t size)
{
  for (size_t i = 0; i < size; i++)
    _sndBuffer[_sndBufferP++] = (char)buffer[i];

  return size;
}

void LpwaV4Udp::stop()
{
  //  Serial.println("@@@@@ LpwaV4Udp::stop() enter");
  if (_socketId == 0)
  {
    Serial.println("<warn> UDP socket not opened!");
  }
  else
  {
    // omajinai
    delay(2000); // 直前に送信を行っていた場合にソケット削除を発行すると送信が中止されるバグあり
    // ソケット削除
    if (!theMurataLpwaCore.sendf("AT%%SOCKETCMD=\"DELETE\",%d\r", _socketId))
      return;
    if (theMurataLpwaCore.waitForResponse("OK\r") < 0)
    {
      return;
    }
  }
  Serial.println("<info> UDP socket closed");
  _sndBufferP = 0;
  _rcvBufferP = 0;
  _readBuffP = 0;
  _socketId = 0;
}

int LpwaV4Udp::available()
{
  //  Serial.println("@@@@@ LpwaV4Udp::available() enter");

  if ((_rcvBufferP - _readBuffP) > 0)
    return (_rcvBufferP - _readBuffP);

  //  Serial.println("@@@@@ LpwaV4Udp::available() step0");

  // 受信イベント確認
  if (theMurataLpwaCore.waitForResponse("\r", NULL, 0, CMD_TIMEOUT_POLL, true) < 0) // ポーリングではタイムアウトメッセージを抑止
    return 0;

  int statTcp = theMurataLpwaCore.getSocketStat();
  if (statTcp == 2)
  {
    Serial.println("<info> Socket termination due to Idle timer expiration.");
    stop();
    return 0;
  }
  if (statTcp == 3)
  {
    Serial.println("<info> Socket terminated by peer.");
    stop();
    return 0;
  }
  if (statTcp == 1)
  {
    //  Serial.println("@@@@@ LpwaV4Udp::available() step1");

    // データ受信
    if (!theMurataLpwaCore.sendf("AT%%SOCKETDATA=\"RECEIVE\",%d,1500\r", _socketId))
      return 0;

    if (theMurataLpwaCore.waitForResponse("OK\r", _mdmBuffer, UDP_BUFF_SIZE * 2, CMD_TIMEOUT_NETWORK) < 0)
      return 0;

    // 応答チェック
    char *pdest = NULL;
    char *mdm_result[10];

    // %SOCKETDATA:<socket_id>,<size>,<remain>,<data>....
    pdest = strstr(_mdmBuffer, "%SOCKETDATA:");
    if (pdest != NULL)
    {
#ifdef COMMAND_DBG
      Serial.print("@@@@@ %SOCKETDATA: ");
      Serial.println(pdest);
#endif // COMMAND_DBG
      int cnt = theMurataLpwaCore.split_resp(pdest, ",", mdm_result, 10);
      if (cnt > 0)
      {
#if 0
        Serial.print("@@@@@ %SOCKETDATA: [");
        Serial.print(mdm_result[0]);
        Serial.print("][");
        Serial.print(mdm_result[1]);
        Serial.print("][");
        Serial.print(mdm_result[2]);
        Serial.print("][");
        Serial.print(mdm_result[3]);
        Serial.println("]");
#endif
        _rcvBufferP = atoi(mdm_result[1]);
        char temp[3];
        for (int i = 0; i < _rcvBufferP; i++)
        {
          temp[0] = mdm_result[3][i * 2 + 1];
          temp[1] = mdm_result[3][i * 2 + 2];
          temp[2] = '\0';
          _rcvBuffer[i] = (char)strtol(temp, NULL, 16);
        }
        _readBuffP = 0;
        return _rcvBufferP;
      }
    }
    return 0;
  }
  return 0;
}

/**
 * 接続しているサーバーからデータを受信します。
 * @return 受信した値 (失敗した場合: -1)
 */
int LpwaV4Udp::read()
{
  if ((_rcvBufferP - _readBuffP) > 0)
  {
    int c = _rcvBuffer[_readBuffP++];
    return c;
  }
  return -1;
}

/**
 * 接続しているサーバーからデータを受信しバッファーに書き込みます。
 * @param buffer 受信したデータを書き込むためのバッファー
 * @param size バッファーに書き込むバイト数
 * @return 書き込まれたバイト数
 */
int LpwaV4Udp::read(uint8_t *buffer, size_t size)
{
  int remain = (_rcvBufferP - _readBuffP);
  int readsize = 0;
  if (remain > size)
  {
    readsize = size;
  }
  else
  {
    readsize = remain;
  }
  char *rdbuff_p = _rcvBuffer + _readBuffP;
  memcpy(buffer, rdbuff_p, readsize);
  _readBuffP += readsize;

  return readsize;
}

/**
 * 次回 read() の呼び出しのときの値を返します。
 * @return 受信した値 (失敗した場合: -1)
 */
int LpwaV4Udp::peek()
{
  if (available() == 0)
    return -1;

  return _rcvBuffer[_readBuffP];
}

/**
 * 次回 read() の呼び出しのときの値を返します。
 * @param buffer 受信したデータを書き込むためのバッファー
 * @param size バッファーに書き込むバイト数
 * @return 受信したバイト数
 */
int LpwaV4Udp::peek(uint8_t *buffer, size_t size)
{
  if (available() == 0)
    return -1;

  int remain = (_rcvBufferP - _readBuffP);
  int readsize = 0;
  if (remain > size)
  {
    readsize = size;
  }
  else
  {
    readsize = remain;
  }
  char *rdbuff_p = _rcvBuffer + _readBuffP;
  memcpy(buffer, rdbuff_p, readsize);

  return readsize;
}
