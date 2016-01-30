#ifndef _WATERING_MODULE_H
#define _WATERING_MODULE_H

#include "AbstractModule.h"
#include "Globals.h"

typedef enum
{
  wwmAutomatic, // в автоматическом режиме
  wwmManual // в ручном режиме
  
} WateringWorkMode; // режим работы полива

typedef struct
{
  bool IsChannelRelayOn; // включено ли реле канала?
  long WateringTimer; // таймер полива для канала
    
} WateringChannel; // канал для полива

class WateringModule : public AbstractModule // модуль управления поливом
{
  private:

  WateringChannel wateringChannels[WATER_RELAYS_COUNT]; // каналы полива
  WateringChannel dummyAllChannels; // управляем всеми каналами посредством этой структуры

  ModuleController* controller; // контроллер
  GlobalSettings* settings; // настройки

  WateringWorkMode workMode; // текущий режим работы

  int8_t lastDOW; // день недели с момента предыдущего опроса
  int8_t currentDOW; // текущий день недели
  int8_t currentHour; // текущий час
  bool bIsRTClockPresent; // флаг наличия модуля часов реального времени

   uint16_t lastBlinkInterval;
   void BlinkWorkMode(uint16_t blinkInterval = 0);

   void UpdateChannel(int8_t channelIdx, WateringChannel* channel, uint16_t dt); // обновляем состояние канала

   void HoldChannelState(int8_t channelIdx, WateringChannel* channel);  // поддерживаем состояние реле для канала.
   void HoldPumpState(WateringOption wateringOption); // поддерживаем состояние реле насоса

   bool IsAnyChannelActive(WateringOption wateringOption); // возвращает true, если хотя бы один из каналов активен
    
  public:
    WateringModule() : AbstractModule(F("WATER")) {}

    bool ExecCommand(const Command& command);
    void Setup();
    void Update(uint16_t dt);

};


#endif
