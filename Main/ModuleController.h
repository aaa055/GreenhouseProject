#ifndef _MODULE_CONTROLLER_H
#define _MODULE_CONTROLLER_H

#include <Arduino.h>
#include "Globals.h"
#include "AbstractModule.h"
#include "CommandParser.h"
#include "TinyVector.h"
#include "Settings.h"

#ifdef USE_DS3231_REALTIME_CLOCK
#include "DS3231Support.h"
#endif

typedef Vector<AbstractModule*> ModulesVec;

typedef void (*CallbackUpdateFunc)(AbstractModule* mod);

class ModuleController
{
 private:
  ModulesVec modules; // список зарегистрированных модулей
  Stream* pStream; // текущий поток вывода по умолчанию
  COMMAND_DESTINATION workAs; // как работаем - как контроллер или дочерний модуль?
  
  String ourID; // ID контроллера

  CommandParser* cParser; // парсер текстовых команд

  GlobalSettings settings; // глобальные настройки

#ifdef USE_DS3231_REALTIME_CLOCK
  DS3231Clock _rtc; // часы реального времени
#endif


public:
  ModuleController(COMMAND_DESTINATION wAs, const String& id);

  void begin(); // начинаем работу

  #ifdef USE_DS3231_REALTIME_CLOCK
  // модуль реального времени
  DS3231Clock& GetClock();
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
  
  void UpdateModules(uint16_t dt, CallbackUpdateFunc func);
  
  COMMAND_DESTINATION GetWorkMode() {return workAs;}
  String GetControllerID() {return ourID;}

  void CallRemoteModuleCommand(AbstractModule* mod, const String& command); // вызывает команду с другой коробочки

  void Publish(AbstractModule* module); // каждый модуль по необходимости дергает этот метод для публикации событий/ответов на запрос
  void PublishToStream(Stream* pStream,bool bOk, const String& Answer); // публикация в Serial

  void SetCommandParser(CommandParser* c) {cParser = c;};
  CommandParser* GetCommandParser() {return cParser;}
  
};

#endif
