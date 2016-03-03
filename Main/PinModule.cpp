#include "Arduino.h"
#include "PinModule.h"
#include "ModuleController.h"

void PinModule::Setup()
{
  Update(0);
}
void PinModule::UpdatePinStates()
{
  uint8_t sz = pinStates.size();
  for(uint8_t i=0;i<sz;i++)
  {
    PIN_STATE* s = &(pinStates[i]);
    
    if(s->isActive && s->hasChanges) // если мы управляем пином
    {
      s->hasChanges = false;
      pinMode(s->pinNumber,OUTPUT); // делаем пин запоминающим значения
      digitalWrite(s->pinNumber,s->pinState); // запоминаем текущее состояние пина
    }
  } // for
  
}
PIN_STATE* PinModule::GetPin(uint8_t pinNumber)
{
  uint8_t sz = pinStates.size();
  for(uint8_t i=0;i<sz;i++)
  {
    PIN_STATE* s = &(pinStates[i]);
    if(s->pinNumber == pinNumber)
      return s;
  } // for
  return NULL;  
}
bool PinModule::PinExist(uint8_t pinNumber)
{
  return (GetPin(pinNumber) != NULL);
}
bool PinModule::AddPin(uint8_t pinNumber,uint8_t currentState)
{
  PIN_STATE* s = GetPin(pinNumber);
  if(s)
  {
    s->pinState = currentState;          
    s->isActive = true;
    s->hasChanges = true; 
    return true;   
  }

  // можем добавлять, т.к. не нашли пин
    PIN_STATE p;
    p.pinNumber = pinNumber;
    p.pinState = currentState;
    p.isActive = true;
    p.hasChanges = true;
    pinMode(pinNumber,OUTPUT);
    pinStates.push_back(p);

  return true;
}
uint8_t PinModule::GetPinState(uint8_t pinNumber)
{
  uint8_t sz = pinStates.size();
  for(uint8_t i=0;i<sz;i++)
  {
    PIN_STATE* s = &(pinStates[i]);
    if(s->pinNumber == pinNumber)
    {
      return s->pinState;
    }
    
  } // for
// не можем читать состояние пина, не зарегистрированного у нас, поскольку
// этим пином может управлять другой модуль, и мы не можем переводить его в режим 
// чтения состояния. Поэтому возвращаем LOW.
  return LOW;
}
void PinModule::Update(uint16_t dt)
{ 
  UNUSED(dt);
  UpdatePinStates(); // обновляем состояние пинов
}

bool  PinModule::ExecCommand(const Command& command)
{
  String answer; answer.reserve(RESERVE_STR_LENGTH);
  answer = PARAMS_MISSED;
  bool answerStatus = false;  
  if(command.GetType() == ctGET) //получить состояние пина
  {
    if(command.GetArgsCount() > 0)
    {
       String strNum = command.GetArg(0);
       uint8_t pinNumber = strNum.toInt();
       uint8_t currentState = GetPinState(pinNumber);
      if(currentState == HIGH)
      {
        answer = strNum + PARAM_DELIMITER + STATE_ON;
      }
      else
      {
        answer = strNum + PARAM_DELIMITER +STATE_OFF;
      }
      answerStatus = true;
    }
    

  // отвечаем на команду
    SetPublishData(&command,true,answer,answerStatus); // готовим данные для публикации
    mainController->Publish(this);
    return answerStatus;
    
  } // if ctGET
  else
  if(command.GetType() == ctSET) // set
  {

    if(command.GetArgsCount() > 1)
    {
      String strNum = command.GetArg(0);
      uint8_t pinNumber = strNum.toInt();
      
      String state = command.GetArg(1);
      state.toUpperCase();

      if(state == STATE_ON_ALT || state == STATE_ON)
      {
        answer = strNum + PARAM_DELIMITER + STATE_ON;
        answerStatus = true;
        AddPin(pinNumber,HIGH);
        
      } // if(state == STATE_ON_ALT || state == STATE_ON)
      else
      if(state == STATE_OFF_ALT || state == STATE_OFF)
      {
        answer = strNum + PARAM_DELIMITER + STATE_OFF;
        answerStatus = true;
        AddPin(pinNumber,LOW);
      } // if(state == STATE_OFF_ALT || state == STATE_OFF)
      else 
      if(state == PIN_TOGGLE) // toggle state
      {
          
           if(!PinExist(pinNumber)) // ещё нет такого пина для слежения
           {
              pinMode(pinNumber,INPUT); // читаем из пина его текущее состояние
              AddPin(pinNumber,digitalRead(pinNumber));
           }
           

          // инвертируем его состояние
          uint8_t sz = pinStates.size();
          for(uint8_t i=0;i<sz;i++)
          {
            PIN_STATE* s = &(pinStates[i]);
            if(s->pinNumber == pinNumber)
            {
              s->pinState = s->pinState == LOW ? HIGH : LOW;
              s->isActive = true;
              s->hasChanges = true;
              answerStatus = true;
              answer = strNum + PARAM_DELIMITER;
              answer +=  (s->pinState == HIGH ? STATE_ON : STATE_OFF);
              break;
            }
            
          } // for
               
      } //  else if(state == PIN_TOGGLE) // toggle state
      else 
      if(state == PIN_DETACH) // не следить за пином
      {
           uint8_t sz = pinStates.size();
          for(uint8_t i=0;i<sz;i++)
          {
            PIN_STATE* s = &(pinStates[i]);
            if(s->pinNumber == pinNumber)
            {
              s->isActive = false;
              s->hasChanges = false;
              answerStatus = true;
              answer =  strNum + PARAM_DELIMITER + PIN_DETACH;
              break;
            }
            
          } // for
       
      } // PIN_DETACH


   
    } // if
    SetPublishData(&command,answerStatus,answer); // готовим данные для публикации
    mainController->Publish(this);
    return answerStatus;

  } // if ctSET

  return false;
}

