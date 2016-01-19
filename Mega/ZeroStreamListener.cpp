#include "ZeroStreamListener.h"
#include "ModuleController.h"
#include "RemoteModule.h"

void ZeroStreamListener::Setup()
{
  // настройка модуля тут
 }

void ZeroStreamListener::Update(uint16_t dt)
{ 
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
                  uint8_t tempCnt = m->State.GetTempSensors();                 
                  uint8_t relayCnt = m->State.GetRelayChannels();

                  for(uint8_t j=0;j<tempCnt;j++)
                    if(m->State.IsTempChanged(j))
                    {
                        hasChanges = true;
                        break;
                    }

                  if(!hasChanges)
                  {
                    for(uint8_t j=0;j<relayCnt;j++)
                      if(m->State.IsRelayStateChanged(j))
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
                 uint8_t tempCnt = m->State.GetTempSensors();
                 uint8_t relayCnt = m->State.GetRelayChannels();

                 for(uint8_t i=0;i<tempCnt;i++)
                 {
                    if( m->State.IsTempChanged(i)) // температура на датчике изменилась
                    {
                      answer += mName + PARAM_DELIMITER + PROP_TEMP + PARAM_DELIMITER + String(i) 
                      + PARAM_DELIMITER + m->State.GetPrevTemp(i) + PARAM_DELIMITER + m->State.GetTemp(i) + NEWLINE;
                    }
                      
                 } // for
                 for(uint8_t i=0;i<relayCnt;i++)
                 {
                    if( m->State.IsRelayStateChanged(i)) // состояние реле изменилось
                    {
                      String prevState, curState;
                      prevState = m->State.GetPrevRelayState(i) ? STATE_ON : STATE_OFF;
                      curState = m->State.GetRelayState(i) ? STATE_ON : STATE_OFF;
                      
                      answer += mName + PARAM_DELIMITER + PROP_RELAY + PARAM_DELIMITER + String(i) + 
                      PARAM_DELIMITER + prevState + PARAM_DELIMITER + curState + NEWLINE;
                    }
                      
                 } // for                 
                 
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
                      uint8_t tempCnt = mod->State.GetTempSensors();

                      answerStatus = true;
                      answer = String(PROP_TEMP_CNT) + PARAM_DELIMITER + String(tempCnt);
                    }
                    else if(propName == PROP_RELAY_CNT) // кол-во каналов реле
                    {
                       // ПРИМЕР:
                      // CTGET=0|PROP|M|RELAY_CNT
                      uint8_t relayCnt = mod->State.GetRelayChannels();
                      
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
                           if(mod->State.HasTemperature()) // если поддерживаем температуру
                           {
                            uint8_t sensorIdx = command.GetArg(3).toInt();

                            if(sensorIdx < mod->State.GetTempSensors())
                            {
                              String curTemp = mod->State.GetTemp(sensorIdx);
                              answerStatus = true;
                              answer = String(PROP_TEMP) + PARAM_DELIMITER + String(sensorIdx) + PARAM_DELIMITER + curTemp;
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
                           if(mod->State.HasRelay()) // если поддерживаем реле
                           {
                            uint8_t relayIdx = command.GetArg(3).toInt();

                              if(relayIdx < mod->State.GetRelayChannels())
                              {
                                String curRelayState = mod->State.GetRelayState(relayIdx) ? STATE_ON : STATE_OFF;
                                answerStatus = true;
                                answer = String(PROP_RELAY) + PARAM_DELIMITER + String(relayIdx) +  PARAM_DELIMITER + curRelayState;
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

                      mod->State.SetTempSensors(tempCnt);
                      
                      answerStatus = true;
                      answer = REG_SUCC;
                    }
                    else if(propName == PROP_RELAY_CNT) // кол-во каналов реле
                    {
                       // ПРИМЕР:
                      // CTSET=0|PROP|M|RELAY_CNT|5
                      uint8_t relayCnt = command.GetArg(3).toInt();

                      mod->State.SetRelayChannels(relayCnt);

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

                           mod->State.SetTemp(sensorIdx,curTemp);
 
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

                           mod->State.SetRelayState(relayIdx,curRelayState);
                           
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

