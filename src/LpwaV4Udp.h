/*
 * LpwaV4Udp.h
 * LTE-M Leaf V4 library
 *
 * 2021.12.28 kt-nakamura@kddi-tech.com
 */
#ifndef LPWA_V4_UDP_H_
#define LPWA_V4_UDP_H_

#include "MurataLpwaCore.h"
#include "typesV4.h"
#include <Arduino.h>
#include <Udp.h>

#define UDP_URL_SIZE 200
#define UDP_BUFF_SIZE 2048

/**
 * UDPソケット通信のためのクラス
 */
class LpwaV4Udp : public UDP
{
  IPAddress _ip;
  char _host[UDP_URL_SIZE];
  uint16_t _port;
  int _socketId;
  int _beginPacket();

  char _sndBuffer[UDP_BUFF_SIZE];
  int _sndBufferP;
  char _rcvBuffer[UDP_BUFF_SIZE];
  int _rcvBufferP;
  char _mdmBuffer[UDP_BUFF_SIZE * 2];
  int _readBuffP;

public:
  LpwaV4Udp();
  ~LpwaV4Udp();

  /**
   * initialize, start listening on specified port.
   * Returns 1 if successful, 0 if there are no sockets
   * available to use
   * @param localPort クライアントポート
   * @return 初期化状態 (1: 利用可能, 0: それ以外)
   * FIXME: クライアントポートは機能しない
   */
  uint8_t begin(uint16_t localPort = 0);

  /** Finish with the UDP socket */
  void stop();

  /**
   * Start building up a packet to send to the remote host specific in host and
   * port Returns 1 if successful, 0 if there was a problem resolving the
   * hostname or port
   * @return 接続状態 (1: 成功, 0: それ以外)
   */
  int beginPacket(String host, uint16_t port)
  {
    return beginPacket(host.c_str(), port);
  }

  /**
   * Start building up a packet to send to the remote host specific in ip and
   * port Returns 1 if successful, 0 if there was a problem with the supplied IP
   * address or port
   * @return 接続状態 (1: 成功, 0: それ以外)
   */
  int beginPacket(IPAddress ip, uint16_t port);

  /**
   * Start building up a packet to send to the remote host specific in host and
   * port Returns 1 if successful, 0 if there was a problem resolving the
   * hostname or port
   * @return 接続状態 (1: 成功, 0: それ以外)
   */
  int beginPacket(const char *host, uint16_t port);

  /**
   * Finish off this packet and send it
   * Returns 1 if the packet was sent successfully, 0 if there was an error
   */
  int endPacket();

  /**
   * Write a single byte into the packet
   */
  size_t write(const uint8_t c);

  /**
   * Write size bytes from buffer into the packet
   */
  size_t write(const uint8_t *buffer, size_t size);

  /**
   * Start processing the next available incoming packet
   * Returns the size of the packet in bytes, or 0 if no packets are available
   */
  int parsePacket() { return available(); }

  /**
   * Number of bytes remaining in the current packet
   */
  int available();

  /**
   * Read a single byte from the current packet
   */
  int read();

  /**
   * Read up to size bytes from the current packet and place them into buffer
   * Returns the number of bytes read, or 0 if none are available
   */
  int read(uint8_t *buffer, size_t size);

  /**
   * Read up to size characters from the current packet and place them into
   * buffer Returns the number of characters read, or 0 if none are available
   */
  int read(char *buffer, size_t size) { return read((uint8_t *)buffer, size); }

  /**
   * Return the next byte from the current packet without moving on to the next
   * byte
   */
  int peek();

  /**
   * 次回 read() の呼び出しのときの値を返します。
   * @param buffer 受信したデータを書き込むためのバッファー
   * @param size バッファーに書き込むバイト数
   * @return 受信したバイト数
   */
  int peek(uint8_t *buffer, size_t size);

  /** 何もしません。 */
  void flush() {}

  /** Return the IP address of the host who sent the current incoming packet */
  IPAddress remoteIP() { return remoteIp(); }

  /** Return the IP address of the host who sent the current incoming packet */
  IPAddress remoteIp()
  {
    IPAddress ip;
    return ip;
  }

  /** Return the port of the host who sent the current incoming packet */
  uint16_t remotePort() { return 0; }

private:
};

#endif
