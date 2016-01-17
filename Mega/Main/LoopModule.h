#ifndef _LOOPMODULE_H
#define _LOOPMODULE_H

#include "AbstractModule.h"
#include "TinyVector.h"

#define MIN_LOOP_PARAMS 4 // минимальное количество параметров, которые надо передать
#define MAX_LOOP_PARAMS 15 // максимальное кол-во параметров, которые можно передать
#define MODULE_ID_IDX 3 // индекс ID модуля в параметрах
#define COMMAND_TYPE_IDX 0 // индекс типа команды в параметрах
#define INTERVAL_IDX 1 // индекс параметра "интервал"
#define COUNT_PASSES_IDX 2 // индекс параметра "кол-во проходов"

/*
 * Структура команды LOOP:
 * 
 * LOOP|SET_OR_GET|INTERVAL|COUNT_PASSES|LINKED_MODULE_ID|PARAMS_FOR_MODULE
 * где
 *  SET_OR_GET = SET или GET команда для связанного модуля
 *  INTERVAL - интервал в мс между вызовами (0 - выключить модуль из циклического опроса)
 *  COUNT_PASSES - кол-во проходов (0 - бесконечно)
 *  LINKED_MODULE_ID - идентификатор модуля для передачи команд
 *  PARAMS_FOR_MODULE - параметры для связанного модуля (разделенные '|')
 *  
 *  Для примера, команда
 *  CTSET=LOOP|SET|500|10|PIN|13|T
 *  передаст параметры 13 и T модулю с идентификатором "PIN" 10 раз через каждые 500 мс
 *  
 *  Выключить модуль из обработки просто:
 *  CTSET=LOOP|SET|0|0|PIN|13
 *  
 *    Если для модуля поступила новая команда - старая перезаписывается, т.е. цепочка команд не поддерживается!
 */

struct LoopLink
{
  Stream* IncomingCtream;
  AbstractModule* linkedModule;
  bool bActive;
  uint16_t lastTimerVal;
  uint16_t interval;
  uint8_t currPass;
  uint8_t countPasses;
  String paramsToPass;
  String typeOfCommand;

};

typedef Vector<LoopLink*> LinkVector;

class LoopModule : public AbstractModule
{
  private:

  LinkVector vec;
  AbstractModule* GetRegisteredModule(const String& moduleID);
  LoopLink* GetLink(const String& moduleID);
  LoopLink* AddLink(const Command& command);
  
  public:
    LoopModule(const String& id) : AbstractModule(id) {}

    bool ExecCommand(const Command& command);
    void Setup();
    void Update(uint16_t dt);
};

#endif
