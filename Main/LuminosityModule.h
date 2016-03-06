#ifndef _LUMINOSITY_MODULE_H
#define _LUMINOSITY_MODULE_H

#include "AbstractModule.h"
#include "InteropStream.h"

#include <Wire.h>

typedef enum
{
  lightAutomatic,
  lightManual
} LightWorkMode; // режим управления досветкой

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

typedef enum { BH1750Address1 = 0x23, BH1750Address2 = 0x5C } BH1750Address; // адрес датчика освещенности на шине I2C

class BH1750Support
{
  private:
    void writeByte(uint8_t toWrite);

    BH1750Address deviceAddress;
    BH1750Mode currentMode;
  
  public:
    BH1750Support();
    
    void begin(BH1750Address addr = BH1750Address1, BH1750Mode mode = ContinuousHighResolution);
    
    void ChangeMode(BH1750Mode newMode);
    void ChangeAddress(BH1750Address newAddr);
     
    long GetCurrentLuminosity();
};


class LuminosityModule : public AbstractModule // модуль управления освещенностью
{
  private:

  GlobalSettings* settings; // настройки

  BlinkModeInterop blinker;


  #if LIGHT_SENSORS_COUNT > 0
  BH1750Support lightMeter; // первый датчик освещенности
  #endif
  
  #if LIGHT_SENSORS_COUNT > 1
  BH1750Support lightMeter2; // второй датчик освещенности
  #endif

  LightWorkMode workMode;
  bool bRelaysIsOn; // включены ли реле досветки?
  uint16_t lastUpdateCall;
    
  public:
    LuminosityModule() : AbstractModule(F("LIGHT"))
    , lastUpdateCall(678) // разнесём опросы датчиков по времени
    {}

    bool ExecCommand(const Command& command, bool wantAnswer);
    void Setup();
    void Update(uint16_t dt);

};


#endif
