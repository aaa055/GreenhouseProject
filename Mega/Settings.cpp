#include "Settings.h"
#include "Globals.h"
#include <EEPROM.h> 

//  ГЛОБАЛЬНЫЕ НАСТРОЙКИ
#define SETT_HEADER1 0xDE // байты, сигнализирующие о наличии сохранённых настроек
#define SETT_HEADER2 0xAD

GlobalSettings::GlobalSettings()
{
  ResetToDefault();
}
void GlobalSettings::ResetToDefault()
{
  tempOpen = DEF_OPEN_TEMP;
  tempClose = DEF_CLOSE_TEMP;
  openInterval = DEF_OPEN_INTERVAL;
  
}

void GlobalSettings::Load()
{  
  uint16_t addr = 0;

  uint8_t h1,h2;
  h1 = EEPROM.read(addr++);
  h2 = EEPROM.read(addr++);

  if(!(h1 == SETT_HEADER1 && h2 == SETT_HEADER2)) // ничего нет в памяти
  {
    ResetToDefault(); // применяем настройки по умолчанию
    Save(); // сохраняем их
    return; // и выходим
  }
  
  // читаем температуру открытия
  tempOpen = EEPROM.read(addr++);

  // читаем температуру закрытия
  tempClose = EEPROM.read(addr++);

  // читаем интервал работы окон
   byte* wrAddr = (byte*) &openInterval;
  
  *wrAddr++ = EEPROM.read(addr++);
  *wrAddr++ = EEPROM.read(addr++);
  *wrAddr++ = EEPROM.read(addr++);
  *wrAddr++ = EEPROM.read(addr++);

  

  // читаем другие настройки!

  
}

void GlobalSettings::Save()
{
  uint16_t addr = 0;

  // пишем наш заголовок, который будет сигнализировать о наличии сохранённых настроек
  EEPROM.write(addr++,SETT_HEADER1);
  EEPROM.write(addr++,SETT_HEADER2);

  // сохраняем температуру открытия
  EEPROM.write(addr++,tempOpen);

  // сохраняем температуру закрытия
  EEPROM.write(addr++,tempClose);

  // сохраняем интервал работы
  const byte* readAddr = (const byte*) &openInterval;
  EEPROM.write(addr++,*readAddr++);
  EEPROM.write(addr++,*readAddr++);
  EEPROM.write(addr++,*readAddr++);
  EEPROM.write(addr++,*readAddr++);
  

  // сохраняем другие настройки!


  
}

