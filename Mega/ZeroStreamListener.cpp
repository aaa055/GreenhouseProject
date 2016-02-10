#include "ZeroStreamListener.h"
#include "ModuleController.h"
#include "RemoteModule.h"

void ZeroStreamListener::Setup()
{
  // настройка модуля тут
 }

void ZeroStreamListener::Update(uint16_t dt)
{ 
  UNUSED(dt);
  // обновление модуля тут

}

bool  ZeroStreamListener::ExecCommand(const Command& command)
{
  ModuleController* c = GetController();
  String answer = UNKNOWN_COMMAND;
  bool answerStatus = false;
  bool shouldAddModuleID = false; 
  
  if(command.GetType() == ctGET) 
  {
      answerStatus = false;
      answer = NOT_SUPPORTED;
      
    String t = command.GetRawArguments();
    t.toUpperCase();
    if(t == GetID()) // нет аргументов
    {
      answerStatus = false;
      answer = PARAMS_MISSED;
    }
    else
    {
        uint8_t argsCnt = command.GetArgsCount();
      if(argsCnt < 1)
      {
        // мало параметров
       answerStatus = false;
        answer = PARAMS_MISSED;
        
      } // if
      else
      {
        t = command.GetArg(0); // получили команду
        t.toUpperCase();
        if(t == PING_COMMAND) // пинг
        {
          answerStatus = true;
          answer = PONG;
          shouldAddModuleID = false;
        } // if
        else
        if(t == SMS_NUMBER_COMMAND) // номер телефона для управления по СМС
        {
          answerStatus = true;
          shouldAddModuleID = false;
          answer = SMS_NUMBER_COMMAND; answer += PARAM_DELIMITER; answer += c->GetSettings()->GetSmsPhoneNumber();
        }
        else if(t == HAS_CHANGES_COMMAND) // есть ли изменения в состоянии модулей?
        {
          // CTGET=0|HAS_CHANGES - ВЕРНЕТ 1, ЕСЛИ ЕСТЬ ИЗМЕНЕНИЯ, И 0 - ЕСЛИ НЕТ
            size_t cnt = c->GetModulesCount();
            bool hasChanges = false;
            for(size_t i=0;i<cnt;i++)
            {
                AbstractModule* m = c->GetModule(i);
                if(m != this)
                {
                  uint8_t tempCnt = m->State.GetStateCount(StateTemperature);//GetTempSensors();                 
                  uint8_t relayCnt = m->State.GetStateCount(StateRelay);//GetRelayChannels();

                  for(uint8_t j=0;j<tempCnt;j++)
                    if(m->State.IsStateChanged(StateTemperature,j))//IsTempChanged(j))
                    {
                        hasChanges = true;
                        break;
                    }

                  if(!hasChanges)
                  {
                    for(uint8_t j=0;j<relayCnt;j++)
                      if(m->State.IsStateChanged(StateRelay,j))//IsRelayStateChanged(j))
                      {
                        hasChanges = true;
                        break;
                      }
                  } // if(!hasChanges)
                  
                  if(hasChanges)
                      break;
                } // if(m != this)
            } // for

             shouldAddModuleID = false;
             answerStatus = true;
             answer = hasChanges ? STATE_ON_ALT : STATE_OFF_ALT;
        }
        else if(t == LIST_CHANGES_COMMAND) // какие изменения в состоянии модулей?
        {
          // CTGET=0|LIST_CHANGES - ВЕРНЕТ список всех измененных состояний во всех модулях
          
          //TODO: ПОТЕНЦИАЛЬНАЯ ДЫРА - ЕСЛИ КОЛ-ВО ИЗМЕНЕНИЙ БОЛЬШОЕ - ТО И СТРОКА С ИЗМЕНЕНИЯМИ БУДЕТ БОЛЬШЕ,
          // ЧЕМ ОБЪЁМ ОПЕРАТИВКИ !!! НАДО ПЕРЕПИСАТЬ, ЧТОБЫ РАБОТАЛО С ПОСЛЕДОВАТЕЛЬНЫМИ ОБРАЩЕНИЯМИ, ЛИБО
          // ПО ОЧЕРЕДИ ПИСАЛО СТРОКИ В ПОТОК !!!
          
            size_t cnt = c->GetModulesCount();
            answer = F("");
           
            for(size_t i=0;i<cnt;i++)
            {
                AbstractModule* m = c->GetModule(i);
                if(m != this)
                {
                 // состояние модуля изменилось, проверяем, чего именно изменилось
                 String mName = m->GetID();
                 uint8_t tempCnt = m->State.GetStateCount(StateTemperature);//GetTempSensors();
                 //uint8_t relayCnt = m->State.GetStateCount(StateRelay);//GetRelayChannels();

                 for(uint8_t i=0;i<tempCnt;i++)
                 {
                    if( m->State.IsStateChanged(StateTemperature,i))//IsTempChanged(i)) // температура на датчике изменилась
                    {
                      OneState* os =  m->State.GetState(StateTemperature,i);
                      if(os)
                      {
                        Temperature* tCurrent = (Temperature*) os->Data;
                        Temperature* tPrev = (Temperature*) os->PreviousData;
                      
                        answer += mName + PARAM_DELIMITER + PROP_TEMP + PARAM_DELIMITER + String(i) 
                        + PARAM_DELIMITER + *tPrev + PARAM_DELIMITER + *tCurrent + NEWLINE;
                      } // if
                    } // if
                      
                 } // for
                 /*
                 for(uint8_t i=0;i<relayCnt;i++) // нам вернули кол-во каналов реле, по 8 в каждом
                 {
                    if( m->State.IsStateChanged(StateRelay,i))//IsRelayStateChanged(i)) // состояние реле изменилось
                    {
                      // в канале есть изменения, надо их искать
                      //Serial.println("RELAY STATE CHANGED!");
                      
                      String prevState, curState;
                      OneState* os =  m->State.GetState(StateRelay,i);
                      if(os)
                      {
                          uint8_t prevRelayStates = *((uint8_t*) os->PreviousData);
                          uint8_t curRelayStates = *((uint8_t*) os->Data);
    
                          for(uint8_t j = 0; j<8;j++)
                          {
                            bool bPrevOn = bitRead(prevRelayStates,j);
                            bool bCurOn = bitRead(curRelayStates,j);
                            if(bPrevOn != bCurOn)
                            {
                               // состояние конкретного реле изменилось, пишем его
                               prevState = bPrevOn ? STATE_ON : STATE_OFF;
                               curState =  bCurOn ? STATE_ON : STATE_OFF;
                               answer += mName + PARAM_DELIMITER + PROP_RELAY + PARAM_DELIMITER + String(i*8 + j) +
                               PARAM_DELIMITER + prevState + PARAM_DELIMITER + curState + NEWLINE;
                            }
                          } // for
                          
                      } // if(os)
                      //prevState = m->State.GetPrevRelayState(i) ? STATE_ON : STATE_OFF;
                      //curState = m->State.GetRelayState(i) ? STATE_ON : STATE_OFF;
                      
                      //answer += mName + PARAM_DELIMITER + PROP_RELAY + PARAM_DELIMITER + String(i) + 
                      //PARAM_DELIMITER + prevState + PARAM_DELIMITER + curState + NEWLINE;
                    } // if
                      
                 } // for 
                 */
                                 
                 
                } // if
            } // for

             shouldAddModuleID = false;
             answerStatus = true;
            
        }        
        else if(t == REGISTERED_MODULES_COMMAND) // пролистать зарегистрированные модули
        {
          shouldAddModuleID = false;
          answerStatus = true;
          answer = F("");
          size_t cnt = c->GetModulesCount();
          for(size_t i=0;i<cnt;i++)
          {
            AbstractModule* mod = c->GetModule(i);

            if(mod != this)
            {
              if(answer.length())
                answer += PARAM_DELIMITER;
              
              answer += mod->GetID();
             
            }// if
              
          } // for
        }
        else if(t == PROPERTIES_COMMAND) // запросили свойства модуля
        {
        // запросили чтение свойств
          String reqID = command.GetArg(1);
          AbstractModule* mod = c->GetModuleByID(reqID);
          if(mod)
          {
                  uint8_t argsCnt = command.GetArgsCount();
                  if(argsCnt < 3)
                  {
                    // мало параметров
                    answerStatus = false;
                    answer = PARAMS_MISSED;
        
                  } // if
                  else
                  {
                    String propName = command.GetArg(2); // имя свойства
                    propName.toUpperCase();
                    
                    if(propName == PROP_TEMP_CNT) // кол-во датчиков температуры
                    {
                       // ПРИМЕР:
                      // CTGET=0|PROP|M|TEMP_CNT
                      uint8_t tempCnt = mod->State.GetStateCount(StateTemperature);//GetTempSensors();

                      answerStatus = true;
                      answer = String(PROP_TEMP_CNT) + PARAM_DELIMITER + String(tempCnt);
                    }
                    else if(propName == PROP_RELAY_CNT) // кол-во каналов реле - в каждом канале - 8 реле
                    {
                       // ПРИМЕР:
                      // CTGET=0|PROP|M|RELAY_CNT
                      uint8_t relayCnt = mod->State.GetStateCount(StateRelay);//GetRelayChannels();
                      
                      answerStatus = true;
                      answer = String(PROP_RELAY_CNT) + PARAM_DELIMITER + String(relayCnt);
                      
                    } // else if
                    else if(propName == PROP_TEMP) // запросили температуру
                    {
                      // CTGET=0|PROP|M|TEMP|0 - пример
                         if(argsCnt < 4)
                         {
                          
                            // мало параметров
                            answerStatus = false;
                            answer = PARAMS_MISSED;
                         }
                         else
                         {
                           // получаем сохраненную температуру от датчика
                           if(mod->State.HasState(StateTemperature))//HasTemperature()) // если поддерживаем температуру
                           {
                            uint8_t sensorIdx = command.GetArg(3).toInt();

                            if(sensorIdx < mod->State.GetStateCount(StateTemperature))//mod->State.GetTempSensors())
                            {
                              OneState* os = mod->State.GetState(StateTemperature,sensorIdx);
                              if(os)
                              {
                                Temperature* t = (Temperature*) os->Data;
                                String curTemp = *t;//mod->State.GetTemp(sensorIdx);
                                answerStatus = true;
                                answer = String(PROP_TEMP) + PARAM_DELIMITER + String(sensorIdx) + PARAM_DELIMITER + curTemp;
                              } // if(os)
                            }
 
                           }
                           
                         } // else
                    } // else if
                    else if(propName == PROP_RELAY) // запросили состояние реле
                    {
                      // CTGET=0|PROP|M|RELAY|0 - пример
                         if(argsCnt < 4)
                         {
                          
                            // мало параметров
                            answerStatus = false;
                            answer = PARAMS_MISSED;
                         }
                         else
                         {
                           // получаем состояние реле
                           if(mod->State.HasState(StateRelay))//HasRelay()) // если поддерживаем реле
                           {
                            uint8_t relayIdx = command.GetArg(3).toInt();

                              if(relayIdx < mod->State.GetStateCount(StateRelay)*8)//GetRelayChannels())
                              {
                                
                                uint8_t stateIdx = relayIdx/8;
                                uint8_t bitNum = relayIdx % 8;

                                OneState* os = mod->State.GetState(StateRelay,stateIdx);
                                if(os)
                                {
                                  bool bOn = bitRead(*((uint8_t*)os->Data),bitNum);
                                
                                  String curRelayState = bOn ? STATE_ON : STATE_OFF;
                                  answerStatus = true;
                                  answer = String(PROP_RELAY) + PARAM_DELIMITER + String(relayIdx) +  PARAM_DELIMITER + curRelayState;
                                } // if(os)
                              }
                             
                            }
                           
                         } // else
                    } // else if
                    else
                    {
                    answerStatus = false;
                    answer = UNKNOWN_PROPERTY;
                    }
                  }
          } // if mod
          else
          {
            // модуль неизвестен
            answerStatus = false;
            answer = UNKNOWN_MODULE;
          } // else
          
        } // PROPERTIES_COMMAND
        else
        {
            // неизвестная команда
        } // else
        
          
      } // else
    } // elsse
    
  } // ctGET
  else
  if(command.GetType() == ctSET) //ЗАРЕГИСТРИРОВАТЬ МОДУЛИ
  {

    String t = command.GetRawArguments();
    t.toUpperCase();
    if(t == GetID()) // нет аргументов
    {
      answerStatus = false;
      answer = PARAMS_MISSED;
    }
    else
    {
      uint8_t argsCnt = command.GetArgsCount();
      if(argsCnt < 2)
      {
        // мало параметров
       answerStatus = false;
        answer = PARAMS_MISSED;
        
      } // if
      else
      {
        t = command.GetArg(0); // получили команду
        t.toUpperCase();
        
      if(t == ADD_COMMAND) // запросили регистрацию нового модуля
       {
          // ищем уже зарегистрированный
          String reqID = command.GetArg(1);
          AbstractModule* mod = c->GetModuleByID(reqID);
          if(mod)
          {
            // модуль уже зарегистрирован
            answerStatus = false;
            answer = REG_ERR; answer += PARAM_DELIMITER; answer += reqID;
          } // if
          else
          {
            // регистрируем новый модуль
            RemoteModule* remMod = new RemoteModule(reqID); 
            c->RegisterModule(remMod);
            answerStatus = true;
            answer = REG_SUCC; answer += PARAM_DELIMITER; answer += reqID;

          } // else
       }
       
       else if(t == SMS_NUMBER_COMMAND) // номер телефона для управления по SMS
       {
          GlobalSettings* sett = c->GetSettings();
          sett->SetSmsPhoneNumber(command.GetArg(1));
          sett->Save();
          answerStatus = true;
          answer = SMS_NUMBER_COMMAND; answer += PARAM_DELIMITER; answer += REG_SUCC;
          
       }
       else
       if(t == SETTIME_COMMAND)
       {
         // установка даты/времени
         String rawDatetime = command.GetArg(1);
         int8_t idx = rawDatetime.indexOf(F(" "));
         String tm, dt;
         if(idx != -1)
         {
          dt = rawDatetime.substring(0,idx);
          tm = rawDatetime.substring(idx+1);

            String month,day,year;
            String hour,minute,sec;
            idx = dt.indexOf(F("."));
            if(idx != -1)
             {
              day = dt.substring(0,idx);
              dt = dt.substring(idx+1);
             }
             
            idx = dt.indexOf(F("."));
            if(idx != -1)
             {
              month = dt.substring(0,idx);
              year = dt.substring(idx+1);
             }

             idx = tm.indexOf(F(":"));
             if(idx != -1)
             {
              hour = tm.substring(0,idx);
              tm = tm.substring(idx+1);
             }

             idx = tm.indexOf(F(":"));
             if(idx != -1)
             {
              minute = tm.substring(0,idx);
              sec = tm.substring(idx+1);
             }

             // вычисляем день недели
             int yearint = year.toInt();
             int monthint = month.toInt();
             int dayint = day.toInt();
             
             int dow;
             byte mArr[12] = {6,2,2,5,0,3,5,1,4,6,2,4};
             dow = (yearint % 100);
             dow = dow*1.25;
             dow += dayint;
             dow += mArr[monthint-1];
             
             if (((yearint % 4)==0) && (monthint<3))
               dow -= 1;
               
             while (dow>7)
               dow -= 7;             

             DS3231Clock cl = c->GetClock();
             cl.setTime(sec.toInt(),minute.toInt(),hour.toInt(),dow,dayint,monthint,yearint);

             answerStatus = true;
             answer = REG_SUCC;
         } // if
       }
       
       else if(t == PROPERTIES_COMMAND)
       {
        // запросили установку свойств
          String reqID = command.GetArg(1);
          AbstractModule* mod = c->GetModuleByID(reqID);
          if(mod)
          {
                  uint8_t argsCnt = command.GetArgsCount();
                  if(argsCnt < 4)
                  {
                    // мало параметров
                    answerStatus = false;
                    answer = PARAMS_MISSED;
        
                  } // if
                  else
                  {
                    String propName = command.GetArg(2); // имя свойства
                    propName.toUpperCase();
                    
                    if(propName == PROP_TEMP_CNT) // кол-во датчиков температуры
                    {
                       // ПРИМЕР:
                      // CTSET=0|PROP|M|TEMP_CNT|2
                      uint8_t tempCnt = command.GetArg(3).toInt();

                      //mod->State.SetTempSensors(tempCnt);
                      for(uint8_t toAdd = 0; toAdd < tempCnt; toAdd++)
                        mod->State.AddState(StateTemperature,toAdd);
                      
                      answerStatus = true;
                      answer = REG_SUCC;
                    }
                    else if(propName == PROP_RELAY_CNT) // кол-во каналов реле
                    {
                       // ПРИМЕР:
                      // CTSET=0|PROP|M|RELAY_CNT|5
                      uint8_t relayCnt = command.GetArg(3).toInt();

                      //mod->State.SetRelayChannels(relayCnt);
                      uint8_t channelsCnt = relayCnt/8;
                      if(relayCnt > 8 && relayCnt % 8)
                        channelsCnt++;

                      if(relayCnt < 9)
                        channelsCnt = 1;
                        

                      for(uint8_t k = 0;k<channelsCnt;k++)
                        mod->State.AddState(StateRelay,k);

                      answerStatus = true;
                      answer = REG_SUCC;
                      
                    } // else if
                    else if(propName == PROP_TEMP) // передали температуру
                    {
                      // CTSET=0|PROP|M|TEMP|0|36,6 - пример
                         if(argsCnt < 5)
                         {
                          
                            // мало параметров
                            answerStatus = false;
                            answer = PARAMS_MISSED;
                         }
                         else
                         {
                           // сохраняем температуру от датчика
                           String curTemp = command.GetArg(4);
                           uint8_t sensorIdx = command.GetArg(3).toInt();

                           Temperature t;
                           t.Value = curTemp.toInt();
                           int8_t idx = curTemp.indexOf(F(","));
                           if(idx != -1)
                           {
                              curTemp = curTemp.substring(idx+1);
                              t.Fract = curTemp.toInt();
                           }

                           //mod->State.SetTemp(sensorIdx,t);
                           mod->State.UpdateState(StateTemperature,sensorIdx,(void*)&t);
 
                            answerStatus = true;
                            answer = REG_SUCC;
                           
                         } // else
                    } // else if
                    else if(propName == PROP_RELAY) // передали состояние реле
                    {
                      // CTSET=0|PROP|M|RELAY|0|ON - пример
                         if(argsCnt < 5)
                         {
                          
                            // мало параметров
                            answerStatus = false;
                            answer = PARAMS_MISSED;
                         }
                         else
                         {
                           // сохраняем состояние реле
                           String curRelayState = command.GetArg(4);
                           uint8_t relayIdx = command.GetArg(3).toInt();

                           uint8_t channelIdx = relayIdx/8;
                           uint8_t bitNum = relayIdx % 8;

                           OneState* os = mod->State.GetState(StateRelay,channelIdx);
                           if(os)
                           {
                            uint8_t curRState = *((uint8_t*)os->Data);
                            bitWrite(curRState,bitNum,(curRelayState == STATE_ON));
                            mod->State.UpdateState(StateRelay,channelIdx,(void*) &curRState);
                           } 

                           //mod->State.SetRelayState(relayIdx,curRelayState);
                           
                            answerStatus = true;
                            answer = REG_SUCC;
                           
                         } // else
                    } // else if
                    else
                    {
                    answerStatus = false;
                    answer = UNKNOWN_PROPERTY;
                    }
                  }
          } // if mod
          else
          {
            // модуль неизвестен
            answerStatus = false;
            answer = UNKNOWN_MODULE;
          } // else
        
       } // PROPERTIES_COMMAND
       else
       {
         // неизвестная команда
       } // else
      } // else argsCount > 1
    } // else
    
  } // if
 
 // отвечаем на команду
    SetPublishData(&command,answerStatus,answer,shouldAddModuleID); // готовим данные для публикации
    c->Publish(this);
    
  return answerStatus;
}

