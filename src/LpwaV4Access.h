/*
 * LpwaV4Access.h
 * LTE-M Leaf V4 library
 *
 * 2021.12.28 kt-nakamura@kddi-tech.com
 */
#ifndef LPWA_V4_ACCESS_H_
#define LPWA_V4_ACCESS_H_

#include "MurataLpwaCore.h"
#include "typesV4.h"

/**
 * LPWAライブラリのベースとなるクラス
 */
class LpwaV4Access
{
public:
  /** 電源をオンにする */
  NetworkStatus begin()
  {
    return theMurataLpwaCore.begin();
    //    return LPWA_FAIL;
  }
  /** 電源をオフにする */
  void end()
  {
    //    return theMurataLpwaCore.end();
    return;
  }
  /** 電源をオフにする */
  void shutdown() { return end(); }
};

#endif
