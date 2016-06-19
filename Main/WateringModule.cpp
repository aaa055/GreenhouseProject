#include "WateringModule.h"
#include "ModuleController.h"
#include <EEPROM.h>
#ifdef USE_LOG_MODULE
#include <SD.h> // пробуем записать статус полива не только в EEPROM, но и на SD-карту, если LOG-модуль есть в прошивке
#endif

#if WATER_RELAYS_COUNT > 0
static uint8_t WATER_RELAYS[] = { WATER_RELAYS_PINS }; // объявляем массив пинов реле
#endif

void WateringModule::Setup()
{
  // настройка модуля тут

  settings = MainController->GetSettings();
  
   #ifdef USE_DS3231_REALTIME_CLOCK
    bIsRTClockPresent = true; // есть часы реального времени
    DS3231Clock watch =  MainController->GetClock();
    DS3231Time t =   watch.getTime();
  #else
    bIsRTClockPresent = false; // нет часов реального времени
  #endif 

#ifdef USE_WATERING_MANUAL_MODE_DIODE
  blinker.begin(DIODE_WATERING_MANUAL_MODE_PIN, F("WM"));  // настраиваем блинкер на нужный пин
#endif

  workMode = wwmAutomatic; // автоматический режим работы

  #if WATER_RELAYS_COUNT > 0
  internalNeedChange = false;
  dummyAllChannels.WateringTimer = 0; // обнуляем таймер полива для всех каналов
  dummyAllChannels.WateringDelta = 0;
  dummyAllChannels.SetRelayOn(false); // все реле выключены
  #endif

  #ifdef USE_DS3231_REALTIME_CLOCK

  
    // смотрим, не поливали ли мы на всех каналах сегодня
    uint8_t today = t.dayOfWeek;
    unsigned long savedWorkTime = 0xFFFFFFFF;
    byte* writeAddr = (byte*) &savedWorkTime;
    uint8_t savedDOW = 0xFF;
    uint16_t curReadAddr = WATERING_STATUS_EEPROM_ADDR;

    bool needToReadFromEEPROM = true; // считаем, что мы должны читать из EEPROM


    if(MainController->HasSDCard())
    {

      char file_name[13] = {0};
      sprintf_P(file_name,(const char*)F("%u.WTR"),0);

      SDFile sdFile = SD.open(file_name,FILE_READ);
      if(sdFile)
      {
        if(sdFile.size() == sizeof(unsigned long) + sizeof(uint8_t))
        {
            // нормальный размер файла, можем читать
            sdFile.read(&savedDOW,sizeof(uint8_t));
            sdFile.read(&savedWorkTime,sizeof(unsigned long));

            needToReadFromEEPROM = false; // прочитали настройки из файла
        
        } // if
        
        sdFile.close();
        
      } // if(sdFile)

    } // if(MainController->HasSDCard())


    if(needToReadFromEEPROM)
    {
      savedDOW = EEPROM.read(curReadAddr++);
  
      *writeAddr++ = EEPROM.read(curReadAddr++);
      *writeAddr++ = EEPROM.read(curReadAddr++);
      *writeAddr++ = EEPROM.read(curReadAddr++);
      *writeAddr = EEPROM.read(curReadAddr++);
      
    } // needToReadFromEEPROM

   
    
    if(savedDOW != 0xFF && savedWorkTime != 0xFFFFFFFF) // есть сохранённое время работы всех каналов на сегодня
    {
      if(savedDOW == today) // поливали на всех каналах сегодня, выставляем таймер канала так, как будто он уже поливался сколько-то времени
      {
        #if WATER_RELAYS_COUNT > 0
        dummyAllChannels.WateringTimer = savedWorkTime + 1;
        #endif
      }
      
    } // if

  lastDOW = t.dayOfWeek; // запоминаем прошлый день недели
  currentDOW = t.dayOfWeek; // запоминаем текущий день недели
  currentHour = t.hour; // запоминаем текущий час
  
  #else

  // нет часов реального времени в прошивке
  lastDOW = 0; // запоминаем прошлый день недели
  currentDOW = 0; // запоминаем текущий день недели
  currentHour = 0; // запоминаем текущий час
    
  #endif

  lastAnyChannelActiveFlag = -1; // ещё не собирали активность каналов
    
  // выключаем все реле
  #if WATER_RELAYS_COUNT > 0
  for(uint8_t i=0;i<WATER_RELAYS_COUNT;i++)
  {
    pinMode(WATER_RELAYS[i],OUTPUT);
    digitalWrite(WATER_RELAYS[i],RELAY_OFF);

    // настраиваем все каналы
    wateringChannels[i].SetRelayOn(false);
    wateringChannels[i].WateringTimer = 0;
    wateringChannels[i].WateringDelta = 0;

    #ifdef USE_DS3231_REALTIME_CLOCK
    
      // смотрим, не поливался ли уже канал сегодня?
      savedWorkTime = 0xFFFFFFFF;
      savedDOW = 0xFF;
      needToReadFromEEPROM = true;

    if(MainController->HasSDCard())
    {
      char file_name[13] = {0};
      sprintf_P(file_name,(const char*)F("%u.WTR"),(i+1));

      SDFile sdFile = SD.open(file_name,FILE_READ);
      if(sdFile)
      {
        if(sdFile.size() == sizeof(unsigned long) + sizeof(uint8_t))
        {
            // нормальный размер файла, можем читать
            sdFile.read(&savedDOW,sizeof(uint8_t));
            sdFile.read(&savedWorkTime,sizeof(unsigned long));
            needToReadFromEEPROM = false; // прочитали настройки из файла
        
        } // if
        
        sdFile.close();
        
      } // if(sdFile)
    } // if(MainController->HasSDCard())
      
      if(needToReadFromEEPROM)
      {
          curReadAddr = WATERING_STATUS_EEPROM_ADDR + (i+1)*5;
          savedDOW = EEPROM.read(curReadAddr++);
    
          writeAddr = (byte*) &savedWorkTime;
         *writeAddr++ = EEPROM.read(curReadAddr++);
         *writeAddr++ = EEPROM.read(curReadAddr++);
         *writeAddr++ = EEPROM.read(curReadAddr++);
         *writeAddr = EEPROM.read(curReadAddr++);
         
      } // needToReadFromEEPROM

     
      if(savedDOW != 0xFF && savedWorkTime != 0xFFFFFFFF )
      {
        if(savedDOW == today) // поливали на канале в этот день недели, выставляем таймер канала так, как будто он уже поливался какое-то время
          wateringChannels[i].WateringTimer = savedWorkTime + 1;
      }
    #endif
  } // for
  #endif // #if WATER_RELAYS_COUNT > 0

#ifdef USE_PUMP_RELAY
  // выключаем реле насоса  
  pinMode(PUMP_RELAY_PIN,OUTPUT);
  digitalWrite(PUMP_RELAY_PIN,RELAY_OFF);
  bPumpIsOn = false;
#endif

    // настраиваем режим работы перед стартом
    uint8_t currentWateringOption = settings->GetWateringOption();
    
    if(currentWateringOption == wateringOFF) // если выключено автоуправление поливом
    {
      workMode = wwmManual; // переходим в ручной режим работы
      #ifdef USE_WATERING_MANUAL_MODE_DIODE
      blinker.blink(WORK_MODE_BLINK_INTERVAL); // зажигаем диод
      #endif
    }
    else
    {
      workMode = wwmAutomatic; // иначе переходим в автоматический режим работы
      #ifdef USE_WATERING_MANUAL_MODE_DIODE
      blinker.blink(); // гасим диод
      #endif
    }
      

}
#if WATER_RELAYS_COUNT > 0
void WateringModule::UpdateChannel(int8_t channelIdx, WateringChannel* channel, uint16_t dt)
{
  
   if(!bIsRTClockPresent)
   {
     // в системе нет модуля часов, в таких условиях мы можем работать только в ручном режиме.
     // поэтому в этой ситуации мы ничего не предпринимаем, поскольку автоматически деградируем
     // в ручной режим работы.
     return;
   }
   
     uint8_t weekDays = channelIdx == -1 ? settings->GetWateringWeekDays() : settings->GetChannelWateringWeekDays(channelIdx);
     uint8_t startWateringTime = channelIdx == -1 ? settings->GetStartWateringTime() : settings->GetChannelStartWateringTime(channelIdx);
     uint16_t timeToWatering = channelIdx == -1 ? settings->GetWateringTime() : settings->GetChannelWateringTime(channelIdx); // время полива (в минутах!)


      // переход через день недели мы фиксируем однократно, поэтому нам важно его не пропустить.
      // можем ли мы работать или нет - неважно, главное - правильно обработать переход через день недели.
      
      if(lastDOW != currentDOW)  // сначала проверяем, не другой ли день недели уже?
      {
        // начался другой день недели. Для одного дня недели у нас установлена
        // продолжительность полива, поэтому, если мы поливали 28 минут вместо 30, например, во вторник, и перешли на среду,
        // то в среду надо полить ещё 2 мин. Поэтому таймер полива переводим в нужный режим:
        // оставляем в нём недополитое время, чтобы учесть, что поливать надо, например, ещё 2 минуты.

         channel->WateringDelta = 0; // обнуляем дельту дополива, т.к. мы в этот день можем и не работать

        if(bitRead(weekDays,currentDOW-1)) // можем работать в этот день недели, значит, надо скорректировать значение таймера
        {
          // вычисляем разницу между полным и отработанным временем
            unsigned long wateringDelta = ((timeToWatering*60000) - channel->WateringTimer);
            // запоминаем для канала дополнительную дельту для работы
            channel->WateringDelta = wateringDelta;
        }

        channel->WateringTimer = 0; // сбрасываем таймер полива, т.к. начался новый день недели
        
      } // if(lastDOW != currentDOW)      


    // проверяем, установлен ли у нас день недели для полива, и настал ли час, с которого можно поливать
    bool canWork = bitRead(weekDays,currentDOW-1) && (currentHour >= startWateringTime);
  
    if(!canWork)
     { 
      //TODO: Закомментировал для теста, потому что меня смущает это обнуление!!!          
      // channel->WateringTimer = 0; // в этот день недели и в этот час работать не можем, однозначно обнуляем таймер полива    
       channel->SetRelayOn(false); // выключаем реле
     }
     else
     {
      // можем работать, смотрим, не вышли ли мы за пределы установленного интервала

      channel->WateringTimer += dt; // прибавляем время работы
  
      // проверяем, можем ли мы ещё работать
      // если полив уже отработал, и юзер прибавит минуту - мы должны поливать ещё минуту,
      // вне зависимости от показания таймера. Поэтому мы при срабатывании условия окончания полива
      // просто отнимаем дельту времени из таймера, таким образом оставляя его застывшим по времени
      // окончания полива
  
      if(channel->WateringTimer > ((timeToWatering*60000) + channel->WateringDelta + dt)) // приплыли, надо выключать полив
      {
        channel->WateringTimer -= (dt + channel->WateringDelta);// оставляем таймер застывшим на окончании полива, плюс маленькая дельта
        channel->WateringDelta = 0; // сбросили дельту дополива

        if(channel->IsChannelRelayOn()) // если канал был включён, значит, он будет выключен, и мы однократно запишем в EEPROM нужное значение
        {
          
          //Тут сохранение в EEPROM статуса, что мы на сегодня уже полили сколько-то времени
          uint16_t wrAddr = WATERING_STATUS_EEPROM_ADDR + (channelIdx+1)*5; // channelIdx == -1 для всех каналов, поэтому прибавляем единичку
          // сохраняем в EEPROM день недели, для которого запомнили значение таймера
          EEPROM.write(wrAddr++,currentDOW);
          
          // сохраняем в EEPROM значение таймера канала
          unsigned long ttw = timeToWatering*60000; // запишем полное время полива на сегодня
          byte* readAddr = (byte*) &ttw;
          for(int i=0;i<4;i++)
            EEPROM.write(wrAddr++,*readAddr++);


         // теперь пишем в файл для дублирования, чтобы не потерять настройки при слетании EEPROM
          if(MainController->HasSDCard())
          {
            char file_name[13] = {0};
            sprintf_P(file_name,(const char*)F("%u.WTR"),(channelIdx+1));
      
            SDFile sdFile = SD.open(file_name,FILE_WRITE | O_TRUNC);
            if(sdFile)
            {              
              sdFile.write(&currentDOW,sizeof(uint8_t));
              sdFile.write((const uint8_t*) &ttw,sizeof(unsigned long));
      
              sdFile.close();
               
            } // if(sdFile)

          } // if(MainController->HasSDCard())
      
            
        } // if(channel->IsChannelRelayOn())

        channel->SetRelayOn(false); // выключаем реле
      
      }
      else
        channel->SetRelayOn(true); // ещё можем работать, продолжаем поливать
     } // else
     
}
void WateringModule::HoldChannelState(int8_t channelIdx, WateringChannel* channel)
{
    uint8_t state = channel->IsChannelRelayOn() ? RELAY_ON : RELAY_OFF;


    if(channelIdx == -1) // работаем со всеми каналами, пишем в пин только тогда, когда состояние реле поменялось
    {
      if(channel->IsChanged() || internalNeedChange)
        for(uint8_t i=0;i<WATER_RELAYS_COUNT;i++)
        {
          digitalWrite(WATER_RELAYS[i],state);      
        } // for
        
      return;
    } // if

    // работаем с одним каналом, пишем в пин только тогда, когда состояние реле поменялось
    
    if(channel->IsChanged() || internalNeedChange)
      digitalWrite(WATER_RELAYS[channelIdx],state);
  
}

bool WateringModule::IsAnyChannelActive(uint8_t wateringOption)
{  
   if(workMode == wwmManual) // в ручном режиме мы управляем только всеми каналами сразу
    return dummyAllChannels.IsChannelRelayOn(); // поэтому смотрим состояние реле на всех каналах

    // в автоматическом режиме мы можем рулить как всеми каналами вместе (wateringOption == wateringWeekDays),
    // так и по отдельности (wateringOption == wateringSeparateChannels). В этом случае надо выяснить, состояние каких каналов
    // смотреть, чтобы понять - активен ли кто-то.

    if(wateringOption == wateringWeekDays)
      return dummyAllChannels.IsChannelRelayOn(); // смотрим состояние реле на всех каналах

    // тут мы рулим всеми каналами по отдельности, поэтому надо проверить - включено ли реле на каком-нибудь из каналов
    for(uint8_t i=0;i<WATER_RELAYS_COUNT;i++)
    {
      if(wateringChannels[i].IsChannelRelayOn())
        return true;
    }

    return false;
}
#endif // #if WATER_RELAYS_COUNT > 0

#ifdef USE_PUMP_RELAY
void WateringModule::HoldPumpState(bool anyChannelActive)
{
  // поддерживаем состояние реле насоса
  if(settings->GetTurnOnPump() != 1) // не надо включать насос
  {
    if(bPumpIsOn) // если был включен - выключаем
    {
      bPumpIsOn = false;
      digitalWrite(PUMP_RELAY_PIN,RELAY_OFF);
    }
    return; // и не будем ничего больше делать
  }
    if(bPumpIsOn != anyChannelActive) // состояние изменилось, пишем в пин только при смене состояния
    {
      bPumpIsOn = anyChannelActive;

     // пишем в реле насоса вкл или выкл в зависимости от настройки "включать насос при поливе"
      digitalWrite(PUMP_RELAY_PIN,bPumpIsOn ? RELAY_ON : RELAY_OFF);
    } 
}
#endif

void WateringModule::Update(uint16_t dt)
{ 
#ifdef USE_WATERING_MANUAL_MODE_DIODE
   blinker.update();
#endif

#if WATER_RELAYS_COUNT > 0
  
uint8_t wateringOption = settings->GetWateringOption(); // получаем опцию управления поливом
bool anyChActive = IsAnyChannelActive(wateringOption);

SAVE_STATUS(WATER_STATUS_BIT, anyChActive ? 1 : 0); // сохраняем состояние полива
SAVE_STATUS(WATER_MODE_BIT,workMode == wwmAutomatic ? 1 : 0); // сохраняем режим работы полива


#ifdef USE_PUMP_RELAY
  // держим состояние реле для насоса
  HoldPumpState(anyChActive);
#endif

  #ifdef USE_DS3231_REALTIME_CLOCK

    // обновляем состояние часов
    DS3231Clock watch =  MainController->GetClock();
    DS3231Time t =   watch.getTime();


    if(currentDOW != t.dayOfWeek)
    {
      // начался новый день недели, принудительно переходим в автоматический режим работы
      // даже если до этого был включен полив командой от пользователя
      workMode = wwmAutomatic;

      //Тут затирание в EEPROM предыдущего сохранённого значения о статусе полива на всех каналах
      uint16_t wrAddr = WATERING_STATUS_EEPROM_ADDR;
      uint8_t bytes_to_write = 5 + WATER_RELAYS_COUNT*5;
      for(uint8_t i=0;i<bytes_to_write;i++)
        EEPROM.write(wrAddr++,0); // для каждого канала по отдельности
    }

    currentDOW = t.dayOfWeek; // сохраняем текущий день недели
    currentHour = t.hour; // сохраняем текущий час
       
  #else

    // модуль часов реального времени не включен в компиляцию, деградируем до ручного режима работы
    settings->SetWateringOption(wateringOFF); // отключим автоматический контроль полива
    workMode = wwmManual; // переходим на ручное управление
    #ifdef USE_WATERING_MANUAL_MODE_DIODE
    blinker.blink(WORK_MODE_BLINK_INTERVAL); // зажигаем диод
    #endif
 
  #endif
  
  if(workMode == wwmAutomatic)
  {
    // автоматический режим работы
 
    // проверяем текущий режим управления каналами полива
    switch(wateringOption)
    {
      case wateringOFF: // автоматическое управление поливом выключено, значит, мы должны перейти в ручной режим работы
          workMode = wwmManual; // переходим в ручной режим работы
          #ifdef USE_WATERING_MANUAL_MODE_DIODE
          blinker.blink(WORK_MODE_BLINK_INTERVAL); // зажигаем диод
          #endif
      break;

      case wateringWeekDays: // // управление поливом по дням недели (все каналы одновременно)
      {
          // обновляем состояние всех каналов - канал станет активным или неактивным после этой проверки
           UpdateChannel(-1,&dummyAllChannels,dt);
           
           // теперь держим текущее состояние реле на всех каналах
           HoldChannelState(-1,&dummyAllChannels);
      }
      break;

      case wateringSeparateChannels: // рулим всеми каналами по отдельности
      {
        dummyAllChannels.SetRelayOn(false); // выключаем реле на всех каналах
        
        for(uint8_t i=0;i<WATER_RELAYS_COUNT;i++)
        {
          UpdateChannel(i,&(wateringChannels[i]),dt); // обновляем канал
          HoldChannelState(i,&(wateringChannels[i]));  // держим его состояние
        } // for
      }
      break;
      
    } // switch(wateringOption)
  }
  else
  {
    // ручной режим работы, просто сохраняем переданный нам статус реле, все каналы - одновременно.
    // обновлять состояние канала не надо, потому что мы в ручном режиме работы.
      HoldChannelState(-1,&dummyAllChannels);
          
  } // else

  // проверяем, есть ли изменения с момента последнего вызова
  if(lastAnyChannelActiveFlag < 0)
  {
    // ещё не собирали статус, собираем первый раз
    lastAnyChannelActiveFlag = IsAnyChannelActive(wateringOption) ? 1 : 0;

    if(lastAnyChannelActiveFlag)
    {
      // если любой канал активен - значит, полив включили, а по умолчанию он выключен.
      // значит, надо записать в лог
      String mess = lastAnyChannelActiveFlag? STATE_ON : STATE_OFF;
      MainController->Log(this,mess);
    }
  }
  else
  {
    // уже собирали, надо проверить с текущим состоянием
    byte nowAnyChannelActive = IsAnyChannelActive(wateringOption) ? 1 : 0;
    
    if(nowAnyChannelActive != lastAnyChannelActiveFlag)
    {
      lastAnyChannelActiveFlag = nowAnyChannelActive; // сохраняем последний статус, чтобы не дёргать запись в лог лишний раз
      // состояние каналов изменилось, пишем в лог
      String mess = lastAnyChannelActiveFlag ? STATE_ON : STATE_OFF;
      MainController->Log(this,mess);
    }
  } // else

  // обновили все каналы, теперь можно сбросить флаг перехода через день недели
  lastDOW = currentDOW; // сделаем вид, что мы ничего не знаем о переходе на новый день недели.
  // таким образом, код перехода на новый день недели выполнится всего один раз при каждом переходе
  // через день недели.

  internalNeedChange = false;

#else
UNUSED(dt);
#endif
  
}
bool  WateringModule::ExecCommand(const Command& command, bool wantAnswer)
{
  UNUSED(wantAnswer);
  PublishSingleton = UNKNOWN_COMMAND;

  size_t argsCount = command.GetArgsCount();
    
  if(command.GetType() == ctSET) 
  {   
      if(argsCount < 1) // не хватает параметров
      {
        PublishSingleton = PARAMS_MISSED;
      }
      else
      {
        String which = command.GetArg(0);
        which.toUpperCase();

        if(which == WATER_SETTINGS_COMMAND)
        {
          if(argsCount > 5)
          {
              // парсим параметры
              uint8_t wateringOption = (uint8_t) atoi(command.GetArg(1)); //String(command.GetArg(1)).toInt();
              uint8_t wateringWeekDays = (uint8_t) atoi(command.GetArg(2)); //String(command.GetArg(2)).toInt();
              uint16_t wateringTime = (uint16_t) atoi(command.GetArg(3)); //String(command.GetArg(3)).toInt();
              uint8_t startWateringTime = (uint8_t) atoi(command.GetArg(4)); //String(command.GetArg(4)).toInt();
              uint8_t turnOnPump = (uint8_t) atoi(command.GetArg(5)); //String(command.GetArg(5)).toInt();
      
              // пишем в настройки
              settings->SetWateringOption(wateringOption);
              settings->SetWateringWeekDays(wateringWeekDays);
              settings->SetWateringTime(wateringTime);
              settings->SetStartWateringTime(startWateringTime);
              settings->SetTurnOnPump(turnOnPump);
      
              // сохраняем настройки
              settings->Save();

              if(wateringOption == wateringOFF) // если выключено автоуправление поливом
              {
                workMode = wwmManual; // переходим в ручной режим работы
                #if WATER_RELAYS_COUNT > 0
                dummyAllChannels.SetRelayOn(false); // принудительно гасим полив на всех каналах
                #endif
                
                #ifdef USE_WATERING_MANUAL_MODE_DIODE
                blinker.blink(WORK_MODE_BLINK_INTERVAL); // зажигаем диод
                #endif
              }
              else
              {
                workMode = wwmAutomatic; // иначе переходим в автоматический режим работы
                #ifdef USE_WATERING_MANUAL_MODE_DIODE
                blinker.blink(); // гасим диод
                #endif
              }
      
              
              PublishSingleton.Status = true;
              PublishSingleton = WATER_SETTINGS_COMMAND; 
              PublishSingleton << PARAM_DELIMITER << REG_SUCC;
          } // argsCount > 3
          else
          {
            // не хватает команд
            PublishSingleton = PARAMS_MISSED;
          }
          
        } // WATER_SETTINGS_COMMAND
        else
        if(which == WATER_CHANNEL_SETTINGS) // настройки канала CTSET=WATER|CH_SETT|IDX|WateringDays|WateringTime|StartTime
        {
           if(argsCount > 4)
           {
              #if WATER_RELAYS_COUNT > 0
                uint8_t channelIdx = (uint8_t) atoi(command.GetArg(1));
                if(channelIdx < WATER_RELAYS_COUNT)
                {
                  // нормальный индекс
                  uint8_t wDays = (uint8_t) atoi(command.GetArg(2));
                  uint16_t wTime =(uint16_t) atoi(command.GetArg(3));
                  uint8_t sTime = (uint8_t) atoi(command.GetArg(4));
                  
                  settings->SetChannelWateringWeekDays(channelIdx,wDays);
                  settings->SetChannelWateringTime(channelIdx,wTime);
                  settings->SetChannelStartWateringTime(channelIdx,sTime);
                  
                  PublishSingleton.Status = true;
                  PublishSingleton = WATER_CHANNEL_SETTINGS; 
                  PublishSingleton << PARAM_DELIMITER << (command.GetArg(1)) << PARAM_DELIMITER << REG_SUCC;
                 
                }
                else
                {
                  // плохой индекс
                  PublishSingleton = UNKNOWN_COMMAND;
                }
             #else
              PublishSingleton = UNKNOWN_COMMAND;
             #endif
           }
           else
           {
            // не хватает команд
            PublishSingleton = PARAMS_MISSED;            
           }
        }
        else
        if(which == WORK_MODE) // CTSET=WATER|MODE|AUTO, CTSET=WATER|MODE|MANUAL
        {
           // попросили установить режим работы
           String param = command.GetArg(1);
           
           if(param == WM_AUTOMATIC)
           {
             workMode = wwmAutomatic; // переходим в автоматический режим работы
             internalNeedChange = true; // говорим, что надо перезаписать в пины реле
             
             #ifdef USE_WATERING_MANUAL_MODE_DIODE
             blinker.blink(); // гасим диод
             #endif
           }
           else
           {
            workMode = wwmManual; // переходим на ручной режим работы
            #ifdef USE_WATERING_MANUAL_MODE_DIODE
            blinker.blink(WORK_MODE_BLINK_INTERVAL); // зажигаем диод
            #endif
           }

              PublishSingleton.Status = true;
              PublishSingleton = WORK_MODE; 
              PublishSingleton << PARAM_DELIMITER << param;

              
        
        } // WORK_MODE
        else 
        if(which == STATE_ON) // попросили включить полив, CTSET=WATER|ON
        {
          if(!command.IsInternal()) // если команда от юзера, то
          {
            workMode = wwmManual; // переходим в ручной режим работы
            #ifdef USE_WATERING_MANUAL_MODE_DIODE
            blinker.blink(WORK_MODE_BLINK_INTERVAL); // зажигаем диод
            #endif
          }
          // если команда не от юзера, а от модуля ALERT, например, то
          // просто выставляя статус реле для всех каналов - мы ничего не добьёмся - 
          // команда проигнорируется, т.к. мы сами обновляем статус каналов.
          // в этом случае - надо переходить на ручное управление, мне кажется.
          // Хотя это - неправильно, должна быть возможность в автоматическом
          // режиме включать/выключать полив из модуля ALERT, без мигания диодом.

          #if WATER_RELAYS_COUNT > 0
          dummyAllChannels.SetRelayOn(true); // включаем реле на всех каналах
          #endif

          PublishSingleton.Status = true;
          PublishSingleton = STATE_ON;
        } // STATE_ON
        else 
        if(which == STATE_OFF) // попросили выключить полив, CTSET=WATER|OFF
        {
          if(!command.IsInternal()) // если команда от юзера, то
          {
            workMode = wwmManual; // переходим в ручной режим работы
            #ifdef USE_WATERING_MANUAL_MODE_DIODE
            blinker.blink(WORK_MODE_BLINK_INTERVAL); // зажигаем диод
            #endif
          }
          // если команда не от юзера, а от модуля ALERT, например, то
          // просто выставляя статус реле для всех каналов - мы ничего не добьёмся - 
          // команда проигнорируется, т.к. мы сами обновляем статус каналов.
          // в этом случае - надо переходить на ручное управление, мне кажется.
          // Хотя это - неправильно, должна быть возможность в автоматическом
          // режиме включать/выключать полив из модуля ALERT, без мигания диодом.
          
          #if WATER_RELAYS_COUNT > 0
          dummyAllChannels.SetRelayOn(false); // выключаем реле на всех каналах
          #endif

          PublishSingleton.Status = true;
          PublishSingleton = STATE_OFF;
          
        } // STATE_OFF        

      } // else
  }
  else
  if(command.GetType() == ctGET) //получить данные
  {    
    if(!argsCount) // нет аргументов, попросили вернуть статус полива
    {
      PublishSingleton.Status = true;
      #if WATER_RELAYS_COUNT > 0
      PublishSingleton = (IsAnyChannelActive(settings->GetWateringOption()) ? STATE_ON : STATE_OFF);
      #else
      PublishSingleton = STATE_OFF;
      #endif
      PublishSingleton << PARAM_DELIMITER << (workMode == wwmAutomatic ? WM_AUTOMATIC : WM_MANUAL);
    }
    else
    {
      String t = command.GetArg(0);
      
        if(t == WATER_SETTINGS_COMMAND) // запросили данные о настройках полива
        {
          PublishSingleton.Status = true;
          PublishSingleton = WATER_SETTINGS_COMMAND; 
          PublishSingleton << PARAM_DELIMITER; 
          PublishSingleton << (settings->GetWateringOption()) << PARAM_DELIMITER;
          PublishSingleton << (settings->GetWateringWeekDays()) << PARAM_DELIMITER;
          PublishSingleton << (settings->GetWateringTime()) << PARAM_DELIMITER;
          PublishSingleton << (settings->GetStartWateringTime()) << PARAM_DELIMITER;
          PublishSingleton << (settings->GetTurnOnPump());
        }
        else
        if(t == WATER_CHANNELS_COUNT_COMMAND)
        {
          PublishSingleton.Status = true;
          PublishSingleton = WATER_CHANNELS_COUNT_COMMAND; 
          PublishSingleton << PARAM_DELIMITER << WATER_RELAYS_COUNT;
          
        }
        else
        if(t == WORK_MODE) // получить режим работы
        {
          PublishSingleton.Status = true;
          PublishSingleton = WORK_MODE; 
          PublishSingleton << PARAM_DELIMITER << (workMode == wwmAutomatic ? WM_AUTOMATIC : WM_MANUAL);
        }
        else
        {
           // команда с аргументами
           if(argsCount > 1)
           {
                t = command.GetArg(0);
    
                if(t == WATER_CHANNEL_SETTINGS)
                {
                  #if WATER_RELAYS_COUNT > 0
                  // запросили настройки канала
                  uint8_t idx = (uint8_t) atoi(command.GetArg(1));
                  
                  if(idx < WATER_RELAYS_COUNT)
                  {
                    PublishSingleton.Status = true;
                 
                    PublishSingleton = WATER_CHANNEL_SETTINGS; 
                    PublishSingleton << PARAM_DELIMITER << (command.GetArg(1)) << PARAM_DELIMITER 
                    << (settings->GetChannelWateringWeekDays(idx)) << PARAM_DELIMITER
                    << (settings->GetChannelWateringTime(idx)) << PARAM_DELIMITER
                    << (settings->GetChannelStartWateringTime(idx));
                  }
                  else
                  {
                    // плохой индекс
                    PublishSingleton = UNKNOWN_COMMAND;
                  }
                  #else
                    PublishSingleton = UNKNOWN_COMMAND;
                  #endif
                          
                } // if
           } // if
        } // else
    } // else have arguments
  } // if ctGET
 
 // отвечаем на команду
    MainController->Publish(this,command);
    
  return PublishSingleton.Status;
}

