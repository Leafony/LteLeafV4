/*
 * LpwaV4Scanner.h
 * LTE-M Leaf V4 library
 *
 * 2021.12.28 kt-nakamura@kddi-tech.com
 */
#ifndef LPWA_V4_SCANNER_H_
#define LPWA_V4_SCANNER_H_

#include "MurataLpwaCore.h"
#include "typesV4.h"

/**
 * ネットワークの診断機能のためのクラス
 */
class LpwaV4Scanner {
public:
  /** 電源をオンにする */
  NetworkStatus begin() { return theMurataLpwaCore.begin(); }
  /** 電源をオフにする */
  void end() { return theMurataLpwaCore.end(); }
  /** 電源をオフにする */
  void shutdown() { return end(); }

  String getCurrentCarrier();
  String getSignalStrength();
};

#endif //LPWA_V4_SCANNER_H_
