#include "ModuleController.h"
#include "InteropStream.h"

#include "UniversalSensors.h"

PublishStruct PublishSingleton;
ModuleController* MainController = NULL;

ModuleController::ModuleController() : cParser(NULL)
#ifdef USE_LOG_MODULE
,logWriter(NULL)
#endif
{
  PublishSingleton.Text.reserve(SHARED_BUFFER_LENGTH); // 500 байт для ответа от модуля должно хватить.
}
#ifdef USE_DS3231_REALTIME_CLOCK
DS3231Clock& ModuleController::GetClock()
{
  return _rtc;
}
#endif
void ModuleController::begin()
{
 // тут можно написать код, который выполнится непосредственно перед началом работы
 
 UniDispatcher.Setup(); // настраиваем диспетчера универсальных датчиков
 
}
void ModuleController::Setup()
{  
  MainController = this;

  settings.Load(); // загружаем настройки

#ifdef USE_DS3231_REALTIME_CLOCK
_rtc.begin();
#endif

#if  defined(USE_WIFI_MODULE) || defined(USE_LOG_MODULE)
  sdCardInitFlag = SD.begin(SDCARD_CS_PIN); // пробуем инициализировать SD-модуль
#endif
  
}

void ModuleController::Log(AbstractModule* mod, const String& message)
{
 #ifdef USE_LOG_MODULE
  if(!logWriter)
    return;

  LogAction la;
  la.RaisedModule = mod;
  la.Message = message;
  logWriter->WriteAction(la); // пишем в лог
 #else
  UNUSED(mod);
  UNUSED(message);
 #endif
}
void ModuleController::RegisterModule(AbstractModule* mod)
{
  if(mod)
  {
    mod->Setup(); // настраиваем
    modules.push_back(mod);
  }
}

void ModuleController::PublishToCommandStream(AbstractModule* module,const Command& sourceCommand)
{

  Stream* ps = sourceCommand.GetIncomingStream();
 
  // Публикуем в переданный стрим
  if(!ps)
  {
#ifdef _DEBUG
  if(PublishSingleton.Text.length())
    Serial.println(String(F("No ps, but have answer: ")) + PublishSingleton.Text);
#endif    
    PublishSingleton.Busy = false; // освобождаем структуру
    return;
  }

     ps->print(PublishSingleton.Status ? OK_ANSWER : ERR_ANSWER);
     ps->print(COMMAND_DELIMITER);

    if(PublishSingleton.AddModuleIDToAnswer && module) // надо добавить имя модуля в ответ
    {
       ps->print(module->GetID());
       ps->print(PARAM_DELIMITER);
    }
    
     ps->println(PublishSingleton.Text);

   
   PublishSingleton.Busy = false; // освобождаем структуру
}

void ModuleController::CallRemoteModuleCommand(AbstractModule* mod, const String& command)
{

  UNUSED(mod);
  UNUSED(command);
  
#ifdef _DEBUG  
  Serial.println("BROADCAST THE COMMAND \"" + command + "\"");
#endif
  
}

void ModuleController::Publish(AbstractModule* module,const Command& sourceCommand)
{

  PublishToCommandStream(module,sourceCommand);
  
}
AbstractModule* ModuleController::GetModuleByID(const String& id)
{
  size_t sz = modules.size();
  for(size_t i=0;i<sz;i++)
  { 
    AbstractModule* mod = modules[i];
    if(!strcmp(mod->GetID(),id.c_str()) )
      return mod;
  } // for
  return NULL;
}

void ModuleController::ProcessModuleCommand(const Command& c, AbstractModule* mod)
{

#ifdef _DEBUG
///Serial.println("called: " +  c.GetTargetModuleID() + PARAM_DELIMITER + c.GetRawArguments());
#endif  

if(!mod) // ничего не передали, надо искать модуль
  mod =  GetModuleByID(c.GetTargetModuleID());
  
 if(!mod)
 {
  //TODO: МОДУЛЬ НЕ НАЙДЕН, ВОТ ЗАСАДА! НО: ТУТ МОЖНО ПЕРЕНАПРАВЛЯТЬ ЗАПРОС БРОАДКАСТОМ В СЕТЬ,
  // МОЖЕТ - КАКОЙ-НИБУДЬ МОДУЛЬ НА НЕЁ ОТВЕТИТ. ЕЩЁ ЛУЧШЕ - КОГДА БУДЕТ РЕАЛИЗОВАНА ПРОЗРАЧНАЯ
  // РЕГИСТРАЦИЯ МОДУЛЕЙ В СИСТЕМЕ - ПРОСТО СМОТРЕТЬ СПИСОК ТАКИХ МОДУЛЕЙ И ИСКАТЬ НУЖНЫЙ МОДУЛЬ ТАМ.
  // ПРИ ТАКОМ ПОДХОДЕ НАКЛАДНЫЕ РАСХОДЫ ВОЗРАСТУТ НЕЗНАЧИТЕЛЬНО.

  // А ПОКА - МЫ ПРОСТО СООБЩАЕМ, ЧТО МОДУЛЬ С ПЕРЕДАННЫМ ИМЕНЕМ НАМ НЕИЗВЕСТЕН.
  // Сообщаем в тот поток, откуда пришел запрос.
  PublishSingleton.AddModuleIDToAnswer = false;
  PublishSingleton.Status = false;
  PublishSingleton = UNKNOWN_MODULE;
  PublishToCommandStream(mod,c);
  return;
 }
     // нашли модуль

CHECK_PUBLISH_CONSISTENCY;

 PublishSingleton.Reset(); // очищаем структуру для публикации
 PublishSingleton.Busy = true; // говорим, что структура занята для публикации
 mod->ExecCommand(c,c.GetIncomingStream() != NULL); // выполняем его команду
 
}

void ModuleController::UpdateModules(uint16_t dt, CallbackUpdateFunc func)
{  
  size_t sz = modules.size();
  for(size_t i=0;i<sz;i++)
  { 
    AbstractModule* mod = modules[i];
   
      // ОБНОВЛЯЕМ СОСТОЯНИЕ МОДУЛЕЙ
      mod->Update(dt);

    if(func) // вызываем функцию после обновления каждого модуля
      func(mod);
  
  } // for
}

