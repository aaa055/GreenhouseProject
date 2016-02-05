#ifndef _ABSTRACT_MODULE_H
#define _ABSTRACT_MODULE_H

#include <WString.h>

#include "Globals.h"
#include "CommandParser.h"
#include "Publishers.h"
#include "ModuleController.h"
#include "TinyVector.h"


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
    return String(Value) + F(",") + (Fract < 10 ? F("0") : F("")) + String(Fract);
  }
};


typedef enum
{
StateTemperature = 1, // есть температурные датчики
StateRelay = 2, // есть реле
StateLuminosity = 4 // есть датчики освещенности

} ModuleStates; // вид состояния

struct OneState
{
    ModuleStates Type; // тип состояния (температура, освещенность, каналы реле)
    uint8_t Index; // индекс (например, датчика температуры)
    void* Data; // данные с датчика
    void* PreviousData; // предыдущие данные с датчика
};

typedef Vector<OneState*> StateVec;

class ModuleState
{
 uint8_t supportedStates; // какие состояния поддерживаем?
 StateVec states; // какие состояния поддерживаем?

 bool IsStateChanged(OneState* s);

public:
  ModuleState();

  bool HasState(ModuleStates state); // проверяет, поддерживаются ли такие состояния?
  bool HasChanges(); // проверяет, есть ли изменения во внутреннем состоянии модуля?
  void AddState(ModuleStates state, uint8_t idx); // добавляем состояние и привязываем его к индексу
  void UpdateState(ModuleStates state, uint8_t idx, void* newData); // обновляем состояние модуля (например, показания с температурных датчиков);
  uint8_t GetStateCount(ModuleStates state); // возвращает кол-во состояний определённого вида (например, кол-во датчиков температуры)
  OneState* GetState(ModuleStates state, uint8_t idx); // возвращает состояние определённого вида по индексу
  bool IsStateChanged(ModuleStates state, uint8_t idx); // проверяет, не изменилось ли состояние по индексу?

 
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
