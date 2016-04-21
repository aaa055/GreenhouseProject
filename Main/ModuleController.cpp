#include "ModuleController.h"
#include "InteropStream.h"
#include "UniversalSensors.h"

PublishStruct PublishSingleton;

ModuleController::ModuleController(uint8_t wAs, const String& id) : workAs(wAs), ourID(id), cParser(NULL)
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
 
 UniDispatcher.Setup(this); // настраиваем диспетчера универсальных датчиков
}
void ModuleController::Setup()
{
  Command::_CMD_GET = CMD_GET;
  Command::_CMD_SET = CMD_SET;
  
  settings.Load(); // загружаем настройки
  ModuleInterop.SetController(this); // устанавливаем контроллер для класса взаимодействия между модулями

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
    mod->SetController(this); // сохраняем указатель на нас
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
  /* ВОТ ТУТ ПОСЫЛКА КОМАНДЫ ПО ПРОТОКОЛУ (НАПРИМЕР RS485) МОДУЛЮ И ПОЛУЧЕНИЕ ОТВЕТА ОТ НЕГО.
   *  
    ВОПРОС, КУДА БУДЕТ ВЫВОДИТЬСЯ ОТВЕТ. ЕСТЬ ЮЗКЕЙС, КОГДА НАДО НАСТРОИТЬ LOOP ДЛЯ ПОЛУЧЕНИЯ
    СОСТОЯНИЯ ЧЕГО-ТО ТАМ, БЕЗ ВЫВОДА ИНФОРМАЦИИ В КАКОЙ-ЛИБО ПОТОК. ПРИ ЭТОМ К МОДУЛЮ МОГУТ
    БЫТЬ ПРИВЯЗАНЫ ПОДПИСЧИКИ, КОТОРЫЕ ТАК И РВУТСЯ ВЫЛОЖИТЬ ЧТО-НИБУДЬ В ПОТОК, С КОТОРОГО 
    ПРИШЛА КОМАНДА (И НЕ ТОЛЬКО В ЭТОТ ПОТОК, К СЛОВУ). ПРОБЛЕМА
    ВОЗНИКАЕТ ВОТ ТУТ: ЕСЛИ МЫ ПИШЕМ ИЗ SERIAL, ТО И ОТВЕТ МЫ ХОТИМ ПОЛУЧИТЬ ТАМ ЖЕ. ИЛИ - 
    НЕ ХОТИМ. ДРУГОЙ ПРИМЕР - ДОЧЕРНИЙ МОДУЛЬ ЗАПИСАЛ НАМ КОМАНДУ НА ДЕРГАНЬЕ СЕБЯ (ДЛЯ
    СЧИТЫВАНИЯ ТЕМПЕРАТУРЫ, НАПРИМЕР) ЧЕРЕЗ WI-FI, СЛЕДОВАТЕЛЬНО, ВЫВОДИТЬ РЕЗУЛЬТАТЫ КОМАНДЫ
    В SERIAL В ЭТОМ СЛУЧАЕ НЕ НАДО, ДАЖЕ ЕСЛИ ДЛЯ МОДУЛЯ ЕСТЬ ПОДПИСЧИК НА ПУБЛИКАЦИЮ В SERIAL.

    КОРОЧЕ: ЕСТЬ СИТУАЦИЯ, КОГДА НАДО ТУПО ЗАПРЕТИТЬ ВЫВОД ОТВЕТА В ЛЮБОЙ ПОТОК, НАПРИМЕР, 
    ЕСЛИ МЫ ПРОСТО ЦИКЛИЧЕСКИ ОПРАШИВАЕМ ДАТЧИК, ЧТОБЫ СОХРАНИТЬ ЕГО СТАТУС ГДЕ-БЫ ТО НИ БЫЛО.

    ХОТЯ - ДАТЧИК И ТАК БУДЕТ ОПРАШИВАТЬСЯ ВНУТРИ UPDATE НУЖНОГО МОДУЛЯ, ЭТО ЗАВИСИТ ОТ РЕАЛИЗАЦИИ
    МОДУЛЯ. ТОГДА В ЭТОМ СЛУЧАЕ ПРИ РАБОТЕ LOOP МОДУЛЬ ПРОСТО ВЫДАСТ НАМ ИНФОРМАЦИЮ С ДАТЧИКА.
    НО ЭТО НЕ ОТМЕНЯЕТ ВОПРОСА О ТОМ, ВЫВОДИТЬ ИЛИ НЕТ РЕЗУЛЬТАТЫ ОТВЕТА В ПОТОК!!!
  */
  
  
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
    if(mod->GetID() == id)
      return mod;
  } // for
  return NULL;
}

void ModuleController::ProcessModuleCommand(const Command& c, AbstractModule* mod, bool checkDestination)
{
  if(checkDestination && (c.GetDestination() != workAs) ) // команда не для нас! 
  {
    return;
  }

#ifdef _DEBUG
///Serial.println("called: " +  c.GetTargetModuleID() + PARAM_DELIMITER + c.GetRawArguments());
#endif  
  /*
   * например, контроллер работает в режиме дочернего модуля, тогда он отзывается только на команды
   * с префиксом CD, тогда как в режиме работы как контроллер - префикс CT.
   * 
   */
  
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

