#ifndef _NEXTION_MODULE_H
#define _NEXTION_MODULE_H

#include "AbstractModule.h"
#include "NextionController.h"
#include "Settings.h"


 typedef struct
 {
  uint8_t sensorType;
  uint8_t sensorIndex;
  const char* moduleName;
    
 } NextionWaitScreenInfo; // структура для хранения информации, которую необходимо показывать на экране ожидания


class NextionModule : public AbstractModule // модуль управления дисплеем Nextion
{
  private:
  
    NextionController nextion; // класс для управления дисплеем
    bool  isDisplaySleep;
    bool bInited;
    
    bool isWindowsOpen;
    bool isWindowAutoMode;
    
    bool isWaterOn;
    bool isWaterAutoMode;
    
    bool isLightOn;
    bool isLightAutoMode;
    
    uint8_t openTemp, closeTemp;

    unsigned long rotationTimer;
    
    GlobalSettings* sett;
    
    void updateDisplayData();
    bool windowChanged,windowModeChanged, waterChanged, waterModeChanged, lightChanged, lightModeChanged, openTempChanged, closeTempChanged;

    void displayNextSensorData(int8_t dir=1);
    int8_t currentSensorIndex;
  
  public:
    NextionModule() : AbstractModule("NXT") {}

    bool ExecCommand(const Command& command, bool wantAnswer);
    void Setup();
    void Update(uint16_t dt);
    
    void SetSleep(bool bSleep);
    void StringReceived(const char* str);

};


#endif
