/*
 * LTE-M リーフV4モデム情報取得
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
  Serial.println("===== Starting LTE-M leaf device info =======");

  // Start LPWA board
  bool connected = false;
  while (!connected)
  {
    if ((lpwaAccess.begin() == LPWA_READY) &&
        (gprs.attachGPRS(GPRS_APN, GPRS_LOGIN, GPRS_PASSWORD, gprs.LPWA_V4_GPRS_BAND_KDDI) == GPRS_READY))
    {
      connected = true;
    }
    else
    {
      Serial.println("connecting.");
    }
  }
  Serial.println("LPWA device activate");

  // get modem parameters
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

  IPAddress IPA = gprs.getIPAddress();
  Serial.print("IP: ");
  Serial.println(IPA);
  delay(5000);
}
