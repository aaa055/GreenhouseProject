#include "Arduino.h"
#include "LoopModule.h"
#include "ModuleController.h"
#include "CommandParser.h"

void LoopModule::Setup()
{
  Update(0);
}

void LoopModule::Update(uint16_t dt)
{
  size_t sz = vec.size();
  for(size_t i=0;i<sz;i++)
  {
      LoopLink* lnk = vec[i];
      if(!lnk->bActive || !lnk->interval || (lnk->countPasses > 0 && lnk->currPass >= lnk->countPasses)) // связанный модуль неактивен
      {
        continue;
      }

      
      lnk->lastTimerVal += dt;
      if(lnk->lastTimerVal >= lnk->interval)
      {
        // need to do the job
        lnk->lastTimerVal = lnk->lastTimerVal - lnk->interval;
        if(lnk->countPasses > 0)
        {
          lnk->currPass++;
          if(lnk->currPass >= lnk->countPasses) // все проходы выполнены
          {
            lnk->currPass = 0;
            lnk->bActive = false;
          } // if
        } // if(lnk->countPasses > 0)

          // конструируем команду
          Command com;
          com.SetIncomingStream(lnk->IncomingCtream); // не забываем сохранить вызвавший команду поток
          com.Construct(lnk->linkedModule->GetID(),lnk->paramsToPass,lnk->typeOfCommand,mainController->GetWorkMode());

          com.SetInternal(true); // говорим, что команда - от одного модуля к другому

          // ПРОСИМ МОДУЛЬ ВЫПОЛНИТЬ КОМАНДУ, КОТОРУЮ ЕМУ НАДО ВЫПОЛНЯТЬ ВРЕМЯ ОТ ВРЕМЕНИ
          // ОБНОВИТ СОСТОЯНИЕ ПЕРИФЕРИИ ДЛЯ ЭТОГО МОДУЛЯ САМ КОНТРОЛЛЕР, ВЫЗВАВ МЕТОД UPDATE МОДУЛЯ
          lnk->linkedModule->ExecCommand(com);

     } // if
      
        
  } // for
}

LoopLink* LoopModule::AddLink(const Command& command)
{

uint8_t paramsCount = command.GetArgsCount(); // сколько параметров пришло?
  if(paramsCount < MIN_LOOP_PARAMS) // мало параметров передано
  {
     SetPublishData(&command,false,PARAMS_MISSED); // сохраняем данные для публикации
     mainController->Publish(this); // публикуем их от своего имени
    return NULL;
  }

 String loopName = command.GetArg(LOOP_NAME_IDX); // получили имя циклически вызываемой команды
 String moduleID = command.GetArg(MODULE_ID_IDX); // получили ID модуля
 LoopLink* lnk = GetLink(loopName);

 if(!lnk) // надо регистрировать новый модуль
  {
    AbstractModule* regModule = GetRegisteredModule(moduleID);
    if(!regModule)
    {
      SetPublishData(&command,false,UNKNOWN_MODULE); // сохраняем данные для публикации
      mainController->Publish(this); // публикуем
      return false;
    }
    // добавляем новую связь
    lnk = new LoopLink;
    if(!lnk)
      return false;

      lnk->linkedModule = regModule;
      vec.push_back(lnk);
      
  } // if

    lnk->loopName = loopName; // сохраняем имя команды
    lnk->IncomingCtream = command.GetIncomingStream();
    lnk->typeOfCommand = command.GetArg(COMMAND_TYPE_IDX);
    lnk->interval = command.GetArg(INTERVAL_IDX).toInt();
    lnk->bActive = (lnk->interval > 0 ? true : false);
    lnk->lastTimerVal = 0;
    lnk->countPasses = command.GetArg(COUNT_PASSES_IDX).toInt();
    lnk->currPass = 0; // ноль проходов

    lnk->paramsToPass = F(""); // чистим параметры
    // сохраняем параметры
    for(uint16_t i=MIN_LOOP_PARAMS;i<paramsCount;i++)
    {
      if(i > MIN_LOOP_PARAMS)
        lnk->paramsToPass += PARAM_DELIMITER;
        
      lnk->paramsToPass += command.GetArg(i);
    } // for

    return lnk;

}

AbstractModule* LoopModule::GetRegisteredModule(const String& moduleID)
{
    return mainController->GetModuleByID(moduleID);
}

LoopLink* LoopModule::GetLink(const String& loopName)
{

        size_t sz = vec.size();
        for(size_t i =0;i<sz;i++)
        { 
          if(vec[i]->loopName == loopName) // нашли команду с таким же именем
            return vec[i];
        } // for
  
  return NULL;
}

bool  LoopModule::ExecCommand(const Command& command)
{
  
  if(command.GetType() == ctGET) // получить состояние связанного модуля
  {

      return AddLink(command) ? true : false;
  } // if
  else
  if(command.GetType() == ctSET) // установить состояние связанного модуля
  {
      return AddLink(command) ? true : false;
  } // if

  return false;
}

