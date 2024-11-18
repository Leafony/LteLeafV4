/*
 * MurataLpwaCore.h
 * LTE-M Leaf V4 library
 *
 * 2021.12.28 kt-nakamura@kddi-tech.com
 */

#ifndef MURATA_LPWA_CORE_H_
#define MURATA_LPWA_CORE_H_

#include "debug.h"
#include "typesV4.h"
#include <Arduino.h>
#include <IPAddress.h>
#include <Wire.h>
#include "LpwaV4Borad.h"

#define CMD_TIMEOUT_DEFAULT 2000
#define CMD_TIMEOUT_NETWORK 5000
#define CMD_TIMEOUT_SHORT 1000
#define CMD_TIMEOUT_POLL 500
#define MAX_COMMAND_SIZE 400

/**
 * theMurataLpwaCoreインスタンスを作るための内部利用クラス
 */
class MurataLpwaCore
{
  PinName _rx;
  PinName _tx;
  HardwareSerial &_serial;
  const unsigned long _baud;
  //  char *_buffer;
  //  int _buffer_p;
  bool _power;
  bool _init;

public:
  MurataLpwaCore(HardwareSerial &serial, const unsigned long baud);
  ~MurataLpwaCore();
  NetworkStatus status;

  NetworkStatus begin();
  void end();
  void power(bool enable);
  void wakeup(bool enable);
  void setBatteryManagement(bool enable);
  bool sendCmd(const char *command = "");
  // ※Stringは禁止 bool sendCmd(const String &command) { sendCmd(command.c_str()); }
  bool sendf(const char *format, ...);
  bool sendBin(uint8_t *data, int size, const unsigned long timeout = CMD_TIMEOUT_NETWORK);
  size_t printf(const char *format, ...);

  char *readBytes(int size, int chunk);
  int poll(const char *expectedVal);
  int split_resp(char *instring, const char *separator, char **result, size_t max_size);
  IPAddress str2Ip(char *ipstring);
  void atohArray(char *chrArray, char *hexArray, size_t size);
  void htoaArray(char *hexArray, char *chrArray, size_t size);

  int waitForResponse(const char *expectedVal, char *respData = NULL, int respSize = 0, const unsigned long timeout = CMD_TIMEOUT_DEFAULT, bool silent = false);

  /**
   * 現在のPDP connection statusを取得する
   * @param なし
   * @return PDP connection status (0:disconnect, 1:connect)
   */
  int getPdpStat()
  {
    return pdp_stat;
  }

  /**
   * PDP connection statusをクリアする
   * @param なし
   * @return なし
   */
  void clrPdpStat()
  {
    pdp_stat = 0;
  }

  /**
   * 最新のSocket eventを取得する
   * @param なし
   * @return Socket event (0:none, 1-4:event)
   */
  int getSocketStat()
  {
    return socket_event;
  }

  /**
   * Socket eventをクリアする
   * @param なし
   * @return なし
   */
  void clrSocketStat()
  {
    socket_event = 0;
  }

  /**
   * 取得されたHTTP response codeを取得する
   * @param なし
   * @return HTTP response code
   */
  int getHttpResp()
  {
    int ret = http_resp;
    http_resp = 0;
    return ret;
  }

  void debugPrint(int lev, const char *format, ...);

private:
  int http_resp; // HTTP response code
  int rcv_length;
  int pdp_stat;
  int socket_event;
};

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_SERIAL)
extern MurataLpwaCore theMurataLpwaCore;
#endif

#endif // MURATA_LPWA_CORE_H_
