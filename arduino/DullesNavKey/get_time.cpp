#include "TimeLib.h"
#include "get_time.h"

bool Clock::isCurrent(){
  return true;
}
int Clock::year(){
  return ::year(now());
}
int Clock::month(){
  return ::month(now());
}
int Clock::day(){
  return ::day(now());
}
int Clock::hours(){
  return (now() % 86400) / 3600;
}
int Clock::minutes(){
  return (now() % 3600) / 60;
}
int Clock::seconds(){
  return (now() % 60);
}

uint32_t current_time = 0;
Clock::Clock(){
}
bool DummyClock::set(uint32_t t){
  return false;
}
DummyClock::DummyClock(){
}
uint32_t DummyClock::now(){
  return current_time++;
}

NTPClock::NTPClock(){
}
bool NTPClock::set(uint32_t t){
  return false;
}

void NTPClock::setup(NTPClient *_timeClient){
  this->timeClient = _timeClient;
  //this->timeClient->setTimeOffset(-240 * 60);
  this->timeClient->begin();
}
void NTPClock::setOffset(int32_t offset_seconds){
  this->timeClient->setTimeOffset(offset_seconds);
}

bool NTPClock::update(){
  return this->timeClient->update();
}
bool NTPClock::isCurrent(){
  return millis() - this->timeClient->_lastUpdate < this->timeClient->_updateInterval;
}

uint32_t NTPClock::now(){
  this->update();
  return this->timeClient->getEpochTime();
}

uint32_t NTPClock::gmt(){
  this->update();
  return this->now() + this->timeClient->_timeOffset;
}

uint32_t NTPClock::dow(){
  this->update();
  return this->timeClient->getDay();
}

time_t ds3231_now(){
  RTC_DS3231 rtc;
  time_t n = rtc.now().unixtime();
  Serial.print("DS3231 Time:");
  Serial.println(n);
  return n;
}
time_t timelib_now(){ // now() so that Clock classes can call the original timelib_now function
  return now();
}
void timelib_setTime(time_t tm){// rename so that Clock classes can call the original timelib_now function
  return setTime(tm);
}

DS3231Clock::DS3231Clock(){
}
void DS3231Clock::setup(){
  setSyncProvider(ds3231_now);
  setSyncInterval(678);
  timelib_setTime(rtc.now().unixtime());
}
uint32_t DS3231Clock::now(){
  return timelib_now();
  //return rtc.now().unixtime();
}

bool DS3231Clock::set(uint32_t t){
  DateTime dt(t);
  rtc.adjust(dt);
  timelib_setTime(rtc.now().unixtime());
  return true;
}

DoomsdayClock::DoomsdayClock(){
}
void DoomsdayClock::setup(Clock *_master, Clock *_backup){
  this->master = _master;
  this->backup = _backup;
}
bool DoomsdayClock::set(uint32_t t){
  return false;
}

uint32_t abs_diff(uint32_t left, uint32_t right){
  uint32_t out;
  
  if (left > right){
    out = left - right;
  }
  else{
    out = right - left;
  }
  return out;
}

uint32_t DoomsdayClock::now(){
  uint32_t out;
  uint32_t m, b;
  const uint32_t tol_sec = 5;
  
  m = master->now();
  b = backup->now();
  
  if(master->isCurrent()){
    out = master->now();
    if(abs_diff(m, b) > tol_sec){ /// false && to ignore ds3231 errors
      /*
      */
      Serial.print("abs diff: ");
      Serial.println(abs_diff(m, b), DEC);
      Serial.print("master: ");
      Serial.println(m, DEC);
      Serial.print("backup: ");
      Serial.println(b, DEC);
      Serial.print("backup wrong: ");
      Serial.println(backup->now(), DEC);

      backup->set(m);
      Serial.print("SET THE BACKUP to master: ");
      Serial.println(m, DEC);
      Serial.print("got: ");
      Serial.println(backup->now(), DEC);
      Serial.println();
      Serial.println();
      delay(1000);
      /*
      */
    }
  }
  else{
    out = backup->now();
  }
  return out;
}
