#ifndef _HUMIDITY_MODULE_H
#define _HUMIDITY_MODULE_H

#include "AbstractModule.h"
#include "DHTSupport.h"



class HumidityModule : public AbstractModule // модуль управления влажностью
{
  private:

    DHTSupport dhtQuery; // класс опроса датчиков DHT
    uint16_t lastUpdateCall;
    
  public:
    HumidityModule() : AbstractModule(F("HUMIDITY")), lastUpdateCall(0) {}

    bool ExecCommand(const Command& command);
    void Setup();
    void Update(uint16_t dt);

};


#endif
