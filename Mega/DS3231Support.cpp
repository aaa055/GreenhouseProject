  #include "DS3231Support.h"

char DS3231Clock::workBuff[12] = {0};

DS3231Clock::DS3231Clock()
{
}
uint8_t DS3231Clock::dec2bcd(uint8_t val)
{
  return( (val/10*16) + (val%10) );
}

uint8_t DS3231Clock::bcd2dec(uint8_t val)
{
  return( (val/16*10) + (val%16) );
}
void DS3231Clock::setTime(const DS3231Time& time)
{
  setTime(time.second, time.minute, time.hour, time.dayOfWeek, time.dayOfMonth, time.month,time.year);
}
void DS3231Clock::setTime(uint8_t second, uint8_t minute, uint8_t hour, uint8_t dayOfWeek, uint8_t dayOfMonth, uint8_t month, uint16_t year)
{
  Wire.beginTransmission(DS3231Address);
  DS3231_WIRE_WRITE(0); // указываем, что начинаем писать с регистра секунд
  DS3231_WIRE_WRITE(dec2bcd(second)); // пишем секунды
  DS3231_WIRE_WRITE(dec2bcd(minute)); // пишем минуты
  DS3231_WIRE_WRITE(dec2bcd(hour)); // пишем часы
  DS3231_WIRE_WRITE(dec2bcd(dayOfWeek)); // пишем день недели
  DS3231_WIRE_WRITE(dec2bcd(dayOfMonth)); // пишем дату
  DS3231_WIRE_WRITE(dec2bcd(month)); // пишем месяц

  while(year > 100) // приводим к диапазону 0-99
    year -= 100;
  
  DS3231_WIRE_WRITE(dec2bcd(year)); // пишем год
  Wire.endTransmission();  
}
DS3231Time DS3231Clock::getTime()
{
  DS3231Time t;

  Wire.beginTransmission(DS3231Address);
  DS3231_WIRE_WRITE(0); // говорим, что мы собираемся читать с регистра 0
  Wire.endTransmission();
  
  Wire.requestFrom(DS3231Address, 7); // читаем 7 байт, начиная с регистра 0
  
  t.second = bcd2dec(DS3231_WIRE_READ() & 0x7F);
  t.minute = bcd2dec(DS3231_WIRE_READ());
  t.hour = bcd2dec(DS3231_WIRE_READ() & 0x3F);
  t.dayOfWeek = bcd2dec(DS3231_WIRE_READ());
  t.dayOfMonth = bcd2dec(DS3231_WIRE_READ());
  t.month = bcd2dec(DS3231_WIRE_READ());
  
  t.year = bcd2dec(DS3231_WIRE_READ());  

  t.year += 2000; // приводим время к нормальному формату
  return t;
}
char* DS3231Clock::getDayOfWeekStr(const DS3231Time& t)
{
  static char* dow[] = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
  return dow[t.dayOfWeek-1];
}
/*
char* DS3231Clock::getMonthStr(const DS3231Time& t)
{
  static char* months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
  return months[t.month-1];
}
*/
char* DS3231Clock::getTimeStr(const DS3231Time& t)
{
  char* writePtr = workBuff;
  
  if(t.hour < 10)
    *writePtr++ = '0';
  else
    *writePtr++ = (t.hour/10) + '0';

  *writePtr++ = (t.hour % 10) + '0';

 *writePtr++ = ':';

 if(t.minute < 10)
  *writePtr++ = '0';
 else
  *writePtr++ = (t.minute/10) + '0';

 *writePtr++ = (t.minute % 10) + '0';

 *writePtr++ = ':';

 if(t.second < 10)
  *writePtr++ = '0';
 else
  *writePtr++ = (t.second/10) + '0';

 *writePtr++ = (t.second % 10) + '0';

 *writePtr = 0;

 return workBuff;
}
char* DS3231Clock::getDateStr(const DS3231Time& t)
{
  char* writePtr = workBuff;
  if(t.dayOfMonth < 10)
    *writePtr++ = '0';
  else
    *writePtr++ = (t.dayOfMonth/10) + '0';
  *writePtr++ = (t.dayOfMonth % 10) + '0';

  *writePtr++ = '.';

  if(t.month < 10)
    *writePtr++ = '0';
  else
    *writePtr++ = (t.month/10) + '0';
  *writePtr++ = (t.month % 10) + '0';

  *writePtr++ = '.';

  *writePtr++ = (t.year/1000) + '0';
  *writePtr++ = (t.year % 1000)/100 + '0';
  *writePtr++ = (t.year % 100)/10 + '0';
  *writePtr++ = (t.year % 10) + '0';  

  *writePtr = 0;

  return workBuff;
}
void DS3231Clock::begin()
{
  Wire.begin();
}

