/*
 * LpwaV4Gprs.h
 * LTE-M Leaf V4 library
 *
 * 2021.12.28 kt-nakamura@kddi-tech.com
 */
#ifndef LPWA_V4_GPRS_H_
#define LPWA_V4_GPRS_H_

#include "MurataLpwaCore.h"
#include "typesV4.h"
#include <IPAddress.h>

/**
 * GPRS接続のためのクラス
 */
class LpwaV4Gprs {
  bool _ready();

public:
  // バンドリスト
  static const uint8_t LPWA_V4_GPRS_BAND_KDDI = 18;
  static const uint8_t LPWA_V4_GPRS_BAND_DOCOMO = 19;

  NetworkStatus attachGprs(const char *apn, const char *username,
                           const char *password,
                           const uint8_t band = LPWA_V4_GPRS_BAND_KDDI,
                           unsigned long timeout = 10000,
                           bool legacyMode = false);
  NetworkStatus attachGPRS(const char *apn, const char *username,
                           const char *password,
                           const uint8_t band = LPWA_V4_GPRS_BAND_KDDI,
                           unsigned long timeout = 10000,
                           bool legacyMode = false) {
    return attachGprs(apn, username, password, band, timeout, legacyMode);
  }
  NetworkStatus dettachGprs();
  IPAddress getIpAddress();
  IPAddress getIPAddress() { return getIpAddress(); }
  String getAvailableOperators();
  String getCellularOperatorSelection();
};

#endif
