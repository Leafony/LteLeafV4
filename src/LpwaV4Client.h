/*
 * LpwaV4Clinet.h
 * LTE-M Leaf V4 library
 *
 * 2021.12.28 kt-nakamura@kddi-tech.com
 */
#ifndef LPWA_V4_CLIENT_H_
#define LPWA_V4_CLIENT_H_

#include "MurataLpwaCore.h"
#include "typesV4.h"
#include <Arduino.h>
#include <Client.h>

// #define RCVBUFFSIZE 2048
#define TCP_URL_SIZE 200
#define TCP_BUFF_SIZE 2048

/**
 * TCPソケット通信のためのクラス
 */
class LpwaV4Client : public Client {
  enum State {
    SOCKET_NOT_DEFINED,
    SOCKET_NOT_USED,
    SOCKET_CONNECTING,
    SOCKET_CONNECTED,
    SOCKET_CLOSING,
    SOCKET_CLOSED,
    ERROR
  };
  LpwaV4Client::State _status();

  IPAddress _ip;
  char _host[TCP_URL_SIZE];
  uint16_t _port;
  int _socketId;

  char _rcvBuffer[TCP_BUFF_SIZE];
  int _rcvBufferP;
  char _mdmBuffer[TCP_BUFF_SIZE*2];
  int _readBuffP;

  int _connect();

public:
  LpwaV4Client();
  ~LpwaV4Client();

  /**
   * 指定されたホスト名とポートに接続します。成功した場合は1を返し、そうでない場合は0を返します。
   * @param host ホスト名
   * @param port ポート番号
   * @return 接続状態 (1: 接続, 0: それ以外)
   */
  int connect(String host, uint16_t port) {
    return 0;
  }

  int connect(IPAddress ip, uint16_t port);
  int connect(const char *host, uint16_t port);
  size_t write(uint8_t c);
  size_t write(const uint8_t *buffer, size_t size);
  int available();
  int read();
  int read(uint8_t *buffer, size_t size);
  int peek();
  int peek(uint8_t *buffer, size_t size);

  /** 何もしません。 */
  void flush() {}

  void stop();
  uint8_t connected();

  /**
   * クライアントが接続されているかどうかを返します。
   * 読み取りに使用できるバッファーが空の場合、サーバーから切断します。
   * @return 接続状態 (true: 接続, false: それ以外)
   */
  operator bool() { return connected(); }
};

#endif
