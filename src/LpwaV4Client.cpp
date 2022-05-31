/*
 * LpwaV4client.cpp
 * LTE-M Leaf V4 library
 *
 * 2021.12.28 kt-nakamura@kddi-tech.com
 */
 #include "LpwaV4Client.h"

LpwaV4Client::State LpwaV4Client::_status() {

  return ERROR;
}

LpwaV4Client::LpwaV4Client() 
  :  _socketId(0), _ip(0, 0, 0, 0), _host(""), _port(0), _readBuffP(0), _rcvBufferP(0) {}

LpwaV4Client::~LpwaV4Client() {}


int LpwaV4Client::_connect() {
  int resp_size = strlen(_host) + 100; // コマンド応答値の長さ(_hostで可変)
  char rcvbuff[resp_size];
  char *start_p = NULL;
  char *end_p = NULL;

  if (_socketId != 0) {
    // 既にソケットが空いている場合は削除する
 //   Serial.println("<warn> TCP socket already opened!");
    if (!theMurataLpwaCore.sendf("AT%%SOCKETCMD=\"DELETE\",%d\r", _socketId))
      Serial.println("<warn> TCP socket close failed(send)");
    if (theMurataLpwaCore.waitForResponse("OK\r") < 0) {
      Serial.println("<warn> TCP socket close failed(rcv)");
    }
    _socketId = 0;
  }

  // ソケット設定コマンド
  if (!theMurataLpwaCore.sendf("AT%%SOCKETCMD=\"ALLOCATE\",0,\"TCP\",\"OPEN\",\"%s\",%d,0,0,,1\r", _host, _port))
    return 0;
  if (theMurataLpwaCore.waitForResponse("OK\r",rcvbuff,resp_size,CMD_TIMEOUT_NETWORK) < 0)
    return 0;

  // コマンド戻り値からソケット番号を取得
  start_p = strstr(rcvbuff, "%SOCKETCMD:");
  if (start_p == NULL) {
    Serial.println("<error> invalid response: ");
    return 0;
  }
  start_p += strlen("%SOCKETCMD:");
  _socketId = atoi(start_p);
 // Serial.print("@@@@@@ LpwaV4Client::_connect() socket_id=");
 // Serial.println(_socketId);

  // ソケット接続
  if (!theMurataLpwaCore.sendf("AT%%SOCKETCMD=\"ACTIVATE\",%d\r",_socketId))
    return 0;
  if (theMurataLpwaCore.waitForResponse("OK\r",NULL,0,5000) < 0) {
    Serial.println("<error> TCP socket failed!");
    return 0;
  }

  // 接続成功
  Serial.println("<info> TCP socket connected");
  _rcvBufferP =0;
  return 1;
}

/**
 * 指定されたIPアドレスとポートに接続します。成功した場合は1を返し、そうでない場合は0を返します。
 * @param ip IPアドレス
 * @param port ポート番号
 * @return 接続状態 (1: 接続, 0: それ以外)
 */
int LpwaV4Client::connect(IPAddress ip, uint16_t port) {
//  Serial.println("@@@@@ LpwaV4Client::connect(ip) enter");
  sprintf(_host, "%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);
  _port = port;
  return _connect();
}

/**
 * 指定されたホスト名とポートに接続します。成功した場合は1を返し、そうでない場合は0を返します。
 * @param host ホスト名
 * @param port ポート番号
 * @return 接続状態 (1: 接続, 0: それ以外)
 */
int LpwaV4Client::connect(const char *host, uint16_t port) {
//  Serial.println("@@@@@ LpwaV4Client::connect(host) enter");
  sprintf(_host, "%s", host);
  _port = port;
  return _connect();

}

/**
 * 接続しているサーバーにデータを送信します。
 * @param c 送信する値
 * @return 送信されたバイト数
 */
size_t LpwaV4Client::write(uint8_t c) { return write(&c, 1); }

/**
 * 接続しているサーバーにデータを送信します。
 * @param buffer 送信するバイト列
 * @param size 送信するバイト数
 * @return 送信されたバイト数
 */
size_t LpwaV4Client::write(const uint8_t *buffer, size_t size) {
  if (_socketId == 0) {
    return 0;
  }

  // TCPコマンドを送信
  if (!theMurataLpwaCore.sendf("AT%%SOCKETDATA=\"SEND\",%d,%d,\"", _socketId, size)) {
    Serial.println("@@@@@ LpwaV4Client::write() error1");
    _rcvBufferP =0;
    return 0;
  }

  // データ部を送信
  for (int i=0;i<size;i++) {
    if (!theMurataLpwaCore.sendf("%02x", buffer[i])) {
      Serial.println("@@@@@ LpwaV4Client::write() error2");
      _rcvBufferP =0;
      return 0;
    }
  }
     
    // 終端コードを送信
  if (!theMurataLpwaCore.sendCmd("\"\r")) {
    Serial.println("@@@@@ LpwaV4Client::write() error3");
    _rcvBufferP =0;
    return 0;
  }

  // コマンド終了待ち
  if (theMurataLpwaCore.waitForResponse("OK\r") < 0) {
    Serial.println("@@@@@ LpwaV4Client::write() error4");
    _rcvBufferP =0;
    return 0;
  }

//  Serial.println("@@@@@ LpwaV4Client::write() exit");
  _rcvBufferP =0;
  return size;
}

/**
 * 読み取りに使用できるバイト数を返します。
 * @return 使用できるバイト数
 */
int LpwaV4Client::available() {
//  Serial.println("@@@@@ LpwaV4Client::available() enter");
  if (_socketId == 0)  {
    return 0;
  }

  if ((_rcvBufferP - _readBuffP) > 0)
    return (_rcvBufferP - _readBuffP);

//  Serial.println("@@@@@ LpwaV4Client::available() step0");

  // 受信イベント確認
  if (theMurataLpwaCore.waitForResponse("\r",NULL,0,CMD_TIMEOUT_POLL,true) < 0)  // ポーリングではタイムアウトメッセージを抑止
    return 0;

  int statTcp = theMurataLpwaCore.getSocketStat();
  if (statTcp == 2) {
    Serial.println("<info> Socket termination due to Idle timer expiration.");
    _rcvBufferP =0;
    _readBuffP = 0;
    return 0;
  }
  if (statTcp == 3) {
    Serial.println("<info> Socket terminated by peer.");
    _rcvBufferP =0;
    _readBuffP = 0;
    return 0;
  }
  if (statTcp == 1) {
  //  Serial.println("@@@@@ LpwaV4Client::available() step1");

    // データ受信
    if (!theMurataLpwaCore.sendf("AT%%SOCKETDATA=\"RECEIVE\",%d,1500\r", _socketId)) 
      return 0;

    if (theMurataLpwaCore.waitForResponse("OK\r",_mdmBuffer,TCP_BUFF_SIZE*2,CMD_TIMEOUT_NETWORK) < 0)
      return 0;

    // 応答チェック
    char *pdest = NULL;
    char* mdm_result[10];

    // %SOCKETDATA:<socket_id>,<size>,<remain>,<data>....
    pdest = strstr(_mdmBuffer, "%SOCKETDATA:");
    if (pdest != NULL) {
#ifdef COMMAND_DBG
      Serial.print("@@@@@ %SOCKETDATA: ");
      Serial.println(pdest);
#endif // COMMAND_DBG
      int cnt = theMurataLpwaCore.split_resp(pdest, ",", mdm_result, 10);
      if (cnt > 0) {
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
        for (int i=0;i<_rcvBufferP;i++) {
          temp[0] = mdm_result[3][i*2 + 1];
          temp[1] = mdm_result[3][i*2 + 2];
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
int LpwaV4Client::read() {
  if ((_rcvBufferP - _readBuffP) > 0) {
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
int LpwaV4Client::read(uint8_t *buffer, size_t size) {
  int remain = (_rcvBufferP - _readBuffP);
  int readsize =0;
  if (remain > size) {
    readsize = size;
  } else {
    readsize = remain;
  }
  char *rdbuff_p = _rcvBuffer + _readBuffP;
  memcpy(buffer,rdbuff_p,readsize);
  _readBuffP += readsize;

  return readsize;
}

/**
 * 次回 read() の呼び出しのときの値を返します。
 * @return 受信した値 (失敗した場合: -1)
 */
int LpwaV4Client::peek() {
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
int LpwaV4Client::peek(uint8_t *buffer, size_t size) {
  if (available() == 0)
    return -1;

  int remain = (_rcvBufferP - _readBuffP);
  int readsize =0;
  if (remain > size) {
    readsize = size;
  } else {
    readsize = remain;
  }
  char *rdbuff_p = _rcvBuffer + _readBuffP;
  memcpy(buffer,rdbuff_p,readsize);

  return readsize;
}

/**
 * サーバーから切断します。
 */
void LpwaV4Client::stop() {
//  Serial.println("@@@@@ LpwaV4Client::stop() enter");
  if (_socketId == 0) {
    Serial.println("<warn> TCP socket not opened!");
  } else {
    // omajinai
    delay (2000); // 直前に送信を行っていた場合にソケット削除を発行すると送信が中止されるバグあり
    // ソケット削除
    if (!theMurataLpwaCore.sendf("AT%%SOCKETCMD=\"DELETE\",%d\r", _socketId))
      return;
    if (theMurataLpwaCore.waitForResponse("OK\r") < 0) {
      return;
    }
  }
  Serial.println("<info> UDP socket closed");
  _rcvBufferP =0;
  _readBuffP = 0;
  _socketId = 0;
}

/**
 * クライアントが接続されているかどうかを返します。
 * @return 接続状態 (1: 接続, 0: それ以外)
 */
uint8_t LpwaV4Client::connected() {
  if (_socketId == 0) {
    return 0;
  }

  return 1;
}
