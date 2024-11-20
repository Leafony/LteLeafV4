/*
 * LpwaV4Gprs.cpp
 * LTE-M Leaf V4 library
 *
 * 2021.12.28 kt-nakamura@kddi-tech.com
 */
#include "LpwaV4Gprs.h"

/**
 * @return GPRS接続状態　(false: 接続中, true: それ以外)
 */
bool LpwaV4Gprs::_ready()
{
  return true;
}

/**
 * 指定したアクセスポイント名 (APN) に接続して、GPRS通信を開始します。
 * 接続完了した場合、GPRS_READY を返します。
 * @param apn 提供されているアクセスポイント名 (APN)
 * @param username APNのユーザー名
 * @param password APNのパスワード
 * @param timeout 接続を待つのを打ち切る時間 (ms)
 * @return ネットワークの状態
 */
NetworkStatus LpwaV4Gprs::attachGprs(const char *apn, const char *username,
                                     const char *password,
                                     unsigned long timeout)
{
  //  Serial.println("@@@@@ LpwaV4Gprs::attachGprs() enter");

  const unsigned long start = millis();

  // disconnect PDN
  bool statCmd = theMurataLpwaCore.sendf("AT%%PDNACT=%d,1\r", 0);
  int a = theMurataLpwaCore.waitForResponse("OK\r", NULL, 0, 3000, true);
  statCmd = theMurataLpwaCore.sendCmd("at%PDNACT?\r");
  a = theMurataLpwaCore.waitForResponse("OK\r");

  // LTE Band 18(800MHz) only
  // if (!theMurataLpwaCore.sendCmd("AT%SETCFG=\"BAND\",\"18\"\r"))
  //   return theMurataLpwaCore.status = LPWA_FAIL;
  // if (theMurataLpwaCore.waitForResponse("OK\r", NULL, 0, 1000) < 0)
  //   return theMurataLpwaCore.status = LPWA_FAIL;

  // if (!theMurataLpwaCore.sendCmd("AT%GETCFG=\"BAND\"\r"))
  //   return theMurataLpwaCore.status = LPWA_FAIL;
  // if (theMurataLpwaCore.waitForResponse("OK\r", NULL, 0, 1000) < 0)
  //   return theMurataLpwaCore.status = LPWA_FAIL;

  // PDP setting
  if (!theMurataLpwaCore.sendf("AT%%PDNSET=1,%s,IP,CHAP,%s,%s,,0,0,0\r", apn, username, password))
    return theMurataLpwaCore.status = LPWA_FAIL;
  if (theMurataLpwaCore.waitForResponse("OK\r") < 0)
    return theMurataLpwaCore.status = LPWA_FAIL;
  delay(20000); // PDP設定後はコマンドを受け付けないため待機

  // check PDP connection
  if (!theMurataLpwaCore.sendCmd("at%PDNACT?\r"))
    return theMurataLpwaCore.status = LPWA_FAIL;
  if (theMurataLpwaCore.waitForResponse("OK\r") < 0)
  {
    delay(1000);
  }

  // PDP connect
  int cntWait = 10;
  while (theMurataLpwaCore.getPdpStat() == 0)
  {
    cntWait--;
    if (cntWait < 1)
    {
      Serial.println("@@@@@ LpwaV4Gprs::attachGprs() PDN timeout");
      return theMurataLpwaCore.status = LPWA_FAIL;
    }

    if (!theMurataLpwaCore.sendf("AT%%PDNACT=%d,1,%s\r", 1, apn))
      return theMurataLpwaCore.status = LPWA_FAIL;
    if (theMurataLpwaCore.waitForResponse("OK\r") < 0)
    {
      delay(5000);
      continue;
    }

    if (!theMurataLpwaCore.sendCmd("at%PDNACT?\r"))
      return theMurataLpwaCore.status = LPWA_FAIL;
    if (theMurataLpwaCore.waitForResponse("OK\r") < 0)
    {
      delay(1000);
      continue;
    }

    delay(1000);
  }

  //  Serial.println("@@@@@ LpwaV4Gprs::attachGprs() exit");
  return GPRS_READY;
}

/**
 * 接続中のGPRS通信を切断します。
 * 接続完了した場合、LPWA_READY を返します。
 * @return ネットワークの状態
 */
NetworkStatus LpwaV4Gprs::dettachGprs()
{
  // disconnect PDN
  if (theMurataLpwaCore.getPdpStat() == 1)
  {
    bool statCmd = theMurataLpwaCore.sendf("AT%%PDNACT=%d,1\r", 0);
    int a = theMurataLpwaCore.waitForResponse("OK\r");
    statCmd = theMurataLpwaCore.sendCmd("at%PDNACT?\r");
    a = theMurataLpwaCore.waitForResponse("OK\r");
    theMurataLpwaCore.clrPdpStat();
  }
  return theMurataLpwaCore.status = LPWA_READY;
}

/**
 * WWANに割り当てられたIPアドレスを返します。
 * 得られない場合は IPAddress(0, 0, 0, 0) を返します。
 * @return WWANに割り当てられたIPアドレス
 */
IPAddress LpwaV4Gprs::getIpAddress()
{
  IPAddress ip;
  char rcvbuff[100];
  if (!theMurataLpwaCore.sendCmd("AT+CGPADDR\r"))
    return theMurataLpwaCore.status = LPWA_FAIL;
  if (theMurataLpwaCore.waitForResponse("OK\r", rcvbuff, 100) < 0)
  {
    return ip;
  }
  // NOTE: AT+CGPADDR: の応答は `+CGPADDR: <context>,<address>` 形式
  // NOTE: address のみ文字列で "" で囲まれる
  ip = theMurataLpwaCore.str2Ip(strstr(rcvbuff, "+CGPADDR: "));

  return ip;
}
