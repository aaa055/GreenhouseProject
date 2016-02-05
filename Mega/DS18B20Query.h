#ifndef _DS18B20_QUERY_H
#define _DS18B20_QUERY_H

#include <Arduino.h>

typedef struct
{
  bool Negative;
  int Whole;
  int Fract;
  
} DS18B20Temperature;

class DS18B20Support
{
  private:

  uint8_t pin;

  public:
    DS18B20Support() : pin(0) {};

    void begin(uint8_t _pin) {pin = _pin;}
    bool readTemperature(DS18B20Temperature* result); 
    
};

#endif
