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
  NetworkStatus attachGprs(const char *apn, const char *username,
                           const char *password, unsigned long timeout = 30000);
  NetworkStatus attachGPRS(const char *apn, const char *username,
                           const char *password,
                           unsigned long timeout = 30000) {
    return attachGprs(apn, username, password, timeout);
  }
  NetworkStatus dettachGprs();
  IPAddress getIpAddress();
  IPAddress getIPAddress() { return getIpAddress(); }
};

#endif
