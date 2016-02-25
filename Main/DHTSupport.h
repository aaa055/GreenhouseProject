#ifndef _DHT_SUPPORT_H
#define _DHT_SUPPORT_H

#if ARDUINO < 100
#include <WProgram.h>
#else
#include <Arduino.h>
#endif

#include "HumidityGlobals.h"

typedef enum { DHT_11, DHT_2x } DHTType; // тип датчика, который опрашиваем, поскольку у DHT11 немного другой формат данных
enum { DHT2x_WAKEUP=1, DHT11_WAKEUP=18 }; // таймауты инициализации для разных типов датчиков



class DHTSupport
{
  private:

  HumidityAnswer answer;

  public:
    DHTSupport();
    const HumidityAnswer& read(uint8_t pin, DHTType sensorType); // читаем показания с датчика
};

#endif
