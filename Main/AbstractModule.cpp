#include "AbstractModule.h"

ModuleState::ModuleState() : supportedStates(0)
{
  
}
bool ModuleState::HasState(ModuleStates state)
{
  return ( (supportedStates & state) == state);
}

void ModuleState::AddState(ModuleStates state, uint8_t idx)
{
    supportedStates |= state;
    OneState* s = new OneState;
    s->Type = state;
    s->Index = idx;

    switch(state)
    {
      case StateTemperature:
      case StateHumidity: // и для влажности используем структуру температуры
      {
      
        Temperature* t1 = new Temperature;
        Temperature* t2 = new Temperature;
        
        t1->Value = NO_TEMPERATURE_DATA; // нет данных о температуре
        t2->Value = NO_TEMPERATURE_DATA;
        t1->Fract = 0;
        t2->Fract = 0;
        
        s->Data = t1;
        s->PreviousData = t2;
      }
        
      break;
#ifdef SAVE_RELAY_STATES
      case StateRelay:
        {
        uint8_t*  ui1 = new uint8_t;
        uint8_t*  ui2 = new uint8_t;

        *ui1 = 0; // никакое реле не включено
        *ui2 = 0;
        
        s->Data = ui1;
        s->PreviousData = ui2;
        }
        
      break;
#endif
      case StateLuminosity:
      {
        long*  ui1 = new long;
        long*  ui2 = new long;

        *ui1 = NO_LUMINOSITY_DATA; // нет данных об освещенности
        *ui2 = NO_LUMINOSITY_DATA;
        
        s->Data = ui1;
        s->PreviousData = ui2;
      }
      break;
    } // switch

    states.push_back(s); // сохраняем состояние
    
}
bool ModuleState::HasChanges()
{
  size_t sz = states.size();
  for(uint8_t i=0;i<sz;i++)
  {
      OneState* s = states[i];

      if(IsStateChanged(s))
        return true;

  } // for

  return false;
  
}
bool ModuleState::IsStateChanged(OneState* s)
{
      switch(s->Type)
      {
        case StateTemperature:
        case StateHumidity: // и для влажности используем структуру температуры
        {
          Temperature* t1 = (Temperature*) s->Data;
          Temperature* t2 = (Temperature*) s->PreviousData;

          if(*t1 != *t2)
            return true; // температура изменилась
        }
        break;
#ifdef SAVE_RELAY_STATES  
        case StateRelay:
        {
          uint8_t*  ui1 = (uint8_t*) s->Data;
          uint8_t*  ui2 = (uint8_t*) s->PreviousData;
  
         if(*ui1 != *ui2)
          return true; // состояние реле изменилось
        }  
        break;
#endif
        case StateLuminosity:
        {
          long*  ui1 = (long*) s->Data;
          long*  ui2 = (long*) s->PreviousData;
  
         if(*ui1 != *ui2)
          return true; // состояние освещенности изменилось
        }  
        break;
      } // switch

 return false;
  
}
bool ModuleState::IsStateChanged(ModuleStates state, uint8_t idx)
{
  size_t sz = states.size();
  for(uint8_t i=0;i<sz;i++)
  {
      OneState* s = states[i];
      
      if(s->Type == state && s->Index == idx)
        return IsStateChanged(s);

  } // for

  return false;
  
}
void ModuleState::UpdateState(ModuleStates state, uint8_t idx, void* newData)
{
  size_t sz = states.size();
  for(uint8_t i=0;i<sz;i++)
  {
      OneState* s = states[i];
      if(s->Type == state && s->Index == idx)
      {
                switch(state)
                {
                  
                  case StateTemperature:
                  case StateHumidity: // и для влажности используем структуру температуры
                  {
                    Temperature* t1 = (Temperature*) s->Data;
                    Temperature* t2 = (Temperature*) s->PreviousData;

                    *t2 = *t1; // сохраняем предыдущую температуру

                    Temperature* tNew = (Temperature*) newData;
                    *t1 = *tNew; // пишем новую
                  } 
                  break;

                  #ifdef SAVE_RELAY_STATES
                  case StateRelay:
                  {
                    uint8_t*  ui1 = (uint8_t*) s->Data;
                    uint8_t*  ui2 = (uint8_t*) s->PreviousData;
            
                    *ui2 = *ui1; // сохраняем предыдущее состояние каналов реле

                    uint8_t* newState = (uint8_t*) newData;
                    *ui1 = *newState; // пишем новое состояние каналов реле
                  }  
                  break;
                  #endif
                  
            
                  case StateLuminosity:
                  {
                    long*  ui1 = (long*) s->Data;
                    long*  ui2 = (long*) s->PreviousData;
            
                    *ui2 = *ui1; // сохраняем предыдущее состояние освещенности

                    long* newState = (long*) newData;
                    *ui1 = *newState; // пишем новое состояние освещенности
                  } 
                  break;
                  
                } // switch
        break;
      } // if
  } // for
}
uint8_t ModuleState::GetStateCount(ModuleStates state)
{
  uint8_t result = 0;
  size_t sz = states.size();
  
  for(uint8_t i=0;i<sz;i++)
  {
      OneState* s = states[i];
      if(s->Type == state)
        result++;
  }
  
  return result;
}
OneState* ModuleState::GetState(ModuleStates state, uint8_t idx)
{
  size_t sz = states.size();
  for(uint8_t i=0;i<sz;i++)
  {
      OneState* s = states[i];
      if(s->Type == state && s->Index == idx)
        return s;
  }

    return NULL;
}

void AbstractModule::Publish()
{
    toPublish.Module = this; // сохраняем указатель на нас, все остальное уже должно быть заполнено
    Stream* streamDefOut = toPublish.SourceCommand->GetIncomingStream(); // в какой поток вывести по умолчанию
    uint8_t streamFillCnt = 0; // был ли вывод в поток, в который мы собираемся вывести информацию?
    
   #ifdef USE_PUBLISHERS 
    for(uint8_t i=0;i<MAX_PUBLISHERS;i++)
    {
      if(publishers[i])
      {
        if(publishers[i]->Publish(&toPublish,streamDefOut)) // публикуем в подписчика
          streamFillCnt++; 
      }
    } // for
    #endif

    if(!streamFillCnt) // в этот поток никто не совался, можно выводить туда
    {
      // публикуем ответ в тот поток, откуда пришла команда
      String txt;
      if(toPublish.AddModuleIDToAnswer) // надо добавить имя модуля в ответ
       txt = toPublish.Module->GetID() + PARAM_DELIMITER;
     
      txt += toPublish.Text;
      
      mainController->PublishToStream(streamDefOut,toPublish.Status,txt);
    } // if
    
}
#ifdef USE_PUBLISHERS
bool AbstractModule::AddPublisher(AbstractPublisher* p)
{
     for(uint8_t i=0;i<MAX_PUBLISHERS;i++)
    {
      if(!publishers[i])
      {
        publishers[i] = p;
        return true;
      }
    } // for
    return false;
}
#endif

