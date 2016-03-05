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
PIN_STATE* PinModule::AddPin(uint8_t pinNumber,uint8_t currentState)
{
  if(!pinNumber) // если номер пина 0 - не надо ничего делать
    return NULL;
    
  PIN_STATE* s = GetPin(pinNumber);
  if(s)
  {
    s->pinState = currentState;          
    s->isActive = true;
    s->hasChanges = true; 
    return s;   
  }

  // можем добавлять, т.к. не нашли пин
    PIN_STATE p;
    p.pinNumber = pinNumber;
    p.pinState = currentState;
    p.isActive = true;
    p.hasChanges = true;
    pinMode(pinNumber,OUTPUT);
    pinStates.push_back(p);

  return &(pinStates[pinStates.size()-1]);
}
uint8_t PinModule::GetPinState(uint8_t pinNumber)
{
  PIN_STATE* s = GetPin(pinNumber);
  if(s)
    return s->pinState;
        
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
       answer = strNum + PARAM_DELIMITER + (currentState == HIGH ? STATE_ON : STATE_OFF);
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
      String strNum = command.GetArg(0); // номер пина

      String state = command.GetArg(1); // статус пина
      state.toUpperCase();
      
      // берём номер первого пина, на который будем опираться
      uint8_t pinNumber = strNum.toInt();

      byte pinLevel = LOW;
      bool bActive = !(state == PIN_DETACH); // если послана команда DETACH - выключаем слежение
   
      if(state == STATE_ON_ALT || state == STATE_ON)
        pinLevel = HIGH;
      else
      if(state == PIN_TOGGLE)
      {
           if(!PinExist(pinNumber)) // ещё нет такого пина для слежения
           {
              pinMode(pinNumber,INPUT); // читаем из пина его текущее состояние
              pinLevel = digitalRead(pinNumber);
              pinLevel = pinLevel == HIGH ? LOW : HIGH;
           }
           else // пин уже существует для слежения
           {
            PIN_STATE* s = GetPin(pinNumber);
            if(s)
            {
              pinLevel = s->pinState;
              pinLevel = pinLevel == LOW ? HIGH : LOW;
            }
           } // else
                 
      } // toggle state

      if(!bActive)
      {
         // пришла команда DETACH, поэтому менять статус пина - не надо
         PIN_STATE* s = GetPin(pinNumber);
         if(s)
          pinLevel = s->pinState; // читаем статус из пина
         else
          AddPin(pinNumber,pinLevel); // пина нету, просто добавляем с уровнем LOW
      }

       // добавляем пин
       PIN_STATE* s = AddPin(pinNumber,pinLevel);
       if(s)
       {
            answerStatus = true;
            answer = strNum + PARAM_DELIMITER;
            answer +=  (s->pinState == HIGH ? STATE_ON : STATE_OFF);
       }

       // добавляем все пины, которые нам передали в параметрах через запятую
       int idx;
       while((idx = strNum.indexOf(F(","))) != -1)
       {
         String pinNum = strNum.substring(0,idx);
         strNum = strNum.substring(idx+1);
         PIN_STATE* s = AddPin(pinNum.toInt(),pinLevel);
         if(s)
         {
          s->isActive = bActive;
          s->hasChanges = bActive;
         }
       } // while
       s =  AddPin(strNum.toInt(),pinLevel);
       if(s)
       {
        s->isActive = bActive;
        s->hasChanges = bActive;
       }
      
    } // if
    SetPublishData(&command,answerStatus,answer); // готовим данные для публикации
    mainController->Publish(this);
    return answerStatus;

  } // if ctSET

  return false;
}

