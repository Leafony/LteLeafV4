/*
 This example enables you to get the PIN lock status
 of a SIM card inserted into a LPWA board.
 Circuit:
 * LPWA board
 * SIM card
 Created 12 Jun 2012
 by David del Peral
*/

// libraries
#include <LpwaV4.h>

// pin manager object
Lpwa lpwaAccess;
LpwaPin pinManager;
LpwaModem lpwaModem;

void setup() {
  Serial.begin(115200);
#ifdef USBD_USE_CDC
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
#endif //  USBD_USE_CDC
  Serial.println("Starting LPWA SIM check");

  // connection state
  bool connected = false;

  // Start LPWA board
  while (!connected) {
    if (lpwaAccess.begin() == LPWA_READY) {
      connected = true;
    } else {
      Serial.println("starting LPWA device.");
      delay(1000);
    }
  }
  Serial.println("LPWA device enabled");

  pinManager.begin();
  // check if the SIM have pin lock
  switch (pinManager.isPIN()) {
  case 0:
    Serial.println("PIN lock is off.");
    break;
  case 1:
    Serial.println("PIN code locked.");
    break;
  case -1:
    Serial.print("PIN and PUK code locked.");
    break;
  case -2:
    // the worst case, PIN and PUK are locked
    Serial.println("PIN and PUK locked. Use PIN2/PUK2 in a mobile phone.");
    while (true)
      ;
  }
  // IMEI, modem unique identifier
  Serial.print("SIM ICCID: ");
  String iccdid = lpwaModem.getIccid();
  if (iccdid != NULL) {
    Serial.println(iccdid);
  }
}

void loop() {
  // do nothing
}
