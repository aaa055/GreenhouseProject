#ifndef _TEMP_SENSORS_H
#define _TEMP_SENSORS_H

#include "AbstractModule.h"
#include "DS18B20Query.h"
#include "InteropStream.h"

typedef enum
{
  wmAutomatic, // автоматический режим управления окнами
  wmManual // мануальный режим управления окнами
  
} WindowWorkMode;

typedef enum
{
  dirNOTHING,
  dirOPEN,
  dirCLOSE
  
} DIRECTION;

class WindowState
{
 private:
 
  unsigned long CurrentPosition; // текущая позиция фрамуги
  unsigned long RequestedPosition; // какую позицию запросили
  bool OnMyWay; // флаг того, что фрамуга в процессе открытия/закрытия
  unsigned long TimerInterval; // сколько работать фрамуге?
  unsigned long TimerTicks; // сколько проработали уже?
  DIRECTION Direction;

  void SwitchRelays(uint8_t rel1State = SHORT_CIRQUIT_STATE, uint8_t rel2State = SHORT_CIRQUIT_STATE);

  uint8_t RelayChannel1;
  uint8_t RelayChannel2;
  uint8_t RelayPin1;
  uint8_t RelayPin2;
  ModuleState* RelayStateHolder;

public:

  bool IsBusy() {return OnMyWay;} // заняты или нет?
  
  bool ChangePosition(DIRECTION dir, unsigned long newPos); // меняет позицию
  
  unsigned long GetCurrentPosition() {return CurrentPosition;}
  unsigned long GetRequestedPosition() {return RequestedPosition;}
  DIRECTION GetDirection() {return Direction;}

  void UpdateState(uint16_t dt); // обновляет состояние фрамуги
  
  void Setup(ModuleState* state, uint8_t relayChannel1, uint8_t relayChannel2, uint8_t relayPin1, uint8_t relayPin2); // настраиваем перед пуском


  WindowState() 
  {
    CurrentPosition = 0;
    RequestedPosition = 0;
    OnMyWay = false;
    TimerInterval = 0;
    TimerTicks = 0;
    RelayChannel1 = 0;
    RelayChannel2 = 0;
    RelayStateHolder = NULL;
  }  
  
  
};

class TempSensors : public AbstractModule // модуль опроса температурных датчиков и управления фрамугами
{
  private:
  
    uint16_t lastUpdateCall;

    WindowState Windows[SUPPORTED_WINDOWS];
    void SetupWindows();


    WindowWorkMode workMode; // текущий режим работы (автоматический или ручной)

    BlinkModeInterop blinker;

    DS18B20Support tempSensor;
    DS18B20Temperature tempData;
    
  public:
    TempSensors() : AbstractModule(F("STATE")){}

    bool ExecCommand(const Command& command, bool wantAnswer);
    void Setup();
    void Update(uint16_t dt);

    WindowWorkMode GetWorkMode() {return workMode;}
    void SetWorkMode(WindowWorkMode m) {workMode = m;}

};

#endif
