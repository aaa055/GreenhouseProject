#ifndef _SOIL_MOISTURE_MODULE_H
#define _SOIL_MOISTURE_MODULE_H

#include "AbstractModule.h"



class SoilMoistureModule : public AbstractModule // модуль датчиков влажности почвы
{
  private:
  
    uint16_t lastUpdateCall;
  
  public:
    SoilMoistureModule() : AbstractModule("SOIL"), lastUpdateCall(SOIL_MOISTURE_UPDATE_INTERVAL-387) {}

    bool ExecCommand(const Command& command, bool wantAnswer);
    void Setup();
    void Update(uint16_t dt);

};


#endif
