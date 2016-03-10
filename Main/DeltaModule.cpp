#include "DeltaModule.h"
#include "ModuleController.h"

DeltaModule* DeltaModule::_thisDeltaModule = NULL; // указатель на экземпляр класса


void DeltaModule::OnDeltaSetCount(uint8_t& count)
{ 
  // нам передали кол-во сохранённых в EEPROM дельт
  #ifdef _DEBUG
  Serial.print(F("Total saved deltas: "));
  Serial.println(count);
  #endif

  UNUSED(count);
  
}
void DeltaModule::OnDeltaRead(uint8_t& _sensorType, String& moduleName1,uint8_t& sensorIdx1, String& moduleName2, uint8_t& sensorIdx2)
{
  // нам передали прочитанные из EEPROM данные одной дельты
  // вызываем yield, поскольку чтение из EEPROM занимает время.
  yield();

  ModuleStates sensorType = (ModuleStates) _sensorType; // приводим тип

  // сначала получаем два модуля, они должны быть уже зарегистрированы, поскольку мы инициализируем дельты в методе Update, который вызывается уже в loop().
  DeltaSettings ds;
  ds.Module1 = DeltaModule::_thisDeltaModule->mainController->GetModuleByID(moduleName1);
  ds.Module2 = DeltaModule::_thisDeltaModule->mainController->GetModuleByID(moduleName2);

  // проверяем, всё ли мы получили правильно
  if(!(ds.Module1 && ds.Module2))
  {
  #ifdef _DEBUG
  Serial.println(F("One of delta modules is inaccessible!"));
  #endif 
  return;
  }

  // теперь проверяем, не указывает ли какое-либо имя модуля на нас.
  if(ds.Module1 == DeltaModule::_thisDeltaModule || ds.Module2 == DeltaModule::_thisDeltaModule)
  {
  #ifdef _DEBUG
  Serial.println(F("One of delta modules is linked to DELTA!"));
  #endif 
    return;
  }

  //теперь проверяем, есть ли у обеих модулей датчики указанного типа
 if(!(ds.Module1->State.HasState(sensorType) && ds.Module2->State.HasState(sensorType)))
 {
  #ifdef _DEBUG
  Serial.print(F("One of delta modules has no state #"));
  Serial.println(sensorType);
  #endif 
  return;
 }

  //теперь проверяем, правильные ли индексы датчиков переданы
 if(!(ds.Module1->State.GetStateCount(sensorType) > sensorIdx1 && ds.Module2->State.GetStateCount(sensorType) > sensorIdx2))
 {
  #ifdef _DEBUG
  Serial.println(F("One of sensors indicies is wrong!"));
  #endif 
  return;
 } 

 // если мы здесь, значит - оба модуля есть в системе, ни один из модулей не указывает на нас, и у каждого из модулей есть датчики определённого типа,
 // плюс индексы назначены правильно.
 // следовательно - мы можем сохранять структуру в векторе.
 ds.SensorType = sensorType;
 ds.SensorIndex1 = sensorIdx1;
 ds.SensorIndex2 = sensorIdx2;

 // теперь не забываем добавить своё внутреннее состояние, которое будет дёргать модуль ALERT, получая показания
 DeltaModule::_thisDeltaModule->State.AddState(sensorType,DeltaModule::_thisDeltaModule->deltas.size()); // индексом виртуального датчика будет размер массива, т.е. автоматически увеличиваться с каждой новой настройкой.

 // записывать в состояние пока ничего не надо, поскольку State сам инициализирует всё значениями по умолчанию.

 // сохраняем настройки в структуру
 DeltaModule::_thisDeltaModule->deltas.push_back(ds);
 
  #ifdef _DEBUG
  Serial.println(F("Delta settings successfully added."));
  #endif 
}
void DeltaModule::OnDeltaGetCount(uint8_t& count)
{
  #ifdef _DEBUG
  Serial.print(F("Requested to write deltas: "));
  Serial.println(DeltaModule::_thisDeltaModule->deltas.size());
  #endif   
  // у нас запросили - сколько установок дельт писать в EEPROM
  count = (uint8_t) DeltaModule::_thisDeltaModule->deltas.size();
  
}
void DeltaModule::OnDeltaWrite(uint8_t& sensorType, String& moduleName1,uint8_t& sensorIdx1, String& moduleName2, uint8_t& sensorIdx2)
{
  // мы передаём данные очередной дельты
  // вызываем yield, поскольку запись в EEPROM занимает время.
  yield();
  
  #ifdef _DEBUG
  Serial.print(F("Store the delta settings #"));
  Serial.println(DeltaModule::_thisDeltaModule->deltaReadIndex);
  #endif

  // получили указатель на структуру
  DeltaSettings* ds = &(DeltaModule::_thisDeltaModule->deltas[DeltaModule::_thisDeltaModule->deltaReadIndex]);
  // передаём её значения
  sensorType = ds->SensorType;
  moduleName1 = ds->Module1->GetID();
  sensorIdx1 = ds->SensorIndex1;
  moduleName2 = ds->Module2->GetID();
  sensorIdx2 = ds->SensorIndex2;

  // передали, увеличили указатель чтения
  DeltaModule::_thisDeltaModule->deltaReadIndex++;

  #ifdef _DEBUG
  Serial.print(F("Delta settings #"));
  Serial.print(DeltaModule::_thisDeltaModule->deltaReadIndex);
  Serial.println(F(" stored."));
  #endif  
}
void DeltaModule::Setup()
{
  // настройка модуля тут
  isDeltasInited = false;
  settings = mainController->GetSettings();
}
void DeltaModule::SaveDeltas()
{
  // сохраняем дельты в EEPROM
  deltaReadIndex = 0;
  #ifdef _DEBUG
  Serial.println(F("Save delta settings..."));
  #endif

  DeltaModule::_thisDeltaModule = this; // сохраняем указатель на себя

  // читаем данные из EEPROM
  settings->WriteDeltaSettings(OnDeltaGetCount, OnDeltaWrite);

  #ifdef _DEBUG
  Serial.println(F("Delta settings saved."));
  #endif    
}
void DeltaModule::Update(uint16_t dt)
{ 

  // обновление модуля тут
  if(!isDeltasInited)
  {
    // инициализируем дельты здесь, поскольку при вызове Setup настройки уже загружены, но наш модуль ещё не зарегистрирован в контроллере
    isDeltasInited = true;
    InitDeltas();
  }

  lastUpdateCall += dt;
  if(lastUpdateCall < DELTA_UPDATE_INTERVAL) // обновляем согласно настроенному интервалу
    return;
  else
    lastUpdateCall = 0;
  
  UpdateDeltas(); // обновляем дельты

}

void DeltaModule::UpdateDeltas()
{
  // обновляем дельты тут. Проходим по всем элементам массива, смотрим, чего там лежит, получаем показания с нужных датчиков - и сохраняем дельты у себя.
  size_t cnt = deltas.size();
  for(size_t i=0;i<cnt;i++)
  {
    DeltaSettings* ds = &(deltas[i]);
    // получили первую настройку дельты, работаем с ней

    // получаем значения двух датчиков

    // первого...
    OneState* os1 = ds->Module1->State.GetState((ModuleStates)ds->SensorType,ds->SensorIndex1);
    #ifdef _DEBUG
    if(!os1)
    {
      Serial.println(F("[ERR] os1 == NULL!"));
      continue;
    }
    #endif

    // и второго
    OneState* os2 = ds->Module2->State.GetState((ModuleStates)ds->SensorType,ds->SensorIndex2);

    #ifdef _DEBUG
    if(!os2)
    {
      Serial.println(F("[ERR] os2 == NULL!"));
      continue;
    }
    #endif

    OneState* deltaState = State.GetState((ModuleStates)ds->SensorType,i); // получаем наше состояние
    #ifdef _DEBUG
    if(!deltaState)
    {
      Serial.println(F("[ERR] deltaState == NULL!"));
      continue;
    }

      // выводим предыдущее значение.
      Serial.print(F("\r\nPrevious deltaState = "));
      Serial.println((String)*deltaState);
      
    #endif
    // и сохраняем в него дельту, индекс при этом должен остаться нетронутым
    if(deltaState && os1 && os2)
      *deltaState = (*os1 - *os2);

    #ifdef _DEBUG

      // протестируем, чего он нам там в виде дельты вывел.
      Serial.print(F("Current deltaState = "));
      Serial.println((String)*deltaState);

      if(deltaState->IsChanged())
      {
       // есть изменения дельты - тестируем для модуля ALERT.
       Serial.println(F("Delta state changed!"));
      }
      

    #endif // _DEBUG
    
  } // for
  
}
void DeltaModule::InitDeltas()
{
  // загружаем дельты из EEPROM
  deltas.Clear();
  #ifdef _DEBUG
  Serial.println(F("Read delta settings..."));
  #endif

  DeltaModule::_thisDeltaModule = this; // сохраняем указатель на себя

  // читаем данные из EEPROM
  settings->ReadDeltaSettings(OnDeltaSetCount, OnDeltaRead);

  #ifdef _DEBUG
/*
  // добавляем свою тестовую дельту
  // читаем из модуля State температуру с DS18B20.
  // читаем из модуля HUMIDITY температуру с DHT22.
  // индексы датчиков и там, и там - 0.
  // запоминаем в свою дельту.
  uint8_t sensorType = StateTemperature;
  String moduleName1 = F("STATE");
  uint8_t sensorIdx1 = 0;
  String moduleName2 = F("HUMIDITY");
  uint8_t sensorIdx2 = 0;
  
  // тупо вызываем функцию, чтобы не париться с настройками
  OnDeltaRead(sensorType, moduleName1,sensorIdx1, moduleName2, sensorIdx2);
*/  
  Serial.println(F("Delta settings readed."));
  #endif
    
}

bool  DeltaModule::ExecCommand(const Command& command, bool wantAnswer)
{
  if(wantAnswer)
    PublishSingleton = UNKNOWN_COMMAND;

  size_t argsCount = command.GetArgsCount();
    
  if(command.GetType() == ctGET)
  {
    if(!argsCount)
    {
      if(wantAnswer)
        PublishSingleton = PARAMS_MISSED;
      
    } // !argsCount
    else
    {
      String arg = command.GetArg(0);
      if(arg == DELTA_COUNT_COMMAND) // получить кол-во дельт, CTGET=DELTA|CNT
      {
        if(wantAnswer)
        {
          PublishSingleton.Status = true;
          PublishSingleton = DELTA_COUNT_COMMAND;
          PublishSingleton << PARAM_DELIMITER << deltas.size();
        }
        
      } // DELTA_COUNT_COMMAND
      else
      if(arg == DELTA_VIEW_COMMAND) // просмотр дельты по индексу, CTGET=DELTA|VIEW|0
      {
         if(argsCount < 2)
         {
          if(wantAnswer)
            PublishSingleton = PARAMS_MISSED;
         } // argsCount < 2
         else
         {
           arg = command.GetArg(1);
           size_t deltaIdx = arg.toInt();
           if(deltaIdx >= deltas.size())
           {
            if(wantAnswer)
              PublishSingleton = PARAMS_MISSED;
              
           } // bad index
           else
           {
            if(wantAnswer)
            {
              PublishSingleton.Status = true;
              PublishSingleton = DELTA_VIEW_COMMAND;
              PublishSingleton << PARAM_DELIMITER << deltaIdx << PARAM_DELIMITER;

              DeltaSettings* ds = &(deltas[deltaIdx]);
              
              String tp; // тип датчика
              switch(ds->SensorType)
              {
                case StateTemperature: tp = PROP_TEMP; break;
                case StateHumidity: tp = PROP_HUMIDITY; break;
                case StateLuminosity: tp = PROP_LIGHT; break;
              }
              
              PublishSingleton << tp << PARAM_DELIMITER << (ds->Module1->GetID()) << PARAM_DELIMITER << ds->SensorIndex1
              << PARAM_DELIMITER << (ds->Module2->GetID()) << PARAM_DELIMITER << ds->SensorIndex2;
              
            } // wantAnswer
           } // else good index
         } // else
      } // DELTA_VIEW_COMMAND
      
    } // have args
    
  } // GET
  else
  if(command.GetType() == ctSET)
  {
    if(!argsCount)
    {
      if(wantAnswer)
        PublishSingleton = PARAMS_MISSED;
    } // !argsCount
    else
    {
       String arg = command.GetArg(0);
       
       if(arg == DELTA_SAVE_COMMAND) // сохранить все настройки дельт в EEPROM, CTSET=DELTA|SAVE
       {
          if(wantAnswer)
          {
            PublishSingleton.Status = true;
            PublishSingleton = DELTA_SAVE_COMMAND;
            PublishSingleton << PARAM_DELIMITER << REG_SUCC;
          }

          SaveDeltas(); // сохраняем дельты
       } // DELTA_SAVE_COMMAND
       else
       if(arg == DELTA_DELETE_COMMAND) // удалить все дельты, CTSET=DELTA|DEL
       {
          if(wantAnswer)
          {
            PublishSingleton.Status = true;
            PublishSingleton = DELTA_DELETE_COMMAND;
            PublishSingleton << PARAM_DELIMITER << REG_SUCC;
          }

          // убираем свои состояния, чтобы модуль ALERT не дёргал там, где ничего нет
          size_t cnt = deltas.size();
          for(size_t i=0;i<cnt;i++)
          {
            DeltaSettings* ds = &(deltas[i]);
            State.RemoveState((ModuleStates)ds->SensorType,i); // просим класс состояний удалить состояние
          } // for

          deltas.Clear(); // чистим дельты
          SaveDeltas(); // сохраняем дельты
        
       } // DELTA_DELETE_COMMAND
       else
       if(arg = DELTA_ADD_COMMAND) // добавить дельту, CTSET=DELTA|ADD|SensorType|ModuleName1|SensorIndex1|ModuleName2|SensorIndex2
       {
          if(argsCount < 6)
          {
            if(wantAnswer)
            {
              PublishSingleton = PARAMS_MISSED;
            }
          } // argsCount < 6
          else
          {
            // парсим аргументы
            DeltaSettings ds; // сюда будем сохранять
            uint8_t readIdx = 1;
            arg = command.GetArg(readIdx++); // читаем тип сенсора
            ds.SensorType = 0;

            // парсим тип датчика
            if(arg == PROP_TEMP)
              ds.SensorType = StateTemperature; // температурная дельта
            else
            if(arg == PROP_HUMIDITY)
              ds.SensorType = StateHumidity; // дельта влажности
            else
            if(arg == PROP_LIGHT)
              ds.SensorType = StateLuminosity; // дельта освещенности

            String moduleName1 = command.GetArg(readIdx++); // читаем имя первого модуля
            arg = command.GetArg(readIdx++); // читаем индекс первого датчика
            ds.SensorIndex1 = arg.toInt();
            
            String moduleName2 = command.GetArg(readIdx++); // читаем имя второго модуля
            arg = command.GetArg(readIdx++); // читаем индекс второго датчика
            ds.SensorIndex2 = arg.toInt();

            ds.Module1 = mainController->GetModuleByID(moduleName1);
            ds.Module2 = mainController->GetModuleByID(moduleName2);

            // проверяем все параметры
            if(!ds.SensorType || // если тип датчика не задан
            !(ds.Module1 && ds.Module2) || // или один из модулей не найден
            ds.Module1 == this || ds.Module2 == this || // или любой из них ссылается на нас
            !ds.Module1->State.HasState((ModuleStates)ds.SensorType) || // или у первого нет нужного типа датчика
            !ds.Module2->State.HasState((ModuleStates)ds.SensorType) || // или у второго нет нужного типа датчика
            ds.SensorIndex1 >= ds.Module1->State.GetStateCount((ModuleStates)ds.SensorType) || // или переданный индекс первого датчика неправильный
            ds.SensorIndex2 >= ds.Module2->State.GetStateCount((ModuleStates)ds.SensorType) // или переданный индекс второго датчика неправильный
            )
            {
              // чего-то пошло не так
              if(wantAnswer)
                PublishSingleton = PARAMS_MISSED;
            }
            else // иначе - всё зашибись, и мы можем добавлять дельту
            {
              // можем добавлять дельту, сохранением занимается команда SAVE.
              // добавляем своё состояние
              State.AddState((ModuleStates)ds.SensorType,deltas.size());
              // теперь сохраняем структуру в вектор.
              deltas.push_back(ds);
              
              if(wantAnswer)
              {
                PublishSingleton.Status = true;
                PublishSingleton = DELTA_ADD_COMMAND;
                PublishSingleton << PARAM_DELIMITER << REG_SUCC << PARAM_DELIMITER << (deltas.size() - 1);
              } // wantAnswer
            } // good params
            
          } // enough args
       } // DELTA_ADD_COMMAND
    } // have args
      
  } // SET


  mainController->Publish(this,command);
  return true;
}
