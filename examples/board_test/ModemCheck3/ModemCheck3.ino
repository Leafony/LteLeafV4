//==============================================================================
// Murata LTE command chekck
//
// 2024/12/25
// U-Tokyo
//==============================================================================
#include <LpwaV4.h>
#include "arduino_secrets.h"




//==============================================================
// define
//==============================================================
#define ATCOMM_NAME_SIZE  10        // number of assigned AT command

#define TX_BUFFSIZE       64        // AT command 
#define RX_BUFFSIZE       100       // Response of AT command

#define UART_RX_BUFFSIZE  64        // UART from PC to STM32

//==============================================================
// instance
//==============================================================
Lpwa lpwaAccess;
GPRS gprs;
LpwaScanner scannerNetworks;
LpwaModem lpwaModem;

//==============================================================
// register
//==============================================================
// Please enter your sensitive data in the Secret tab or arduino_secrets.h
// APN data
const char GPRS_APN[] = SECRET_GPRS_APN;
const char GPRS_LOGIN[] = SECRET_GPRS_LOGIN;
const char GPRS_PASSWORD[] = SECRET_GPRS_PASSWORD;

// Save data variables
String IMEI = "";

//--------------------
// AT command buffer
//--------------------
const char *atCommNames[ATCOMM_NAME_SIZE + 1] = {
                      "AT",                   // at0: check LTE module alive
                      "AT+CFUN=1,1",          // at1: reset defualt
                      "AT+CPIN?",             // at2: check SIM card
                      "AT+COPS=?",            // at3: reserch available network venders
                      "AT+COPS?",             // at4: check connected connected vender
                      "AT+COPS=0",            // at5: auto mode
                      "AT+COPS=1,2,",         // at6: manual mode
                      "AT%GETCFG=\"BAND\"",   // at7: check connected band
                      "AT%SETCFG=\"BAND\",\"",// at8: connect selected band
                      "AT+CSQ"                // at9: check wave signel strength
};

char sendBuff[TX_BUFFSIZE];
char atCommBuff[TX_BUFFSIZE];
String atResp;

bool isATcorrect= 0;

//--------------------
// UART recieve buffer
//--------------------
uint8_t buffCount;
char rxBuff[UART_RX_BUFFSIZE];

bool resDataFlg = 0;
bool isFinished = 0;
bool isRecieved = 0;

//==============================================================
// AT command
//==============================================================-
String atComm(char *trsbuff){

  char rcvbuff[RX_BUFFSIZE];
  char *start_p = NULL;
  char *end_p = NULL;

  if (!theMurataLpwaCore.sendCmd((const char*)trsbuff))
    return "";

  if (theMurataLpwaCore.waitForResponse("OK\r", rcvbuff, 50000) < 0)
    return "";

  start_p = strstr(rcvbuff, "\r\n");
  if (start_p == NULL)
    return "";
  while (1)
  {
    if ((*start_p == '\r') || (*start_p == '\n') || (*start_p == ' '))
    {
      start_p++;
    }
    else
    {
      break;
    }
  }
  end_p = strstr(start_p, "\r");
  if (end_p == NULL)
    return "";
  *end_p = 0x0;
  String model = start_p;
  return model;
}


//==============================================================
// menue
//==============================================================-
void putMenu(void)
{
  Serial.println("------------------------------------------");
  Serial.println("<<< AT command menue>>>");
  Serial.println("   at0         :" + String(atCommNames[0]));
  Serial.println("   at1         :" + String(atCommNames[1]));
  Serial.println("   at2         :" + String(atCommNames[2]));
  Serial.println("   at3         :" + String(atCommNames[3]));
  Serial.println("   at4         :" + String(atCommNames[4]));
  Serial.println("   at5         :" + String(atCommNames[5]));
  Serial.println("   at6 number  :" + String(atCommNames[6]) + "number");
  Serial.println("   at7         :" + String(atCommNames[7]));
  Serial.println("   at8 number  :" + String(atCommNames[8]) + "number\"");
  Serial.println("   at9         :" + String(atCommNames[9]));
  Serial.println("------------------------------------------");
  Serial.println();
}

//==============================================================
// UART from PC to STM32
//==============================================================-
void readUart(){

  char c;

  resDataFlg = 0;
  
  //-------------------
  // read data
  //------------------
  if(Serial.available()){

    //-------------------
    // data clear
    //-------------------
    rxUARTClear();

    //-------------------
    // read buffer
    //-------------------
    isFinished = 0;
    isRecieved = 0;
    buffCount   = 0;
    
    while(isFinished == 0){

      if(Serial.available()){

         c = Serial.read();

         if(c != 0){

           rxBuff[buffCount] = c;
           buffCount += 1;
         }
         
      }
      else{

        delay(100);           // wait 1ms in case of no data in buffer
      }

      //-------------------------------------------------------
      // recieve <CR>
      //-------------------------------------------------------
      if(c == '\r'){

        isRecieved = 1;
      }
        
      //-------------------------------------------------------
      // recieve <LF>
      //-------------------------------------------------------
      else if((c == '\n') && (isRecieved == 1)){

        isFinished = 1;

        resDataFlg = 1;
      }
        
      //--------------------------------------------------
      // recieve not <LF>
      //--------------------------------------------------          
      else{

        isRecieved = 0;
      }

      //--------------------------------------------------
      // finish if recieved over maximum (include "\r\n")
      //--------------------------------------------------         
      if(buffCount > (UART_RX_BUFFSIZE -1)){

        isFinished = 1;
      }      

    }
  }
}

//---------------------------------------
// clear rxUART
//---------------------------------------
void rxUARTClear(){

  for(uint8_t i=0; i<UART_RX_BUFFSIZE; i++){

    rxBuff[i] = 0;
  } 
}

//==============================================================
// setup
//==============================================================
void setup() {

  bool connected = false;

  Serial.begin(115200);

  Serial.println("---------------------------------");

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
  String model = lpwaModem.getModel();
  Serial.print("Modem model: ");
  Serial.println(model);

  String version = lpwaModem.getFwVersion();
  Serial.print("Firmware version: ");
  Serial.println(version);

  //IPAddress IPA = gprs.getIPAddress();
  //Serial.print("IP: ");
  //Serial.println(IPA);

  // TEST code
  // currently connected carrier
  //String CA = scannerNetworks.getCurrentCarrier();
  //Serial.print("Current carrier: ");
  //Serial.println(CA);

  // IMEI, modem unique identifier
  Serial.print("Modem IMEI: ");
  IMEI = lpwaModem.getIMEI();
  if (IMEI != NULL)
  {
    Serial.println(IMEI);
  }
  
  Serial.println("---------------------------------");
  Serial.println();
}

//==============================================================
// loop
//==============================================================
void loop() {

  readUart();

  if(resDataFlg == 1){

    resDataFlg = 0;

    //-----------------------------------------------
    // AT command
    //-----------------------------------------------
    if((rxBuff[0] == 'a') && (rxBuff[1] == 't' ) ){

      isATcorrect = 0;

      memset((char*)sendBuff, 0, TX_BUFFSIZE);
      memset((char*)atCommBuff, 0, TX_BUFFSIZE);

      //-------------------------
      // check at command: at*
      //-------------------------
      switch(rxBuff[2]){

        //----------------------
        // at0
        //----------------------
        case '0':

          isATcorrect = 1;
          strcat(atCommBuff,atCommNames[0]);
        break;

        //----------------------
        // at1
        //----------------------
        case '1':
          isATcorrect = 1;
          strcat(atCommBuff,atCommNames[1]);

        break;

        //----------------------
        // at2
        //----------------------
        case '2':
          isATcorrect = 1;
          strcat(atCommBuff,atCommNames[2]);

        break;

        //----------------------
        // at3
        //----------------------
        case '3':
          isATcorrect = 1;
          strcat(atCommBuff,atCommNames[3]);

        break;
        //----------------------
        // at4
        //----------------------
        case '4':
          isATcorrect = 1;
          strcat(atCommBuff,atCommNames[4]);

        break;

        //----------------------
        // at5
        //----------------------
        case '5':
          isATcorrect = 1;
          strcat(atCommBuff,atCommNames[5]);

        break;

        //----------------------
        // at6
        //----------------------
        case '6':

          isATcorrect = 1;
          strcat(atCommBuff,atCommNames[6]);

          for(uint8_t i = 4; i < (buffCount-2); i++){

            sprintf(sendBuff,"%c",rxBuff[i]);
            strcat(atCommBuff,sendBuff);
          }
        break;

        //----------------------
        // at7
        //----------------------
        case '7':
          isATcorrect = 1;
          strcat(atCommBuff,atCommNames[7]);


        break;

        //----------------------
        // at8
        //----------------------
        case '8':

          isATcorrect = 1;
          strcat(atCommBuff,atCommNames[8]);

          for(uint8_t i = 4; i < (buffCount-2); i++){

            sprintf(sendBuff,"%c",rxBuff[i]);
            strcat(atCommBuff,sendBuff);
          }

          sprintf((char*)sendBuff,"\"");
          strcat(atCommBuff,sendBuff);
        break;
        //----------------------
        // at9
        //----------------------
        case '9':
          isATcorrect = 1;
          strcat(atCommBuff,atCommNames[9]);

        break;
      }

      //-------------------------
      // send AT command with "\r"
      //-------------------------
      if(isATcorrect == 1){

        Serial.println(String(atCommBuff));

        sprintf((char*)sendBuff,"\r");
        strcat(atCommBuff,sendBuff);

        atResp = atComm(atCommBuff);

        Serial.println(atResp);
        Serial.flush();
      }
      else{

        Serial.println("Error");
      }

    }
    //-----------------------------------------------
    // menue
    //-----------------------------------------------
    else if(rxBuff[0] == 'm'){

      putMenu();
      Serial.flush();
    }
    else{

      Serial.println("Error");
    }

    Serial.println();
  }
}
