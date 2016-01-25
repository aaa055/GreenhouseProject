#ifndef _ABSTRACT_MODULE_H
#define _ABSTRACT_MODULE_H

#include <WString.h>

#include "Globals.h"
#include "CommandParser.h"
#include "Publishers.h"
#include "ModuleController.h"

class ModuleController;

struct Temperature // структура показаний с датчика температуры
{
  int8_t Value; // значение градусов (-128 - 127)
  uint8_t Fract; // сотые доли градуса (значение после запятой)

  bool operator!=(const Temperature& rhs)
  {
    return !(Value == rhs.Value && Fract == rhs.Fract);
  }

  bool operator==(const Temperature& rhs)
  {
    return (Value == rhs.Value && Fract == rhs.Fract);
  }

  operator String() // возвращаем значение температуры как строку
  {
    return String(Value) + F(",") + String(Fract);
  }
};

class ModuleState // состояние модуля - датчики, реле и пр.
{
  uint8_t TempSensors; // кол-во температурных датчиков
  Temperature prevTemp[MAX_TEMP_SENSORS]; // предыдущие показания температуры
  Temperature Temp[MAX_TEMP_SENSORS]; // температура по датчикам

  uint8_t RelayChannels; // кол-во каналов реле, МАКСИМУМ - 8

  uint8_t prevRelayStates; // предыдущее состояние каналов реле
  uint8_t RelayStates; // текущее состояние каналов реле
 
public:

  
  bool HasTemperature() {return TempSensors > 0;} // поддерживаем датчики температуры?
  bool HasRelay() {return RelayChannels > 0; } // поддерживаем реле?
   
  void SetTempSensors(uint8_t cnt);
  uint8_t GetTempSensors() {return TempSensors;}

  void SetRelayChannels(uint8_t cnt);
  uint8_t GetRelayChannels() {return RelayChannels;}

  bool GetRelayState(uint8_t idx);
  bool GetPrevRelayState(uint8_t idx);

  void SetRelayState(uint8_t idx,const String& state);
  void SetRelayState(uint8_t idx,bool bOn);
  bool IsRelayStateChanged(uint8_t idx); // изменилось ли состояние канала реле?

  Temperature GetTemp(uint8_t idx);
  Temperature GetPrevTemp(uint8_t idx);
  
  void SetTemp(uint8_t idx, const Temperature& dt);
  bool IsTempChanged(uint8_t idx); // изменилась ли температура на датчике по переданному индексу?
  
  ModuleState() :
  TempSensors(0)
  , RelayChannels(0)
  , prevRelayStates(0)
  , RelayStates(0)
  {

    for(uint8_t i=0;i<MAX_TEMP_SENSORS;i++)
    {
        Temp[i].Value = NO_TEMPERATURE_DATA;
        prevTemp[i].Value = NO_TEMPERATURE_DATA;

        Temp[i].Fract = 0;
        prevTemp[i].Fract = 0;

    } // for
  }
};

class AbstractModule
{
  private:
    String moduleID;    
    ModuleController* controller;
    AbstractPublisher* publishers[MAX_PUBLISHERS];
    
    void InitPublishers()
    {
      for(uint8_t i=0;i<MAX_PUBLISHERS;i++)
        publishers[i] = NULL;
    }

protected:

  PublishStruct toPublish; // структура для публикации данных от модуля
  void SetPublishData(const Command* srcCommand, bool stat, const String& text,bool addModuleID = true, void* data = NULL)
  {
    toPublish.SourceCommand = srcCommand;
    toPublish.Module = this;
    toPublish.Status = stat;
    toPublish.Text = text;
    toPublish.AddModuleIDToAnswer = addModuleID;
    toPublish.Data = data;
  }
    
public:

  AbstractModule(const String& id) : moduleID(id), controller(NULL){ InitPublishers(); }

  ModuleState State; // текущее состояние модуля

  bool AddPublisher(AbstractPublisher* p);

  // публикуем изменения на всех подписчиков
  void Publish();

  
  void SetController(ModuleController* c) {controller = c;}
  ModuleController* GetController() {return controller;}
  String GetID() {return moduleID;}
  void SetID(const String& id) {moduleID = id;}

  // функции, перегружаемые наследниками
  virtual bool ExecCommand(const Command& command) = 0; // вызывается при приходе текстовой команды для модуля 
  virtual void Setup() = 0; // вызывается для настроек модуля
  virtual void Update(uint16_t dt) = 0; // обновляет состояние модуля (для поддержки состояния периферии, например, включение диода)
  
};

#endif
