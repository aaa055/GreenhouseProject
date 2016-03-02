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
      h.Value = NO_TEMPERATURE_DATA;
      h.Fract = 0;
      
      t.Value = NO_TEMPERATURE_DATA;
      t.Fract = 0;

      HumidityAnswer answer = QuerySensor(HUMIDITY_SENSORS_ARRAY[i].pin,HUMIDITY_SENSORS_ARRAY[i].type);

      if(answer.IsOK)
      {
        h.Value = answer.Humidity;
        h.Fract = answer.HumidityDecimal;

        t.Value = answer.Temperature;
        t.Fract = answer.TemperatureDecimal;
      } // if

      // сохраняем данные в состоянии модуля
      State.UpdateState(StateTemperature,i,(void*)&t);
      State.UpdateState(StateHumidity,i,(void*)&h);
   }  // for

}

bool  HumidityModule::ExecCommand(const Command& command)
{

  String answer; answer.reserve(RESERVE_STR_LENGTH);
  answer = NOT_SUPPORTED;
  bool answerStatus = false;

  if(command.GetType() == ctSET) // установка свойств
  {
    
  } // ctSET
  else
  if(command.GetType() == ctGET) // запрос свойств
  {
      uint8_t argsCnt = command.GetArgsCount();
      if(argsCnt < 1)
      {
        answer = PARAMS_MISSED; // не хватает параметров
        
      } // argsCnt < 1
      else
      {
        // argsCnt >= 1
        String param = command.GetArg(0);
        if(param == PROP_CNT) // запросили данные о кол-ве датчиков: GTGET=HUMIDITY|CNT
        {
          answerStatus = true;
          answer = PROP_CNT; answer += PARAM_DELIMITER; answer += String(SUPPORTED_HUMIDITY_SENSORS);
        } // PROP_CNT
        else
        if(param == ALL) // запросили показания со всех датчиков
        {
          answerStatus = true;
          answer = String(SUPPORTED_HUMIDITY_SENSORS);
          
          for(uint8_t i=0;i<SUPPORTED_HUMIDITY_SENSORS;i++)
          {

             OneState* stateTemp = State.GetState(StateTemperature,i);
             OneState* stateHumidity = State.GetState(StateHumidity,i);
             if(stateTemp && stateHumidity)
             {
                answer += PARAM_DELIMITER;
                Temperature* t = (Temperature*) stateTemp->Data;
                Humidity* h = (Humidity*) stateHumidity->Data;
                answer += *h;
                answer += PARAM_DELIMITER;
                answer += *t;
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
            answer = NOT_SUPPORTED;
          } // плохой индекс
          else
          {
             answer = param;
             OneState* stateTemp = State.GetState(StateTemperature,idx);
             OneState* stateHumidity = State.GetState(StateHumidity,idx);
             if(stateTemp && stateHumidity)
             {
                answerStatus = true;
                answer += PARAM_DELIMITER;
                Temperature* t = (Temperature*) stateTemp->Data;
                Humidity* h = (Humidity*) stateHumidity->Data;
                answer += *h;
                answer += PARAM_DELIMITER;
                answer += *t;
             } // if
            
          } // else нормальный индекс
          
        } // else показания по индексу
        
      } // else
    
  } // ctGET
  

  SetPublishData(&command,answerStatus,answer); // готовим данные для публикации
  mainController->Publish(this);
    
  return answerStatus;
}

