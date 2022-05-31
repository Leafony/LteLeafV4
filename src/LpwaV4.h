/*
 * LpwaV4.h
 * LTE-M Leaf V4 library
 *
 * 2021.12.28 kt-nakamura@kddi-tech.com
 */
#ifndef LPWA_V4_H_
#define LPWA_V4_H_

#include "LpwaV4Borad.h"

#include "LpwaV4Access.h"
#include "LpwaV4Client.h"
#include "LpwaV4Ctrl.h"
#include "LpwaV4Gprs.h"
#include "LpwaV4HttpClient.h"
#include "LpwaV4Modem.h"
#include "MurataLpwaCore.h"
#include "LpwaV4Pin.h"
#include "LpwaV4Scanner.h"
#include "LpwaV4Udp.h"

typedef LpwaV4Access Lpwa;
typedef LpwaV4Access LpwaAccess;
typedef LpwaV4Modem LpwaModem;
typedef LpwaV4Ctrl LpwaCtrl;
typedef LpwaV4Pin LpwaPin;
typedef LpwaV4Scanner LpwaScanner;
typedef LpwaV4Gprs Gprs;
typedef LpwaV4Gprs GPRS;
typedef LpwaV4Client LpwaClient;
typedef LpwaV4Udp LpwaUdp;
typedef LpwaV4HttpClient LpwaHttpClient;
#endif
