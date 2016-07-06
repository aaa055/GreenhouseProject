#ifndef DS3231SUPPORT_H
#define DS3231SUPPORT_H

#include <Wire.h>
#include "AbstractModule.h"

#if (ARDUINO >= 100)
  #define DS3231_WIRE_READ Wire.read
  #define DS3231_WIRE_WRITE Wire.write
#else
  #define DS3231_WIRE_READ Wire.receive
  #define DS3231_WIRE_WRITE Wire.send
#endif

struct DS3231Time // данные по текущему времени
{
  uint8_t second; // секунда (0-59)
  uint8_t minute; // минута (0-59)
  uint8_t hour; // час (0-23)
  uint8_t dayOfWeek; // день недели (1 - понедельник и т.д.)
  uint8_t dayOfMonth; // день месяца (0-31)
  uint8_t month; // месяц(1-12)
  uint16_t year; // формат - ХХХХ
}; 

enum { DS3231Address = 0x68 }; // адрес датчика

class DS3231Clock
{

  private:

    uint8_t dec2bcd(uint8_t val);
    uint8_t bcd2dec(uint8_t val);

    static char workBuff[12]; // буфер под дату/время
  
  public:
    DS3231Clock();

    void setTime(uint8_t second, uint8_t minute, uint8_t hour, uint8_t dayOfWeek, uint8_t dayOfMonth, uint8_t month, uint16_t year);
    void setTime(const DS3231Time& time);

    const char* getDayOfWeekStr(const DS3231Time& t);
   // char* getMonthStr(const DS3231Time& t);
    const char* getTimeStr(const DS3231Time& t);
    const char* getDateStr(const DS3231Time& t);

    DS3231Time getTime();

    Temperature getTemperature();
 
    void begin();
};



#endif
