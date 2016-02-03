#include "ModuleController.h"
#include "InteropStream.h"

ModuleController::ModuleController(COMMAND_DESTINATION wAs, const String& id) : workAs(wAs), ourID(id), cParser(NULL)
{
  settings.Load(); // загружаем настройки
}
#ifdef USE_DS3231_REALTIME_CLOCK
DS3231Clock& ModuleController::GetClock()
{
  return _rtc;
}
#endif
void ModuleController::begin()
{
#ifdef USE_DS3231_REALTIME_CLOCK
_rtc.begin();
#endif

  ModuleInterop.SetController(this); // устанавливаем контроллер для класса взаимодействия между модулями
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

void ModuleController::PublishToStream(Stream* pStream,bool bOk, const String& Answer)
{
  // Публикуем в переданный стрим
  if(!pStream)
  {
    return;
  } 
     pStream->print(bOk ? OK_ANSWER : ERR_ANSWER);
     pStream->print(COMMAND_DELIMITER);
    
     pStream->println(Answer);
}

void ModuleController::CallRemoteModuleCommand(AbstractModule* mod, const String& command)
{
  pStream->println("BROADCAST THE COMMAND \"" + command + "\"");

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

void ModuleController::Publish(AbstractModule* module)
{

  if(module)
  {
    // публикуем ответ для всех, кто подписался на ответ от модуля
    module->Publish();
  } // if(module)
  
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

void ModuleController::ProcessModuleCommand(const Command& c, bool checkDestination)
{
  if(checkDestination && (c.GetDestination() != workAs) ) // команда не для нас! 
  {
    return;
  }
  /*
   * например, контроллер работает в режиме дочернего модуля, тогда он отзывается только на команды
   * с префиксом CD, тогда как в режиме работы как контроллер - префикс CT.
   * 
   */
  
 AbstractModule* mod =  GetModuleByID(c.GetTargetModuleID());
 if(!mod)
 {
  //TODO: МОДУЛЬ НЕ НАЙДЕН, ВОТ ЗАСАДА! НО: ТУТ МОЖНО ПЕРЕНАПРАВЛЯТЬ ЗАПРОС БРОАДКАСТОМ В СЕТЬ,
  // МОЖЕТ - КАКОЙ-НИБУДЬ МОДУЛЬ НА НЕЁ ОТВЕТИТ. ЕЩЁ ЛУЧШЕ - КОГДА БУДЕТ РЕАЛИЗОВАНА ПРОЗРАЧНАЯ
  // РЕГИСТРАЦИЯ МОДУЛЕЙ В СИСТЕМЕ - ПРОСТО СМОТРЕТЬ СПИСОК ТАКИХ МОДУЛЕЙ И ИСКАТЬ НУЖНЫЙ МОДУЛЬ ТАМ.
  // ПРИ ТАКОМ ПОДХОДЕ НАКЛАДНЫЕ РАСХОДЫ ВОЗРАСТУТ НЕЗНАЧИТЕЛЬНО.

  // А ПОКА - МЫ ПРОСТО СООБЩАЕМ, ЧТО МОДУЛЬ С ПЕРЕДАННЫМ ИМЕНЕМ НАМ НЕИЗВЕСТЕН.
  // Сообщаем в тот поток, откуда пришел запрос.
  PublishToStream(c.GetIncomingStream(),false,UNKNOWN_MODULE);
  return;
 }
     // нашли модуль
 mod->ExecCommand(c); // выполняем его команду
 
}

void ModuleController::UpdateModules(uint16_t dt)
{
  size_t sz = modules.size();
  for(size_t i=0;i<sz;i++)
  { 
    AbstractModule* mod = modules[i];
    if(mod)
    {
      // ОБНОВЛЯЕМ СОСТОЯНИЕ МОДУЛЕЙ
      mod->Update(dt);
    } // if
  } // for
}

