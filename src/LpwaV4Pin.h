/*
 * LpwaV4Pin.h
 * LTE-M Leaf V4 library
 *
 * 2021.12.28 kt-nakamura@kddi-tech.com
 */
#ifndef LPWA_V4_PIN_H_
#define LPWA_V4_PIN_H_

#include "MurataLpwaCore.h"
#include "typesV4.h"

/**
 * SIM PINコードの診断機能のためのクラス
 */
class LpwaV4Pin {
public:
  /** 電源をオンにする */
  NetworkStatus begin() { return theMurataLpwaCore.begin(); }
  /** 電源をオフにする */
  void end() { return theMurataLpwaCore.end(); }
  /** 電源をオフにする */
  void shutdown() { return end(); }

  /**
   * Check if PIN lock or PUK lock is activated
   * @return 0 if PIN lock is off, 1 if PIN lock is on, -1 if PUK lock is on,
   * -2 if error exists
   */
  int isPin();

  /**
   * Check if PIN lock or PUK lock is activated
   * @return 0 if PIN lock is off, 1 if PIN lock is on, -1 if PUK lock is on,
   * -2 if error exists
   */
  int isPIN() { return isPin(); }
};

#endif
