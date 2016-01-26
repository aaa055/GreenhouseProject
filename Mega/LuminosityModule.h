#ifndef _LUMINOSITY_MODULE_H
#define _LUMINOSITY_MODULE_H

#include "AbstractModule.h"
#include <Wire.h>
#include <BH1750.h>


class LuminosityModule : public AbstractModule // модуль управления освещенностью
{
  private:

  BH1750 lightMeter;
  uint16_t GetCurrentLuminosity();
  
  public:
    LuminosityModule() : AbstractModule(F("LIGHT")) {}

    bool ExecCommand(const Command& command);
    void Setup();
    void Update(uint16_t dt);

};


#endif
