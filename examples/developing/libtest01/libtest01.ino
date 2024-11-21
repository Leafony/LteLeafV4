/*
 * LTE-M リーフV4テストベンチ
 *
 */

// libraries
#include <LpwaV4.h>

#include "arduino_secrets.h"
// Please enter your sensitive data in the Secret tab or arduino_secrets.h
// APN data
const char GPRS_APN[] = SECRET_GPRS_APN;
const char GPRS_LOGIN[] = SECRET_GPRS_LOGIN;
const char GPRS_PASSWORD[] = SECRET_GPRS_PASSWORD;

// initialize the library instance
GPRS gprs;
Lpwa lpwaAccess;
LpwaScanner scannerNetworks;
LpwaModem lpwaModem;

// Save data variables
String IMEI = "";

void setup()
{
  Serial.begin(115200);
#ifdef USBD_USE_CDC
  while (!Serial)
  {
    ; // wait for serial port to connect. Needed for native USB port only
  }
#endif //  USBD_USE_CDC
  Serial.println("===== Starting LTE-M leaf V4 test-bench=======");

  // Start LPWA board
  bool connected = false;
  while (!connected)
  {
    if ((lpwaAccess.begin() == LPWA_READY) &&
        (gprs.attachGPRS(GPRS_APN, GPRS_LOGIN, GPRS_PASSWORD) == GPRS_READY))
    {
      connected = true;
    }
    else
    {
      Serial.println("connecting.");
    }
  }
  Serial.println("LPWA device activate");

  String model = lpwaModem.getModel();
  Serial.print("Modem model: ");
  Serial.println(model);

  String version = lpwaModem.getFwVersion();
  Serial.print("Firmware version: ");
  Serial.println(version);

  IPAddress IPA = gprs.getIPAddress();
  Serial.print("IP: ");
  Serial.println(IPA);

  // TEST code
  // currently connected carrier
  String CA = scannerNetworks.getCurrentCarrier();
  Serial.print("Current carrier: ");
  Serial.println(CA);

  // IMEI, modem unique identifier
  Serial.print("Modem IMEI: ");
  IMEI = lpwaModem.getIMEI();
  if (IMEI != NULL)
  {
    Serial.println(IMEI);
  }
}

void loop()
{
  Serial.println("@@@@ loop");

  // TEST code
#if 0
   gprs.dettachGprs();
#endif

#if 1
  // returns strength and ber
  // signal strength in 0-31 scale. 31 means power > 51dBm
  // BER is the Bit Error Rate. 0-7 scale. 99=not detectable
  String csq = scannerNetworks.getSignalStrength();
  Serial.print("Signal Strength: ");
  Serial.print(csq);
  Serial.println(" [0-31]");
#endif

#if 1
  // network time
  Serial.print("Network Time: ");
  Serial.println(lpwaModem.getTime());
#endif
  delay(5000);
}
