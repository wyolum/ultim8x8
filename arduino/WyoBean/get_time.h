#ifndef GET_TIME_H
#define GET_TIME_H

#include "TimeLib.h"
#include <stdint.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <RTClib.h>

class Clock{
 public:
  Clock();
  virtual uint32_t now();
  bool isCurrent();
  virtual bool set(uint32_t _t);
  int year();
  int month();
  int day();
  int hours();
  int minutes();
  int seconds();
};

class DummyClock : public Clock{
 public:
  DummyClock();
  bool set(uint32_t _t);
  uint32_t now();
};

class NTPClock : public Clock{
 public:
  NTPClock();
  bool set(uint32_t _t);  
  bool isCurrent();
  void setup(NTPClient *_timeClient);
  void setOffset(int32_t offset_seconds);
  NTPClient *timeClient;
  uint32_t now();
  bool update();
};

class DS3231Clock : public Clock{
 public:
  RTC_DS3231 rtc;
  DS3231Clock();
  void setup();
  uint32_t now();
  bool set(uint32_t t);
};

class DoomsdayClock : public Clock{
 public:
  Clock *master;
  Clock *backup;

  DoomsdayClock();
  bool set(uint32_t _t);
  void setup(Clock* _master, Clock* _backup);
  uint32_t now();
};

#endif
