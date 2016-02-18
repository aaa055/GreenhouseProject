#ifndef _STAT_MODULE_H
#define _STAT_MODULE_H

#include "AbstractModule.h"
#include "Globals.h"

class StatModule : public AbstractModule
{
  private:
   unsigned long uptime;
  public:
    StatModule() : AbstractModule(F("STAT")) {}

    bool ExecCommand(const Command& command);
    void Setup();
    void Update(uint16_t dt);
};
#endif
