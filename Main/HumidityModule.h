#ifndef _HUMIDITY_MODULE_H
#define _HUMIDITY_MODULE_H

#include "AbstractModule.h"
#include "DHTSupport.h"
#include "HumidityGlobals.h"

typedef struct
{
  uint8_t pin;
  HumiditySensorType type;
  
} HumiditySensorRecord;

class HumidityModule : public AbstractModule // модуль управления влажностью
{
  private:

    DHTSupport dhtQuery; // класс опроса датчиков DHT
    uint16_t lastUpdateCall;

    HumidityAnswer dummyAnswer;

    const HumidityAnswer& QuerySensor(uint8_t pin, HumiditySensorType type); // опрашивает сенсор
    
  public:
    HumidityModule() : AbstractModule(F("HUMIDITY"))
    , lastUpdateCall(256) // разнесём опросы датчиков по времени
    {}

    bool ExecCommand(const Command& command);
    void Setup();
    void Update(uint16_t dt);

};


#endif
