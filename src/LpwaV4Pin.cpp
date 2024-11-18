/*
 * LpwaV4Pin.cpp
 * LTE-M Leaf V4 library
 *
 * 2021.12.28 kt-nakamura@kddi-tech.com
 */
#include "LpwaV4Pin.h"

int LpwaV4Pin::isPin()
{
  char rcvbuff[100];
  char *find_p;
  if (!theMurataLpwaCore.sendCmd("AT+CPIN?\r"))
    return -1;
  if (theMurataLpwaCore.waitForResponse("OK\r", rcvbuff, 100) < 0)
  {
    return -3;
  }

  // +CPIN: <code>
  find_p = strstr(rcvbuff, "READY\r");
  if (find_p != NULL)
    return 0;

  find_p = strstr(rcvbuff, " SIM PIN\r");
  if (find_p != NULL)
    return 1;

  return -1;
}
