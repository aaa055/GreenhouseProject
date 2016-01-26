#include "WateringModule.h"
#include "ModuleController.h"

void WateringModule::Setup()
{
  // настройка модуля тут
 }

void WateringModule::Update(uint16_t dt)
{ 
  // обновление модуля тут

}

bool  WateringModule::ExecCommand(const Command& command)
{
  ModuleController* c = GetController();
  GlobalSettings* settings = c->GetSettings();
  
  String answer = UNKNOWN_COMMAND;
  
  bool answerStatus = false; 
  
  if(command.GetType() == ctSET) 
  {
      //answerStatus = false;
      //answer = NOT_SUPPORTED;
      uint8_t argsCount = command.GetArgsCount();
      
      if(argsCount < 4) // не хватает параметров
      {
        answerStatus = false;
        answer = PARAMS_MISSED;
      }
      else
      {
        // парсим параметры
        WateringOption wateringOption = (WateringOption) command.GetArg(1).toInt();
        uint8_t wateringWeekDays = command.GetArg(2).toInt();
        uint16_t wateringTime = command.GetArg(3).toInt();

        // пишем в настройки
        settings->SetWateringOption(wateringOption);
        settings->SetWateringWeekDays(wateringWeekDays);
        settings->SetWateringTime(wateringTime);

        // сохраняем настройки
        settings->Save();

        
        answerStatus = true;
        answer = WATER_SETTINGS_COMMAND; answer += PARAM_DELIMITER;
        answer += REG_SUCC;

      } // else
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
    if(t == WATER_SETTINGS_COMMAND) // запросили данные о настройках полива
    {
      answerStatus = true;
      answer = WATER_SETTINGS_COMMAND; answer += PARAM_DELIMITER; 
      answer += String(settings->GetWateringOption()); answer += PARAM_DELIMITER;
      answer += String(settings->GetWateringWeekDays()); answer += PARAM_DELIMITER;
      answer += String(settings->GetWateringTime());
    }
    else
    {
      // неизвестная команда
    } // else
    
  } // if
 
 // отвечаем на команду
    SetPublishData(&command,answerStatus,answer); // готовим данные для публикации
    c->Publish(this);
    
  return answerStatus;
}

