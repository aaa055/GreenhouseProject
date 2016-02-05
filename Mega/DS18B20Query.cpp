#include "DS18B20Query.h"
#include <OneWire.h>
#include "Globals.h"

bool DS18B20Support::readTemperature(DS18B20Temperature* result)
{
  result->Whole = NO_TEMPERATURE_DATA; // нет данных с датчика
  result->Fract = 0;

  if(!pin)
    return false;


  OneWire ow(pin);

  if(!ow.reset()) // нет датчика
    return false;

  byte data[9];
   
  ow.write(0xCC); // пофиг на адреса
  ow.write(0x44); // запускаем преобразование

  ow.reset();
  ow.write(0xCC); // пофиг на адреса
  ow.write(0xBE); // читаем scratchpad датчика на пине

  for(uint8_t i=0;i<9;i++)
    data[i] = ow.read();


 if ( OneWire::crc8( data, 8) != data[8]) // проверяем контрольную сумму
      return false;
  
  int loByte = data[0];
  int hiByte = data[1];

  int temp = (hiByte << 8) + loByte;
  
  result->Negative = (temp & 0x8000);
  
  if(result->Negative)
    temp = (temp ^ 0xFFFF) + 1;

  int tc_100 = (6 * temp) + temp/4;

  result->Whole = tc_100/100;
  result->Fract = tc_100 % 100;

  return true;
    
}

