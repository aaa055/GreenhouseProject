#include "Settings.h"
#include "Globals.h"
#include <EEPROM.h> 

//  ГЛОБАЛЬНЫЕ НАСТРОЙКИ
GlobalSettings::GlobalSettings()
{
  ResetToDefault();
}
void GlobalSettings::WriteDeltaSettings(DeltaCountFunction OnDeltaGetCount, DeltaReadWriteFunction OnDeltaWrite)
{
  if(!(OnDeltaGetCount && OnDeltaWrite)) // обработчики не заданы
    return;

  uint16_t writeAddr = DELTA_SETTINGS_EEPROM_ADDR;

  // записываем заголовок
  EEPROM.write(writeAddr++,SETT_HEADER1);
  EEPROM.write(writeAddr++,SETT_HEADER2);
  

  uint8_t deltaCount = 0;

  // получаем кол-во дельт
  OnDeltaGetCount(deltaCount);

  // записываем кол-во дельт
  EEPROM.write(writeAddr++,deltaCount);

  //теперь пишем дельты
  for(uint8_t i=0;i<deltaCount;i++)
  {
    String name1,name2;
    uint8_t sensorType = 0,sensorIdx1 = 0,sensorIdx2 = 0;

    // получаем настройки дельт
    OnDeltaWrite(sensorType,name1,sensorIdx1,name2,sensorIdx2);

    // получили, можем сохранять. Каждая запись дельт идёт так:
  
  // 1 байт - тип датчика (температура, влажность, освещенность)
  
  // 1 байт - длина имени модуля 1
  // N байт - имя модуля 1
  // 1 байт - индекс датчика модуля 1
  
  // 1 байт - длина имени модуля 2
  // N байт - имя модуля 2
  // 1 байт - индекс датчика модуля 1

    // пишем тип датчика
     EEPROM.write(writeAddr++,sensorType);

     // пишем длину имени модуля 1
     uint8_t nameLen = name1.length();
     EEPROM.write(writeAddr++,nameLen);

     // пишем имя модуля 1
     const char* namePtr = name1.c_str();
     for(uint8_t idx=0;idx<nameLen; idx++)
      EEPROM.write(writeAddr++,*namePtr++);

     // пишем индекс датчика 1
     EEPROM.write(writeAddr++,sensorIdx1);


     // пишем длину имени модуля 2
     nameLen = name2.length();
     EEPROM.write(writeAddr++,nameLen);

     // пишем имя модуля 2
     namePtr = name2.c_str();
     for(uint8_t idx=0;idx<nameLen; idx++)
      EEPROM.write(writeAddr++,*namePtr++);

     // пишем индекс датчика 2
     EEPROM.write(writeAddr++,sensorIdx2);
     
    
  } // for

  // записали, отдыхаем
    
  
}
void GlobalSettings::ReadDeltaSettings(DeltaCountFunction OnDeltaSetCount, DeltaReadWriteFunction OnDeltaRead)
{
  if(!(OnDeltaSetCount && OnDeltaRead)) // обработчики не заданы
    return;

  uint16_t readAddr = DELTA_SETTINGS_EEPROM_ADDR;
  uint8_t h1,h2;
  
  h1 = EEPROM.read(readAddr++);
  h2 = EEPROM.read(readAddr++);

  uint8_t deltaCount = 0;

  if(!(h1 == SETT_HEADER1 && h2 == SETT_HEADER2)) // в памяти нет данных о сохранённых настройках дельт
  {
    
    OnDeltaSetCount(deltaCount); // сообщаем, что мы прочитали 0 настроек
    return; // и выходим
  }

  // читаем кол-во настроек
  deltaCount = EEPROM.read(readAddr++);
  if(deltaCount == 0xFF) // ничего нет
    deltaCount = 0; // сбрасываем в ноль
    
  OnDeltaSetCount(deltaCount); // сообщаем, что мы прочитали N настроек

  // читаем настройки дельт. В памяти каждая запись дельт идёт так:
  
  // 1 байт - тип датчика (температура, влажность, освещенность)
  
  // 1 байт - длина имени модуля 1
  // N байт - имя модуля 1
  // 1 байт - индекс датчика модуля 1
  
  // 1 байт - длина имени модуля 2
  // N байт - имя модуля 2
  // 1 байт - индекс датчика модуля 1
  
  // теперь читаем настройки
  for(uint8_t i=0;i<deltaCount;i++)
  {
    // читаем тип датчика
    uint8_t sensorType = EEPROM.read(readAddr++);

    // читаем длину имени модуля 1
    uint8_t nameLen = EEPROM.read(readAddr++);
    
    // резервируем память
    String name1; name1.reserve(nameLen + 1);
    
    // читаем имя модуля 1
    for(uint8_t idx = 0; idx < nameLen; idx++)
      name1 += (char) EEPROM.read(readAddr++);

    // читаем индекс датчика модуля 1
    uint8_t sensorIdx1 = EEPROM.read(readAddr++);

    // читаем длину имени модуля 2 
    nameLen = EEPROM.read(readAddr++);
    
    // резервируем память
    String name2; name2.reserve(nameLen + 1);

    // читаем имя модуля 2
    for(uint8_t idx = 0; idx < nameLen; idx++)
      name2 += (char) EEPROM.read(readAddr++);

    // читаем индекс датчика модуля 2
    uint8_t sensorIdx2 = EEPROM.read(readAddr++);

    // всё прочитали - можем вызывать функцию, нам переданную
    OnDeltaRead(sensorType,name1,sensorIdx1,name2,sensorIdx2);
    
  } // for

  // всё прочитали, отдыхаем
  
    
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
  wifiState = 0x01; // первый бит устанавливаем, говорим, что мы коннектимся к роутеру
  controllerID = 0; // по умолчанию 0 как ID контроллера
}
void GlobalSettings::SetControllerID(uint8_t val)
{
  controllerID = val;
  EEPROM.write(CONTROLLER_ID_EEPROM_ADDR,controllerID);
}
void GlobalSettings::Load()
{  
  uint16_t readPtr = 0; // сбрасываем указатель чтения на начало памяти

  // читаем ID контроллера
  uint8_t cid = EEPROM.read(CONTROLLER_ID_EEPROM_ADDR);
  if(cid != 0xFF)
    controllerID = cid;

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
  *wrAddr = EEPROM.read(readPtr++);

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
    wateringOption = bOpt;
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
    *wrAddr = EEPROM.read(readPtr++);
  }

  // читаем время начала полива
  bOpt = EEPROM.read(readPtr++);
  if(bOpt != 0xFF) // есть время начала полива
  {
    startWateringTime = bOpt;
  } // if
  
 // читаем , включать ли насос во время полива?
  bOpt = EEPROM.read(readPtr++);
  if(bOpt != 0xFF) // есть настройка включение насоса
  {
    turnOnPump = bOpt;
  } // if

  // читаем сохранённое кол-во настроек каналов полива
  bOpt = EEPROM.read(readPtr++);
  uint8_t addToAddr = 0; // сколько пропустить при чтении каналов, чтобы нормально прочитать следующую настройку.
  // нужно, если сначала скомпилировали с 8 каналами, сохранили настройки из конфигуратора, а потом - перекомпилировали
  // в 2 канала. Нам надо вычитать первые два, а остальные 6 - пропустить, чтобы не покалечить настройки.
  
  if(bOpt != 0xFF)
  {
    // есть сохранённое кол-во каналов, читаем каналы
    if(bOpt > WATER_RELAYS_COUNT) // только сначала убедимся, что мы не вылезем за границы массива
    {
      addToAddr = (bOpt - WATER_RELAYS_COUNT)*4; // 4 байта в каждой структуре настроек
      bOpt = WATER_RELAYS_COUNT;
    }

    // теперь мы можем читать настройки каналов
    uint16_t wTimeHelper = 0;
    
    for(uint8_t i=0;i<bOpt;i++)
    {
      wateringChannelsOptions[i].wateringWeekDays = EEPROM.read(readPtr++);
      
      wrAddr = (byte*) &wTimeHelper;
      *wrAddr++ = EEPROM.read(readPtr++);
      *wrAddr = EEPROM.read(readPtr++);
      wateringChannelsOptions[i].wateringTime = wTimeHelper;
      
      wateringChannelsOptions[i].startWateringTime = EEPROM.read(readPtr++);
    } // for
    
  } // if(bOpt != 0xFF)

    // переходим на следующую настройку
     readPtr += addToAddr;

   wifiState = EEPROM.read(readPtr++);
   if(wifiState != 0xFF) // есть сохраненные настройки Wi-Fi
   {
        // читаем ID точки доступа
        routerID = F("");
         uint8_t str_len = EEPROM.read(readPtr++);
          for(uint8_t i=0;i<str_len;i++)
            routerID += (char) EEPROM.read(readPtr++);

        // читаем пароль к точке доступа
        routerPassword = F("");
         str_len = EEPROM.read(readPtr++);
          for(uint8_t i=0;i<str_len;i++)
            routerPassword += (char) EEPROM.read(readPtr++);

        // читаем название нашей точки доступа
        stationID = F("");
         str_len = EEPROM.read(readPtr++);
          for(uint8_t i=0;i<str_len;i++)
            stationID += (char) EEPROM.read(readPtr++);

        // читаем пароль к нашей точке доступа
        stationPassword = F("");
         str_len = EEPROM.read(readPtr++);
          for(uint8_t i=0;i<str_len;i++)
            stationPassword += (char) EEPROM.read(readPtr++);
  }
   else
   {
      wifiState = 0x01;
      // применяем настройки по умолчанию
      routerID = ROUTER_ID;
      routerPassword = ROUTER_PASSWORD;
      stationID = STATION_ID;
      stationPassword = STATION_PASSWORD;

      if(!routerID.length()) // если нет названия точки доступа роутера - не коннектимся к нему
        wifiState = 0; 
   }

   
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
  EEPROM.write(addr++,*readAddr);

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
  EEPROM.write(addr++,*readAddr);

  // сохраняем время начала полива
   EEPROM.write(addr++,startWateringTime);
 
  // сохраняем опцию включения насоса при поливе
   EEPROM.write(addr++,turnOnPump);

   // сохраняем кол-во каналов полива
   EEPROM.write(addr++,WATER_RELAYS_COUNT);

   // пишем настройки каналов полива
   #if WATER_RELAYS_COUNT > 0
   for(uint8_t i=0;i<WATER_RELAYS_COUNT;i++)
   {
      EEPROM.write(addr++,wateringChannelsOptions[i].wateringWeekDays);
      readAddr = (const byte*) &(wateringChannelsOptions[i].wateringTime);
      EEPROM.write(addr++,*readAddr++);
      EEPROM.write(addr++,*readAddr);
      EEPROM.write(addr++,wateringChannelsOptions[i].startWateringTime);
   } // for
   #endif

 // сохраняем настройки Wi-Fi
 EEPROM.write(addr++,wifiState);

// сохраняем ID роутера
  uint8_t str_len = routerID.length();
  EEPROM.write(addr++,str_len);
  
  const char* str_p = routerID.c_str();
  for(uint8_t i=0;i<str_len;i++)
    EEPROM.write(addr++, *str_p++);


 // сохраняем пароль к роутеру
  str_len = routerPassword.length();
  EEPROM.write(addr++,str_len);
  
  str_p = routerPassword.c_str();
  for(uint8_t i=0;i<str_len;i++)
    EEPROM.write(addr++, *str_p++);

 // сохраняем название нашей точки доступа
  str_len = stationID.length();
  EEPROM.write(addr++,str_len);
  
  str_p = stationID.c_str();
  for(uint8_t i=0;i<str_len;i++)
    EEPROM.write(addr++, *str_p++);

 // сохраняем пароль к нашей точке доступа
  str_len = stationPassword.length();
  EEPROM.write(addr++,str_len);
  
  str_p = stationPassword.c_str();
  for(uint8_t i=0;i<str_len;i++)
    EEPROM.write(addr++, *str_p++);
  
  // сохраняем другие настройки!


  
}

