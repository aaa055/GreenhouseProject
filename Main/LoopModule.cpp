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
        

          // конструируем команду
       
          Command com;
          com.Construct(lnk->linkedModule->GetID(),lnk->paramsToPass.c_str(),lnk->commandType);


          com.SetInternal(true); // говорим, что команда - от одного модуля к другому

          MainController->ProcessModuleCommand(com,lnk->linkedModule);

        if(lnk->countPasses > 0)
        {
          lnk->currPass++;
          if(lnk->currPass >= lnk->countPasses) // все проходы выполнены
          {
            lnk->currPass = 0;
            lnk->bActive = false;            
          } // if
        } // if(lnk->countPasses > 0)          

     } // if
      
        
  } // for
}

LoopLink* LoopModule::AddLink(const Command& command, bool wantAnswer)
{
  
uint8_t paramsCount = command.GetArgsCount(); // сколько параметров пришло?
  if(paramsCount < MIN_LOOP_PARAMS) // мало параметров передано
  {
    if(wantAnswer) 
      PublishSingleton = PARAMS_MISSED;
      
    MainController->Publish(this,command); // публикуем их от своего имени
    return NULL;
  }

 const char* loopName = command.GetArg(LOOP_NAME_IDX); // получили имя циклически вызываемой команды
 String moduleID = command.GetArg(MODULE_ID_IDX); // получили ID модуля
 LoopLink* lnk = GetLink(loopName);

 if(!lnk) // надо регистрировать новый модуль
  {
    AbstractModule* regModule = GetRegisteredModule(moduleID);
    if(!regModule)
    {
      if(wantAnswer) 
        PublishSingleton = UNKNOWN_MODULE;
        
      MainController->Publish(this,command); // публикуем
      return false;
    }
    // добавляем новую связь
    lnk = new LoopLink;
    if(!lnk)
      return false;

      lnk->linkedModule = regModule;
      vec.push_back(lnk);
      
  } // if

    delete [] lnk->loopName;
    uint8_t len = strlen(loopName);
    lnk->loopName = new char[len + 1];
    strcpy(lnk->loopName,loopName); // сохраняем имя команды
    lnk->loopName[len] = 0;
    
    lnk->commandType = !strcmp_P(command.GetArg(COMMAND_TYPE_IDX), (const char*) F("SET")) ? ctSET : ctGET;
    lnk->interval = atol(command.GetArg(INTERVAL_IDX));
    lnk->bActive = (lnk->interval > 0 ? true : false);
    lnk->lastTimerVal = 0;
    lnk->countPasses = (uint8_t) atoi(command.GetArg(COUNT_PASSES_IDX));
    lnk->currPass = 0; // ноль проходов

    lnk->paramsToPass = F(""); // чистим параметры
    // сохраняем параметры
    for(uint16_t i=MIN_LOOP_PARAMS;i<paramsCount;i++)
    {
      if(i > MIN_LOOP_PARAMS)
        lnk->paramsToPass += PARAM_DELIMITER;
        
      lnk->paramsToPass += command.GetArg(i);
    } // for

     if(wantAnswer) 
      PublishSingleton = REG_SUCC;
      
     PublishSingleton.Status = true;
     MainController->Publish(this,command); // публикуем их от своего имени

    return lnk;

}

AbstractModule* LoopModule::GetRegisteredModule(const String& moduleID)
{
    return MainController->GetModuleByID(moduleID);
}

LoopLink* LoopModule::GetLink(const char* loopName)
{

        size_t sz = vec.size();
        for(size_t i =0;i<sz;i++)
        { 
          if(!strcmp(vec[i]->loopName,loopName)) // нашли команду с таким же именем
            return vec[i];
        } // for
  
  return NULL;
}

bool  LoopModule::ExecCommand(const Command& command, bool wantAnswer)
{
  
  if(command.GetType() == ctGET) // получить состояние связанного модуля
  {

      return AddLink(command,wantAnswer) ? true : false;
  } // if
  else
  if(command.GetType() == ctSET) // установить состояние связанного модуля
  {
      return AddLink(command,wantAnswer) ? true : false;
  } // if

  return true;
}

