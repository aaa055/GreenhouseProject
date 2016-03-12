#include "ZeroStreamListener.h"
#include "ModuleController.h"
#ifdef USE_REMOTE_MODULES
#include "RemoteModule.h"
#endif

void ZeroStreamListener::Setup()
{
  // настройка модуля тут
 }

void ZeroStreamListener::Update(uint16_t dt)
{ 
  UNUSED(dt);
  // обновление модуля тут

}

bool  ZeroStreamListener::ExecCommand(const Command& command, bool wantAnswer)
{
  if(wantAnswer) PublishSingleton = UNKNOWN_COMMAND;

  bool canPublish = true; // флаг, что можем публиковать

   size_t argsCnt = command.GetArgsCount();
  
  if(command.GetType() == ctGET) 
  {
     PublishSingleton = NOT_SUPPORTED;      
    if(!argsCnt) // нет аргументов
    {
      PublishSingleton = PARAMS_MISSED;
    }
    else
    {
      if(argsCnt < 1)
      {
        // мало параметров
        PublishSingleton = PARAMS_MISSED;
        
      } // if
      else
      {
        String t = command.GetArg(0); // получили команду
        t.toUpperCase();
        if(t == PING_COMMAND) // пинг
        {
          PublishSingleton.Status = true;
          PublishSingleton = PONG;
          PublishSingleton.AddModuleIDToAnswer = false;
        } // if
        else
        if(t == SMS_NUMBER_COMMAND) // номер телефона для управления по СМС
        {
          PublishSingleton.Status = true;
          PublishSingleton.AddModuleIDToAnswer = false;
          PublishSingleton = SMS_NUMBER_COMMAND; 
          PublishSingleton << PARAM_DELIMITER << mainController->GetSettings()->GetSmsPhoneNumber();
        }
        else if(t == HAS_CHANGES_COMMAND) // есть ли изменения в состоянии модулей?
        {
          // CTGET=0|HAS_CHANGES - ВЕРНЕТ 1, ЕСЛИ ЕСТЬ ИЗМЕНЕНИЯ, И 0 - ЕСЛИ НЕТ
            size_t cnt = mainController->GetModulesCount();
            bool hasChanges = false;
            for(size_t i=0;i<cnt;i++)
            {
                AbstractModule* m = mainController->GetModule(i);
                if(m != this)
                {
                  hasChanges = m->State.HasChanges();
                  
                  if(hasChanges)
                      break;
                } // if(m != this)
            } // for

             PublishSingleton.AddModuleIDToAnswer = false;
             PublishSingleton.Status = true;
             PublishSingleton = (hasChanges ? STATE_ON_ALT : STATE_OFF_ALT);
        }
        else if(t == STATUS_COMMAND) // получить статус всего железного добра
        {
          if(wantAnswer)
          {
            // входящий поток установлен, значит, можем писать прямо в него
            canPublish = false; // скажем, что мы не хотим публиковать через контроллер - будем писать в поток сами
            Stream* pStream = command.GetIncomingStream();
            pStream->print(OK_ANSWER);
            pStream->print(COMMAND_DELIMITER);

            WORK_STATUS.WriteStatus(pStream,true); // просим записать статус

            // тут можем писать остальные статусы, типа показаний датчиков и т.п.

            size_t modulesCount = mainController->GetModulesCount(); // получаем кол-во зарегистрированных модулей

            const char* noDataByte = "FF"; // байт - нет данных с датчика

            // пробегаем по всем модулям
            String moduleName;
            moduleName.reserve(20);
            
            for(size_t i=0;i<modulesCount;i++)
            {
              yield(); // немного даём поработать другим модулям

              AbstractModule* mod = mainController->GetModule(i);
              if(mod == this) // себя пропускаем
                continue;

              // проверяем, не пустой ли модуль. для этого смотрим, сколько у него датчиков вообще
              uint8_t tempCount = mod->State.GetStateCount(StateTemperature);
              uint8_t humCount = mod->State.GetStateCount(StateHumidity);
              uint8_t lightCount = mod->State.GetStateCount(StateLuminosity);

              if((tempCount + humCount + lightCount) < 1) // пустой модуль, без датчиков
                continue;
              

            // показание каждого модуля идут так:
            // 1 байт - длина ID модуля
              moduleName = mod->GetID();
              uint8_t mnamelen = moduleName.length();
              pStream->write(WorkStatus::ToHex(mnamelen).c_str());
            // далее идёт имя модуля
              pStream->write(moduleName.c_str());
            
            
            // затем идут данные из модуля, сначала - показания температуры:
            
            // 1 байт - кол-во датчиков температуры
              pStream->write(WorkStatus::ToHex(tempCount).c_str());

              for(uint8_t cntr=0;cntr<tempCount;cntr++)
              {
                yield(); // немного даём поработать другим модулям
                
                OneState* os = mod->State.GetStateByOrder(StateTemperature,cntr);
                // потом идут пакеты температуры. каждый пакет состоит из:
                // 1 байт - индекс датчика
                pStream->write(WorkStatus::ToHex(os->GetIndex()).c_str());
                // 2 байта - его показания, мы пишем любые показания, даже если датчика нет на линии
                TemperaturePair tp = *os;
                if(tp.Current.Value != NO_TEMPERATURE_DATA)
                {
                  pStream->write(WorkStatus::ToHex(tp.Current.Value).c_str());
                  pStream->write(WorkStatus::ToHex(tp.Current.Fract).c_str());
                }
                else
                {
                  // датчика нет на линии, пишем FFFF
                  pStream->write(noDataByte);
                  pStream->write(noDataByte);
                }
                 
              } // for

              // затем идёт кол-во датчиков влажности, 1 байт.
              pStream->write(WorkStatus::ToHex(humCount).c_str());
              
              for(uint8_t cntr=0;cntr<humCount;cntr++)
              {
                yield(); // немного даём поработать другим модулям
                
                OneState* os = mod->State.GetStateByOrder(StateHumidity,cntr);
                // затем идут показания датчиков влажности, каждый пакет состоит из:
                // 1 байт - индекс датчика
                pStream->write(WorkStatus::ToHex(os->GetIndex()).c_str());
                // 2 байта - его показания, мы пишем любые показания, даже если датчика нет на линии
                HumidityPair hp = *os;
                if(hp.Current.Value != NO_TEMPERATURE_DATA)
                {
                  pStream->write(WorkStatus::ToHex(hp.Current.Value).c_str());
                  pStream->write(WorkStatus::ToHex(hp.Current.Fract).c_str());
                }
                else
                {
                  // датчика нет на линии, пишем FFFF
                  pStream->write(noDataByte);
                  pStream->write(noDataByte);
                }

              } // for

            // затем идут показания датчиков освещенности:
            // 1 байт - кол-во датчиков
              pStream->write(WorkStatus::ToHex(lightCount).c_str());
            
            // затем идут пакеты с данными:
                for(uint8_t cntr=0;cntr < lightCount; cntr++)
                {
                  yield(); // немного даём поработать другим модулям
                  
                  OneState* os = mod->State.GetStateByOrder(StateLuminosity,cntr);
                  // 1 байт - индекс датчика
                  pStream->write(WorkStatus::ToHex(os->GetIndex()).c_str());
                  
                  // 2 байта - его показания, мы пишем любые показания, даже если датчика нет на линии
                  LuminosityPair lp = *os;
                  long lum = lp.Current;
                  if(lum != NO_LUMINOSITY_DATA)
                  {
                    byte* b = (byte*)&lum;
                    pStream->write(WorkStatus::ToHex(*(b+1)).c_str());
                    pStream->write(WorkStatus::ToHex(*b).c_str());
                  }
                  else
                  {
                    // датчика нет на линии, пишем FFFF
                    pStream->write(noDataByte);
                    pStream->write(noDataByte);
                  }

                } // for

            } // for
            

            pStream->print(NEWLINE); // пишем перевод строки
            
          } // wantAnswer
          
        } // STATUS_COMMAND
        else if(t == LIST_CHANGES_COMMAND) // какие изменения в состоянии модулей?
        {
          // CTGET=0|LIST_CHANGES - ВЕРНЕТ список всех измененных состояний во всех модулях
          
          //TODO: ПОТЕНЦИАЛЬНАЯ ДЫРА - ЕСЛИ КОЛ-ВО ИЗМЕНЕНИЙ БОЛЬШОЕ - ТО И СТРОКА С ИЗМЕНЕНИЯМИ БУДЕТ БОЛЬШЕ,
          // ЧЕМ ОБЪЁМ ОПЕРАТИВКИ !!! НАДО ПЕРЕПИСАТЬ, ЧТОБЫ РАБОТАЛО С ПОСЛЕДОВАТЕЛЬНЫМИ ОБРАЩЕНИЯМИ, ЛИБО
          // ПО ОЧЕРЕДИ ПИСАЛО СТРОКИ В ПОТОК !!!
          PublishSingleton = F("");
          
            size_t cnt = mainController->GetModulesCount();
           
            for(size_t i=0;i<cnt;i++)
            {
                AbstractModule* m = mainController->GetModule(i);
                if(m != this)
                {
                 // состояние модуля изменилось, проверяем, чего именно изменилось
                 String mName = m->GetID();
                 uint8_t tempCnt = m->State.GetStateCount(StateTemperature);
                 #ifdef SAVE_RELAY_STATES
                 //uint8_t relayCnt = m->State.GetStateCount(StateRelay);
                 #endif

                 for(uint8_t i=0;i<tempCnt;i++)
                 {
                    OneState* os = m->State.GetStateByOrder(StateTemperature,i);
                    if( os && os->IsChanged() ) // температура на датчике изменилась
                    {
                      OneState* os =  m->State.GetStateByOrder(StateTemperature,i);
                      if(os)
                      {
                        TemperaturePair tp = *os;
                      
                        PublishSingleton << mName << PARAM_DELIMITER << PROP_TEMP << PARAM_DELIMITER << i 
                        << PARAM_DELIMITER << (tp.Prev) << PARAM_DELIMITER << (tp.Current) << NEWLINE;
                      } // if
                    } // if
                      
                 } // for
                 
                } // if
            } // for

             PublishSingleton.AddModuleIDToAnswer = false;
             PublishSingleton.Status = true;
            
        }        
        else if(t == REGISTERED_MODULES_COMMAND) // пролистать зарегистрированные модули
        {
          PublishSingleton.AddModuleIDToAnswer = false;
          PublishSingleton.Status = true;
          PublishSingleton = F("");
          size_t cnt = mainController->GetModulesCount();
          for(size_t i=0;i<cnt;i++)
          {
            AbstractModule* mod = mainController->GetModule(i);

            if(mod != this)
            {
              if(PublishSingleton.Text.length())
                PublishSingleton << PARAM_DELIMITER;
              
              PublishSingleton << mod->GetID();
             
            }// if
              
          } // for
        }
        else if(t == PROPERTIES_COMMAND) // запросили свойства модуля
        {
        // запросили чтение свойств
          String reqID = command.GetArg(1);
          AbstractModule* mod = mainController->GetModuleByID(reqID);
          if(mod)
          {
                  if(argsCnt < 3)
                  {
                    // мало параметров
                    PublishSingleton = PARAMS_MISSED;
        
                  } // if
                  else
                  {
                    String propName = command.GetArg(2); // имя свойства
                    propName.toUpperCase();
                    
                    if(propName == PROP_TEMP_CNT) // кол-во датчиков температуры
                    {
                       // ПРИМЕР:
                      // CTGET=0|PROP|M|TEMP_CNT
                      uint8_t tempCnt = mod->State.GetStateCount(StateTemperature);

                      PublishSingleton.Status = true;
                      PublishSingleton.AddModuleIDToAnswer = false;
                      PublishSingleton = reqID;
                      PublishSingleton << PARAM_DELIMITER << PROP_TEMP_CNT << PARAM_DELIMITER  << tempCnt;
                    }
                    else if(propName == PROP_RELAY_CNT) // кол-во каналов реле - в каждом канале - 8 реле
                    {
                      #ifdef SAVE_RELAY_STATES
                       // ПРИМЕР:
                      // CTGET=0|PROP|M|RELAY_CNT
                      uint8_t relayCnt = mod->State.GetStateCount(StateRelay);
                      
                      PublishSingleton.Status = true;
                      PublishSingleton = reqID;
                      PublishSingleton << PARAM_DELIMITER << PROP_RELAY_CNT << PARAM_DELIMITER  << relayCnt;
                      #else
                      PublishSingleton = NOT_SUPPORTED;
                      #endif
                      
                    } // else if
                    else if(propName == PROP_TEMP) // запросили температуру
                    {
                      // CTGET=0|PROP|M|TEMP|0 - пример
                         if(argsCnt < 4)
                         {
                            // мало параметров
                            PublishSingleton = PARAMS_MISSED;
                         }
                         else
                         {
                           // получаем сохраненную температуру от датчика
                           if(mod->State.HasState(StateTemperature)) // если поддерживаем температуру
                           {
                            uint8_t sensorIdx = String(command.GetArg(3)).toInt();

                              OneState* os = mod->State.GetState(StateTemperature,sensorIdx);
                              if(os)
                              {
                                TemperaturePair tp = *os;
                                String curTemp = tp.Current;
                                PublishSingleton.Status = true;
                                PublishSingleton.AddModuleIDToAnswer = false;
                                PublishSingleton = reqID;
                                PublishSingleton << PARAM_DELIMITER << PROP_TEMP << PARAM_DELIMITER  << sensorIdx << PARAM_DELIMITER << curTemp;
                              } // if(os)
                            
 
                           }
                           
                         } // else
                    } // else if
                    else if(propName == PROP_RELAY) // запросили состояние реле
                    {
                      // CTGET=0|PROP|M|RELAY|0 - пример
                         if(argsCnt < 4)
                         {
                            // мало параметров
                            PublishSingleton = PARAMS_MISSED;
                         }
                         else
                         {
                          #ifdef SAVE_RELAY_STATES
                           // получаем состояние реле
                           if(mod->State.HasState(StateRelay)) // если поддерживаем реле
                           {
                            uint8_t relayIdx = String(command.GetArg(3)).toInt();
                                
                                uint8_t stateIdx = relayIdx/8;
                                uint8_t bitNum = relayIdx % 8;

                                OneState* os = mod->State.GetState(StateRelay,stateIdx);
                                if(os)
                                {
                                  RelayPair rp = *os;
                                  bool bOn = bitRead(rp.Current,bitNum);
                                  PublishSingleton.Status = true;
                                  PublishSingleton.AddModuleIDToAnswer = false;
                                  PublishSingleton = reqID;
                                  PublishSingleton << PARAM_DELIMITER << PROP_RELAY << PARAM_DELIMITER  << relayIdx <<  PARAM_DELIMITER << (bOn ? STATE_ON : STATE_OFF);
                                } // if(os)
                             
                             
                            } // if has relay
                            else
                            {
                              PublishSingleton = NOT_SUPPORTED;  
                            }
                           #else
                            PublishSingleton = NOT_SUPPORTED; 
                           #endif
                         } // else
                    } // else if
                    else
                    {
                    PublishSingleton = UNKNOWN_PROPERTY;
                    }
                  }
          } // if mod
          else
          {
            // модуль неизвестен
            PublishSingleton = UNKNOWN_MODULE;
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

    if(!argsCnt) // нет аргументов
    {
      PublishSingleton = PARAMS_MISSED;
    }
    else
    {
      if(argsCnt < 2)
      {
        // мало параметров
        PublishSingleton = PARAMS_MISSED;        
      } // if
      else
      {
        String t = command.GetArg(0); // получили команду
        t.toUpperCase();
        
      #ifdef USE_REMOTE_MODULES 
      if(t == ADD_COMMAND) // запросили регистрацию нового модуля
       {
          // ищем уже зарегистрированный
          String reqID = command.GetArg(1);
          AbstractModule* mod = c->GetModuleByID(reqID);
          if(mod)
          {
            // модуль уже зарегистрирован
            PublishSingleton = REG_ERR; 
            PublishSingleton << PARAM_DELIMITER << reqID;
          } // if
          else
          {
            // регистрируем новый модуль
            RemoteModule* remMod = new RemoteModule(reqID); 
            c->RegisterModule(remMod);
            PublishSingleton.Status = true;
            PublishSingleton = REG_SUCC; 
            PublishSingleton << PARAM_DELIMITER << reqID;

          } // else
       }
       
       else 
       #endif
       if(t == SMS_NUMBER_COMMAND) // номер телефона для управления по SMS
       {
          GlobalSettings* sett = mainController->GetSettings();
          sett->SetSmsPhoneNumber(command.GetArg(1));
          sett->Save();
          PublishSingleton.Status = true;
          PublishSingleton = SMS_NUMBER_COMMAND; 
          PublishSingleton << PARAM_DELIMITER << REG_SUCC;
          
       }
       #ifdef USE_DS3231_REALTIME_CLOCK
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

            
             DS3231Clock cl = mainController->GetClock();
             cl.setTime(sec.toInt(),minute.toInt(),hour.toInt(),dow,dayint,monthint,yearint);

             PublishSingleton.Status = true;
             PublishSingleton = REG_SUCC;
         } // if
       }
       #endif
       
       else if(t == PROPERTIES_COMMAND)
       {
        // запросили установку свойств
          String reqID = command.GetArg(1);
          AbstractModule* mod = mainController->GetModuleByID(reqID);
          if(mod)
          {
                  if(argsCnt < 4)
                  {
                    // мало параметров
                    PublishSingleton = PARAMS_MISSED;
        
                  } // if
                  else
                  {
                    String propName = command.GetArg(2); // имя свойства
                    propName.toUpperCase();
                    
                    if(propName == PROP_TEMP_CNT) // кол-во датчиков температуры
                    {
                       // ПРИМЕР:
                      // CTSET=0|PROP|M|TEMP_CNT|2
                      uint8_t tempCnt = String(command.GetArg(3)).toInt();

                      //mod->State.SetTempSensors(tempCnt);
                      for(uint8_t toAdd = 0; toAdd < tempCnt; toAdd++)
                        mod->State.AddState(StateTemperature,toAdd);
                      
                      PublishSingleton.Status = true;
                      PublishSingleton = REG_SUCC;
                    }
                    else if(propName == PROP_RELAY_CNT) // кол-во каналов реле
                    {
                      #ifdef SAVE_RELAY_STATES
                      
                       // ПРИМЕР:
                      // CTSET=0|PROP|M|RELAY_CNT|5
                      uint8_t relayCnt = String(command.GetArg(3)).toInt();

                      uint8_t channelsCnt = relayCnt/8;
                      if(relayCnt > 8 && relayCnt % 8)
                        channelsCnt++;

                      if(relayCnt < 9)
                        channelsCnt = 1;
                        

                      for(uint8_t k = 0;k<channelsCnt;k++)
                        mod->State.AddState(StateRelay,k);

                      #endif

                      PublishSingleton.Status = true;
                      PublishSingleton = REG_SUCC;
                      
                    } // else if
                    else if(propName == PROP_TEMP) // передали температуру
                    {
                      // CTSET=0|PROP|M|TEMP|0|36,6 - пример
                         if(argsCnt < 5)
                         {
                          
                            // мало параметров
                            PublishSingleton = PARAMS_MISSED;
                         }
                         else
                         {
                           // сохраняем температуру от датчика
                           String curTemp = command.GetArg(4);
                           uint8_t sensorIdx = String(command.GetArg(3)).toInt();

                           Temperature t;
                           t.Value = curTemp.toInt();
                           int8_t idx = curTemp.indexOf(F(","));
                           if(idx != -1)
                           {
                              curTemp = curTemp.substring(idx+1);
                              t.Fract = curTemp.toInt();
                           }

                           mod->State.UpdateState(StateTemperature,sensorIdx,(void*)&t); // пытаемся обновить показания
 
                            PublishSingleton.Status = true;
                            PublishSingleton = REG_SUCC;
                           
                         } // else
                    } // else if
                    else if(propName == PROP_RELAY) // передали состояние реле
                    {
                      // CTSET=0|PROP|M|RELAY|0|ON - пример
                         if(argsCnt < 5)
                         {
                          
                            // мало параметров
                            PublishSingleton = PARAMS_MISSED;
                         }
                         else
                         {
                          #ifdef SAVE_RELAY_STATES
                           // сохраняем состояние реле
                           String curRelayState = command.GetArg(4);
                           uint8_t relayIdx = String(command.GetArg(3)).toInt();

                           uint8_t channelIdx = relayIdx/8;
                           uint8_t bitNum = relayIdx % 8;
                            
                           OneState* os = mod->State.GetState(StateRelay,channelIdx);
                           if(os)
                           {
                            RelayPair rp = *os;
                            uint8_t curRState = rp.Current;
                            bitWrite(curRState,bitNum,(curRelayState == STATE_ON));
                            os->Update((void*) &curRState);
                           } 
                           #endif
                           
                            PublishSingleton.Status = true;
                            PublishSingleton = REG_SUCC;
                           
                         } // else
                    } // else if
                    else
                    {
                    PublishSingleton = UNKNOWN_PROPERTY;
                    }
                  }
          } // if mod
          else
          {
            // модуль неизвестен
            PublishSingleton = UNKNOWN_MODULE;
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
 if(canPublish) // можем публиковать
  mainController->Publish(this,command);
 else
  PublishSingleton = F(""); // просто очищаем общий буфер
    
  return PublishSingleton.Status;
}

