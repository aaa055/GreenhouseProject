#include "RemoteModule.h"
#include "ModuleController.h"

void RemoteModule::Setup()
{
  // настройка модуля тут
 }

void RemoteModule::Update(uint16_t dt)
{ 
  UNUSED(dt);
  // обновление модуля тут

}

bool  RemoteModule::ExecCommand(const Command& command)
{
  ModuleController* c = GetController();

  // конструируем команду
  String tp = command.GetStringType();
  String textCommand = String(CHILD_PREFIX) + tp + COMMAND_DELIMITER + GetID() + PARAM_DELIMITER + command.GetRawArguments();

   // просим контроллер послать команду другой коробочке
    c->CallRemoteModuleCommand(this,textCommand);
  

  return true;
}

