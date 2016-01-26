#ifndef _WATERING_MODULE_H
#define _WATERING_MODULE_H

#include "AbstractModule.h"



class WateringModule : public AbstractModule // модуль управления поливом
{
  private:
  public:
    WateringModule() : AbstractModule(F("WATER")) {}

    bool ExecCommand(const Command& command);
    void Setup();
    void Update(uint16_t dt);

};


#endif
