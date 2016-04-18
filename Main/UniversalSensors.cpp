#include "UniversalSensors.h"
#include <OneWire.h>


UniSensor1Wire::UniSensor1Wire()
{
  mainController = NULL;
  temperatureModule = NULL;
  humidityModule = NULL;
  luminosityModule = NULL;
  soilMoistureModule = NULL;
  
  lastCheckedSensor = -1;
  _pin = 0;
  isPermanentConnection = false;
  
  // забиваем вообще всё значением NO_SENSOR_REGISTERED, чтобы наверняка быть уверенными,
  // что когда мы будем читать из скратчпада - не попадутся данные, которые можно интерпретировать
  // как индекс датчика в системе.
  memset(scratch,NO_SENSOR_REGISTERED,sizeof(scratch));
  
  bInited = false;

}

void UniSensor1Wire::begin(unsigned long interval, bool permanentConnection, uint8_t pin, ModuleController* controller)
{
  queryInterval = interval;
  isPermanentConnection = permanentConnection;
  _pin = pin;
  mainController = controller;
  timer = 0;
}
void UniSensor1Wire::update(uint16_t dt)
{
  if(!bInited) // ещё не инициализировались, делаем это
  {
    bInited = true;
    
    // получаем ссылки на все модули, на этом шагу они уже зарегистрированы в системе,
    // все предварительно настроенные датчики прописаны, и мы можем безопасно ими оперировать.
    
    temperatureModule = mainController->GetModuleByID(F("STATE"));
    humidityModule = mainController->GetModuleByID(F("HUMIDITY"));
    luminosityModule = mainController->GetModuleByID(F("LIGHT"));
    soilMoistureModule = mainController->GetModuleByID(F("SOIL"));
  }

  if(!isPermanentConnection) // просто следим, не появится ли датчик на линии для регистрации в системе
  {
    if(!present()) // если нет датчика на линии
    {
      lastCheckedSensor = -1; // сбрасываем ID последнего проверенного сенсора
      return; // выходим
    }
     
     if(lastCheckedSensor != scratch[RF_ID_IDX]) // появился новый датчик
     {
       lastCheckedSensor = scratch[RF_ID_IDX]; // запоминаем ID последнего проверенного датчика

       if(!readScratchpad()) // читаем скратчпад датчика
          return; // если не прочитали - выходим
       
       if(needToConfigure()) // проверяем, надо ли его сконфигурировать под наш контроллер
          configure(false); // если да - конфигурируем, включая RF в настройках.
     }
      
    return; // выходим, датчик сконфигурирован
  }
  
  
  // здесь мы работаем в режиме постоянного соединения датчика с контроллером, т.е. как проводной.
  // читаем данные с датчика, постоянно висящего на линии.
  
  timer += dt;
  if(timer >= queryInterval)
  {
    // настало время опроса
    timer -= queryInterval; // сбрасываем таймер
    
    if(!present()) // нет датчика на линии
    {
      offLine(); // сообщаем, что показаний с датчика нет
      updateData(); // обновляем данные в контроллере
      return;
    }
    
     if(!readScratchpad()) // читаем скратчпад датчика
        return;
     
     if(needToConfigure()) // проверяем, надо ли его сконфигурировать
        configure(true); // если да - конфигурируем, выключая RF в настройках.
        
     updateData(); // обновляем данные с датчика в контроллере   
    
  } // if
}
bool UniSensor1Wire::present() // проверяем, есть ли датчик на линии
{
  if(!_pin)
    return false;
    
  OneWire ow(_pin);

  if(!ow.reset()) // нет датчика
    return false;
    
   return true; // датчик откликнулся импульсом Presence   
}
bool UniSensor1Wire::readScratchpad() // читаем скратчпад
{
    // но сначала запускаем конвертацию, чтобы в следующий раз нам вернулись актуальные данные
    OneWire ow(_pin);
    ow.write(UNI_START_MEASURE); // посылаем команду на старт измерений
    ow.reset(); // говорим, что команда послана полностью
    
    // теперь читаем скратчпад
    ow.write(UNI_READ_SCRATCHPAD); // посылаем команду на чтение скратчпада
    
    // читаем скратчпад
    for(uint8_t i=0;i<UNI_SCRATCH_SIZE;i++)
      scratch[i] = ow.read();
      
    
    // проверяем контрольную сумму
    return OneWire::crc8(scratch, UNI_SCRATCH_SIZE-1) == scratch[UNI_SCRATCH_SIZE-1];
    
}
void UniSensor1Wire::updateData() // обновляем данные с датчика в контроллере
{
  // переходим на начало данных
  uint8_t idx = DATA_START_IDX;
  for(uint8_t i=0;i<3;i++, idx += 6)
  {
      uint8_t sensorType = scratch[idx+1];

      switch(sensorType)
      {
        case uniNone: // нет никакого сенсора там, ничего не делаем
        break;
        
        case uniTemp: // температура
        {  
          if((scratch[idx] != NO_SENSOR_REGISTERED) && temperatureModule)
          {
            // индекс датчика прописан у модуля, теперь нам надо убедиться, что он есть
            // и в контроллере. Для этого посмотрим - есть ли датчик с таким индексом в модуле температур.
            OneState* os = temperatureModule->State.GetState(StateTemperature,scratch[idx]);
            if(!os)
            {
              // такого датчика нет, добавляем новый
              os = temperatureModule->State.AddState(StateTemperature,scratch[idx]);
            } // if
            
            // теперь работаем с датчиком и обновляем его показания. Первый байт данных у нас - температура до запятой, второй - после запятой.
            Temperature t = Temperature((int8_t) scratch[idx+2],scratch[idx+3]);
            os->Update((void*)&t); // обновляем показания
            
          } // if
        }
        break;
        
        case uniHumidity: // температура и влажность
        {
          if((scratch[idx] != NO_SENSOR_REGISTERED) && humidityModule)
          {
            // индекс датчика прописан у модуля, теперь нам надо убедиться, что он есть
            // и в контроллере. Для этого посмотрим - есть ли датчик с таким индексом в модуле влажности.
            OneState* os = humidityModule->State.GetState(StateHumidity,scratch[idx]);
            if(!os)
            {
              // такого датчика нет, добавляем новый
              os = humidityModule->State.AddState(StateHumidity,scratch[idx]);
            } // if
            
            // теперь работаем с датчиком и обновляем его показания. Первый байт данных у нас - влажность до запятой, второй - после запятой.
            Humidity h = Humidity((int8_t)scratch[idx+2],scratch[idx+3]);
            os->Update((void*)&h); // обновляем показания
            
            // теперь работаем с температурой, вычитываем 3 и 4 байт данных, 3-й - это температура до запятой, 4-й - после
            os = humidityModule->State.GetState(StateTemperature,scratch[idx]);
            if(!os)
              os = humidityModule->State.AddState(StateTemperature,scratch[idx]);
              
            Temperature t = Temperature((int8_t)scratch[idx+4],scratch[idx+5]);
            os->Update((void*)&t); // обновляем показания
            
          } // if
        }
        break;
        
        case uniLuminosity: // освещённость, четыре байта
        {
          if((scratch[idx] != NO_SENSOR_REGISTERED) && luminosityModule)
          {
            // индекс датчика прописан у модуля, теперь нам надо убедиться, что он есть
            // и в контроллере. Для этого посмотрим - есть ли датчик с таким индексом в модуле освещенности.
            OneState* os = luminosityModule->State.GetState(StateLuminosity,scratch[idx]);
            if(!os)
            {
              // такого датчика нет, добавляем новый
              os = luminosityModule->State.AddState(StateLuminosity,scratch[idx]);
            } // if
            
            // теперь работаем с датчиком и обновляем его показания. У нас идут 4 байта на показания, копируем их все.
            long lum;
            uint8_t* bPtr = &(scratch[idx+2]);
            memcpy(&lum,bPtr,4);
            os->Update((void*)&lum); // обновляем показания
            
          } // if        
        }
        break;
        
        case uniSoilMoisture: // влажность почвы, два байта
        {
          if((scratch[idx] != NO_SENSOR_REGISTERED) && soilMoistureModule)
          {
            // индекс датчика прописан у модуля, теперь нам надо убедиться, что он есть
            // и в контроллере. Для этого посмотрим - есть ли датчик с таким индексом в модуле влажности почвы.
            OneState* os = soilMoistureModule->State.GetState(StateSoilMoisture,scratch[idx]);
            if(!os)
            {
              // такого датчика нет, добавляем новый
              os = soilMoistureModule->State.AddState(StateSoilMoisture,scratch[idx]);
            } // if
            
            // теперь работаем с датчиком и обновляем его показания. Первый байт данных у нас - влажность до запятой, второй - после запятой.
            Humidity h = Humidity((int8_t) scratch[idx+2],scratch[idx+3]);
            os->Update((void*)&h); // обновляем показания
            
          } // if
        }
        break;
      } // switch
      
  } // for   
}
void UniSensor1Wire::configure(bool disableRF) // конфигурируем датчик
{
  // пробегаемся по всем типам датчиков, которые есть в железке,
  // назначаем каждому из них свой индекс и регистрируем данные датчики в нужных модулях контроллера.
  
  //TODO: Выставляем флаг "Выключить RF", в зависимости от переданной настройки!!!
  
  uint8_t idx = DATA_START_IDX;
  for(uint8_t i=0;i<3;i++, idx += 6)
  {
      uint8_t sensorType = scratch[idx+1];

      switch(sensorType)
      {
        case uniNone: // ничего нету, нас наиппали, расходимся.
        break;
        
        case uniTemp: // температура
        {  
          // только если датчик уже не был зарегистрирован. Обрабатываем такую ситуацию, 
          // поскольку мы можем менять не все настройки, а только некоторые, например, вкл/выкл RF.
          if(scratch[idx] == NO_SENSOR_REGISTERED) 
            scratch[idx] = registerTemperatureSensor();
        }
        break;
        
        case uniHumidity: // температура и влажность
        {
          if(scratch[idx] == NO_SENSOR_REGISTERED)
            scratch[idx] = registerHumiditySensor();
        }
        break;
        
        case uniLuminosity: // освещённость, четыре байта
        {
          if(scratch[idx] == NO_SENSOR_REGISTERED)
            scratch[idx] = registerLuminositySensor();
        }
        break;
        
        case uniSoilMoisture: // влажность почвы, два байта
        {
          if(scratch[idx] == NO_SENSOR_REGISTERED)
            scratch[idx] = registerSoilMoistureSensor();
        }
        break;
      } // switch
      
  } // for 
  
  // теперь пишем в датчик скратчпад с новой конфигурацией
  writeScratchpad();
}
void UniSensor1Wire::writeScratchpad() // пишем в скратчпад
{
   OneWire ow(_pin);
   if(!ow.reset()) // не сложилось
    return;
    
  scratch[CONTROLLER_ID_IDX] = UNIQUE_CONTROLLER_ID; // выставляем ID нашего контроллера
  
  //TODO: Тут пишем флаг "Включить или выключить RF", в зависимости от того, в каком режиме мы работаем!!!
  /*
    if(isPermanentConnection) // мы постоянно висим на проводе
      scratch[RF_ENABLED] = 0; // выключаем RF
    else // работаем в режиме регистрации датчиков
      scratch[RF_ENABLED] = 1; // включаем RF
  */
  
  ow.write(UNI_WRITE_SCRATCHPAD); // говорим, что хотим записать скратчпад
  
  // теперь пишем данные
   for(uint8_t i=0;i<UNI_SCRATCH_SIZE;i++)
    ow.write(scratch[i]);
  
  // говорим, что всё записали
  ow.reset();
    
}
bool UniSensor1Wire::needToConfigure() // проверяем, нуждается ли датчик в конфигурировании.
{ 
  //TODO: В ситуации, когда датчик вынули с линии и у него отключён RF в настройках,
  // затем этот же датчик воткнули в линию регистрации - надо проверять флаг выключения RF в настройках!
  
  /* 
  if( scratch[RF_DISABLED] && !isPermanentConnection) // когда у датчика вырублен RF в настройках и мы работаем в режиме регистрации беспроводных - конфигурируем датчик по-любому!!!
      return true;
  */

  // признаком этого служит неназначенный controller_id в скратчпаде, или ID, который не равен нашему.
  return scratch[CONTROLLER_ID_IDX]  != UNIQUE_CONTROLLER_ID;
}
uint8_t UniSensor1Wire::registerTemperatureSensor() // регистрируем новый температурный датчик
{
  if(!temperatureModule)
    return NO_SENSOR_REGISTERED; // нет модуля в прошивке
    
    // получаем кол-во зарегистрированных до этого датчиков температуры
    uint8_t cnt = temperatureModule->State.GetStateCount(StateTemperature);
    // добавляем новый температурный сенсор
    OneState* os = temperatureModule->State.AddState(StateTemperature,cnt);
    if(!os)
      return NO_SENSOR_REGISTERED;
    
    return cnt; // возвращаем индекс нового сенсора
    
}
uint8_t UniSensor1Wire::registerHumiditySensor() // регистрируем пару датчиков влажность+температура
{
  if(!humidityModule)
    return NO_SENSOR_REGISTERED; // нет модуля в прошивке
    
    // получаем кол-во зарегистрированных до этого датчиков температуры
    uint8_t cntTemp = humidityModule->State.GetStateCount(StateTemperature);
    uint8_t cntHumidity = humidityModule->State.GetStateCount(StateHumidity);
    
    if(cntTemp != cntHumidity)
      return NO_SENSOR_REGISTERED; // нельзя регистрировать, т.к. датчики получат разные индексы. Влажность у нас всегда идёт в паре с температурой!

    // добавляем новый температурный сенсор
    OneState* os = humidityModule->State.AddState(StateTemperature,cntTemp);
    if(!os)
      return NO_SENSOR_REGISTERED;
      
    os = humidityModule->State.AddState(StateHumidity,cntTemp); 
    if(!os)
      return NO_SENSOR_REGISTERED;
    
    return cntTemp; // возвращаем индекс нового сенсора
    
}
uint8_t UniSensor1Wire::registerLuminositySensor() // регистрируем датчик освещенности
{
  if(!luminosityModule)
    return NO_SENSOR_REGISTERED; // нет модуля в прошивке
    
    // получаем кол-во зарегистрированных до этого датчиков освещенности
    uint8_t cnt = luminosityModule->State.GetStateCount(StateLuminosity);
    // добавляем новый сенсор освещенности
    OneState* os = luminosityModule->State.AddState(StateLuminosity,cnt);
    if(!os)
      return NO_SENSOR_REGISTERED;
    
    return cnt; // возвращаем индекс нового сенсора
    
}
uint8_t UniSensor1Wire::registerSoilMoistureSensor() // регистрируем датчик влажности почвы
{
  if(!soilMoistureModule)
    return NO_SENSOR_REGISTERED; // нет модуля в прошивке
    
    // получаем кол-во зарегистрированных до этого датчиков влажности почвы
    uint8_t cnt = soilMoistureModule->State.GetStateCount(StateSoilMoisture);
    // добавляем новый сенсор влажности почвы
    OneState* os = soilMoistureModule->State.AddState(StateSoilMoisture,cnt);
    if(!os)
      return NO_SENSOR_REGISTERED;
    
    return cnt; // возвращаем индекс нового сенсора
    
}
void UniSensor1Wire::offLine() // датчика нет на линии, а ведь был, гад
{
  // заполняем скратчпад данными вида <нет данных>
  uint8_t idx = DATA_START_IDX;
  for(uint8_t i=0;i<3;i++, idx += 6)
  {
      uint8_t sensorType = scratch[idx+1];

      switch(sensorType)
      {
        case uniNone: // нет сенсора
        break;
        
        case uniTemp: // температура, два байта значащие, пишем в первый - NO_TEMPERATURE_DATA
        {
          int8_t t = NO_TEMPERATURE_DATA;
          uint8_t* bWrite = &(scratch[idx+2]); // указатель на начало данных
          memcpy(bWrite,&t,sizeof(t));
          
        }
        break;
        
        case uniHumidity: // температура и влажность, пишем все четыре байта
        {
          int8_t t = NO_TEMPERATURE_DATA;
          uint8_t* bWrite = &(scratch[idx+2]); // указатель на начало данных
          
          memcpy(bWrite,&t,sizeof(t)); // пишем первый байт (температура до запятой)

          bWrite += 2;
          memcpy(bWrite,&t,sizeof(t)); // пишем третий байт (влажность до запятой)
        }
        break;
        
        case uniLuminosity: // освещённость, четыре байта
        {
          long t = NO_LUMINOSITY_DATA;
          uint8_t* bWrite = &(scratch[idx+2]); // указатель на начало данных
          memcpy(bWrite,&t,sizeof(t));

        }
        break;
        
        case uniSoilMoisture: // влажность почвы, два байта
        {
          int8_t t = NO_TEMPERATURE_DATA;
          uint8_t* bWrite = &(scratch[idx+2]); // указатель на начало данных
          
           memcpy(bWrite,&t,sizeof(t)); // пишем первый байт (влажность до запятой)
        }
        break;
      } // switch
      
  } // for
}
