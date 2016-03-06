#ifndef _ABSTRACT_MODULE_H
#define _ABSTRACT_MODULE_H

#include <WString.h>

class ModuleController; // forward declaration

#include "Globals.h"
#include "CommandParser.h"
#include "TinyVector.h"

// структура для публикации
struct PublishStruct
{
  bool Status; // Статус ответа на запрос: false - ошибка, true - нет ошибки
  String Text; // текстовое сообщение о публикации, общий для всех буфер
  bool AddModuleIDToAnswer; // добавлять ли имя модуля в ответ?
  void* Data; // любая информация, в зависимости от типа модуля
  bool Busy; // флаг, что структура занята для записи

  void Reset()
  {
    Status = false;
    Text = F("");
    AddModuleIDToAnswer = true;
    Data = NULL;
    Busy = false;
  }
  
};

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

// влажность у нас может храниться так же, как и температура, поэтому
// незачем плодить вторую структуру - просто делаем typedef.
typedef struct Temperature Humidity; 


typedef enum
{
StateTemperature = 1, // есть температурные датчики
#ifdef SAVE_RELAY_STATES
StateRelay = 2, // есть реле
#endif
StateLuminosity = 4, // есть датчики освещенности
StateHumidity = 8 // есть датчики влажности

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
    
protected:

  ModuleController* mainController;
    
public:

  AbstractModule(const String& id) : moduleID(id),mainController(NULL)
  { 

  }

  ModuleState State; // текущее состояние модуля
 
  void SetController(ModuleController* c) {mainController = c;}
  ModuleController* GetController() {return mainController;}
  String GetID() {return moduleID;}
  void SetID(const String& id) {moduleID = id;}

  // функции, перегружаемые наследниками
  virtual bool ExecCommand(const Command& command, bool wantAnswer) = 0; // вызывается при приходе текстовой команды для модуля (wantAnswer - ждут ли от нас текстового ответа) 
  virtual void Setup() = 0; // вызывается для настроек модуля
  virtual void Update(uint16_t dt) = 0; // обновляет состояние модуля (для поддержки состояния периферии, например, включение диода)
  
};

#endif
