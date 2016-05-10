#include "SoilMoistureModule.h"
#include "ModuleController.h"

#if SUPPORTED_SOIL_MOISTURE_SENSORS > 0
static uint8_t SOIL_MOISTURE_SENSORS_ARRAY[] = { SOIL_MOISTURE_SENSORS };
#endif

void SoilMoistureModule::Setup()
{
  // настройка модуля тут
  
  #if SUPPORTED_SOIL_MOISTURE_SENSORS > 0
    for(uint8_t i=0;i<SUPPORTED_SOIL_MOISTURE_SENSORS;i++)
    {
      State.AddState(StateSoilMoisture,i); // добавляем датчики влажности почвы
    } // for
  #endif
 }

void SoilMoistureModule::Update(uint16_t dt)
{ 

  // обновление модуля тут
  
  lastUpdateCall += dt;
  if(lastUpdateCall < SOIL_MOISTURE_UPDATE_INTERVAL) // обновляем согласно настроенному интервалу
    return;
  else
    lastUpdateCall = 0; 
    
    
    #if SUPPORTED_SOIL_MOISTURE_SENSORS > 0
      for(uint8_t i=0;i<SUPPORTED_SOIL_MOISTURE_SENSORS;i++)
      {
        int val = analogRead(SOIL_MOISTURE_SENSORS_ARRAY[i]);

        // теперь нам надо отразить показания между SOIL_MOISTURE_100_PERCENT и SOIL_MOISTURE_0_PERCENT
        int percentsInterval = map(val,SOIL_MOISTURE_0_PERCENT,SOIL_MOISTURE_100_PERCENT,0,10000);
        
        if(SOIL_MOISTURE_0_PERCENT < SOIL_MOISTURE_100_PERCENT)
          percentsInterval = 10000 - percentsInterval;
       
        Humidity h;
        h.Value = percentsInterval/100;
        h.Fract  = percentsInterval%100;
        if(h.Value > 99)
        {
          h.Value = 100;
          h.Fract = 0;
        }
        
        // обновляем состояние  
        State.UpdateState(StateSoilMoisture,i,(void*)&h);
      }
    #endif
  

}

bool  SoilMoistureModule::ExecCommand(const Command& command, bool wantAnswer)
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
        String param = command.GetArg(0);
        
        if(param == ALL) // запросили показания со всех датчиков: CTGET=SOIL|ALL
        {
          PublishSingleton.Status = true;
          uint8_t _cnt = State.GetStateCount(StateSoilMoisture);
          if(wantAnswer) 
            PublishSingleton = _cnt;
          
          for(uint8_t i=0;i<_cnt;i++)
          {

             OneState* stateHumidity = State.GetStateByOrder(StateSoilMoisture,i);
             if(stateHumidity)
             {
                HumidityPair hp = *stateHumidity;
              
                if(wantAnswer) 
                {
                  PublishSingleton << PARAM_DELIMITER << (hp.Current);
                }
             } // if
          } // for        
        } // param == ALL
        else
        if(param == PROP_CNT) // запросили данные о кол-ве датчиков: CTGET=SOIL|CNT
        {
          PublishSingleton.Status = true;
          if(wantAnswer) 
          {
            PublishSingleton = PROP_CNT; 
            uint8_t _cnt = State.GetStateCount(StateSoilMoisture);
            PublishSingleton << PARAM_DELIMITER << _cnt;
          }
        } // PROP_CNT
        else
        if(param != GetID()) // если только не запросили без параметров
        {
 // запросили показания с датчика по индексу
          uint8_t idx = param.toInt();
          uint8_t _cnt = State.GetStateCount(StateSoilMoisture);
          
          if(idx >= _cnt)
          {
            // плохой индекс
            if(wantAnswer) 
              PublishSingleton = NOT_SUPPORTED;
          } // плохой индекс
          else
          {
             if(wantAnswer) 
              PublishSingleton = param;
              
             OneState* stateHumidity = State.GetStateByOrder(StateSoilMoisture,idx);
             if(stateHumidity)
             {
                PublishSingleton.Status = true;
                HumidityPair hp = *stateHumidity;
                
                if(wantAnswer)
                {
                  PublishSingleton << PARAM_DELIMITER << (hp.Current);
                }
             } // if
            
          } // else нормальный индекс        
        } // if param != GetID()
        
      } // else
  }
  
  mainController->Publish(this,command); 
  
  return true;
}

