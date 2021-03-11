#ifndef ESPBREW_NTP_H
#define ESPBREW_NTP_H

#include "time.h"

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = -3600 * 3;

String GetLocalTime()
{
  String S_returnValue = "00:00:00";
  struct tm timeinfo;
  if(getLocalTime(&timeinfo))
  {
    char s[64];
    strftime(s, 64, "%H:%M:%S", &timeinfo); /* formatting time*/
    S_returnValue = String(s);
    
#ifdef _DEBUG_
    Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
#endif
  }
  
  return S_returnValue;
}

void NTPSetup()
{
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

#endif /* ESPBREW_NTP_H */
