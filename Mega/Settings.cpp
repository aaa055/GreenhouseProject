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
  wateringOption = wateringOFF;
  wateringWeekDays = 0;
  wateringTime = 0;
  startWateringTime = 12;
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

  // читаем номер телефона для управления по СМС
  uint8_t smsnumlen = EEPROM.read(readPtr++);
  if(smsnumlen != 0xFF) // есть номер телефона
  {
    for(uint8_t i=0;i<smsnumlen;i++)
      smsPhoneNumber += (char) EEPROM.read(readPtr++);
  }

  // читаем установку контроля за поливом
  uint8_t bOpt = EEPROM.read(readPtr++);
  if(bOpt != 0xFF) // есть настройка контроля за поливом
  {
    wateringOption = (WateringOption) bOpt;
  } // if
  
 // читаем установку дней недели полива
  bOpt = EEPROM.read(readPtr++);
  if(bOpt != 0xFF) // есть настройка дней недели
  {
    wateringWeekDays = bOpt;
  } // if

  // читаем время полива
  bOpt = EEPROM.read(readPtr);
  if(bOpt != 0xFF) // есть настройка длительности полива
  {
    // читаем длительность полива
    wrAddr = (byte*) &wateringTime;
    *wrAddr++ = EEPROM.read(readPtr++);
    *wrAddr++ = EEPROM.read(readPtr++);
  }

  // читаем время начала полива
  bOpt = EEPROM.read(readPtr++);
  if(bOpt != 0xFF) // есть время начала полива
  {
    startWateringTime = bOpt;
  } // if
  

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

  // сохраняем номер телефона для управления по смс
  uint8_t smsnumlen = smsPhoneNumber.length();
  EEPROM.write(addr++,smsnumlen);
  
  const char* sms_c = smsPhoneNumber.c_str();
  for(uint8_t i=0;i<smsnumlen;i++)
  {
    EEPROM.write(addr++, *sms_c++);
  }

  // сохраняем опцию контроля за поливом
  EEPROM.write(addr++,wateringOption);
  
  // сохраняем дни недели для полива
  EEPROM.write(addr++,wateringWeekDays);

  // сохраняем продолжительность полива
  readAddr = (const byte*) &wateringTime;
  EEPROM.write(addr++,*readAddr++);
  EEPROM.write(addr++,*readAddr++);

  // сохраняем время начала полива
   EEPROM.write(addr++,startWateringTime);
 
  
  // сохраняем другие настройки!


  
}

