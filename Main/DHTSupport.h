#ifndef _DHT_SUPPORT_H
#define _DHT_SUPPORT_H

#if ARDUINO < 100
#include <WProgram.h>
#else
#include <Arduino.h>
#endif

typedef enum { DHT11, DHT2x } DHTType; // тип датчика, который опрашиваем, поскольку у DHT11 немного другой формат данных
enum { DHT2x_WAKEUP=1, DHT11_WAKEUP=18 }; // таймауты инициализации для разных типов датчиков

typedef struct
{
  bool IsOK; // данные получены?
  uint8_t Humidity; // целое значение влажности
  uint8_t HumidityDecimal; // значение влажности после запятой
  int8_t Temperature; // целое значение температуры
  uint8_t TemperatureDecimal; // значение температуры после запятой
  
} DHTAnswer; // ответ от датчика

class DHTSupport
{
  private:

  DHTAnswer answer;

  public:
    DHTSupport();
    const DHTAnswer& read(uint8_t pin, DHTType sensorType); // читаем показания с датчика
};

#endif
