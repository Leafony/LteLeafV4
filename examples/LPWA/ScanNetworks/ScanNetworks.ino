/*
 LPWA Scan Networks
 This example prints out the IMEI number of the modem,
 then checks to see if it's connected to a carrier. If so,
 it prints the phone number associated with the card.
 Then it scans for nearby networks and prints out their signal strengths.
 Circuit:
 * LPWA board
 * SIM card
 Created 8 Mar 2012
 by Tom Igoe, implemented by Javier Carazo
 Modified 4 Feb 2013
 by Scott Fitzgerald
*/

// libraries
#include <LpwaV4.h>

// initialize the library instance
Lpwa lpwaAccess;
LpwaScanner scannerNetworks;
LpwaModem lpwaModem;

// Save data variables
String IMEI = "";

void setup() {
  Serial.begin(115200);
#ifdef USBD_USE_CDC
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
#endif //  USBD_USE_CDC
  Serial.println("Starting LPWA Network Scanner");

  scannerNetworks.begin();

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
  
  // get modem parameters
  // IMEI, modem unique identifier
  Serial.print("Modem IMEI: ");
  IMEI = lpwaModem.getIMEI();
  if (IMEI != NULL) {
    Serial.println(IMEI);
  }
}

void loop() {
  // currently connected carrier
  Serial.print("Current carrier: ");
  Serial.println(scannerNetworks.getCurrentCarrier());

  // returns strength and ber
  // signal strength in 0-31 scale. 31 means power > 51dBm
  // BER is the Bit Error Rate. 0-7 scale. 99=not detectable
  Serial.print("Signal Strength: ");
  Serial.print(scannerNetworks.getSignalStrength());
  Serial.println(" [0-31]");

  // network time
  Serial.print("Network Time: ");
  Serial.println(lpwaModem.getTime());

  delay(5000);
}
