/*
 * SleepDevice.ino
 * LTE-MリーフVer4とSTM32リーフの省電力評価スケッチ
 *
 *  created 6 Jan 2022
 * by kt-nakamura
 * 
 * このソースコードをビルドする際は次のライブラリをインストールすること
 * STM32LowPower-master
 * STM32RTC-master

 */

// LPWA 省電力モード
// いずれの１つを選択すること
//#define USE_D_HIBERNATE1
//#define USE_D_HIBERNATE2
//#define USE_DEEP_SLEEP
#define USE_POWEROFF

// STM32 省電力
#define STM32_DEEP_SLEEP

#ifdef STM32_DEEP_SLEEP
 #include "STM32LowPower.h"
 #include <STM32RTC.h>
#endif

#include <LpwaV4.h>
#include "arduino_secrets.h"
// Please enter your sensitive data in the Secret tab or arduino_secrets.h
// APN data
const char GPRS_APN[] = SECRET_GPRS_APN;
const char GPRS_LOGIN[] = SECRET_GPRS_LOGIN;
const char GPRS_PASSWORD[] = SECRET_GPRS_PASSWORD;

GPRS gprs;
LpwaAccess lpwaAccess;
LpwaCtrl pmctrl;

#ifdef STM32_DEEP_SLEEP
 // STM32 RTC 
 STM32RTC& rtc = STM32RTC::getInstance();
#endif

void setup() {
  Serial.begin(115200);
#ifdef USBD_USE_CDC
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
#endif //  USBD_USE_CDC
  Serial.println("Starting LPWA power control.");

  // LPWAデバイスの初期化
  bool lpwa_enabled = false;
  while (!lpwa_enabled) {
    if (lpwaAccess.begin() == LPWA_READY) {
      lpwa_enabled = true;
    } else {
      Serial.println("starting LPWA device.");
      delay(1000);
    }
  }
  Serial.println("LPWA device enabled");

#ifdef STM32_DEEP_SLEEP
  LowPower.begin();
  rtc.begin(); // initialize RTC 24H format
#endif
}

void loop() {
  // LPWA接続開始
  Serial.println("connect PDN");
  bool pdn_connected = false;
  while (!pdn_connected) {
    if(gprs.attachGPRS(GPRS_APN, GPRS_LOGIN, GPRS_PASSWORD) == GPRS_READY) {
      pdn_connected = true;
    } else {
      Serial.println("connecting.");
      delay(1000);
    }
  }
  // LPWA接続成功＆IP取得
  Serial.println("connected");
  Serial.print("IP:");
  Serial.println(gprs.getIPAddress());
  delay(5000);

  // LPWA接続終了
  Serial.println("disconnect PDN");
  NetworkStatus a = gprs.dettachGprs();
  delay(5000);

  // 省電力状態に移行
  Serial.println("power-down LPWA device");
#ifdef USE_D_HIBERNATE1
  pmctrl.powerDown(LPWA_SLEEP1);
#endif
#ifdef USE_D_HIBERNATE2
  pmctrl.powerDown(LPWA_SLEEP2);
#endif
#ifdef USE_DEEP_SLEEP
  pmctrl.powerDown(LPWA_SLEEP3);
#endif
#ifdef USE_POWEROFF
  pmctrl.powerDown(LPWA_OFF);
#endif

#ifdef STM32_DEEP_SLEEP
  Serial.println("STM32 power-down -- wait 20sec");
  Serial.flush();
  LowPower.deepSleep(20000); // deepSleep
#else
  Serial.println("wait 20sec");
  delay(20000);
#endif

  // 省電力状態からの復帰
  Serial.println("resume LPWA device");
  pmctrl.powerDown(LPWA_NORMAL);
  delay(3000);
}
