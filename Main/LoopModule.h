#ifndef _LOOPMODULE_H
#define _LOOPMODULE_H

#include "AbstractModule.h"
#include "TinyVector.h"

struct LoopLink // структура хранения информации для отсыла команды связанному модулю
{
  char* loopName; // имя команды
  String paramsToPass; // параметры, которые надо передать связанному модулю
  AbstractModule* linkedModule; // модуль, которому мы пересылаем команду через нужные интервалы
  bool bActive; // флаг активности работы
  unsigned long lastTimerVal; // последнее значение таймера
  unsigned long interval; // интервал работы команды
  uint8_t currPass; // номер текущего прохода
  uint8_t countPasses; // сколько проходов сделать всего
  uint8_t commandType; // тип команды, которую надо передать связанному модулю

  LoopLink()
  {
    loopName = NULL;
    linkedModule = NULL;
  }
};

typedef Vector<LoopLink*> LinkVector;

class LoopModule : public AbstractModule
{
  private:

  LinkVector vec;
  AbstractModule* GetRegisteredModule(const String& moduleID);
  LoopLink* GetLink(const char* loopName);
  LoopLink* AddLink(const Command& command, bool wantAnswer);
  
  public:
    LoopModule() : AbstractModule("LOOP") {}

    bool ExecCommand(const Command& command, bool wantAnswer);
    void Setup();
    void Update(uint16_t dt);
};

#endif
