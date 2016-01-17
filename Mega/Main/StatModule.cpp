#include "StatModule.h"
#include "ModuleController.h"

// выводит свободную память
int freeRam() 
{
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}


void StatModule::Setup()
{
  // настройка модуля статистики тут
  uptime = 0;
}

void StatModule::Update(uint16_t dt)
{ 
  // обновление модуля статистики тут
  uptime += dt;
}

bool  StatModule::ExecCommand(const Command& command)
{
  ModuleController* c = GetController();
  String answer = UNKNOWN_COMMAND;
  bool answerStatus = false; 
  if(command.GetType() == ctSET) 
  {
      answerStatus = false;
      answer = NOT_SUPPORTED;
  }
  else
  if(command.GetType() == ctGET) //получить статистику
  {

    String t = command.GetRawArguments();
    t.toUpperCase();
    if(t == GetID()) // нет аргументов
    {
      answerStatus = false;
      answer = PARAMS_MISSED;
    }
    else
    if(t == FREERAM_COMMAND) // запросили данные о свободной памяти
    {
      answerStatus = true;
      answer = FREERAM_COMMAND; answer += PARAM_DELIMITER; answer += freeRam();
    }
    else
    if(t == UPTIME_COMMAND) // запросили данные об аптайме
    {
      answerStatus = true;
      answer = UPTIME_COMMAND; answer += PARAM_DELIMITER; answer +=  (unsigned long) uptime/1000;
    }
 #ifdef USE_DS3231_REALTIME_CLOCK   
    else if(t == CURDATETIME_COMMAND)
    {
       DS3231 rtc = c->GetClock();
       String s = rtc.getDOWStr();
       s += F(" ");
       s += rtc.getDateStr();
       s += F(" ");
       s += rtc.getTimeStr();
       answerStatus = true;
       answer = s;     
    }
  #endif  
    else
    {
      // неизвестная команда
    } // else
    
  } // if
 
 // отвечаем на команду
    SetPublishData(&command,answerStatus,answer,false); // готовим данные для публикации
    c->Publish(this);
    
  return answerStatus;
}

