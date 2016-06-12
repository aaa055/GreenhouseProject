#ifndef _WATERING_MODULE_H
#define _WATERING_MODULE_H

#include "AbstractModule.h"
#include "Globals.h"
#include "InteropStream.h"


typedef enum
{
  wwmAutomatic, // в автоматическом режиме
  wwmManual // в ручном режиме
  
} WateringWorkMode; // режим работы полива

class WateringChannel // канал для полива
{
  
private:
  
  bool rel_on; // включено ли реле канала?
  bool last_rel_on; // последнее состояние реле канала

public:

  bool IsChannelRelayOn() {return rel_on;}
  void SetRelayOn(bool bOn) { last_rel_on = rel_on; rel_on = bOn; }
  bool IsChanged() {return last_rel_on != rel_on; }
  
  unsigned long WateringTimer; // таймер полива для канала
  unsigned long WateringDelta; // дельта дополива
    
}; 

class WateringModule : public AbstractModule // модуль управления поливом
{
  private:

  #if WATER_RELAYS_COUNT > 0
  
  WateringChannel wateringChannels[WATER_RELAYS_COUNT]; // каналы полива
  WateringChannel dummyAllChannels; // управляем всеми каналами посредством этой структуры
  void UpdateChannel(int8_t channelIdx, WateringChannel* channel, uint16_t dt); // обновляем состояние канала
  void HoldChannelState(int8_t channelIdx, WateringChannel* channel);  // поддерживаем состояние реле для канала.
  bool IsAnyChannelActive(uint8_t wateringOption); // возвращает true, если хотя бы один из каналов активен

  bool internalNeedChange;

  #endif


  GlobalSettings* settings; // настройки

  uint8_t workMode; // текущий режим работы

  int8_t lastAnyChannelActiveFlag; // флаг последнего состояния активности каналов

  uint8_t lastDOW; // день недели с момента предыдущего опроса
  uint8_t currentDOW; // текущий день недели
  uint8_t currentHour; // текущий час
  bool bIsRTClockPresent; // флаг наличия модуля часов реального времени
#ifdef USE_WATERING_MANUAL_MODE_DIODE
  BlinkModeInterop blinker;
#endif



#ifdef USE_PUMP_RELAY   
   void HoldPumpState(bool anyChannelActive); // поддерживаем состояние реле насоса
   bool bPumpIsOn;
#endif

    
  public:
    WateringModule() : AbstractModule("WATER") {}

    bool ExecCommand(const Command& command, bool wantAnswer);
    void Setup();
    void Update(uint16_t dt);

};


#endif
