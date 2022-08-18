 /*
 * LpwaV4Borad.h
 * LTE-M Leaf V4 library
 * LTE-Mリーフボード定義ファイル
 *
 * 2022.5.31 update PVT pin-assign kt-nakamura@kddi-tech.com
 * 2021.12.28 kt-nakamura@kddi-tech.com
 */
#ifndef LPWA_V4_BOARD_H_
#define LPWA_V4_BOARD_H_

//#define LTE_LEAF_V4EVT // EVT,DVT1 only

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
 #define MDM_USART_CTS PB5
 #define MDM_USART_RTS PA8
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

// I2C ADC ADC101C027
#define ADC101C027_ADR 0x50
#define ADC101C027_CONVERSION_RESULT  0x0
#define ADC101C027_ALERT_STATUS       0x1
#define ADC101C027_CONFIGRATION       0x2
#define ADC101C027_LOW_LIMIT          0x3
#define ADC101C027_HIGH_LIMIT         0x4
#define ADC101C027_HYSTEREIS          0x5
#define ADC101C027_LOWEST_CONVERSION  0x6
#define ADC101C027_HIGHEST_CONVERSION 0x7
#define ADC101C027_VA                 3300 // 3300mV
#define ADC101C027_TIMEOUT            100

#endif // LPWA_V4_BOARD_H_
