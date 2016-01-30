#ifndef _LUMINOSITY_MODULE_H
#define _LUMINOSITY_MODULE_H

#include "AbstractModule.h"
#include <Wire.h>


// смотрим, какие методы Wire актуальны - чтоб не париться с препроцессором в рабочем коде
#if (ARDUINO >= 100)
  #define BH1750_WIRE_READ Wire.read
  #define BH1750_WIRE_WRITE Wire.write
#else
  #define BH1750_WIRE_READ Wire.receive
  #define BH1750_WIRE_WRITE Wire.send
#endif

typedef enum
{
  ContinuousHighResolution = 0x10,
  ContinuousHighResolution2 = 0x11,
  ContinuousLowResolution = 0x13
    
} BH1750Mode;

enum { BH1750PowerOff=0x00, BH1750PowerOn=0x01, BH1750Reset = 0x07 };

enum { BH1750Address = 0x23 }; // адрес датчика освещенности на шине I2C

class BH1750Support
{
  private:
    void writeByte(uint8_t toWrite);
  
  public:
    BH1750Support();
    
    void begin(BH1750Mode mode = ContinuousHighResolution);
    void ChangeMode(BH1750Mode newMode);
     
    uint16_t GetCurrentLuminosity();
};


class LuminosityModule : public AbstractModule // модуль управления освещенностью
{
  private:

  BH1750Support lightMeter;
    
  public:
    LuminosityModule() : AbstractModule(F("LIGHT")) {}

    bool ExecCommand(const Command& command);
    void Setup();
    void Update(uint16_t dt);

};


#endif
