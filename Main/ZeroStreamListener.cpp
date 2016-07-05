#include "ZeroStreamListener.h"
#include "ModuleController.h"
#ifdef USE_REMOTE_MODULES
#include "RemoteModule.h"
#endif

#include "UniversalSensors.h"
#include "InteropStream.h"

#ifdef USE_UNIVERSAL_SENSORS

  #ifdef UNI_USE_REGISTRATION_LINE
    UniRegistrationLine uniRegistrator(UNI_REGISTRATION_PIN);
  #endif // UNI_USE_REGISTRATION_LINE
  
  #if UNI_WIRED_MODULES_COUNT > 0
    UniPermanentLine uniWiredModules[UNI_WIRED_MODULES_COUNT] = { UNI_WIRED_MODULES };
  #endif

#endif // USE_UNIVERSAL_SENSORS

void(* resetFunc) (void) = 0;

void ZeroStreamListener::Setup()
{
  // настройка модуля тут
 }

void ZeroStreamListener::Update(uint16_t dt)
{
#ifdef USE_UNIVERSAL_SENSORS


  #if UNI_WIRED_MODULES_COUNT > 0
    for(uint8_t i=0;i<UNI_WIRED_MODULES_COUNT;i++)
      uniWiredModules[i].Update(dt);
  #endif
  
#endif // USE_UNIVERSAL_SENSORS

  UNUSED(dt);
  // обновление модуля тут

}

void ZeroStreamListener::PrintSensorsValues(uint8_t totalCount,ModuleStates wantedState,AbstractModule* module, Stream* outStream)
{
  if(!totalCount) // нечего писать
    return;

  // буфер под сырые данные, у нас максимум 4 байта на показание с датчика
  static uint8_t raw_data[sizeof(unsigned long)] = {0};
  const char* noDataByte = "FF"; // байт - нет данных с датчика

  // пишем количество датчиков
  outStream->write(WorkStatus::ToHex(totalCount));

  for(uint8_t cntr=0;cntr<totalCount;cntr++)
  {
    yield(); // немного даём поработать другим модулям
    
    // получаем нужное состояние
    OneState* os = module->State.GetStateByOrder(wantedState,cntr);
    
    // потом идут пакеты данных, каждый пакет состоит из:
    // 1 байт - индекс датчика
    outStream->write(WorkStatus::ToHex(os->GetIndex()));

    // N байт - его показания, мы пишем любые показания, даже если датчика нет на линии

    // копируем сырые данные
    uint8_t rawDataSize = os->GetRawData(raw_data);

    // сырые данные идут от младшего байта к старшему, но их надо слать
    // старшим байтом вперёд.
    
      if(os->HasData())
      {
        do
        {
          rawDataSize--;

          outStream->write(WorkStatus::ToHex(raw_data[rawDataSize]));
          
        } while(rawDataSize > 0);
        
      } // if
      else
      {
        // датчика нет на линии, пишем FF столько раз, сколько байт сырых данных мы получили
        for(uint8_t i=0;i<rawDataSize;i++)
          outStream->write(noDataByte);

      }    
  } // for
  
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
        #if defined(USE_UNIVERSAL_SENSORS) && defined(UNI_USE_REGISTRATION_LINE)
        else
        if(t == UNI_SEARCH) // поиск универсального модуля на линии регистрации
        {
          PublishSingleton.AddModuleIDToAnswer = false;
          
          if(uniRegistrator.IsModulePresent())
          {
            // датчик найден, отправляем его внутреннее состояние
            PublishSingleton.Status = true;

            UniRawScratchpad scratch;
            uniRegistrator.CopyScratchpad(&scratch);
            byte* raw = (byte*) &scratch;
            
            PublishSingleton = "";
            
            // теперь пишем весь скратчпад вызывающему, пущай сам разбирается, как с ним быть
            for(byte i=0;i<sizeof(UniRawScratchpad);i++)
            {
              PublishSingleton << WorkStatus::ToHex(raw[i]);
            } // for

          } // if
          else
          {
            // датчика нету
            PublishSingleton = UNI_NOT_FOUND;
          } // else
        }
        #endif // UNI_USE_REGISTRATION_LINE
        else
        if(t == ID_COMMAND)
        {
          PublishSingleton.Status = true;
          PublishSingleton.AddModuleIDToAnswer = false;
          PublishSingleton = ID_COMMAND; 
          PublishSingleton << PARAM_DELIMITER << MainController->GetSettings()->GetControllerID();
        }
        else
        if(t == WIRED_COMMAND) // получить количество жёстко указанных в прошивке обычных датчиков
        {
          PublishSingleton.Status = true;
          PublishSingleton.AddModuleIDToAnswer = false;
          PublishSingleton = WIRED_COMMAND;

          PublishSingleton << PARAM_DELIMITER << UniDispatcher.GetHardCodedSensorsCount(uniTemp);
          PublishSingleton << PARAM_DELIMITER << UniDispatcher.GetHardCodedSensorsCount(uniHumidity);
          PublishSingleton << PARAM_DELIMITER << UniDispatcher.GetHardCodedSensorsCount(uniLuminosity);
          PublishSingleton << PARAM_DELIMITER << UniDispatcher.GetHardCodedSensorsCount(uniSoilMoisture);
          PublishSingleton << PARAM_DELIMITER << UniDispatcher.GetHardCodedSensorsCount(uniPH);
          //TODO: Тут остальные типы датчиков указывать !!!
                     
        }
        else
        if(t == UNI_COUNT_COMMAND) // получить количество зарегистрированных универсальных датчиков
        {
          PublishSingleton.Status = true;
          PublishSingleton.AddModuleIDToAnswer = false;
          PublishSingleton = UNI_COUNT_COMMAND;

          PublishSingleton << PARAM_DELIMITER << UniDispatcher.GetUniSensorsCount(uniTemp);
          PublishSingleton << PARAM_DELIMITER << UniDispatcher.GetUniSensorsCount(uniHumidity);
          PublishSingleton << PARAM_DELIMITER << UniDispatcher.GetUniSensorsCount(uniLuminosity);
          PublishSingleton << PARAM_DELIMITER << UniDispatcher.GetUniSensorsCount(uniSoilMoisture);
          PublishSingleton << PARAM_DELIMITER << UniDispatcher.GetUniSensorsCount(uniPH);
          //TODO: Тут остальные типы датчиков указывать !!!
                     
        }

        
        else
        if(t == SMS_NUMBER_COMMAND) // номер телефона для управления по СМС
        {
          PublishSingleton.Status = true;
          PublishSingleton.AddModuleIDToAnswer = false;
          PublishSingleton = SMS_NUMBER_COMMAND; 
          PublishSingleton << PARAM_DELIMITER << MainController->GetSettings()->GetSmsPhoneNumber();
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

            size_t modulesCount = MainController->GetModulesCount(); // получаем кол-во зарегистрированных модулей

          //  const char* noDataByte = "FF"; // байт - нет данных с датчика

            // пробегаем по всем модулям
            String moduleName;
            moduleName.reserve(20);
            
            for(size_t i=0;i<modulesCount;i++)
            {
              yield(); // немного даём поработать другим модулям

              AbstractModule* mod = MainController->GetModule(i);
              if(mod == this) // себя пропускаем
                continue;

              // проверяем, не пустой ли модуль. для этого смотрим, сколько у него датчиков вообще
              uint8_t tempCount = mod->State.GetStateCount(StateTemperature);
              uint8_t humCount = mod->State.GetStateCount(StateHumidity);
              uint8_t lightCount = mod->State.GetStateCount(StateLuminosity);
              uint8_t waterflowCountInstant = mod->State.GetStateCount(StateWaterFlowInstant);
              uint8_t waterflowCount = mod->State.GetStateCount(StateWaterFlowIncremental);
              uint8_t soilMoistureCount = mod->State.GetStateCount(StateSoilMoisture); 
              uint8_t phCount = mod->State.GetStateCount(StatePH); 
              
              //TODO: тут другие типы датчиков!!!

              if((tempCount + humCount + lightCount + waterflowCountInstant + waterflowCount + soilMoistureCount + phCount) < 1) // пустой модуль, без интересующих нас датчиков
                continue;

              uint8_t flags = 0;
              if(tempCount) flags |= StateTemperature;
              if(humCount) flags |= StateHumidity;
              if(lightCount) flags |= StateLuminosity;
              if(waterflowCountInstant) flags |= StateWaterFlowInstant;
              if(waterflowCount) flags |= StateWaterFlowIncremental;
              if(soilMoistureCount) flags |= StateSoilMoisture;
              if(phCount) flags |= StatePH;
              //TODO: Тут другие типы датчиков!!!

            // показание каждого модуля идут так:
            
            // 1 байт - флаги о том, какие датчики есть
             pStream->write(WorkStatus::ToHex(flags));
            
            // 1 байт - длина ID модуля
              moduleName = mod->GetID();
              uint8_t mnamelen = moduleName.length();
              pStream->write(WorkStatus::ToHex(mnamelen));
            // далее идёт имя модуля
              pStream->write(moduleName.c_str());
            
            
              // затем идут данные из модуля, сначала - показания температуры, если они есть
              PrintSensorsValues(tempCount,StateTemperature,mod,pStream);
              // затем идёт кол-во датчиков влажности, если они есть
              PrintSensorsValues(humCount,StateHumidity,mod,pStream);
              // затем идут показания датчиков освещенности, если они есть
              PrintSensorsValues(lightCount,StateLuminosity,mod,pStream);
              // затем идут моментальные показания датчиков расхода воды, если они есть
              PrintSensorsValues(waterflowCountInstant,StateWaterFlowInstant,mod,pStream);
              // затем идут накопительные показания датчиков расхода воды, если они есть
              PrintSensorsValues(waterflowCount,StateWaterFlowIncremental,mod,pStream);
              // затем идут датчики влажности почвы, если они есть
              PrintSensorsValues(soilMoistureCount,StateSoilMoisture,mod,pStream);
              // затем идут датчики pH, если они есть
              PrintSensorsValues(phCount,StatePH,mod,pStream);
            
              //TODO: тут другие типы датчиков!!!

            } // for
            

            pStream->print(NEWLINE); // пишем перевод строки
            
          } // wantAnswer
          
        } // STATUS_COMMAND     
        else if(t == REGISTERED_MODULES_COMMAND) // пролистать зарегистрированные модули
        {
          PublishSingleton.AddModuleIDToAnswer = false;
          PublishSingleton.Status = true;
          PublishSingleton = F("");
          size_t cnt = MainController->GetModulesCount();
          for(size_t i=0;i<cnt;i++)
          {
            AbstractModule* mod = MainController->GetModule(i);

            if(mod != this)
            {
              if(PublishSingleton.Text.length())
                PublishSingleton << PARAM_DELIMITER;
              
              PublishSingleton << mod->GetID();
             
            }// if
              
          } // for
        }
        else
        {
            // неизвестная команда
        } // else
        
          
      } // else
    } // elsse
    
  } // ctGET
  else
  if(command.GetType() == ctSET)
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
        String t = command.GetArg(0);    

        if(t == RESET_COMMAND)
        {
          resetFunc(); // ресетимся, писать в ответ ничего не надо
        } // RESET_COMMAND
        else
        if(t == F("AUTO")) // CTSET=0|AUTO - перевести в автоматический режим
        {
          // очищаем общий буфер ответов
          PublishSingleton = "";

          // выполняем команды
          ModuleInterop.QueryCommand(ctSET, F("STATE|MODE|AUTO"),false,false);
          ModuleInterop.QueryCommand(ctSET, F("WATER|MODE|AUTO"),false,false);
          ModuleInterop.QueryCommand(ctSET, F("LIGHT|MODE|AUTO"),false,false);

          // говорим, что выполнили
          PublishSingleton = REG_SUCC;
          PublishSingleton.Status = true;
        
        } // AUTO
                
      } // if
      else
      {
        String t = command.GetArg(0); // получили команду
        
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
          GlobalSettings* sett = MainController->GetSettings();
          sett->SetSmsPhoneNumber(command.GetArg(1));
          sett->Save();
          PublishSingleton.Status = true;
          PublishSingleton = SMS_NUMBER_COMMAND; 
          PublishSingleton << PARAM_DELIMITER << REG_SUCC;
          
       }
        #if defined(USE_UNIVERSAL_SENSORS) && defined(UNI_USE_REGISTRATION_LINE)
        else
        if(t == UNI_REGISTER) // зарегистрировать универсальный модуль, висящий на линии
        {
          PublishSingleton.AddModuleIDToAnswer = false;

              if(uniRegistrator.IsModulePresent())
              {
                // модуль есть на линии, регистрируем его в системе.
                // сначала вычитываем переданный скратчпад и назначаем его модулю.
                // считаем, что на вызывающей стороне разобрались, что с чем, с остальным
                // разберётся модуль регистрации.
                const char* scratchData = command.GetArg(1);
                // теперь конвертируем данные скратчпада из текстового представления в нормальное
                char buff[3] = {0};
                uint8_t len = strlen(scratchData);

                UniRawScratchpad scratch;
                byte* raw = (byte* )&scratch;

                for(uint8_t i=0;i<len;i+=2)
                {
                  buff[0] = scratchData[i];
                  buff[1] = scratchData[i+1];
                  *raw = WorkStatus::FromHex(buff);
                  raw++;
                } // for
                
                if(uniRegistrator.SetScratchpadData(&scratch))
                {
                  uniRegistrator.Register();
                              
                  PublishSingleton.Status = true;
                  PublishSingleton = REG_SUCC;
                } // if
                else
                {
                   // разные типы скратчпадов, возможно, подсоединили другой модуль
                   PublishSingleton = UNI_DIFFERENT_SCRATCHPAD;
                }
                
              } // if
              else
              {
                // модуля нет на линии
                PublishSingleton = UNI_NOT_FOUND;
              }
              
          
        } // UNI_REGISTER
        #endif // UNI_USE_REGISTRATION_LINE
       
       else if(t == ID_COMMAND)
       {
          //String newID = command.GetArg(1);
          MainController->GetSettings()->SetControllerID((uint8_t)atoi(command.GetArg(1)));
          PublishSingleton.Status = true;
          PublishSingleton = ID_COMMAND; 
          PublishSingleton << PARAM_DELIMITER << REG_SUCC;
        
       }       
       #ifdef USE_DS3231_REALTIME_CLOCK
       else if(t == SETTIME_COMMAND)
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

            
             DS3231Clock cl = MainController->GetClock();
             cl.setTime(sec.toInt(),minute.toInt(),hour.toInt(),dow,dayint,monthint,yearint);

             PublishSingleton.Status = true;
             PublishSingleton = REG_SUCC;
         } // if
       }
       #endif
       else
       {
         // неизвестная команда
       } // else
      } // else argsCount > 1
    } // else
    
  } // if
 
 // отвечаем на команду
 if(canPublish) // можем публиковать
  MainController->Publish(this,command);
 else
  PublishSingleton = F(""); // просто очищаем общий буфер
    
  return PublishSingleton.Status;
}

