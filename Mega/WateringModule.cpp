#include "WateringModule.h"
#include "ModuleController.h"

static uint8_t WATER_RELAYS[] = { WATER_RELAYS_PINS }; // объявляем массив пинов реле

void WateringModule::Setup()
{
  // настройка модуля тут

  workMode = wwmAutomatic; // автоматический режим работы
  wateringTimer = 0; // обнуляем таймер полива
  lastDOW = -1; // неизвестный день недели

  bRelaysIsOn = false; // все реле выключены
  State.SetRelayChannels(WATER_RELAYS_COUNT); // устанавливаем кол-во каналов реле

  // выключаем все реле
  for(uint8_t i=0;i<WATER_RELAYS_COUNT;i++)
  {
    pinMode(WATER_RELAYS[i],OUTPUT);
    digitalWrite(WATER_RELAYS[i],WATER_RELAY_OFF);
    State.SetRelayState(i,bRelaysIsOn);
  }

    // настраиваем режим работы перед стартом
    WateringOption currentWateringOption = GetController()->GetSettings()->GetWateringOption();
    
    if(currentWateringOption == wateringOFF) // если выключено автоуправление поливом
    {
      workMode = wwmManual; // переходим в ручной режим работы
      BlinkWorkMode(WORK_MODE_BLINK_INTERVAL); // зажигаем диод
    }
    else
    {
      workMode = wwmAutomatic; // иначе переходим в автоматический режим работы
      BlinkWorkMode(); // гасим диод
    }
      

}

void WateringModule::Update(uint16_t dt)
{ 

  // сохраняем текущее состояние реле
  uint8_t state = bRelaysIsOn ? WATER_RELAY_ON : WATER_RELAY_OFF;
   
  for(uint8_t i=0;i<WATER_RELAYS_COUNT;i++)
  {
    digitalWrite(WATER_RELAYS[i],state);
    State.SetRelayState(i,bRelaysIsOn);
  } 
  
  // проверяем, можем ли мы работать?
  ModuleController* controller = GetController();
  GlobalSettings* settings = controller->GetSettings();
  WateringOption wo = settings->GetWateringOption(); // получаем опцию управления поливом


  // проверяем её только в автоматическом режиме работы, при ручном режиме - нас попросит включить/выключить полив сам пользователь
  if(workMode == wwmAutomatic)
  {
      switch(wo)
      {
        case wateringOFF: // автоматическое управление поливом выключено
          bRelaysIsOn = false; // выключаем все реле в следующем вызове Update
        break; // wateringOFF

        case wateringWeekDays: // // управление поливом по дням недели
        
        #ifdef USE_DS3231_REALTIME_CLOCK
          // только если в системе есть модуль часов
          uint8_t weekDays = settings->GetWateringWeekDays();
          DS3231 watch =  controller->GetClock();
          Time t =   watch.getTime();
          bool canWork = bitRead(weekDays,t.dow-1); // проверяем, установлен ли у нас день недели для полива

          if(lastDOW == -1) // если мы не сохраняли текущий день недели, то
            lastDOW = t.dow; // сохраним его, чтобы потом проверять переход через дни недели

          if(!canWork)
           {            
             wateringTimer = 0; // в этот день недели работать не можем, однозначно обнуляем таймер полива    
             bRelaysIsOn = false; // выключаем реле
           }
           else
           {
            // можем работать, смотрим, не вышли ли мы за пределы установленного интервала

             uint16_t timeToWatering = settings->GetWateringTime(); // время полива (в минутах!)

            if(lastDOW != t.dow)  // сначала проверяем, не другой ли день недели уже?
            {
              // начался другой день недели
              
              lastDOW = t.dow; // сохраняем текущий день недели
              // начался другой день недели, в который мы можем работать. Для одного дня недели у нас установлена
              // продолжительность полива, поэтому, если мы поливали 30 мин, например, во вторник, и перешли на среду,
              // то и в среду надо полить 30 мин. Поэтому таймер полива переводим в нужный режим:
              // оставляем в нём недополитое время, чтобы учесть, что поливать надо, например, 32 минуты.

              //               разница между полным и отработанным временем
              wateringTimer = -((timeToWatering*60*1000) - wateringTimer); // загоняем в минус, чтобы добавить недостающие минуты к работе
            }
            
            wateringTimer += dt; // прибавляем время работы

            // проверяем, можем ли мы ещё работать
            // если полив уже отработал, и юзер прибавит минуту - мы должны поливать ещё минуту,
            // вне зависимости от показания таймера. Поэтому мы при срабатывании условия окончания полива
            // просто отнимаем дельту времени из таймера, таким образом оставляя его застывшим по времени
            // окончания полива

            if(wateringTimer > (timeToWatering*60000) + dt) // приплыли, надо выключать полив
            {
              wateringTimer -= dt;// оставляем таймер застывшим на окончании полива, плюс маленькая дельта
              bRelaysIsOn = false;
            }
            else
              bRelaysIsOn = true; // ещё можем работать, продолжаем поливать
           } // else
        #endif
        
        break; // wateringWeekDays
        
      } // switch
  } // if(workMode == wwmAutomatic)
  else
  {
    // мы в ручном режиме работы, пользователь сам будет включать/выключать полив
  } // else
  

}
void WateringModule::BlinkWorkMode(uint16_t blinkInterval) // мигаем диодом индикации ручного режима работы
{
  String s = F("CTSET=LOOP|SET|");
  s += blinkInterval;
  s+= F("|0|PIN|");
  s += DIODE_WATERING_MANUAL_MODE_PIN;
  s += F("|T");

    ModuleController* c = GetController();
    CommandParser* cParser = c->GetCommandParser();
      Command cmd;
      if(cParser->ParseCommand(s, c->GetControllerID(), cmd))
      {
         cmd.SetInternal(true); // говорим, что команда - от одного модуля к другому

        // НЕ БУДЕМ НИКУДА ПЛЕВАТЬСЯ ОТВЕТОМ ОТ МОДУЛЯ
        //cmd.SetIncomingStream(pStream);
        c->ProcessModuleCommand(cmd,false); // не проверяем адресата, т.к. он может быть удаленной коробочкой    
      } // if  

      if(!blinkInterval) // не надо зажигать диод, принудительно гасим его
      {
        s = CMD_PREFIX;
        s += CMD_SET;
        s += F("=PIN|");
        s += DIODE_WATERING_MANUAL_MODE_PIN;
        s += PARAM_DELIMITER;
        s += F("0");
        
        cParser->ParseCommand(s, c->GetControllerID(), cmd);
        cmd.SetInternal(true); 
        c->ProcessModuleCommand(cmd,false);
      } // if
  
}

bool  WateringModule::ExecCommand(const Command& command)
{
  ModuleController* c = GetController();
  GlobalSettings* settings = c->GetSettings();
  
  String answer = UNKNOWN_COMMAND;
  
  bool answerStatus = false; 
  
  if(command.GetType() == ctSET) 
  {
      uint8_t argsCount = command.GetArgsCount();
      
      if(argsCount < 1) // не хватает параметров
      {
        answerStatus = false;
        answer = PARAMS_MISSED;
      }
      else
      {
        String which = command.GetArg(0);
        which.toUpperCase();

        if(which == WATER_SETTINGS_COMMAND)
        {
          if(argsCount > 3)
          {
              // парсим параметры
              WateringOption wateringOption = (WateringOption) command.GetArg(1).toInt();
              uint8_t wateringWeekDays = command.GetArg(2).toInt();
              uint16_t wateringTime = command.GetArg(3).toInt();
      
              // пишем в настройки
              settings->SetWateringOption(wateringOption);
              settings->SetWateringWeekDays(wateringWeekDays);
              settings->SetWateringTime(wateringTime);
      
              // сохраняем настройки
              settings->Save();

              if(wateringOption == wateringOFF) // если выключено автоуправление поливом
              {
                workMode = wwmManual; // переходим в ручной режим работы
                bRelaysIsOn = false; // принудительно гасим полив
                BlinkWorkMode(WORK_MODE_BLINK_INTERVAL); // зажигаем диод
              }
              else
              {
                workMode = wwmAutomatic; // иначе переходим в автоматический режим работы
                BlinkWorkMode(); // гасим диод
              }
      
              
              answerStatus = true;
              answer = WATER_SETTINGS_COMMAND; answer += PARAM_DELIMITER;
              answer += REG_SUCC;
          } // argsCount > 3
          else
          {
            // не хватает команд
            answerStatus = false;
            answer = PARAMS_MISSED;
          }
          
        } // WATER_SETTINGS_COMMAND
        else
        if(which == WORK_MODE) // CTSET=WATER|MODE|AUTO, CTSET=WATER|MODE|MANUAL
        {
           // попросили установить режим работы
           String param = command.GetArg(1);
           
           if(param == WM_AUTOMATIC)
           {
              // переходим в автоматический режим работы
               workMode = wwmAutomatic;

             BlinkWorkMode(0); // гасим диод
           }
           else
           {
            workMode = wwmManual; // переходим на ручной режим работы
            
           BlinkWorkMode(WORK_MODE_BLINK_INTERVAL); // зажигаем диод
           }

              answerStatus = true;
              answer = WORK_MODE; answer += PARAM_DELIMITER;
              answer += param;
        
        } // WORK_MODE
        else 
        if(which == STATE_ON) // попросили включить полив, CTSET=WATER|ON
        {
          if(!command.IsInternal()) // если команда от юзера, то
          {
            workMode = wwmManual; // переходим в ручной режим работы
            BlinkWorkMode(WORK_MODE_BLINK_INTERVAL); // зажигаем диод
          }

          bRelaysIsOn = true; // включаем реле

          answerStatus = true;
          answer = STATE_ON;
        } // STATE_ON
        else 
        if(which == STATE_OFF) // попросили выключить полив, CTSET=WATER|OFF
        {
          if(!command.IsInternal()) // если команда от юзера, то
          {
            workMode = wwmManual; // переходим в ручной режим работы
            BlinkWorkMode(WORK_MODE_BLINK_INTERVAL); // зажигаем диод
          }

          bRelaysIsOn = false; // выключаем реле

          answerStatus = true;
          answer = STATE_OFF;
          
        } // STATE_OFF        

      } // else
  }
  else
  if(command.GetType() == ctGET) //получить данные
  {

    String t = command.GetRawArguments();
    t.toUpperCase();
    if(t == GetID()) // нет аргументов, попросили вернуть статус полива
    {
      answerStatus = true;
      answer = bRelaysIsOn ? STATE_ON : STATE_OFF;
    }
    else
    if(t == WATER_SETTINGS_COMMAND) // запросили данные о настройках полива
    {
      answerStatus = true;
      answer = WATER_SETTINGS_COMMAND; answer += PARAM_DELIMITER; 
      answer += String(settings->GetWateringOption()); answer += PARAM_DELIMITER;
      answer += String(settings->GetWateringWeekDays()); answer += PARAM_DELIMITER;
      answer += String(settings->GetWateringTime());
    }
    else
    if(t == WORK_MODE) // получить режим работы
    {
      answerStatus = true;
      answer = WORK_MODE; answer += PARAM_DELIMITER;
      answer += workMode == wwmAutomatic ? WM_AUTOMATIC : WM_MANUAL;
    }
    else
    {
      // неизвестная команда
    } // else
    
  } // if
 
 // отвечаем на команду
    SetPublishData(&command,answerStatus,answer); // готовим данные для публикации
    c->Publish(this);
    
  return answerStatus;
}

