#ifndef _MODULE_CONTROLLER_H
#define _MODULE_CONTROLLER_H

#include <Arduino.h>
#include "Globals.h"
#include "AbstractModule.h"
#include "CommandParser.h"
#include "TinyVector.h"
#include "Settings.h"

#ifdef USE_DS3231_REALTIME_CLOCK
#include <DS3231.h>
#endif

typedef Vector<AbstractModule*> ModulesVec;



class ModuleController
{
 private:
  ModulesVec modules;
  Stream* pStream; // текущий поток вывода по умолчанию
  COMMAND_DESTINATION workAs; // как работаем - как контроллер или дочерний модуль?
  String ourID;

  CommandParser* cParser;

  GlobalSettings settings;

public:
  ModuleController(COMMAND_DESTINATION wAs, const String& id);

  void Begin(); // начинаем работу

  #ifdef USE_DS3231_REALTIME_CLOCK
  // модуль реального времени
  DS3231& GetClock();
  #endif

  // возвращает текущие настройки контроллера
  GlobalSettings* GetSettings() {return &settings;}

  // устанавливает текущий поток, в который надо выводить ответы
  void SetCurrentStream(Stream* s) {pStream = s;}

  // возвращает текущий поток
  Stream* GetCurrentStream() {return pStream;}
 
  size_t GetModulesCount() {return modules.size(); }
  AbstractModule* GetModule(size_t idx) {return modules[idx]; }
  AbstractModule* GetModuleByID(const String& id);

  void RegisterModule(AbstractModule* mod);
  void ProcessModuleCommand(const Command& c, bool checkDestination=true);
  
  void UpdateModules(uint16_t dt);
  
  COMMAND_DESTINATION GetWorkMode() {return workAs;}
  String GetControllerID() {return ourID;}

  void CallRemoteModuleCommand(AbstractModule* mod, const String& command); // вызывает команду с другой коробочки

  void Publish(AbstractModule* module); // каждый модуль по необходимости дергает этот метод для публикации событий/ответов на запрос
  void PublishToStream(Stream* pStream,bool bOk, const String& Answer); // публикация в Serial

  void SetCommandParser(CommandParser* c) {cParser = c;};
  CommandParser* GetCommandParser() {return cParser;}
  
};

#endif
