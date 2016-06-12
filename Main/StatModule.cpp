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

bool  StatModule::ExecCommand(const Command& command, bool wantAnswer)
{
  if(wantAnswer) PublishSingleton = UNKNOWN_COMMAND;
  PublishSingleton.AddModuleIDToAnswer = false;

  size_t argsCount = command.GetArgsCount();
  
  if(command.GetType() == ctSET) 
  {
      if(wantAnswer) 
        PublishSingleton = NOT_SUPPORTED;
  }
  else
  if(command.GetType() == ctGET) //получить статистику
  {

    if(!argsCount) // нет аргументов
    {
      if(wantAnswer) PublishSingleton = PARAMS_MISSED;
    }
    else
    {
        String t = command.GetArg(0);

        if(t == FREERAM_COMMAND) // запросили данные о свободной памяти
        {
         PublishSingleton.Status = true;
          if(wantAnswer) 
          {
            PublishSingleton = FREERAM_COMMAND; 
            PublishSingleton << PARAM_DELIMITER << freeRam();
          }
        }
        else
        if(t == UPTIME_COMMAND) // запросили данные об аптайме
        {
          PublishSingleton.Status = true;
          if(wantAnswer) 
          {
            PublishSingleton = UPTIME_COMMAND; 
            PublishSingleton << PARAM_DELIMITER <<  (unsigned long) uptime/1000;
          }
        }
     #ifdef USE_DS3231_REALTIME_CLOCK   
        else if(t == CURDATETIME_COMMAND)
        {
           DS3231Clock rtc = MainController->GetClock();
           DS3231Time tm = rtc.getTime();
           if(wantAnswer) 
           {
             PublishSingleton = rtc.getDayOfWeekStr(tm);
             PublishSingleton << F(" ") << (rtc.getDateStr(tm)) << F(" ");
             // Глюк компилятора? Если поставить все команды в одну строку - вместо времени ещё раз выведется дата! 
             PublishSingleton << rtc.getTimeStr(tm);
           }
          PublishSingleton.Status = true;
        }
      #endif  
        else
        {
          // неизвестная команда
        } // else

    }// have arguments
    
  } // if
 
 // отвечаем на команду
    MainController->Publish(this,command);
    
  return PublishSingleton.Status;
}

