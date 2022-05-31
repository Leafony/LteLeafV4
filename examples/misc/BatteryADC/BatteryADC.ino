/*
 * BatteryADC.ino
 * LPWA leaf Battery voltage
 * 
 * LPWAリーフV4に接続されたバッテリ電圧を読み取ります
 */

#include "LpwaV4.h"

Lpwa lpwaAccess;
LpwaCtrl pmctrl;

void setup() {
  Serial.begin(115200);
#ifdef USBD_USE_CDC
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
#endif //  USBD_USE_CDC

  // Start LPWA board
  bool connected = false;
  while (!connected) {
    if (lpwaAccess.begin() == LPWA_READY) {
      connected = true;
    } else {
      Serial.println("starting LPWA device.");
      delay(1000);
    }
  }
  Serial.println("LPWA device enabled");
  
  Serial.println("\nLPWA leaf battery ADC");
}
 
void loop() {
  int batt = pmctrl.getBattLevel();
  Serial.print("battery:");
  Serial.print(batt);
  Serial.println(" mV");
  delay(1000);          
}
