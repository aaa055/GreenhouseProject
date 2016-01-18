#include "Settings.h"
#include "Globals.h"
#include <EEPROM.h> 

//  ГЛОБАЛЬНЫЕ НАСТРОЙКИ
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
  uint16_t readPtr = 0; // сбрасываем указатель чтения на начало памяти

  // читаем заголовок
  uint8_t h1,h2;
  h1 = EEPROM.read(readPtr++);
  h2 = EEPROM.read(readPtr++);

  if(!(h1 == SETT_HEADER1 && h2 == SETT_HEADER2)) // ничего нет в памяти
  {
    ResetToDefault(); // применяем настройки по умолчанию
    Save(); // сохраняем их
    return; // и выходим
  }
  
  // читаем температуру открытия
  tempOpen = EEPROM.read(readPtr++);

  // читаем температуру закрытия
  tempClose = EEPROM.read(readPtr++);

  // читаем интервал работы окон
   byte* wrAddr = (byte*) &openInterval;
  
  *wrAddr++ = EEPROM.read(readPtr++);
  *wrAddr++ = EEPROM.read(readPtr++);
  *wrAddr++ = EEPROM.read(readPtr++);
  *wrAddr++ = EEPROM.read(readPtr++);

  

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

