#ifndef _LOOPMODULE_H
#define _LOOPMODULE_H

#include "AbstractModule.h"
#include "TinyVector.h"

struct LoopLink // структура хранения информации для отсыла команды связанному модулю
{
  String loopName; // имя команды
  Stream* IncomingCtream; // поток, с которого пришёл запрос на выполнение команды
  AbstractModule* linkedModule; // модуль, которому мы пересылаем команду через нужные интервалы
  bool bActive; // флаг активности работы
  uint16_t lastTimerVal; // последнее значение таймера
  uint16_t interval; // интервал работы команды
  uint8_t currPass; // номер текущего прохода
  uint8_t countPasses; // сколько проходов сделать всего
  String paramsToPass; // параметры, которые надо передать связанному модулю
  String typeOfCommand; // тип команды, которую надо передать связанному модулю

};

typedef Vector<LoopLink*> LinkVector;

class LoopModule : public AbstractModule
{
  private:

  LinkVector vec;
  AbstractModule* GetRegisteredModule(const String& moduleID);
  LoopLink* GetLink(const String& loopName);
  LoopLink* AddLink(const Command& command);
  
  public:
    LoopModule() : AbstractModule(F("LOOP")) {}

    bool ExecCommand(const Command& command);
    void Setup();
    void Update(uint16_t dt);
};

#endif
