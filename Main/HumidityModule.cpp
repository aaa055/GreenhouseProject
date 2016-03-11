#include "HumidityModule.h"
#include "ModuleController.h"

static HumiditySensorRecord HUMIDITY_SENSORS_ARRAY[] = { HUMIDITY_SENSORS };


void HumidityModule::Setup()
{
  // настройка модуля тут

  dummyAnswer.IsOK = false;

  for(uint8_t i=0;i<SUPPORTED_HUMIDITY_SENSORS;i++)
   {
    State.AddState(StateHumidity,i); // поддерживаем и влажность,
    State.AddState(StateTemperature,i); // и температуру
    // запускаем конвертацию с датчиков при старте, через 2 секунды нам вернётся измеренная влажность и температура
    QuerySensor(HUMIDITY_SENSORS_ARRAY[i].pin,HUMIDITY_SENSORS_ARRAY[i].type);
   }  
 }
const HumidityAnswer& HumidityModule::QuerySensor(uint8_t pin, HumiditySensorType type)
{
  switch(type)
  {
    case DHT11:
    {
      return dhtQuery.read(pin,DHT_11);
    }
    break;
    
    case DHT2x:
    {
      return dhtQuery.read(pin,DHT_2x);
    }
    break;
  }

  return dummyAnswer;
}
void HumidityModule::Update(uint16_t dt)
{ 
  // обновление модуля тут
 
  lastUpdateCall += dt;
  if(lastUpdateCall < HUMIDITY_UPDATE_INTERVAL) // обновляем согласно настроенному интервалу
    return;
  else
    lastUpdateCall = 0; 

  // получаем данные с датчиков влажности
  Humidity h;
  Temperature t;
  for(uint8_t i=0;i<SUPPORTED_HUMIDITY_SENSORS;i++)
   {
      HumidityAnswer answer = QuerySensor(HUMIDITY_SENSORS_ARRAY[i].pin,HUMIDITY_SENSORS_ARRAY[i].type);

      if(answer.IsOK)
      {
        h.Value = answer.Humidity;
        h.Fract = answer.HumidityDecimal;

        t.Value = answer.Temperature;
        t.Fract = answer.TemperatureDecimal;
      } // if

      // сохраняем данные в состоянии модуля - индексы мы назначаем сами, последовательно, поэтому дыр в нумерации датчиков нет
      State.UpdateState(StateTemperature,i,(void*)&t);
      State.UpdateState(StateHumidity,i,(void*)&h);
   }  // for

}

bool  HumidityModule::ExecCommand(const Command& command,bool wantAnswer)
{

  if(wantAnswer) 
    PublishSingleton = NOT_SUPPORTED;

  if(command.GetType() == ctSET) // установка свойств
  {
    
  } // ctSET
  else
  if(command.GetType() == ctGET) // запрос свойств
  {
      uint8_t argsCnt = command.GetArgsCount();
      if(argsCnt < 1)
      {
        if(wantAnswer) 
          PublishSingleton = PARAMS_MISSED; // не хватает параметров
        
      } // argsCnt < 1
      else
      {
        // argsCnt >= 1
        String param = command.GetArg(0);
        if(param == PROP_CNT) // запросили данные о кол-ве датчиков: CTGET=HUMIDITY|CNT
        {
          PublishSingleton.Status = true;
          if(wantAnswer) 
          {
            PublishSingleton = PROP_CNT; 
            PublishSingleton << PARAM_DELIMITER << SUPPORTED_HUMIDITY_SENSORS;
          }
        } // PROP_CNT
        else
        if(param == ALL) // запросили показания со всех датчиков
        {
          PublishSingleton.Status = true;
          if(wantAnswer) 
            PublishSingleton = SUPPORTED_HUMIDITY_SENSORS;
          
          for(uint8_t i=0;i<SUPPORTED_HUMIDITY_SENSORS;i++)
          {

             OneState* stateTemp = State.GetState(StateTemperature,i);
             OneState* stateHumidity = State.GetState(StateHumidity,i);
             if(stateTemp && stateHumidity)
             {
                TemperaturePair tp = *stateTemp;
                HumidityPair hp = *stateHumidity;
              
                if(wantAnswer) 
                {
                  PublishSingleton << PARAM_DELIMITER << (hp.Current) << PARAM_DELIMITER << (tp.Current);
                }
             } // if
          } // for
                    
        } // all data
        else
        if(param != GetID()) // если только не запросили без параметров
        {
          // запросили показания с датчика по индексу
          uint8_t idx = param.toInt();
          if(idx >= SUPPORTED_HUMIDITY_SENSORS)
          {
            // плохой индекс
            if(wantAnswer) 
              PublishSingleton = NOT_SUPPORTED;
          } // плохой индекс
          else
          {
             if(wantAnswer) 
              PublishSingleton = param;
              
             OneState* stateTemp = State.GetState(StateTemperature,idx);
             OneState* stateHumidity = State.GetState(StateHumidity,idx);
             if(stateTemp && stateHumidity)
             {
                PublishSingleton.Status = true;

                TemperaturePair tp = *stateTemp;
                HumidityPair hp = *stateHumidity;
                
                if(wantAnswer)
                {
                  PublishSingleton << PARAM_DELIMITER << (hp.Current) << PARAM_DELIMITER << (tp.Current);
                }
             } // if
            
          } // else нормальный индекс
          
        } // else показания по индексу
        
      } // else
    
  } // ctGET
  

  mainController->Publish(this,command);    
  return true;
}

