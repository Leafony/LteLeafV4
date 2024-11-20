/*
 * LpwaV4Modem.h
 * LTE-M Leaf V4 library
 *
 * 2021.12.28 kt-nakamura@kddi-tech.com
 */
#ifndef LPWA_V4_MODEM_H_
#define LPWA_V4_MODEM_H_

#include "MurataLpwaCore.h"
#include "typesV4.h"
#include <Arduino.h>

/**
 * モデムの診断機能ためのクラス
 */
class LpwaV4Modem
{
public:
  /** 電源をオンにする */
  NetworkStatus begin() { return theMurataLpwaCore.begin(); }
  /** 電源をオフにする */
  void end() { return theMurataLpwaCore.end(); }
  /** 電源をオフにする */
  void shutdown() { return end(); }

  String getModel();
  String getFwVersion();

  String getImei();
  String getIMEI() { return getImei(); }

  String getIccid();
  String getICCID() { return getIccid(); }

  String getTime();
};
#endif
