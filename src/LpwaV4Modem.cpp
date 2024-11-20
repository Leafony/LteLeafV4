/*
 * LpwaV4Modem.cpp
 * LTE-M Leaf V4 library
 *
 * 2021.12.28 kt-nakamura@kddi-tech.com
 */
#include "LpwaV4Modem.h"

/**
 * モデムのモデル番号を取得します。begin() の後に呼び出してください。
 * @return モデムのモデル番号
 */
String LpwaV4Modem::getModel()
{
  char rcvbuff[100];
  char *start_p = NULL;
  char *end_p = NULL;

  if (!theMurataLpwaCore.sendCmd("AT+CGMM\r"))
    return "";
  if (theMurataLpwaCore.waitForResponse("OK\r", rcvbuff, 100) < 0)
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

/**
 * モデムのFWバージョンを取得します。begin() の後に呼び出してください。
 * @return モデムのFWバージョン
 */
String LpwaV4Modem::getFwVersion()
{
  char rcvbuff[100];
  char *start_p = NULL;
  char *end_p = NULL;

  if (!theMurataLpwaCore.sendCmd("AT+CGMR\r"))
    return "";
  if (theMurataLpwaCore.waitForResponse("OK\r", rcvbuff, 100) < 0)
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
  String fw = start_p;
  return fw;
}

/**
 * モデムのIMEI番号を取得します。begin() の後に呼び出してください。
 * @return モデムのIMEI番号
 */
String LpwaV4Modem::getImei()
{
  char rcvbuff[100];
  char *start_p = NULL;
  char *end_p = NULL;

  if (!theMurataLpwaCore.sendCmd("AT+CGSN\r"))
    return "";
  if (theMurataLpwaCore.waitForResponse("OK\r", rcvbuff, 100) < 0)
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
  String imei = start_p;
  return imei;
}

/**
 * SIMのICCID番号を取得します。begin() の後に呼び出してください。
 * @return SIMのICCID番号、SIM情報が読めない場合は"no-SIM"
 */
String LpwaV4Modem::getIccid()
{
  char rcvbuff[100];
  char *start_p = NULL;
  char *end_p = NULL;

  // SIM読み込み完了までICCIDが取れないためリトライを行う
  int waiting = 5; // リトライ回数
  while (waiting > 0)
  {
    if (!theMurataLpwaCore.sendCmd("AT%CCID\r"))
      return "";
    if (theMurataLpwaCore.waitForResponse("OK\r", rcvbuff, 100, 3000, true) >= 0)
    {
      start_p = strstr(rcvbuff, "%CCID: ");
      if (start_p != NULL)
      {
        start_p = start_p + 7;
        end_p = strstr(start_p, "\r");
        *end_p = 0x0;
        String iccid = start_p;
        return iccid;
      }
    }
    Serial.println("<warn> retry read SIM");
    waiting--;
    delay(1000);
  }
  Serial.println("<error> can't read SIM info!");
  return "no-SIM";
}

/**
 * 現在時刻を取得します。
 * NOTE: 圏外では"00/00/00,00:00:00-00" が得られる
 * @return 現在時刻
 */
String LpwaV4Modem::getTime()
{
  char rcvbuff[100];
  char *start_p = NULL;
  char *end_p = NULL;

  if (!theMurataLpwaCore.sendCmd("AT+CCLK?\r"))
    return "";
  if (theMurataLpwaCore.waitForResponse("OK\r", rcvbuff, 100) < 0)
    return "";

  start_p = strstr(rcvbuff, "+CCLK: \"");
  if (start_p == NULL)
    return "";
  start_p = start_p + 8;
  end_p = strstr(start_p, "\"");
  *end_p = 0x0;
  String currentTime = start_p;
  return currentTime;
}
