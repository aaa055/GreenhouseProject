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

bool  RemoteModule::ExecCommand(const Command& command, bool wantAnswer)
{
  UNUSED(wantAnswer);
  UNUSED(command);

  return true;
}

