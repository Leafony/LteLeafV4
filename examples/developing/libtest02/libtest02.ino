/*

  Udp data I/O test

*/
#include <LpwaV4.h>

#include "arduino_secrets.h"
// Please enter your sensitive data in the Secret tab or arduino_secrets.h
// APN data
const char GPRS_APN[] = SECRET_GPRS_APN;
const char GPRS_LOGIN[] = SECRET_GPRS_LOGIN;
const char GPRS_PASSWORD[] = SECRET_GPRS_PASSWORD;

//#define ENDPOINT_URL "140.227.119.36"
#define ENDPOINT_URL "wsproxy-proxy-server.kddi-tech.com"
#define ENDPOINT_PORT 12347

// initialize the library instance
LpwaClient client;
GPRS gprs;
LpwaAccess lpwaAccess;

// A UDP instance to let us send and receive packets over UDP
LpwaUdp Udp;



void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
#ifdef USBD_USE_CDC
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
#endif //  USBD_USE_CDC
  Serial.println("Starting Arduino UDP binary test");

  // connection state
  bool connected = false;

  // After starting the modem with LpwaAccess.begin()
  // attach the shield to the GPRS network with the APN, login and password
  while (!connected) {
    if ((lpwaAccess.begin() == LPWA_READY) &&
        (gprs.attachGPRS(GPRS_APN, GPRS_LOGIN, GPRS_PASSWORD) == GPRS_READY)) {
      connected = true;
    } else {
      Serial.println("connecting.");
      delay(1000);
    }
  }

}

void loop() {
 Serial.println("@@@@@ loop()");
 char sendBytes[256];
 char recvBytes[1500];
 for (int i=0;i<256;i++) {
   sendBytes[i] = i;
 }
 int a = Udp.begin();
 // 最大サイズ送信(1020byte)
  a = Udp.beginPacket(ENDPOINT_URL, ENDPOINT_PORT);
  if (!Udp.write((uint8_t *)sendBytes,256)) {
    Serial.println("<error> UDP write failed"); 
  }
  if (!Udp.write((uint8_t *)sendBytes,256)) {
    Serial.println("<error> UDP write failed");
  }
  if (!Udp.write((uint8_t *)sendBytes,256)) {
    Serial.println("<error> UDP write failed");
  }
  if (!Udp.write((uint8_t *)sendBytes,252)) {
    Serial.println("<error> UDP write failed");
  }
#if 0
#endif
  Udp.endPacket();
#if 1
  for (int i=0;i<5;i++){
    delay (1000);
    if (Udp.parsePacket() > 0) {
      int rcvlen = Udp.available();
      Serial.print("packet received sise: ");
      Serial.println(rcvlen);
      Udp.read(recvBytes, rcvlen);
      for (int j=0;j<rcvlen;j++) {
        if (recvBytes[j] != (j & 0xff)) {
          Serial.print("data error! send:");
          Serial.print(j & 0xff,HEX);
          Serial.print(" recv:");
          Serial.println(recvBytes[j],HEX);
        }
      }
    }
  }
#endif
  Udp.stop();
  delay (10000);
}
