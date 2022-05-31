/*
 * LpwaV4Ctrl.cpp
 * LTE-M Leaf V4 library
 *
 * 2021.12.28 kt-nakamura@kddi-tech.com
 */
#include "LpwaV4Ctrl.h"
  
LpwaV4Ctrl::LpwaV4Ctrl(){
  pwrState = LPWA_NORMAL;
}

/**
 * LPWAリーフの電力制御を行う
 * @param enable 電力ステート
 * @return なし
 */
void LpwaV4Ctrl::powerCtrl(bool enable) {
  if (enable && (pwrState != LPWA_NORMAL)) {
      theMurataLpwaCore.begin();
      pwrState = LPWA_NORMAL; 
  } else {
      theMurataLpwaCore.end();
      pwrState = LPWA_OFF; 
  }
}

/**
 * LPWAリーフの消費電力制御を行う
 * @param state 電力制御ステート
 * @return なし
 */
void LpwaV4Ctrl::powerDown(int state) {
  switch(state) {
    case LPWA_OFF: // power off
      theMurataLpwaCore.end();
      pwrState = LPWA_OFF; 
      break;
    case LPWA_NORMAL: // normal state
      if ((pwrState == LPWA_SLEEP1) || (pwrState == LPWA_SLEEP2) || (pwrState == LPWA_SLEEP3)) {
        theMurataLpwaCore.wakeup(true);
        pwrState = LPWA_NORMAL; 
        break;
      }
      if (pwrState != LPWA_NORMAL) {
        theMurataLpwaCore.end();
        theMurataLpwaCore.begin();
      }
      pwrState = LPWA_NORMAL; 
      break;
    case LPWA_RESET: // reset
      theMurataLpwaCore.end();
      theMurataLpwaCore.begin();
      pwrState = LPWA_NORMAL; 
      break;
    case LPWA_SLEEP1:
      if (pwrState == LPWA_NORMAL) {
        if (!theMurataLpwaCore.sendCmd("at%devcmd=PSMAX,DH1\r"))
          Serial.println("Settings Error01");
        if (theMurataLpwaCore.waitForResponse("OK\r") < 0) {
          Serial.println("Settings Error02");
        }

        theMurataLpwaCore.wakeup(false);
        pwrState = LPWA_SLEEP1; 
      }
      break;
    case LPWA_SLEEP2:
      if (pwrState == LPWA_NORMAL) {
        if (!theMurataLpwaCore.sendCmd("at%devcmd=PSMAX,DS\r"))
          Serial.println("Settings Error01");
        if (theMurataLpwaCore.waitForResponse("OK\r") < 0) {
          Serial.println("Settings Error02");
        }
        theMurataLpwaCore.wakeup(false);
        pwrState = LPWA_SLEEP2; 
      }
      break;
    case LPWA_SLEEP3:
      if (pwrState == LPWA_NORMAL) {
        if (!theMurataLpwaCore.sendCmd("at%devcmd=PSMAX,DS\r"))
          Serial.println("Settings Error01");
        if (theMurataLpwaCore.waitForResponse("OK\r") < 0) {
          Serial.println("Settings Error02");
        }
        theMurataLpwaCore.wakeup(false);
        pwrState = LPWA_SLEEP3; 
      }
      break;
  }
}

int readAdc101C027() {
  Wire.endTransmission();
  uint8_t timeout=0;
  int data = 0;
  Wire.requestFrom(ADC101C027_ADR, (uint8_t) 0x02);
  while(Wire.available() < 1) {
    timeout++;
    if(timeout > TCA6408_TIMEOUT) {
      return -1;
    }
    delay(1);
  }
  // BATT入力はバッテリ電圧の1/2
  // ADCは3.3Vフルスケールで10bit分解能
  // 3300*2 / 1024 -> 825/64
  data = data | ((Wire.read() & 0x0f) << 6);
  data = data | ((Wire.read() & 0xfc) >> 2) ;
  return (data * ADC101C027_VA) >> 9;
}

/**
 * LPWAリーフのバッテリ電圧を取得する
 * @param なし
 * @return 電圧値(単位mV)
 */
int LpwaV4Ctrl::getBattLevel() {
  int retval = 0;
  // バッテリ検出を有効にする
  theMurataLpwaCore.setBatteryManagement(true);
  delay(2000); // ADC入力が安定するまで待つ

  // ADCを４回読んで平均化
  for (int i=0;i<4;i++) {
    retval += readAdc101C027();
    delay(50);
  }
  theMurataLpwaCore.setBatteryManagement(false);

  return retval >>2;
}
