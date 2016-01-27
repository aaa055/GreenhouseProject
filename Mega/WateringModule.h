#ifndef _WATERING_MODULE_H
#define _WATERING_MODULE_H

#include "AbstractModule.h"
#include "Globals.h"

typedef enum
{
  wwmAutomatic, // в автоматическом режиме
  wwmManual // в ручном режиме
  
} WateringWorkMode; // режим работы полива

class WateringModule : public AbstractModule // модуль управления поливом
{
  private:

  bool bRelaysIsOn; // включены ли реле?
  WateringWorkMode workMode; // текущий режим работы
  long wateringTimer; // таймер полива

  int8_t lastDOW; // день недели с момента последнего опроса

   void BlinkWorkMode(uint16_t blinkInterval = 0);
  
  public:
    WateringModule() : AbstractModule(F("WATER")) {}

    bool ExecCommand(const Command& command);
    void Setup();
    void Update(uint16_t dt);

};


#endif
