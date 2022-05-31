#include <Wire.h>

/*
 * 機能拡張
 *  行内で次のエスケープシーケンスを使用可
 *   \r \n \t \xnn \\
 * 
 *  その他コマンド
 *   #cr 1  行末に'\r'を付けて送信
 *   #cr 0  行末に'\r'を付けないで送信
 *   #hex 0 HEX表示なし
 *   #hex 1 HEX表示あり
 */

#define LTE_LEAF_V4EVT // EVT,DVT1 only

#ifdef LTE_LEAF_V4EVT // EVT,DVT1 はDVT2,PVTと端子割り当てが違う
 // STM32 signal
 #define I2C2_SCL PB_8
 #define I2C2_SDA PB_9
 #define MDM_USART_TXD PA_9
 #define MDM_USART_RXD PA_10
 #define MDM_USART_CTS PA7
 #define MDM_USART_RTS PA6
 // I2C GPIO signal
 #define LTE_PWR_ON     (1 << 0) // Modem Main power
 #define LTE_SHUTDOWNn  (1 << 1)  // Modem Shutdown
 #define LTE_WAKEUP     (1 << 2) // Modem Wakeup
 #define GPS_PWR_ON     (1 << 3) // GPS Antena Power
 #define BM_ON          (1 << 4) // battery mesurent ON
 // I2C GPIO input
 #define LTE_RST_STS    (1 << 6) // Modem GPIO50(reset status)
 #define LTE_SC_SWP     (1 << 7) // Host Wake-Up request

#else // DVT2,PVT
 // STM32 signal
 #define I2C2_SCL PB_8
 #define I2C2_SDA PB_9
 #define MDM_USART_TXD PA_9
 #define MDM_USART_RXD PA_10
 #define MDM_USART_CTS PA7
 #define MDM_USART_RTS PA6
 // I2C GPIO signal
 #define LTE_PWR_ON     (1 << 0) // Modem Main power
 #define LTE_SHUTDOWNn  (1 << 1)  // Modem Shutdown
 #define LTE_WAKEUP     (1 << 2) // Modem Wakeup
 #define GPS_PWR_ON     (1 << 3) // GPS Antena Power
 #define BM_ON          (1 << 4) // battery mesurent ON
 // I2C GPIO input
 #define LTE_RST_STS    (1 << 6) // Modem GPIO50(reset status)
 #define LTE_SC_SWP     (1 << 7) // Host Wake-Up request
#endif // LTE_LEAF_V4EVT

// I2C GPIO TCA6408
#define TCA6408_ADDR 0x21
#define TCA6408_TIMEOUT 100
#define TCA6408_INPUT               0x00
#define TCA6408_OUTPUT              0x01
#define TCA6408_POLARITY_INVERSION  0x02
#define TCA6408_CONFIGURATION       0x03

HardwareSerial serialX(MDM_USART_RXD, MDM_USART_TXD); // RX,TX
char line[300];
int line_p = 0;
int esc = 0;
int crmode = 1;
int hexmode = 0;
int cnt = 0;

// I2C GPIO データを書き込む
void writeTca6408(uint8_t data, uint8_t reg) {
  Wire.beginTransmission(TCA6408_ADDR);
  Wire.write((uint8_t) reg);
  Wire.write((uint8_t) data);
  Wire.endTransmission();
  return;
}

bool readTca6408(uint8_t *data, uint8_t reg) {
  Wire.beginTransmission(TCA6408_ADDR);
  Wire.write((uint8_t) reg);
  Wire.endTransmission();
  uint8_t timeout=0;

  Wire.requestFrom(TCA6408_ADDR, (uint8_t) 0x01);
  while(Wire.available() < 1) {
    timeout++;
    if(timeout > TCA6408_TIMEOUT) {
      return(true);
    }
    delay(1);
  }
  *data = Wire.read();
  return(false);
}

void modemWrite(uint8_t data) {
//  Serial.print("<"); Serial.print(data,HEX); Serial.print(">");
  serialX.write(data);
}
void setup() {
  Serial.begin(115200);
  serialX.begin(115200);
  delay(1000);
  Serial.println("Starting typeSC1 EVK modem check");

  pinMode(MDM_USART_CTS, OUTPUT);
  digitalWrite(MDM_USART_CTS, LOW);
  pinMode(MDM_USART_RTS, INPUT);
 
  Wire.setSDA(I2C2_SDA);
  Wire.setSCL(I2C2_SCL);
  Wire.begin();

  // LTE-Mモデムの電源オン→ウェイクアップ
  writeTca6408(0x0, TCA6408_OUTPUT); // clear output register
  writeTca6408(LTE_RST_STS |LTE_SC_SWP , TCA6408_CONFIGURATION ); // input bit[7:6}
  writeTca6408(LTE_PWR_ON | LTE_SHUTDOWNn, TCA6408_OUTPUT); //
  delay(1000);
  writeTca6408(LTE_PWR_ON | LTE_SHUTDOWNn | LTE_WAKEUP , TCA6408_OUTPUT); // Write values to IO-expander
  delay(3000);

  Serial.println("typeSC1 power-on");
  line_p = 0;
}

void loop() {
  // put your main code here, to run repeatedly:
#if 1
  if (Serial.available()) {      // If anything comes in Serial (USB),
//    serialX.write(Serial.read());   // read it and send it out Serial1 (pins 0 & 1)
      char c = Serial.read();
      if(c == '\r') {
        if(strstr(line,"#cr ")==line) {
          crmode = atoi(line+4);
          Serial.print("cr mode = ");
          Serial.println(crmode);
          line[line_p = 0] = 0;
          return;
        }
        if(strstr(line,"#hex ")==line) {
          hexmode = atoi(line+5);
          Serial.print("hex mode = ");
          Serial.println(hexmode);
          line[line_p = 0] = 0;
          return;
        }
        Serial.print("Send> [");
        Serial.print(line);
        Serial.println("]");
        esc = 0;
        for(int i = 0; i < line_p; ++i) {
          char d = line[i];
          char d2;
          if(esc) {
            switch(d) {
            case 'r': d = '\r'; break;
            case 'n': d = '\n'; break;
            case 't': d = '\t'; break;
            case 'b': d = '\b'; break;
            case 'x':
              d = toupper(line[++i]);
              d = (d>='A') ? d-'A'+10 : d-'0';
              d2 = toupper(line[++i]);
              d2 = (d2>='A') ? d2-'A'+10 : d2-'0';
              d = (d << 4) + d2;
              break;
            }
            modemWrite(d);
            esc = 0;
          }
          else {
            if(d == '\\')
              esc = 1;
            else {
              modemWrite(d);
            }
          }
        }
        if(crmode)
          modemWrite('\r');
        line[line_p = 0] = 0;
      }
      else {
        line[line_p++] = c;
        line[line_p] = 0;
      }
  }
  if (serialX.available()) {     // If anything comes in Serial1 (pins 0 & 1)
//    Serial.write(serialX.read());   // read it and send it out Serial (USB)
    uint8_t data = serialX.read();
    Serial.write(data);   // read it and send it out Serial (USB)
    if(hexmode) {
      Serial.print("[");
      Serial.print(data,HEX);
      Serial.print("]");
    }
  }
#endif

#ifdef USE_LOCAL_LOOPBACK 
  if (Serial.available()) {      // If anything comes in Serial (USB),
    Serial.write(Serial.read());   // read it and send it out Serial1 (pins 0 & 1)
  }
#endif

}