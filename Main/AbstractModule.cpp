#include "AbstractModule.h"
#include "ModuleController.h"

PublishStruct& PublishStruct::operator=(const String& src)
{
  this->Text = src;
  return *this;
}
PublishStruct& PublishStruct::operator=(const char* src)
{
  this->Text = src;
  return *this;
}
PublishStruct& PublishStruct::operator=(char src)
{
  this->Text = src;
  return *this;  
}
PublishStruct& PublishStruct::operator=(const __FlashStringHelper *src)
{
  this->Text = src;
  return *this;    
}
PublishStruct& PublishStruct::operator=(unsigned long src)
{
  this->Text = src;
  return *this;    
  
}
PublishStruct& PublishStruct::operator=(int src)
{
  this->Text = src;
  return *this;      
}
PublishStruct& PublishStruct::operator=(long src)
{
  this->Text = src;
  return *this;      
  
}
PublishStruct& PublishStruct::operator<<(const String& src)
{
  this->Text += src;
  return *this;
}
PublishStruct& PublishStruct::operator<<(const char* src)
{
  this->Text += src;
  return *this;
}
PublishStruct& PublishStruct::operator<<(char src)
{
  this->Text += src;
  return *this;  
}
PublishStruct& PublishStruct::operator<<(const __FlashStringHelper *src)
{
  this->Text += src;
  return *this;    
}
PublishStruct& PublishStruct::operator<<(unsigned long src)
{
  this->Text += src;
  return *this;    
  
}
PublishStruct& PublishStruct::operator<<(int src)
{
  this->Text += src;
  return *this;      
}
PublishStruct& PublishStruct::operator<<(unsigned int src)
{
  this->Text += src;
  return *this;        
}
PublishStruct& PublishStruct::operator<<(long src)
{
  this->Text += src;
  return *this;      
}

WorkStatus::WorkStatus()
{
  memset(statuses,0,sizeof(uint8_t)*STATUSES_BYTES);
}
void WorkStatus::SetStatus(uint8_t bitNum, bool bOn)
{
  uint8_t byte_num = bitNum/8;
  uint8_t bit_num = bitNum%8;

  bitWrite(statuses[byte_num],bit_num,(bOn ? 1 : 0));
}
bool WorkStatus::GetStatus(uint8_t bitNum)
{
  uint8_t byte_num = bitNum/8;
  uint8_t bit_num = bitNum%8;
  return bitRead(statuses[byte_num],bit_num) ? true : false; 
}

const char HEX_CHARS[]  PROGMEM = {"0123456789ABCDEF"};
String WorkStatus::ToHex(int i)
{  
  
  String Out;
  int idx = i & 0xF;
  char char1 = (char) pgm_read_byte_near( HEX_CHARS + idx );
  i>>=4;
  idx = i & 0xF;
  char char2 = (char) pgm_read_byte_near( HEX_CHARS + idx );
  Out = String(char2); Out += String(char1);
  
  return Out; 
}
void WorkStatus::WriteStatus(Stream* pStream, bool bAsTextHex)
{
  if(!pStream)
    return;
    
  for(uint8_t i=0;i<STATUSES_BYTES;i++)
  {
    if(!bAsTextHex)
      pStream->write(statuses[i]);
    else
    {
      pStream->print(WorkStatus::ToHex(statuses[i]));
    }
  } // for
}

WorkStatus WORK_STATUS; // экземпляр класса состояний

void OneState::Update(void* newData) // обновляем внутреннее состояние
{
     switch(Type)
    {
      
      case StateTemperature:
      case StateHumidity: // и для влажности используем структуру температуры
      {
        Temperature* t1 = (Temperature*) Data;
        Temperature* t2 = (Temperature*) PreviousData;

        *t2 = *t1; // сохраняем предыдущую температуру

        Temperature* tNew = (Temperature*) newData;
        *t1 = *tNew; // пишем новую
      } 
      break;

      #ifdef SAVE_RELAY_STATES
      case StateRelay:
      {
        uint8_t*  ui1 = (uint8_t*) Data;
        uint8_t*  ui2 = (uint8_t*) PreviousData;

        *ui2 = *ui1; // сохраняем предыдущее состояние каналов реле

        uint8_t* newState = (uint8_t*) newData;
        *ui1 = *newState; // пишем новое состояние каналов реле
      }  
      break;
      #endif
      

      case StateLuminosity:
      {
        long*  ui1 = (long*) Data;
        long*  ui2 = (long*) PreviousData;

        *ui2 = *ui1; // сохраняем предыдущее состояние освещенности

        long* newState = (long*) newData;
        *ui1 = *newState; // пишем новое состояние освещенности
      } 
      break;

      case StateWaterFlowInstant: // работаем с датчиками расхода воды
      case StateWaterFlowIncremental:
      {
        unsigned long*  ui1 = (unsigned long*) Data;
        unsigned long*  ui2 = (unsigned long*) PreviousData;

        *ui2 = *ui1; // сохраняем предыдущее состояние расхода воды

        unsigned long* newState = (unsigned long*) newData;
        *ui1 = *newState; // пишем новое состояние расхода воды
        
      }
      break;
      
    } // switch
 
}
void OneState::Init(ModuleStates state, uint8_t idx)
{
    Type = state;
    Index = idx;

    switch(state)
    {
      case StateTemperature:
      case StateHumidity: // и для влажности используем структуру температуры
      {
      
        Temperature* t1 = new Temperature;
        Temperature* t2 = new Temperature;
        
        Data = t1;
        PreviousData = t2;
      }
        
      break;
#ifdef SAVE_RELAY_STATES
      case StateRelay:
        {
        uint8_t*  ui1 = new uint8_t;
        uint8_t*  ui2 = new uint8_t;

        *ui1 = 0; // никакое реле не включено
        *ui2 = 0;
        
        Data = ui1;
        PreviousData = ui2;
        }
        
      break;
#endif
      case StateLuminosity:
      {
        long*  ui1 = new long;
        long*  ui2 = new long;

        *ui1 = NO_LUMINOSITY_DATA; // нет данных об освещенности
        *ui2 = NO_LUMINOSITY_DATA;
        
        Data = ui1;
        PreviousData = ui2;
      }
      break;

      case StateWaterFlowInstant:
      case StateWaterFlowIncremental:
      {
        unsigned long*  ui1 = new unsigned long;
        unsigned long*  ui2 = new unsigned long;

        *ui1 = 0; // нет данных о расходе воды
        *ui2 = 0;
        
        Data = ui1;
        PreviousData = ui2;
        
      }
      break;
    } // switch
  
}
OneState::operator String() // выводим текущие значения как строку
{
    switch(Type)
    {
      case StateTemperature:
      case StateHumidity: // и для влажности используем структуру температуры
      {
      
        Temperature* t1 = (Temperature*) Data;
        return *t1;
      }
        
#ifdef SAVE_RELAY_STATES
      case StateRelay:
        {
          uint8_t*  ui1 = (uint8_t*) Data;
          return String(*ui1);
        }
        
#endif
      case StateLuminosity:
      {
        long*  ul1 = (long*) Data;
        return String(*ul1);
      }

      case StateWaterFlowInstant:
      case StateWaterFlowIncremental:
      {
        unsigned long*  ul1 = (unsigned long*) Data;
        return String(*ul1);        
      }
    } // switch

return String();
}
OneState& OneState::operator=(const OneState& rhs)
{

  if(this == &rhs)
    return *this;

  if(Type != rhs.Type)
  {
  #ifdef _DEBUG
  Serial.println(F("[ERR] OneState::operator= - called with different types!"));
  #endif
    return *this;
  }

      switch(Type)
      {
        case StateTemperature:
        case StateHumidity: // и для влажности используем структуру температуры
        {
          Temperature* rhs_t1 = (Temperature*) rhs.Data;
          Temperature* rhs_t2 = (Temperature*) rhs.PreviousData;

          Temperature* this_t1 = (Temperature*) Data;
          Temperature* this_t2 = (Temperature*) PreviousData;

          *this_t1 = *rhs_t1;
          *this_t2 = *rhs_t2;
          
        }
        break;
#ifdef SAVE_RELAY_STATES  
        case StateRelay:
        {
          uint8_t*  rhs_ui1 = (uint8_t*) rhs.Data;
          uint8_t*  rhs_ui2 = (uint8_t*) rhs.PreviousData;
  
          uint8_t*  this_ui1 = (uint8_t*) Data;
          uint8_t*  this_ui2 = (uint8_t*) PreviousData;

          *this_ui1 = *rhs_ui1;
          *this_ui2 = *rhs_ui2;
          
         
        }  
        break;
#endif
        case StateLuminosity:
        {
          long*  rhs_ui1 = (long*) rhs.Data;
          long*  rhs_ui2 = (long*) rhs.PreviousData;
  
          long*  this_ui1 = (long*) Data;
          long*  this_ui2 = (long*) PreviousData;

          *this_ui1 = *rhs_ui1;
          *this_ui2 = *rhs_ui2;
        }  
        break;

        case StateWaterFlowInstant:
        case StateWaterFlowIncremental:
        {
          unsigned long*  rhs_ui1 = (unsigned long*) rhs.Data;
          unsigned long*  rhs_ui2 = (unsigned long*) rhs.PreviousData;
  
          unsigned long*  this_ui1 = (unsigned long*) Data;
          unsigned long*  this_ui2 = (unsigned long*) PreviousData;

          *this_ui1 = *rhs_ui1;
          *this_ui2 = *rhs_ui2;
        }  
        break;
      
      } // switch
  

  return *this;
}
bool OneState::IsChanged()
{
      switch(Type)
      {
        case StateTemperature:
        case StateHumidity: // и для влажности используем структуру температуры
        {
          Temperature* t1 = (Temperature*) Data;
          Temperature* t2 = (Temperature*) PreviousData;

          if(*t1 != *t2)
            return true; // температура изменилась
        }
        break;
#ifdef SAVE_RELAY_STATES  
        case StateRelay:
        {
          uint8_t*  ui1 = (uint8_t*) Data;
          uint8_t*  ui2 = (uint8_t*) PreviousData;
  
         if(*ui1 != *ui2)
          return true; // состояние реле изменилось
        }  
        break;
#endif
        case StateLuminosity:
        {
          long*  ui1 = (long*) Data;
          long*  ui2 = (long*) PreviousData;
  
         if(*ui1 != *ui2)
          return true; // состояние освещенности изменилось
        }  
        break;

        case StateWaterFlowInstant:
        case StateWaterFlowIncremental:
        {
          unsigned long*  ui1 = (unsigned long*) Data;
          unsigned long*  ui2 = (unsigned long*) PreviousData;
  
         if(*ui1 != *ui2)
          return true; // состояние освещенности изменилось
        }  
        break;

      
      } // switch

 return false;
  
}
OneState::~OneState()
{
  // подчищаем за собой
  
       switch(Type)
      {
        case StateTemperature:
        case StateHumidity: // и для влажности используем структуру температуры
        {
          Temperature* t1 = (Temperature*) Data;
          Temperature* t2 = (Temperature*) PreviousData;

          delete t1;
          delete t2;
        }
        break;
#ifdef SAVE_RELAY_STATES  
        case StateRelay:
        {
          uint8_t*  ui1 = (uint8_t*) Data;
          uint8_t*  ui2 = (uint8_t*) PreviousData;
  
          delete ui1;
          delete ui2;
        }  
        break;
#endif
        case StateLuminosity:
        {
          long*  ui1 = (long*) Data;
          long*  ui2 = (long*) PreviousData;
  
          delete ui1;
          delete ui2;
        }  
        break;

        case StateWaterFlowInstant:
        case StateWaterFlowIncremental:
        {
          unsigned long*  ui1 = (unsigned long*) Data;
          unsigned long*  ui2 = (unsigned long*) PreviousData;
  
          delete ui1;
          delete ui2;
        }  
        break;
      
      } // switch
 
  
}
OneState::operator HumidityPair()
{
  if(Type != StateHumidity)
  {
  #ifdef _DEBUG
    Serial.println(F("[ERR] OneState:operator HumidityPair() - !StateHumidity"));
  #endif
  return HumidityPair(Humidity(),Humidity()); // undefined behaviour
  }

    return HumidityPair(*((Humidity*)PreviousData),*((Humidity*)Data));  
}
OneState::operator TemperaturePair()
{
  if(Type != StateTemperature)
  {
  #ifdef _DEBUG
    Serial.println(F("[ERR] OneState:operator TemperaturePair() - !StateTemperature"));
  #endif
  return TemperaturePair(Temperature(),Temperature()); // undefined behaviour
  }

    return TemperaturePair(*((Temperature*)PreviousData),*((Temperature*)Data));
}
OneState::operator LuminosityPair()
{
  if(Type != StateLuminosity)
  {
  #ifdef _DEBUG
    Serial.println(F("[ERR] OneState:operator LuminosityPair() - !StateLuminosity"));
  #endif
  return LuminosityPair(0,0); // undefined behaviour
  }
  return LuminosityPair(*((long*)PreviousData),*((long*)Data));   
}
OneState::operator WaterFlowPair()
{
  if(!(Type == StateWaterFlowInstant || Type == StateWaterFlowIncremental))
  {
  #ifdef _DEBUG
    Serial.println(F("[ERR] OneState:operator WaterFlowPair() - !StateWaterFlow"));
  #endif
  return WaterFlowPair(0,0); // undefined behaviour
  }
  return WaterFlowPair(*((unsigned long*)PreviousData),*((unsigned long*)Data));   
}

#ifdef SAVE_RELAY_STATES
OneState::operator RelayPair()
{
  if(Type != StateRelay)
  {
  #ifdef _DEBUG
    Serial.println(F("[ERR] OneState:operator RelayPair() - !StateRelay"));
  #endif
  return RelayPair(0,0); // undefined behaviour
  }

  return RelayPair(*((uint8_t*)PreviousData),*((uint8_t*)Data)); 
}
#endif
OneState operator-(const OneState& left, const OneState& right)
{
  OneState result(left.Type,left.Index); // инициализируем

  if(left.Type != right.Type)
  {
  #ifdef _DEBUG
    Serial.println(F("[ERR] OneState operator- - Different types!"));
  #endif
  return result; // undefined behaviour
  }
  
      switch(left.Type)
      {
        case StateTemperature:
        case StateHumidity: // и для влажности используем структуру температуры
        {
          Temperature* t1 = (Temperature*) left.Data;
          Temperature* t2 = (Temperature*) right.Data;


          Temperature* thisT = (Temperature*) result.Data;
          if(t1->Value != NO_TEMPERATURE_DATA && t2->Value != NO_TEMPERATURE_DATA) // только если есть показания с датчиков
              *thisT = (*t1 - *t2); // получаем дельту текущих изменений
          
          t1 = (Temperature*) left.PreviousData;
          t2 = (Temperature*) right.PreviousData;

          thisT = (Temperature*) result.PreviousData;
          if(t1->Value != NO_TEMPERATURE_DATA && t2->Value != NO_TEMPERATURE_DATA) // только если есть показания с датчиков
              *thisT = (*t1 - *t2); // получаем дельту предыдущих изменений
        
        }
        break;
#ifdef SAVE_RELAY_STATES  
        case StateRelay:
        {
          uint8_t*  ui1 = (uint8_t*) left.Data;
          uint8_t*  ui2 = (uint8_t*) right.Data;

          uint8_t* thisUi = (uint8_t*) result.Data;

          // получаем дельту текущих изменений
          *thisUi = abs((*ui1 - *ui2));

          ui1 = (uint8_t*) left.PreviousData;
          ui2 = (uint8_t*) right.PreviousData;

          thisUi = (uint8_t*) result.PreviousData;

          // получаем дельту предыдущих изменений
          *thisUi = abs((*ui1 - *ui2));
  
        }  
        break;
#endif
        case StateLuminosity:
        {
          long*  ui1 = (long*) left.Data;
          long*  ui2 = (long*) right.Data;

          long* thisLong = (long*) result.Data;

          // получаем дельту текущих изменений
          if(*ui1 != NO_LUMINOSITY_DATA && *ui2 != NO_LUMINOSITY_DATA) // только если есть показания с датчиков
            *thisLong = abs((*ui1 - *ui2));

          ui1 = (long*) left.PreviousData;
          ui2 = (long*) right.PreviousData;

          thisLong = (long*) result.PreviousData;

          // получаем дельту предыдущих изменений
          if(*ui1 != NO_LUMINOSITY_DATA && *ui2 != NO_LUMINOSITY_DATA) // только если есть показания с датчиков
            *thisLong = abs((*ui1 - *ui2));   
        }  
        break;

        case StateWaterFlowInstant:
        case StateWaterFlowIncremental:
        {
          unsigned long*  ui1 = (unsigned long*) left.Data;
          unsigned long*  ui2 = (unsigned long*) right.Data;

          unsigned long* thisUi = (unsigned long*) result.Data;

          // получаем дельту текущих изменений
          *thisUi = abs((*ui1 - *ui2));

          ui1 = (unsigned long*) left.PreviousData;
          ui2 = (unsigned long*) right.PreviousData;

          thisUi = (unsigned long*) result.PreviousData;

          // получаем дельту предыдущих изменений
          *thisUi = abs((*ui1 - *ui2));
  
        }  
        break;

        
      } // switch
      
  return result;
}

Temperature::Temperature()
{
  Value = NO_TEMPERATURE_DATA;
  Fract = 0;
}
Temperature operator-(const Temperature& left, const Temperature& right) 
{
  
  if(left.Value == NO_TEMPERATURE_DATA || right.Value == NO_TEMPERATURE_DATA)
  {
    // разница температур, когда нет показаний на одном из датчиков - всегда имеет значение NO_TEMPERATURE_DATA
    return Temperature(NO_TEMPERATURE_DATA,0);
  }  

// получаем разницу двух температур
  int8_t sign1 = 1;
  int8_t sign2 = 1;
  
  if(left.Value < 0) // первая температура отрицательная
    sign1 = -1;
  
  if(right.Value < 0) // вторая температура отрицательная
    sign2 = -1;

  // получаем абсолютные значения температур, с учётом сотых, и умножаем их на знак. 
  // Знак сбрасываем, чтобы правильно сконвертировать в целое число, поскольку, если температура
  // отрицательная - то Fract её прибавит: (Fract = 5, Value = -10: -10*100 + 5 = -995 (!) вместо -1005.)
  long lVal = (abs(left.Value)*100 + left.Fract)*sign1;
  long rVal = (abs(right.Value*100) + right.Fract)*sign2;


  long res = abs((lVal - rVal));
  
    return Temperature(res/100, res%100); // дельта у нас всегда положительная.
}
ModuleState::ModuleState() : supportedStates(0)
{
  
}
bool ModuleState::HasState(ModuleStates state)
{
  return ( (supportedStates & state) == state);
}
void ModuleState::RemoveState(ModuleStates state, uint8_t idx)
{
  size_t cnt = states.size();
  for(size_t i=0;i<cnt;i++)
  {
    OneState* os = states[i];
    if(os->GetType() == state && os->GetIndex() == idx)
    {
      // нашли нужное состояние, удаляем его
      delete os;
      // теперь сдвигаем на пустое место
      size_t wIdx = i;
      while(wIdx < cnt-1)
      {
       states[wIdx] = states[wIdx+1]; 
       wIdx++;
      } // while
      // удаляем последний элемент (по сути, внутри вектора просто сдвинется указатель записи, и всё).
      states.pop();

      break; // выходим из цикла
    } // if
  } // for

  // теперь проверяем - если больше нет такого состояния - обнуляем его флаг.
  if(!HasState(state)) // нет такого состояния
    supportedStates &= ~state; // инвертируем все биты в state, кроме выставленного, и применяем эту маску к supportedStates. 
    // В результате в supportedStates очистятся только те биты, которые были выставлены в state.
}
OneState* ModuleState::AddState(ModuleStates state, uint8_t idx)
{
    supportedStates |= state;
    OneState* s = new OneState(state,idx);
    states.push_back(s); // сохраняем состояние
    
    return s;
}
bool ModuleState::HasChanges()
{
  size_t sz = states.size();
  for(size_t i=0;i<sz;i++)
  {
      OneState* s = states[i];

      if(s->IsChanged())
        return true;

  } // for

  return false;
  
}
void ModuleState::UpdateState(ModuleStates state, uint8_t idx, void* newData)
{
  size_t sz = states.size();
  for(size_t i=0;i<sz;i++)
  {
      OneState* s = states[i];
      if(s->GetType() == state && s->GetIndex() == idx)
      {
        s->Update(newData);
        return;
      } // if
  } // for

#ifdef _DEBUG
Serial.println(F("[ERR] - UpdateState FAILED!"));
#endif  
}
uint8_t ModuleState::GetStateCount(ModuleStates state)
{
  uint8_t result = 0;
  size_t sz = states.size();
  
  for(size_t i=0;i<sz;i++)
  {
      OneState* s = states[i];
      if(s->GetType() == state)
        result++;
  }
  
  return result;
}
OneState* ModuleState::GetStateByOrder(ModuleStates state, uint8_t orderNum)
{
  size_t sz = states.size();
  uint8_t cntr = 0;
  for(size_t i=0;i<sz;i++)
  {
      OneState* s = states[i];
      if(s->GetType() == state)
      {
        if(cntr == orderNum)
          return s;

          cntr++;
      }
  }

    return NULL;  
}
OneState* ModuleState::GetState(ModuleStates state, uint8_t idx)
{
  size_t sz = states.size();
  for(size_t i=0;i<sz;i++)
  {
      OneState* s = states[i];
      if(s->GetType() == state && s->GetIndex() == idx)
        return s;
  }

    return NULL;
}

char SD_BUFFER[SD_BUFFER_LENGTH] = {0};

