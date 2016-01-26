#include "LuminosityModule.h"
#include "ModuleController.h"

void LuminosityModule::Setup()
{
  lightMeter.begin(); // запускаем датчик освещенности
  // настройка модуля тут
  
 }

void LuminosityModule::Update(uint16_t dt)
{ 
  // обновление модуля тут

}
uint16_t LuminosityModule::GetCurrentLuminosity()
{
  return lightMeter.readLightLevel();
}
bool  LuminosityModule::ExecCommand(const Command& command)
{
  ModuleController* c = GetController();
  GlobalSettings* settings = c->GetSettings();
  
  String answer = UNKNOWN_COMMAND;
  
  bool answerStatus = false; 
  
  if(command.GetType() == ctSET) 
  {
      answerStatus = false;
      answer = NOT_SUPPORTED;      
  }
  else
  if(command.GetType() == ctGET) //получить данные
  {

    String t = command.GetRawArguments();
    t.toUpperCase();
    if(t == GetID()) // нет аргументов, попросили дать показания с датчика
    {
      answerStatus = true;
      answer = String(GetCurrentLuminosity());
    }
    else
    {
      // разбор других команд
    } // else
    
  } // if
 
 // отвечаем на команду
    SetPublishData(&command,answerStatus,answer); // готовим данные для публикации
    c->Publish(this);
    
  return answerStatus;
}

