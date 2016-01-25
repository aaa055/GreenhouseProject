#include "AbstractModule.h"

void ModuleState::SetTempSensors(uint8_t cnt)
{
  if(cnt > MAX_TEMP_SENSORS)
    cnt = MAX_TEMP_SENSORS;

    TempSensors = cnt;
}
Temperature ModuleState::GetTemp(uint8_t idx)
{
  if(idx >= MAX_TEMP_SENSORS)
    idx = MAX_TEMP_SENSORS - 1;

  return Temp[idx];
}
Temperature ModuleState::GetPrevTemp(uint8_t idx)
{
  if(idx >= MAX_TEMP_SENSORS)
    idx = MAX_TEMP_SENSORS - 1;

  return prevTemp[idx];
}
void ModuleState::SetTemp(uint8_t idx, const Temperature& dt)
{ 
  if(idx >= MAX_TEMP_SENSORS)
    idx = MAX_TEMP_SENSORS - 1;

  prevTemp[idx] = Temp[idx]; // сохраняем предыдущую температуру
  Temp[idx] = dt; // записываем новую

  
}
bool ModuleState::GetRelayState(uint8_t idx)
{
    if(idx >= MAX_RELAY_CHANNELS)
      idx = MAX_RELAY_CHANNELS - 1;
 
    return bitRead(RelayStates,idx);

}
bool ModuleState::GetPrevRelayState(uint8_t idx)
{
    if(idx >= MAX_RELAY_CHANNELS)
      idx = MAX_RELAY_CHANNELS - 1;
      
     return bitRead(prevRelayStates,idx);

}
bool  ModuleState::IsTempChanged(uint8_t idx)
{
   if(idx >= MAX_TEMP_SENSORS)
    idx = MAX_TEMP_SENSORS - 1;

     return Temp[idx] != prevTemp[idx];
}
void ModuleState::SetRelayState(uint8_t idx,const String& state)
{
  SetRelayState(idx,(state == STATE_ON || state == STATE_ON_ALT));  
}
void  ModuleState::SetRelayState(uint8_t idx,bool bOn)
{
    if(idx >= MAX_RELAY_CHANNELS)
      idx = MAX_RELAY_CHANNELS - 1;

    bitWrite(prevRelayStates,idx,bitRead(RelayStates,idx));
    bitWrite(RelayStates,idx, bOn);
  
}
bool ModuleState::IsRelayStateChanged(uint8_t idx)
{
    if(idx >= MAX_RELAY_CHANNELS)
      idx = MAX_RELAY_CHANNELS - 1;  

  return bitRead(prevRelayStates,idx) != bitRead(RelayStates,idx);
}
void ModuleState::SetRelayChannels(uint8_t cnt)
{
  if(cnt > MAX_RELAY_CHANNELS)
    cnt = MAX_RELAY_CHANNELS;

    RelayChannels = cnt;
  
}

  void AbstractModule::Publish()
  {
    toPublish.Module = this; // сохраняем указатель на нас, все остальное уже должно быть заполнено
    Stream* streamDefOut = toPublish.SourceCommand->GetIncomingStream(); // в какой поток вывести по умолчанию
    uint8_t streamFillCnt = 0; // был ли вывод в поток, в который мы собираемся вывести информацию?
    
    for(uint8_t i=0;i<MAX_PUBLISHERS;i++)
    {
      if(publishers[i])
      {
        if(publishers[i]->Publish(&toPublish,streamDefOut)) // публикуем в подписчика
          streamFillCnt++; 
      }
    } // for

    if(!streamFillCnt) // в этот поток никто не совался, можно выводить туда
    {
      // публикуем ответ в тот поток, откуда пришла команда
      String txt;
      if(toPublish.AddModuleIDToAnswer) // надо добавить имя модуля в ответ
       txt = toPublish.Module->GetID() + PARAM_DELIMITER;
     
      txt += toPublish.Text;
      
      controller->PublishToStream(streamDefOut,toPublish.Status,txt);
    } // if
    
  }

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

