#ifndef _TEMP_SENSORS_H
#define _TEMP_SENSORS_H

#include "AbstractModule.h"
#include "DS18B20Query.h"
#include "InteropStream.h"

typedef struct
{
  uint8_t pin;
  uint8_t type;
  
} TempSensorSettings; // настройки сенсоров

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

class TempSensors;

class WindowState
{
 private:
 
  unsigned long CurrentPosition; // текущая позиция фрамуги
  unsigned long RequestedPosition; // какую позицию запросили
  bool OnMyWay; // флаг того, что фрамуга в процессе открытия/закрытия
  unsigned long TimerInterval; // сколько работать фрамуге?
  unsigned long TimerTicks; // сколько проработали уже?
  uint8_t Direction; // направление, которое задали

  void SwitchRelays(uint8_t rel1State = SHORT_CIRQUIT_STATE, uint8_t rel2State = SHORT_CIRQUIT_STATE);

  uint8_t RelayChannel1;
  uint8_t RelayChannel2;
 // ModuleState* RelayStateHolder;
  TempSensors* Parent;

public:

  bool IsBusy() {return OnMyWay;} // заняты или нет?
  
  bool ChangePosition(uint8_t dir, unsigned long newPos); // меняет позицию
  
  unsigned long GetCurrentPosition() {return CurrentPosition;}
  unsigned long GetRequestedPosition() {return RequestedPosition;}
  uint8_t GetDirection() {return Direction;}

  void UpdateState(uint16_t dt); // обновляет состояние фрамуги
  
  void Setup(TempSensors* parent,uint8_t relayChannel1, uint8_t relayChannel2); // настраиваем перед пуском


  WindowState() 
  {
    CurrentPosition = 0;
    RequestedPosition = 0;
    OnMyWay = false;
    TimerInterval = 0;
    TimerTicks = 0;
    RelayChannel1 = 0;
    RelayChannel2 = 0;
    Direction = dirNOTHING;
  }  
  
  
};

class TempSensors : public AbstractModule // модуль опроса температурных датчиков и управления фрамугами
{
  private:
  
    uint16_t lastUpdateCall;

    WindowState Windows[SUPPORTED_WINDOWS];
    void SetupWindows();

    #ifdef USE_WINDOWS_SHIFT_REGISTER
    void WriteToShiftRegister(); // пишем в сдвиговый регистр
    uint8_t* shiftRegisterData; // данные для сдвигового регистра
    uint8_t* lastShiftRegisterData; // последние данные, запиханные в сдвиговый регистр (чтоб не дёргать каждый раз, а только при изменениях)
    uint8_t shiftRegisterDataSize; // кол-во байт, хранящихся в массиве для сдвигового регистра
    #endif


    uint8_t workMode; // текущий режим работы (автоматический или ручной)
    // добавляем сюда небольшое значение, когда меняется режим работы.
    // это нужно для того, чтобы нормально работали правила, когда
    // происходит смена режима работы.
    uint8_t smallSensorsChange; 

#ifdef USE_WINDOWS_MANUAL_MODE_DIODE
    BlinkModeInterop blinker;
#endif    

    DS18B20Support tempSensor;
    DS18B20Temperature tempData;
    
  public:
    TempSensors() : AbstractModule("STATE"){}

    bool ExecCommand(const Command& command, bool wantAnswer);
    void Setup();
    void Update(uint16_t dt);

    uint8_t GetWorkMode() {return workMode;}
    void SetWorkMode(uint8_t m) {workMode = m;}

    void SaveChannelState(uint8_t channel, uint8_t state); // сохраняем состояние каналов

};

#endif
