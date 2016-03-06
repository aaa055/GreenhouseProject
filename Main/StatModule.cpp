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
  if(wantAnswer) PublishSingleton.Text = UNKNOWN_COMMAND;
  PublishSingleton.AddModuleIDToAnswer = false;
  
  if(command.GetType() == ctSET) 
  {
      if(wantAnswer) PublishSingleton.Text = NOT_SUPPORTED;
  }
  else
  if(command.GetType() == ctGET) //получить статистику
  {

    String t = command.GetRawArguments();
    t.toUpperCase();
    if(t == GetID()) // нет аргументов
    {
      if(wantAnswer) PublishSingleton.Text = PARAMS_MISSED;
    }
    else
    if(t == FREERAM_COMMAND) // запросили данные о свободной памяти
    {
     PublishSingleton.Status = true;
      if(wantAnswer) 
      {
        PublishSingleton.Text = FREERAM_COMMAND; PublishSingleton.Text += PARAM_DELIMITER; PublishSingleton.Text += freeRam();
      }
    }
    else
    if(t == UPTIME_COMMAND) // запросили данные об аптайме
    {
      PublishSingleton.Status = true;
      if(wantAnswer) 
      {
        PublishSingleton.Text = UPTIME_COMMAND; PublishSingleton.Text += PARAM_DELIMITER; PublishSingleton.Text +=  (unsigned long) uptime/1000;
      }
    }
 #ifdef USE_DS3231_REALTIME_CLOCK   
    else if(t == CURDATETIME_COMMAND)
    {
       DS3231Clock rtc = mainController->GetClock();
       DS3231Time tm = rtc.getTime();
       if(wantAnswer) 
       {
         PublishSingleton.Text = rtc.getDayOfWeekStr(tm);
         PublishSingleton.Text += F(" ");
         PublishSingleton.Text += rtc.getDateStr(tm);
         PublishSingleton.Text += F(" ");
         PublishSingleton.Text += rtc.getTimeStr(tm);
       }
      PublishSingleton.Status = true;
    }
  #endif  
    else
    {
      // неизвестная команда
    } // else
    
  } // if
 
 // отвечаем на команду
    mainController->Publish(this,command);
    
  return PublishSingleton.Status;
}

