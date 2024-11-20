/*
 * LpwaV4Ctrl.h
 * LTE-M Leaf V4 library
 *
 * 2021.12.28 kt-nakamura@kddi-tech.com
 */
#ifndef LPWA_V4_MODEM_CTRL_H_
#define LPWA_V4_MODEM_CTRL_H_

#include <Arduino.h>
#include "LpwaV4Borad.h"
#include "MurataLpwaCore.h"
#include "typesV4.h"
#include <Wire.h>

#define LPWA_OFF 0
#define LPWA_NORMAL 1
#define LPWA_RESET 2
#define LPWA_SLEEP1 3 // DEEP HIBERNATE1
#define LPWA_SLEEP2 4 // DEEP HIBERNATE1
#define LPWA_SLEEP3 5 // DEEP SLEEP

class LpwaV4Ctrl
{
  int pwrState;

public:
  LpwaV4Ctrl();
  void powerCtrl(bool enable);
  void powerDown(int state);
  int getBattLevel();
};

#endif // LPWA_V4_MODEM_CTRL_H_
