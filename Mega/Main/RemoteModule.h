#ifndef _REMOTE_MODULE_H
#define _REMOTE_MODULE_H

#include "AbstractModule.h"



class RemoteModule : public AbstractModule // модуль пересыла команд удалённому устройству
{
  private:
  public:
    RemoteModule(const String& id) : AbstractModule(id) {}

    bool ExecCommand(const Command& command);
    void Setup();
    void Update(uint16_t dt);

};


#endif
