#include "UniversalSensors.h"
#include <OneWire.h>
#include <EEPROM.h>
#include "InteropStream.h"
//-------------------------------------------------------------------------------------------------------------------------------------------------------
UniRegDispatcher UniDispatcher;
UniScratchpadClass UniScratchpad; // наш пишичитай скратчпада
UniClientsFactory UniFactory; // наша фабрика клиентов
UniRawScratchpad SHARED_SCRATCHPAD; // общий скратчпад для классов опроса модулей, висящих на линиях
//-------------------------------------------------------------------------------------------------------------------------------------------------------
#ifdef USE_UNI_NEXTION_MODULE
  UniNextionWaitScreenData UNI_NX_SENSORS_DATA[] = { UNI_NEXTION_WAIT_SCREEN_SENSORS, {0,0,""} };
#endif
//-------------------------------------------------------------------------------------------------------------------------------------------------------
// UniClientsFactory
//-------------------------------------------------------------------------------------------------------------------------------------------------------
UniClientsFactory::UniClientsFactory()
{
  
}
//-------------------------------------------------------------------------------------------------------------------------------------------------------
AbstractUniClient* UniClientsFactory::GetClient(UniRawScratchpad* scratchpad)
{
  if(!scratchpad)
    return &dummyClient;

  UniClientType ct = (UniClientType) scratchpad->head.packet_type;
  
  switch(ct)
  {
    case uniSensorsClient:
      return &sensorsClient;

    case uniNextionClient:
      #ifdef USE_UNI_NEXTION_MODULE
        return &nextionClient;
      #else
      break;
      #endif

    case uniExecutionClient:
    #ifdef USE_UNI_EXECUTION_MODULE
      return &executionClient;
    #else
      break;
    #endif
  }

  return &dummyClient;
}
//-------------------------------------------------------------------------------------------------------------------------------------------------------
#ifdef USE_UNI_EXECUTION_MODULE
//-------------------------------------------------------------------------------------------------------------------------------------------------------
UniExecutionModuleClient::UniExecutionModuleClient()
{
  
}
//-------------------------------------------------------------------------------------------------------------------------------------------------------
void UniExecutionModuleClient::Register(UniRawScratchpad* scratchpad)
{
  // нам регистрироваться в системе дополнительно не надо
  UNUSED(scratchpad);
}
//-------------------------------------------------------------------------------------------------------------------------------------------------------
void UniExecutionModuleClient::Update(UniRawScratchpad* scratchpad, bool isModuleOnline)
{
  if(!isModuleOnline) // когда модуль офлайн - ничего делать не надо
    return;

   // приводим к типу нашего скратча
   UniExecutionModuleScratchpad* ourScratch = (UniExecutionModuleScratchpad*) &(scratchpad->data);

   // получаем состояние контроллера
   ControllerState state = WORK_STATUS.GetState();

   // теперь проходимся по всем слотам
   for(byte i=0;i<8;i++)
   {
      byte slotStatus = 0; // статус слота - 0 по умолчанию
      
      switch(ourScratch->slots[i].slotType)
      {
        case slotEmpty: // пустой слот, ничего делать не надо
        case 0xFF: // если вычитали из EEPROM, а там ничего не было
        break;

        case slotWindowLeftChannel:
        {
          // состояние левого канала окна, в slotLinkedData - номер окна
          byte windowNumber = ourScratch->slots[i].slotLinkedData;
          if(windowNumber < 16)
          {
            // окна у нас нумеруются от 0 до 15, всего 16 окон.
            // на каждое окно - два бита, для левого и правого канала.
            // следовательно, чтобы получить стартовый бит - надо номер окна
            // умножить на 2.
            byte bitNum = windowNumber*2;           
            if(state.WindowsState & (1 << bitNum))
              slotStatus = 1; // выставляем в слоте значение 1
          }
        }
        break;

        case slotWindowRightChannel:
        {
          // состояние левого канала окна, в slotLinkedData - номер окна
          byte windowNumber = ourScratch->slots[i].slotLinkedData;
          if(windowNumber < 16)
          {
            // окна у нас нумеруются от 0 до 15, всего 16 окон.
            // на каждое окно - два бита, для левого и правого канала.
            // следовательно, чтобы получить стартовый бит - надо номер окна
            // умножить на 2.
            byte bitNum = windowNumber*2;

            // поскольку канал у нас правый - его бит идёт следом за левым.
            bitNum++;
                       
            if(state.WindowsState & (1 << bitNum))
              slotStatus = 1; // выставляем в слоте значение 1
          }
        }
        break;

        case slotWateringChannel:
        {
          // состояние канала полива, в slotLinkedData - номер канала полива
          byte wateringChannel = ourScratch->slots[i].slotLinkedData;
          if(wateringChannel< 8)
          {
            if(state.WaterChannelsState & (1 << wateringChannel))
              slotStatus = 1; // выставляем в слоте значение 1
              
          }
        }        
        break;

        case slotLightChannel:
        {
          // состояние канала досветки, в slotLinkedData - номер канала досветки
          byte lightChannel = ourScratch->slots[i].slotLinkedData;
          if(lightChannel < 8)
          {
            if(state.LightChannelsState & (1 << lightChannel))
              slotStatus = 1; // выставляем в слоте значение 1
              
          }
        }
        break;

        case slotPin:
        {
          // получаем статус пина
          byte pinNumber = ourScratch->slots[i].slotLinkedData;
          byte byteNum = pinNumber/8;
          byte bitNum = pinNumber%8;

          if(byteNum < 8)
          {
            // если нужный бит с номером пина установлен - на пине высокий уровень
            if(state.PinsState[byteNum] & (1 << bitNum))
              slotStatus = 1; // выставляем в слоте значение 1
          }
          
        }
        break;
        
      } // switch

      // мы получили slotStatus, записываем его обратно в слот
      ourScratch->slots[i].slotStatus = slotStatus;
   } // for

   // пишем актуальное состояние слотов клиенту
   UniScratchpad.begin(pin,scratchpad);
   UniScratchpad.write();
}
//-------------------------------------------------------------------------------------------------------------------------------------------------------
#endif // USE_UNI_EXECUTION_MODULE
//-------------------------------------------------------------------------------------------------------------------------------------------------------
#ifdef USE_UNI_NEXTION_MODULE
//-------------------------------------------------------------------------------------------------------------------------------------------------------
// NextionUniClient
//-------------------------------------------------------------------------------------------------------------------------------------------------------
NextionUniClient::NextionUniClient()
{
  updateTimer = 0;
  tempChanged = false;
}
//-------------------------------------------------------------------------------------------------------------------------------------------------------
void NextionUniClient::Register(UniRawScratchpad* scratchpad)
{
  UNUSED(scratchpad);
  // нам регистрироваться не надо, ничего не делаем
}
//-------------------------------------------------------------------------------------------------------------------------------------------------------
void NextionUniClient::Update(UniRawScratchpad* scratchpad, bool isModuleOnline)
{
  // тут обновляем данные, полученные с Nextion, и записываем ему текущее состояние
  if(!isModuleOnline) // не надо ничего делать
    return;

  // сначала проверяем, чего там у нас нажато в дисплее
  UniNextionScratchpad ourScratch;
  memcpy(&ourScratch,scratchpad->data,sizeof(UniNextionScratchpad));

  byte changesCount = 0; // кол-во изменений, если оно больше нуля - мы запишем скратч обратно, не дожидаясь наступления интервала обновления

  if(WORK_STATUS.IsModeChanged()) // были изменения в режиме работы
    changesCount++;

  if(bitRead(ourScratch.nextionStatus1,0))
  {
   // Serial.println("close windows");
    bitWrite(ourScratch.nextionStatus1,0,0);
    changesCount++;
    ModuleInterop.QueryCommand(ctSET, F("STATE|WINDOW|ALL|CLOSE"),false,false);  
  }

  if(bitRead(ourScratch.nextionStatus1,1))
  {
  //  Serial.println("open windows");
    bitWrite(ourScratch.nextionStatus1,1, 0);
    changesCount++;
    ModuleInterop.QueryCommand(ctSET, F("STATE|WINDOW|ALL|OPEN"),false,false);  
  }

  if(bitRead(ourScratch.nextionStatus1,2))
  {
 //   Serial.println("windows auto mode");
    bitWrite(ourScratch.nextionStatus1,2, 0);
    changesCount++;
    ModuleInterop.QueryCommand(ctSET, F("STATE|MODE|AUTO"),false,false);  
  }

  if(bitRead(ourScratch.nextionStatus1,3))
  {
 //   Serial.println("windows manual mode");
    bitWrite(ourScratch.nextionStatus1,3,0);
    changesCount++;
    ModuleInterop.QueryCommand(ctSET, F("STATE|MODE|MANUAL"),false,false);  
  }

  if(bitRead(ourScratch.nextionStatus1,4))
  {
  //  Serial.println("water on");
    bitWrite(ourScratch.nextionStatus1,4,0);
    changesCount++;
    ModuleInterop.QueryCommand(ctSET, F("WATER|ON"),false,false);  
  }

  if(bitRead(ourScratch.nextionStatus1,5))
  {
 //   Serial.println("water off");
    bitWrite(ourScratch.nextionStatus1,5, 0);
    changesCount++;
    ModuleInterop.QueryCommand(ctSET, F("WATER|OFF"),false,false);  
  }

  if(bitRead(ourScratch.nextionStatus1,6))
  {
  //  Serial.println("water auto mode");
    bitWrite(ourScratch.nextionStatus1,6,0);
    changesCount++;
    ModuleInterop.QueryCommand(ctSET, F("WATER|MODE|AUTO"),false,false);  
  }

  if(bitRead(ourScratch.nextionStatus1,7))
  {
 //   Serial.println("water manual mode");
    bitWrite(ourScratch.nextionStatus1,7, 0);
    changesCount++;
    ModuleInterop.QueryCommand(ctSET, F("WATER|MODE|MANUAL"),false,false);  
  }

  if(bitRead(ourScratch.nextionStatus2,0))
  {
  //  Serial.println("light on");
    bitWrite(ourScratch.nextionStatus2,0,0);
    changesCount++;
    ModuleInterop.QueryCommand(ctSET, F("LIGHT|ON"),false,false);  
  }

  if(bitRead(ourScratch.nextionStatus2,1))
  {
 //   Serial.println("light off");
    bitWrite(ourScratch.nextionStatus2,1, 0);
    changesCount++;
    ModuleInterop.QueryCommand(ctSET, F("LIGHT|OFF"),false,false);  
  }

  if(bitRead(ourScratch.nextionStatus2,2))
  {
 //   Serial.println("light auto mode");
    bitWrite(ourScratch.nextionStatus2,2, 0);
    changesCount++;
    ModuleInterop.QueryCommand(ctSET, F("LIGHT|MODE|AUTO"),false,false);  
  }

  if(bitRead(ourScratch.nextionStatus2,3))
  {
 //   Serial.println("light manual mode");
    bitWrite(ourScratch.nextionStatus2,3, 0);
    changesCount++;
    ModuleInterop.QueryCommand(ctSET, F("LIGHT|MODE|MANUAL"),false,false);  
  }

  if(bitRead(ourScratch.nextionStatus2,4))
  {
 //   Serial.println("open temp inc");
    bitWrite(ourScratch.nextionStatus2,4, 0);
    changesCount++;
    byte tmp = MainController->GetSettings()->GetOpenTemp();
    if(tmp < 50)
      ++tmp;  

    MainController->GetSettings()->SetOpenTemp(tmp);
    tempChanged = true;
  }

  if(bitRead(ourScratch.nextionStatus2,5))
  {
 //   Serial.println("open temp dec");
    bitWrite(ourScratch.nextionStatus2,5, 0);
    changesCount++;
    byte tmp = MainController->GetSettings()->GetOpenTemp();
    if(tmp > 0)
      --tmp;  

    MainController->GetSettings()->SetOpenTemp(tmp);
    tempChanged = true;
  }

  if(bitRead(ourScratch.nextionStatus2,6))
  {
  //  Serial.println("close temp inc");
    bitWrite(ourScratch.nextionStatus2,6, 0);
    changesCount++;
    byte tmp = MainController->GetSettings()->GetCloseTemp();
    if(tmp < 50)
      ++tmp;  

    MainController->GetSettings()->SetCloseTemp(tmp);
    tempChanged = true;
  }

  if(bitRead(ourScratch.nextionStatus2,7))
  {
 //   Serial.println("close temp dec");
    bitWrite(ourScratch.nextionStatus2,7, 0);
    changesCount++;
    byte tmp = MainController->GetSettings()->GetCloseTemp();
    if(tmp > 0)
      --tmp;  

    MainController->GetSettings()->SetCloseTemp(tmp);
    tempChanged = true;
  }

  if(bitRead(ourScratch.controllerStatus,6)) // дисплей заснул, можно сохранять настройки
  {
    bitWrite(ourScratch.controllerStatus,6,0); 
  //  Serial.println("enter sleep");
    if(tempChanged)
      MainController->GetSettings()->Save();

    tempChanged = false;
  }

  // теперь проверяем, надо ли нам записывать настройки немедленно
   unsigned long curMillis = millis();
   bool needToWrite = (changesCount > 0) || (curMillis - updateTimer > 1000);
   if(needToWrite)
   {
    // надо записать текущее положение дел в Nextion
      updateTimer = curMillis;

      bitWrite(ourScratch.controllerStatus,0, WORK_STATUS.GetStatus(WINDOWS_STATUS_BIT));
      bitWrite(ourScratch.controllerStatus,1, WORK_STATUS.GetStatus(WINDOWS_MODE_BIT));
      bitWrite(ourScratch.controllerStatus,2, WORK_STATUS.GetStatus(WATER_STATUS_BIT));
      bitWrite(ourScratch.controllerStatus,3, WORK_STATUS.GetStatus(WATER_MODE_BIT));
      bitWrite(ourScratch.controllerStatus,4, WORK_STATUS.GetStatus(LIGHT_STATUS_BIT));
      bitWrite(ourScratch.controllerStatus,5, WORK_STATUS.GetStatus(LIGHT_MODE_BIT));

      GlobalSettings* sett = MainController->GetSettings();
      ourScratch.openTemperature = sett->GetOpenTemp();
      ourScratch.closeTemperature = sett->GetCloseTemp();

      // теперь пишем показания с датчиков
      ourScratch.dataCount = 0;
      
      byte cntr = 0;
      while(UNI_NX_SENSORS_DATA[cntr].sensorType > 0)
      {
        AbstractModule* module = MainController->GetModuleByID(UNI_NX_SENSORS_DATA[cntr].moduleName);
        if(module)
        {
          OneState* os = module->State.GetState((ModuleStates)UNI_NX_SENSORS_DATA[cntr].sensorType,UNI_NX_SENSORS_DATA[cntr].sensorIndex);
          if(os)
          {
            // получили состояние, теперь пишем его в скратч
            if(os->HasData())
            {
              byte buff[4] = {0};
              os->GetRawData(buff);
              ourScratch.data[ourScratch.dataCount].sensorType = UNI_NX_SENSORS_DATA[cntr].sensorType;
              memcpy(ourScratch.data[ourScratch.dataCount].sensorData,buff,2);
              ourScratch.dataCount++;
            }
          } // if(os)
        } // if(module)

        cntr++;

        if(ourScratch.dataCount > 4)
          break;
      } // while
      

      // копируем скратчпад обратно
      memcpy(scratchpad->data,&ourScratch,sizeof(UniNextionScratchpad));

      // и пишем его в Nextion
      UniScratchpad.begin(pin,scratchpad);
      UniScratchpad.write();
      
   } // needToWrite
   
}
//-------------------------------------------------------------------------------------------------------------------------------------------------------
#endif // USE_UNI_NEXTION_MODULE
//-------------------------------------------------------------------------------------------------------------------------------------------------------
// SensorsUniClient
//-------------------------------------------------------------------------------------------------------------------------------------------------------
SensorsUniClient::SensorsUniClient() : AbstractUniClient()
{
  measureTimer = 0;
}
//-------------------------------------------------------------------------------------------------------------------------------------------------------
void SensorsUniClient::Register(UniRawScratchpad* scratchpad)
{
  // регистрируем модуль тут, добавляя нужные индексы датчиков в контроллер
  UniSensorsScratchpad* ourScrath = (UniSensorsScratchpad*) &(scratchpad->data);
  byte addedCount = 0;

  for(byte i=0;i<MAX_UNI_SENSORS;i++)
  {
    byte type = ourScrath->sensors[i].type;
    if(type == NO_SENSOR_REGISTERED) // нет типа датчика 
      continue;

    UniSensorType ut = (UniSensorType) type;
    
    if(ut == uniNone) // нет типа датчика
      continue;

    // имеем тип датчика, можем регистрировать
    if(UniDispatcher.AddUniSensor(ut,ourScrath->sensors[i].index))
      addedCount++;
    
  } // for

  if(addedCount > 0) // добавили датчики, надо сохранить состояние контроллера в EEPROM
    UniDispatcher.SaveState();
}
//-------------------------------------------------------------------------------------------------------------------------------------------------------
void SensorsUniClient::Update(UniRawScratchpad* scratchpad, bool isModuleOnline)
{
  
    // тут обновляем данные, полученный по проводу с модуля. 
    // нам передали адрес скратчпада, куда можно писать данные, полученные
    // с клиента, при необходимости.

    // нас дёргают после вычитки скратчпада из модуля, всё, что мы должны сделать - 
    // это обновить данные в контроллере.

    UniSensorsScratchpad* ourScratch = (UniSensorsScratchpad*) &(scratchpad->data);
    UniSensorState states;
    
    for(byte i=0;i<MAX_UNI_SENSORS;i++)
    {

      byte type = ourScratch->sensors[i].type;
      if(type == NO_SENSOR_REGISTERED) // нет типа датчика 
        continue;
  
      UniSensorType ut = (UniSensorType) type;
      
      if(ut == uniNone) // нет типа датчика
        continue;
      
      if(UniDispatcher.GetRegisteredStates(ut, ourScratch->sensors[i].index, states))
      {
        // получили состояния, можно обновлять
        UpdateStateData(states, &(ourScratch->sensors[i]), isModuleOnline);
      } // if
    } // for

    // тут запускаем конвертацию, чтобы при следующем вызове вычитать актуальные данные.
    // конвертацию не стоит запускать чаще, чем в 5, скажем, секунд.
    unsigned long curMillis = millis();
    if(curMillis - measureTimer > 5000)
    {
      measureTimer = curMillis;
      UniScratchpad.begin(pin,scratchpad);
      UniScratchpad.startMeasure();
    }

}
//-------------------------------------------------------------------------------------------------------------------------------------------------------
void SensorsUniClient::UpdateStateData(const UniSensorState& states,const UniSensorData* data,bool IsModuleOnline)
{
  if(!(states.State1 || states.State2))
    return; // не найдено ни одного состояния  

  UpdateOneState(states.State1,data,IsModuleOnline);
  UpdateOneState(states.State2,data,IsModuleOnline);  

}
//-------------------------------------------------------------------------------------------------------------------------------------------------------
void SensorsUniClient::UpdateOneState(OneState* os, const UniSensorData* dataPacket, bool IsModuleOnline)
{
    if(!os)
      return;

   uint8_t sensorIndex = dataPacket->index;
   uint8_t sensorType = dataPacket->type;
   uint8_t dataIndex = 0;

   if(sensorIndex == NO_SENSOR_REGISTERED || sensorType == NO_SENSOR_REGISTERED || sensorType == uniNone)
    return; // нет датчика вообще

   switch(os->GetType())
   {
      case StateTemperature:
      {
        if(sensorType == uniHumidity) // если тип датчика - влажность, значит температура у нас идёт после влажности, в 3-м и 4-м байтах
        {
          dataIndex++; dataIndex++;
        }

        int8_t dt = (int8_t) dataPacket->data[dataIndex++];
        uint8_t dt2 =  dataPacket->data[dataIndex];

        
        int8_t b1 = IsModuleOnline ? dt : NO_TEMPERATURE_DATA;             
        uint8_t b2 = IsModuleOnline ? dt2 : 0;

        Temperature t(b1, b2);
        os->Update(&t);
        
      }
      break;

      case StateHumidity:
      case StateSoilMoisture:
      {
        int8_t dt = (int8_t)  dataPacket->data[dataIndex++];
        uint8_t dt2 =  dataPacket->data[dataIndex];
        
        int8_t b1 = IsModuleOnline ? dt : NO_TEMPERATURE_DATA;    
        uint8_t b2 = IsModuleOnline ? dt2 : 0;
        
        Humidity h(b1, b2);
        os->Update(&h);        
      }
      break;

      case StateLuminosity:
      {
        unsigned long lum = NO_LUMINOSITY_DATA;
        
        if(IsModuleOnline)
          memcpy(&lum, dataPacket->data, 4);

        os->Update(&lum);
        
      }
      break;

      case StateWaterFlowInstant:
      case StateWaterFlowIncremental:
      case StatePH:
      case StateUnknown:
      
      break;
      
    
   } // switch
  
}
//-------------------------------------------------------------------------------------------------------------------------------------------------------
#if UNI_WIRED_MODULES_COUNT > 0
//-------------------------------------------------------------------------------------------------------------------------------------------------------
UniPermanentLine::UniPermanentLine(uint8_t pinNumber)
{
  pin = pinNumber;
  timer = random(0,UNI_MODULE_UPDATE_INTERVAL); // разнесём опрос датчиков по времени
  lastClient = NULL;

}
//-------------------------------------------------------------------------------------------------------------------------------------------------------
bool UniPermanentLine::IsRegistered()
{
  if(SHARED_SCRATCHPAD.head.packet_type == uniNextionClient) // для дисплея Nextion не требуется регистрация 
    return true;
    
  return ( SHARED_SCRATCHPAD.head.controller_id == UniDispatcher.GetControllerID() );
}
//-------------------------------------------------------------------------------------------------------------------------------------------------------
void UniPermanentLine::Update(uint16_t dt)
{
  timer += dt;

  if(timer < UNI_MODULE_UPDATE_INTERVAL) // рано обновлять
    return;

  timer -= UNI_MODULE_UPDATE_INTERVAL; // сбрасываем таймер

  // теперь обновляем последнего клиента, если он был.
  // говорим ему, чтобы обновился, как будто модуля нет на линии.
  if(lastClient)
    lastClient->Update(&SHARED_SCRATCHPAD,false);

  // теперь пытаемся прочитать скратчпад
  UniScratchpad.begin(pin,&SHARED_SCRATCHPAD);
  
  if(UniScratchpad.read())
  {
    // прочитали, значит, датчик есть на линии.
    
    // проверяем, зарегистрирован ли модуль у нас?
    if(!IsRegistered()) // модуль не зарегистрирован у нас
      return;
      
    // получаем клиента для прочитанного скратчпада
    lastClient = UniFactory.GetClient(&SHARED_SCRATCHPAD);
    lastClient->SetPin(pin); // назначаем тот же самый пин, что у нас    
    lastClient->Update(&SHARED_SCRATCHPAD,true);
    
  } // if
  else
  {
    // на линии никого нет
  }
  
}
//-------------------------------------------------------------------------------------------------------------------------------------------------------
#endif
//-------------------------------------------------------------------------------------------------------------------------------------------------------
// UniRegDispatcher
//-------------------------------------------------------------------------------------------------------------------------------------------------------
UniRegDispatcher::UniRegDispatcher()
{
  temperatureModule = NULL;
  humidityModule = NULL;
  luminosityModule = NULL;
  soilMoistureModule = NULL;
  phModule = NULL;  

  currentTemperatureCount = 0;
  currentHumidityCount = 0;
  currentLuminosityCount = 0;
  currentSoilMoistureCount = 0;

  hardCodedTemperatureCount = 0;
  hardCodedHumidityCount = 0;
  hardCodedLuminosityCount = 0;
  hardCodedSoilMoistureCount = 0;
  hardCodedPHCount = 0;
    
}
//-------------------------------------------------------------------------------------------------------------------------------------------------------
bool UniRegDispatcher::AddUniSensor(UniSensorType type, uint8_t sensorIndex)
{
  // добавляем состояние для датчика в систему. Состояние надо добавлять только тогда,
  // когда переданный индекс датчика не укладывается в уже выданный диапазон.
  // например, переданный индекс - 0, и кол-во выданных до этого индексов - 0, следовательно,
  // мы не попадаем в выданный диапазон. Или - переданный индекс - 1, кол-во ранее выданных - 0,
  // значит, мы должны добавить 2 новых состояния.

  // если sensorIndex == 0xFF - ничего делать не надо
  if(sensorIndex == NO_SENSOR_REGISTERED) // попросили зарегистрировать датчик без назначенного ранее индекса, ошибка.
    return false;
  
   switch(type)
  {
    case uniNone:  // нет датчика
      return false;
    
    case uniTemp:  // температурный датчик
      if(temperatureModule)
      {
          if(sensorIndex < currentTemperatureCount) // попадаем в диапазон уже выданных
            return false;

          // здесь sensorIndex больше либо равен currentTemperatureCount, следовательно, мы не попадаем в диапазон
          uint8_t to_add = (sensorIndex - currentTemperatureCount) + 1;

          for(uint8_t cntr = 0; cntr < to_add; cntr++)
          {
            temperatureModule->State.AddState(StateTemperature,hardCodedTemperatureCount + currentTemperatureCount + cntr);
          } // for

          // сохраняем кол-во добавленных
          currentTemperatureCount += to_add;
          
        return true;
      } // if(temperatureModule)
      else
        return false;
    
    case uniHumidity: 
    if(humidityModule)
      {

          if(sensorIndex < currentHumidityCount) // попадаем в диапазон уже выданных
            return false;

          // здесь sensorIndex больше либо равен currentHumidityCount, следовательно, мы не попадаем в диапазон
          uint8_t to_add = (sensorIndex - currentHumidityCount) + 1;

          for(uint8_t cntr = 0; cntr < to_add; cntr++)
          {
            humidityModule->State.AddState(StateTemperature,hardCodedHumidityCount + currentHumidityCount + cntr);
            humidityModule->State.AddState(StateHumidity,hardCodedHumidityCount + currentHumidityCount + cntr);
          } // for

          // сохраняем кол-во добавленных
          currentHumidityCount += to_add;
          
        return true;
        
      }
      else
        return false;
    
    case uniLuminosity: 
    if(luminosityModule)
      {

          if(sensorIndex < currentLuminosityCount) // попадаем в диапазон уже выданных
            return false;

          // здесь sensorIndex больше либо равен currentLuminosityCount, следовательно, мы не попадаем в диапазон
          uint8_t to_add = (sensorIndex - currentLuminosityCount) + 1;

          for(uint8_t cntr = 0; cntr < to_add; cntr++)
          {
            luminosityModule->State.AddState(StateLuminosity,hardCodedLuminosityCount + currentLuminosityCount + cntr);
          } // for

          // сохраняем кол-во добавленных
          currentLuminosityCount += to_add;
          
        return true;
      }    
      else
        return false;
    
    case uniSoilMoisture: 
     if(soilMoistureModule)
      {
     
          if(sensorIndex < currentSoilMoistureCount) // попадаем в диапазон уже выданных
            return false;

          // здесь sensorIndex больше либо равен currentSoilMoistureCount, следовательно, мы не попадаем в диапазон
          uint8_t to_add = (sensorIndex - currentSoilMoistureCount) + 1;

          for(uint8_t cntr = 0; cntr < to_add; cntr++)
          {
            soilMoistureModule->State.AddState(StateSoilMoisture,hardCodedSoilMoistureCount + currentSoilMoistureCount + cntr);
          } // for

          // сохраняем кол-во добавленных
          currentSoilMoistureCount += to_add;
          
        return true;
      } 
      else
        return false;


    case uniPH:
      return false;
  } 

  return false;
}
//-------------------------------------------------------------------------------------------------------------------------------------------------------
uint8_t UniRegDispatcher::GetUniSensorsCount(UniSensorType type)
{
  switch(type)
  {
    case uniNone: return 0;
    case uniTemp: return currentTemperatureCount;
    case uniHumidity: return currentHumidityCount;
    case uniLuminosity: return currentLuminosityCount;
    case uniSoilMoisture: return currentSoilMoistureCount;
    case uniPH: return 0;
  }

  return 0;  
}
//-------------------------------------------------------------------------------------------------------------------------------------------------------
uint8_t UniRegDispatcher::GetHardCodedSensorsCount(UniSensorType type)
{
  switch(type)
  {
    case uniNone: return 0;
    case uniTemp: return hardCodedTemperatureCount;
    case uniHumidity: return hardCodedHumidityCount;
    case uniLuminosity: return hardCodedLuminosityCount;
    case uniSoilMoisture: return hardCodedSoilMoistureCount;
    case uniPH: return hardCodedPHCount;
  }

  return 0;
}
//-------------------------------------------------------------------------------------------------------------------------------------------------------
void UniRegDispatcher::Setup()
{
    temperatureModule = MainController->GetModuleByID(F("STATE"));
    if(temperatureModule)
      hardCodedTemperatureCount = temperatureModule->State.GetStateCount(StateTemperature);
    
    humidityModule = MainController->GetModuleByID(F("HUMIDITY"));
    if(humidityModule)
      hardCodedHumidityCount = humidityModule->State.GetStateCount(StateHumidity);
    
    luminosityModule = MainController->GetModuleByID(F("LIGHT"));
    if(luminosityModule)
      hardCodedLuminosityCount = luminosityModule->State.GetStateCount(StateLuminosity);

    soilMoistureModule = MainController->GetModuleByID(F("SOIL"));
    if(soilMoistureModule)
      hardCodedSoilMoistureCount = soilMoistureModule->State.GetStateCount(StateSoilMoisture);

    phModule = MainController->GetModuleByID(F("PH"));
    if(phModule)
      hardCodedPHCount = phModule->State.GetStateCount(StatePH);


    ReadState(); // читаем последнее запомненное состояние
    RestoreState(); // восстанавливаем последнее запомненное состояние
}
//-------------------------------------------------------------------------------------------------------------------------------------------------------
void UniRegDispatcher::ReadState()
{
  //Тут читаем последнее запомненное состояние по индексам сенсоров
  uint16_t addr = UNI_SENSOR_INDICIES_EEPROM_ADDR;
  uint8_t val = EEPROM.read(addr++);
  if(val != 0xFF)
    currentTemperatureCount = val;

  val = EEPROM.read(addr++);
  if(val != 0xFF)
    currentHumidityCount = val;

  val = EEPROM.read(addr++);
  if(val != 0xFF)
    currentLuminosityCount = val;

  val = EEPROM.read(addr++);
  if(val != 0xFF)
    currentSoilMoistureCount = val;
   
}
//-------------------------------------------------------------------------------------------------------------------------------------------------------
void UniRegDispatcher::RestoreState()
{
  //Тут восстанавливаем последнее запомненное состояние индексов сенсоров.
  // добавляем новые датчики в нужный модуль до тех пор, пока
  // их кол-во не сравняется с сохранённым последним выданным индексом.
  // индексы универсальным датчикам выдаются, начиная с 0, при этом данный индекс является
  // виртуальным, поэтому нам всегда надо добавить датчик в конец
  // списка, после жёстко указанных в прошивке датчиков. Такой подход
  // обеспечит нормальную работу универсальных датчиков вне зависимости
  // от настроек прошивки.
  
  if(temperatureModule)
  {
    uint8_t cntr = 0;    
    while(cntr < currentTemperatureCount)
    {
      temperatureModule->State.AddState(StateTemperature, hardCodedTemperatureCount + cntr);
      cntr++;
    }
    
  } // if(temperatureModule)

  if(humidityModule)
  {
    uint8_t cntr = 0;
    while(cntr < currentHumidityCount)
    {
      humidityModule->State.AddState(StateTemperature, hardCodedHumidityCount + cntr);
      humidityModule->State.AddState(StateHumidity, hardCodedHumidityCount + cntr);
      cntr++;
    }
    
  } // if(humidityModule)

 if(luminosityModule)
  {
    uint8_t cntr = 0;
    while(cntr < currentLuminosityCount)
    {
      luminosityModule->State.AddState(StateLuminosity, hardCodedLuminosityCount + cntr);
      cntr++;
    }
    
  } // if(luminosityModule)  

if(soilMoistureModule)
  {
    uint8_t cntr = 0;
    while(cntr < currentSoilMoistureCount)
    {
      soilMoistureModule->State.AddState(StateSoilMoisture, hardCodedSoilMoistureCount + cntr);
      cntr++;
    }
    
  } // if(soilMoistureModule) 

 // Что мы сделали? Мы добавили N виртуальных датчиков в каждый модуль, основываясь на ранее сохранённой информации.
 // в результате в контроллере появились датчики с показаниями <нет данных>, и показания с них обновятся, как только
 // поступит информация от них с универсальных модулей.
  
}
//-------------------------------------------------------------------------------------------------------------------------------------------------------
void UniRegDispatcher::SaveState()
{
  //Тут сохранение текущего состояния в EEPROM
  uint16_t addr = UNI_SENSOR_INDICIES_EEPROM_ADDR;  
  EEPROM.write(addr++,currentTemperatureCount);
  EEPROM.write(addr++,currentHumidityCount);
  EEPROM.write(addr++,currentLuminosityCount);
  EEPROM.write(addr++,currentSoilMoistureCount);
}
//-------------------------------------------------------------------------------------------------------------------------------------------------------
bool UniRegDispatcher::GetRegisteredStates(UniSensorType type, uint8_t sensorIndex, UniSensorState& resultStates)
{
   // смотрим тип сенсора, получаем состояния
   switch(type)
   {
    case uniNone: return false;
    
    case uniTemp: 
    {
        if(!temperatureModule)
          return false; // нет модуля температур в прошивке

       // получаем состояние. Поскольку индексы виртуальных датчиков у нас относительные, то прибавляем
       // к индексу датчика кол-во жёстко прописанных в прошивке. В результате получаем абсолютный индекс датчика в системе.
       resultStates.State1 = temperatureModule->State.GetState(StateTemperature,hardCodedTemperatureCount + sensorIndex);

       return (resultStates.State1 != NULL);
       
    }
    break;
    
    case uniHumidity: 
    {
        if(!humidityModule)
          return false; // нет модуля влажности в прошивке

       resultStates.State1 = humidityModule->State.GetState(StateTemperature,hardCodedHumidityCount + sensorIndex);
       resultStates.State2 = humidityModule->State.GetState(StateHumidity,hardCodedHumidityCount + sensorIndex);

       return (resultStates.State1 != NULL);

    }
    break;
    
    case uniLuminosity: 
    {
        if(!luminosityModule)
          return false; // нет модуля освещенности в прошивке

       resultStates.State1 = luminosityModule->State.GetState(StateLuminosity,hardCodedLuminosityCount + sensorIndex);
       return (resultStates.State1 != NULL);      
    }
    break;
    
    case uniSoilMoisture: 
    {
        if(!soilMoistureModule)
          return false; // нет модуля влажности почвы в прошивке

       resultStates.State1 = soilMoistureModule->State.GetState(StateSoilMoisture,hardCodedSoilMoistureCount + sensorIndex);
       return (resultStates.State1 != NULL);
      
    }
    break;

    case uniPH:
    break;
   } // switch

  return false;    
 
}
//-------------------------------------------------------------------------------------------------------------------------------------------------------
uint8_t UniRegDispatcher::GetControllerID()
{
  return MainController->GetSettings()->GetControllerID(); 
}
//-------------------------------------------------------------------------------------------------------------------------------------------------------
// UniScratchpadClass
//-------------------------------------------------------------------------------------------------------------------------------------------------------
UniScratchpadClass::UniScratchpadClass()
{
  pin = 0;
  scratchpad = NULL;
}
//-------------------------------------------------------------------------------------------------------------------------------------------------------
bool UniScratchpadClass::canWork()
{
  return (pin > 0 && scratchpad != NULL);
}
//-------------------------------------------------------------------------------------------------------------------------------------------------------
void UniScratchpadClass::begin(byte _pin,UniRawScratchpad* scratch)
{
  pin = _pin;
  scratchpad = scratch;
}
//-------------------------------------------------------------------------------------------------------------------------------------------------------
bool UniScratchpadClass::read()
{
  if(!canWork())
    return false;
    
    OneWire ow(pin);
    
    if(!ow.reset()) // нет датчика на линии
      return false; 

    // теперь читаем скратчпад
    ow.write(0xCC, 1);
    ow.write(UNI_READ_SCRATCHPAD,1); // посылаем команду на чтение скратчпада

    byte* raw = (byte*) scratchpad;
    // читаем скратчпад
    for(uint8_t i=0;i<sizeof(UniRawScratchpad);i++)
      raw[i] = ow.read();
      
    // проверяем контрольную сумму
    return OneWire::crc8(raw, sizeof(UniRawScratchpad)-1) == raw[sizeof(UniRawScratchpad)-1];
}
//-------------------------------------------------------------------------------------------------------------------------------------------------------
bool UniScratchpadClass::startMeasure()
{
  if(!canWork())
    return false;
    
    OneWire ow(pin);
    
    if(!ow.reset()) // нет датчика на линии
      return false; 

    ow.write(0xCC, 1);
    ow.write(UNI_START_MEASURE,1); // посылаем команду на старт измерений
    
    return ow.reset();
  
}
//-------------------------------------------------------------------------------------------------------------------------------------------------------
bool UniScratchpadClass::write()
{
  if(!canWork())
    return false;
    
    OneWire ow(pin);
    
  // выставляем ID нашего контроллера
  scratchpad->head.controller_id = UniDispatcher.GetControllerID();
  
  // подсчитываем контрольную сумму и записываем её в последний байт скратчпада
  scratchpad->crc8 = OneWire::crc8((byte*) scratchpad, sizeof(UniRawScratchpad)-1);

  if(!ow.reset()) // нет датчика на линии
    return false; 

  ow.write(0xCC, 1);
  ow.write(UNI_WRITE_SCRATCHPAD,1); // говорим, что хотим записать скратчпад

  byte* raw = (byte*) scratchpad;
  // теперь пишем данные
   for(uint8_t i=0;i<sizeof(UniRawScratchpad);i++)
    ow.write(raw[i]);

   return ow.reset();
}
//-------------------------------------------------------------------------------------------------------------------------------------------------------
bool UniScratchpadClass::save()
{
  if(!canWork())
    return false;
    
  OneWire ow(pin);

  if(!ow.reset())
    return false;
    
  // записываем всё в EEPROM
  ow.write(0xCC, 1);
  ow.write(UNI_SAVE_EEPROM,1);
  delay(100);
   
  return ow.reset();   
}
//-------------------------------------------------------------------------------------------------------------------------------------------------------
// UniRegistrationLine
//-------------------------------------------------------------------------------------------------------------------------------------------------------
UniRegistrationLine::UniRegistrationLine(byte _pin)
{
  pin = _pin;

  memset(&scratchpad,0xFF,sizeof(scratchpad));
}
//-------------------------------------------------------------------------------------------------------------------------------------------------------
bool UniRegistrationLine::IsModulePresent()
{
  // проверяем, есть ли модуль на линии, простой вычиткой скратчпада
  UniScratchpad.begin(pin,&scratchpad);

   return UniScratchpad.read();
}
//-------------------------------------------------------------------------------------------------------------------------------------------------------
void UniRegistrationLine::CopyScratchpad(UniRawScratchpad* dest)
{
  memcpy(dest,&scratchpad,sizeof(UniRawScratchpad));
}
//-------------------------------------------------------------------------------------------------------------------------------------------------------
void UniRegistrationLine::Register()
{
  // регистрируем модуль в системе. Чего там творится в скратчпаде - нас не колышет, это делает конфигуратор: назначает индексы и т.п.

  // однако, в зависимости от типа пакета, нам надо обновить состояние контроллера (например, добавить индексы виртуальных датчиков в систему).
  // это делается всегда, вне зависимости от того, был ранее зарегистрирован модуль или нет - индексы всегда поддерживаются в актуальном
  // состоянии - переназначили мы их или нет. Считаем, что в случаем универсального модуля с датчиками конфигуратор сам правильно расставил
  // все индексы, и нам достаточно только поддержать актуальное состояние индексов у контроллера.

  // подобная настройка при регистрации разных типов модулей может иметь различное поведение, поэтому мы должны работать с разными субъектами
  // такой настройки.

  // получаем клиента
  AbstractUniClient* client = UniFactory.GetClient(&scratchpad);
  
  // просим клиента зарегистрировать модуль в системе, чего он там будет делать - дело десятое.
  client->Register(&scratchpad);

  // теперь мы смело можем писать скратчпад обратно в модуль
  UniScratchpad.begin(pin,&scratchpad);
  
  if(UniScratchpad.write())
    UniScratchpad.save();

}
//-------------------------------------------------------------------------------------------------------------------------------------------------------
bool UniRegistrationLine::IsSameScratchpadType(UniRawScratchpad* src)
{
  if(!src)
    return false;

  return (scratchpad.head.packet_type == src->head.packet_type);
}
//-------------------------------------------------------------------------------------------------------------------------------------------------------
bool UniRegistrationLine::SetScratchpadData(UniRawScratchpad* src)
{
  if(!IsSameScratchpadType(src)) // разные типы пакетов в переданном скратчпаде и вычитанном, нельзя копировать
    return false;

  memcpy(&scratchpad,src,sizeof(UniRawScratchpad));
  return true;
}
//-------------------------------------------------------------------------------------------------------------------------------------------------------
