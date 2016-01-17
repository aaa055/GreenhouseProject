#include "AlertModule.h"
#include "ModuleController.h"

AlertRule::AlertRule()
{
  linkedModule = NULL;
  bEnabled = true;
  whichTime = 0;
  workTime = 0;
  linkedRulesCnt = 0;
  timerTicks = 0;
  canWork = true;
  
}
void AlertRule::Update(uint16_t dt
  #ifdef USE_DS3231_REALTIME_CLOCK 
     ,uint8_t currentHour // текущий час
    , uint8_t currentMinute // текущая минута
  #endif
 )
{
  canWork = true; // считаем, что мы можем работать
  
  #ifdef USE_DS3231_REALTIME_CLOCK

   // поставим заглушку, чтобы не забыть
   Serial.println("TEST ALERT CODE WITH USE_DS3231_REALTIME_CLOCK!"); delay(5000);

      // проверяем, можем ли мы работать со временем?
      if(whichTime > 0)
      {
        // время срабатывания выставлено, надо проверять интервал
 
        switch(whichTime) // когда работаем?
        {
          case 1: // утро, 07:00-12:00
            canWork = 7*60 <= (currentHour*60 + currentMinute) < 12*60;
         break;

         case 2: // день, 12:00-19:00
            canWork = 12*60 <= (currentHour*60 + currentMinute) < 19*60;
         break;

         case 3: // вечер, 19:00-24:00
            canWork = 19*60 <= (currentHour*60 + currentMinute) < 24*60;
         break;

         case 4: // ночь, 00:00-07:00
            canWork = 0 <= (currentHour*60 + currentMinute) < 7*60;
         break;

        } // switch

        if(!canWork) // не наше время работы, игнорируем
        {
          timerTicks = 0; // по-любому обнуляем таймер работы, потому что мы все равно работать не можем
        }
      } // if whichTime > 0

  #endif

    if(canWork) // можем работать, выставляем внутренний статус дальше
    {
        if(workTime > 0)
        {
          // выставлена продолжительность работы правила, обновляем внутренний таймер
          timerTicks += dt;
          if(timerTicks > workTime)
          {
            // работу закончили, выставляем флаг работы
            canWork = false;
          }
          
        } // if workTime > 0
    } // if canWork
  
}
bool AlertRule::HasAlert()
{
  if(!linkedModule || !bEnabled || !canWork)
    return false;


  switch(target)
  {
    case rtTemp: // проверяем температуру
      if(!linkedModule->State.HasTemperature()) // не поддерживаем температуру
        return false;

      if(!linkedModule->State.IsTempChanged(tempSensorIdx)) // ничего не изменилось
      {
        return false;
      }
       int curTemp = linkedModule->State.GetTemp(tempSensorIdx).toInt();

       switch(operand)
       {
          case roLessThan: return curTemp < tempAlert;
          case roLessOrEqual: return curTemp <= tempAlert;
          case roGreaterThan: return curTemp > tempAlert;
          case roGreaterOrEqual: return curTemp >= tempAlert;
          default: return false;
       } // switch
        
    break; // rtTemp
  } // switch

  return false;
}
String AlertRule::GetLinkedRuleName(uint8_t idx)
{
    if(idx >= linkedRulesCnt)
      idx = linkedRulesCnt-1;

   return linkedRuleNames[idx];
  
}
bool AlertRule::Construct(AbstractModule* lm, const Command& command)
{
  // конструируем команду
  linkedModule = lm;

  // пришла такая команда, например:
 // RULE_ADD|N1|STATE|TEMP|0|>|25|0|0|N3|CTSET=STATE|WINDOW|ALL|OPEN|5000
 // эта команда приходит в AlertModule::ExecCommand, как команда CTSET=ALERT|....
  uint8_t argsCnt = command.GetArgsCount();
  if(argsCnt < 10) // мало аргументов
    return false;

  uint8_t curArgIdx = 1;
  
  // ищем имя
  ruleName = command.GetArg(curArgIdx++);
  alertRule = ruleName + PARAM_DELIMITER;
 

  // записываем имя связанного модуля
  alertRule += command.GetArg(curArgIdx++) + PARAM_DELIMITER;
  
  String ruleTargetStr = command.GetArg(curArgIdx++);
  
  if(ruleTargetStr == PROP_TEMP) // следим за температурой
    target = rtTemp;

  alertRule += ruleTargetStr + PARAM_DELIMITER;


  alertRule += command.GetArg(curArgIdx) + PARAM_DELIMITER;
  tempSensorIdx = command.GetArg(curArgIdx++).toInt();
  String op = command.GetArg(curArgIdx++);
  alertRule += op + PARAM_DELIMITER;
  
  if(op == GREATER_THAN)
    operand = roGreaterThan;
  else if(op == LESS_THAN)
    operand = roLessThan;
  else if(op == LESS_OR_EQUAL_THAN)
    operand = roLessOrEqual;
  else if(op == GREATER_OR_EQUAL_THAN)
    operand = roGreaterOrEqual;

  alertRule += command.GetArg(curArgIdx) + PARAM_DELIMITER;
  tempAlert = command.GetArg(curArgIdx++).toInt();


  // дошли до температуры, после неё - настройки срабатывания

  // следом идёт время, когда правило работает (0-всегда, 1-Утром (07:00-12:00), 2-Днем (12:00-19:00), 3-Вечером (19:00-24:00), 4-Ночью (00:00-07:00)
  alertRule += command.GetArg(curArgIdx) + PARAM_DELIMITER;
  whichTime = command.GetArg(curArgIdx++).toInt();

  
  // дальше идёт продолжительность работы, в минутах
  alertRule += command.GetArg(curArgIdx) + PARAM_DELIMITER;
  workTime = command.GetArg(curArgIdx++).toInt()*60*1000; // переводим в миллисекунды

  
  // далее идут правила, при срабатывании которых данное правило работать не будет
  alertRule += command.GetArg(curArgIdx);
  String linkedRules = command.GetArg(curArgIdx++);


  // парсим имена связанных правил
  if(linkedRules != F("_")) // есть связанные правила
  {

        int curNameIdx = 0;
        linkedRulesCnt = 0;
 
        while(curNameIdx != -1)
        {
          curNameIdx = linkedRules.indexOf(F(",")); // парсим по запятой
          if(curNameIdx == -1)
          {
           if(linkedRules.length() > 0)
           {
              linkedRuleNames[linkedRulesCnt++] = linkedRules;
           }
              
            break;
          } // if
          String param = linkedRules.substring(0,curNameIdx);
          linkedRules = linkedRules.substring(curNameIdx+1,linkedRules.length());
          if(param.length() > 0)
          {
             linkedRuleNames[linkedRulesCnt++] = param;
          }
          
        } // while

 
    
  } // if

  // сохраняем команду, которую надо передать какому-либо модулю
  
  targetCommand = F("");
  
  for(uint8_t i=curArgIdx;i<argsCnt;i++)
  { 
    if(targetCommand.length())
      targetCommand += PARAM_DELIMITER;

      targetCommand += command.GetArg(i);
  } // for
 
  
}
bool AlertModule::AddRule(AbstractModule* m, const Command& c)
{
  if(nextNoRuleIdx >=  MAX_ALERT_RULES)
    return false;

   AlertRule* ar = new AlertRule;
   if(!ar)
    return false;

   if(!ar->Construct(m,c))
   {
    delete ar;
    return false;
   }
   alertRules[nextNoRuleIdx] = ar;

    nextNoRuleIdx++;
    rulesCnt++;
}

void AlertModule::Setup()
{
  // настройка модуля алертов тут
  curAlertIdx = 0; // нет событий пока
  cntAlerts = 0; 
  
  for(uint8_t i = 0;i<MAX_STORED_ALERTS;i++)
  {
    strAlerts[i] = ""; // резервируем события
  } // for

}
void AlertModule::InitRules()
{
  nextNoRuleIdx = 0; // индекс следующего пустого места
  rulesCnt = 0; // кол-вo правил
  for(uint8_t i=0;i<MAX_ALERT_RULES;i++)
  {
    alertRules[i] = NULL;
  } // for
  
}
void AlertModule::Update(uint16_t dt)
{ 
  // обновление модуля алертов тут
  ModuleController* c = GetController();
  CommandParser* cParser = c->GetCommandParser();
  String controllerID = c->GetControllerID();
 
#ifdef USE_DS3231_REALTIME_CLOCK
  DS3231 rtc = c->GetClock();
  Time tm = rtc.getTime();
#endif


  Vector<AlertRule*> raisedAlerts;
  
  for(uint8_t i=0;i<nextNoRuleIdx;i++)
  {
    AlertRule* r = alertRules[i];
    if(!r)
      break;

      // сначала обновляем состояние правила
      r->Update(dt
#ifdef USE_DS3231_REALTIME_CLOCK
,tm.hour, tm.min
#endif
        );

      
      if(r->HasAlert())
      {
        // помещаем это правило в список сработавших правил
        raisedAlerts.push_back(r);
      } // if(r->HasAlert())
  } // for

  // проверяем список сработавших правил, на предмет связи их с другими сработавшими правилами
  RulesVector workRules; // правила, с которыми будем работать после разрешения конфликтов

  // разрешаем конфликты. Для этого надо пройти по всем цепочкам правил и разрешить все зависимости,
  // например: у нас есть три сработавших правила: 1 - просто, второе - не выполнять, если сработало
  // правило 3, 3 - не выполнять, если сработало правило 1. Очевидно, что в конечном списке
  // должны остаться правила 1 и 2, а не только 1, как будет, если смотреть правила поочерёдно,
  // и отбрасывать без учёта цепочек зависимостей.

  SolveConflicts(raisedAlerts,workRules); // разрешаем все конфликты
  
  // тут можем работать со сработавшими правилами спокойно
  size_t sz = workRules.size();
  
  for(size_t i=0;i<sz;i++)
  {
    // для каждого правила в списке вызываем связанную команду
    AlertRule* r = workRules[i];
    
    String tc = r->GetTargetCommand();

    if(tc.length()) // надо отправлять команду
    {
      Command cmd;
      if(cParser->ParseCommand(tc, controllerID, cmd))
      {
         cmd.SetInternal(true); // говорим, что команда - от одного модуля к другому

        // НЕ БУДЕМ НИКУДА ПЛЕВАТЬСЯ ОТВЕТОМ ОТ МОДУЛЯ
        //cmd.SetIncomingStream(pStream);
        c->ProcessModuleCommand(cmd,false); // не проверяем адресата, т.к. он может быть удаленной коробочкой    
      } // if  

      
    } // if(tc.length())
   AddAlert(r->GetAlertRule());   
  } // for
  
}
bool AlertModule::CanWorkWithRule(RulesVector& checkedRules, AlertRule* rule, RulesVector& raisedAlerts)
{
  uint8_t cnt = rule->GetLinkedRulesCount();
  if(!cnt)
    return true; // нет связанных правил, при срабатывании которых мы должны игнорировать текущее


  // проверяем, есть ли мы уже в списке проверенных правил, если есть - считаем, что с этим правилом работать нельзя,
  // и удаляем его из списка
  size_t sz = checkedRules.size();
  for(size_t i=0;i<sz;i++)
  {
      if(checkedRules[i] == rule) // нашли кольцевую рекурсию
      {
        return false;
      }
  } // for

  for(uint8_t i=0;i<cnt;i++)
  {
    // проходимся по всем именам связанных с нашим правил, и разрешаем конфликты по цепочке
    String linkedRuleName = rule->GetLinkedRuleName(i);
    AlertRule* linkedRule = GetLinkedRule(linkedRuleName,raisedAlerts);
        
      if(linkedRule) // нашли сработавшее правило, от которого мы зависим
      {
        if(!linkedRule->GetLinkedRulesCount()) // сразу нашли сработавшее связанное правило без зависимостей
          return false; // с текущим правилом работать нельзя!
          
          checkedRules.push_back(rule); // помещаем текущий проверяемый узел в стек, чтобы не было кольцевой рекурсии

          // К СОЖАЛЕНИЮ, РЕШАЕМ РЕКУРСИВНО :(
          if(CanWorkWithRule(checkedRules,linkedRule,raisedAlerts)) // с правилом, на которое мы завязаны, работать можно, поэтому игнорируем текущее
            return false;
            
          checkedRules.pop(); // извлекаем последний элемент
 
      } // if linkedRule
    
    
  } // for
return true;
  
}
AlertRule* AlertModule::GetLinkedRule(const String& linkedRuleName,RulesVector& raisedAlerts)
{
  size_t sz = raisedAlerts.size();
  for(size_t i=0;i<sz;i++)
  {
      AlertRule* r = raisedAlerts[i];
      if(r->GetName() == linkedRuleName)
        return r;
  }
  return NULL;
}
void AlertModule::SolveConflicts(RulesVector& raisedAlerts,RulesVector& workRules)
{
  // разрешаем конфликты
  RulesVector checkedRules; // список уже проверенных правил, для разрешения кольцевой рекурсии
  
  size_t sz = raisedAlerts.size();
    for(size_t i=0;i<sz;i++)
    {
        AlertRule* rule = raisedAlerts[i];
        if(CanWorkWithRule(checkedRules, rule, raisedAlerts)) // разрешили все зависимости
            workRules.push_back(rule);
          
    } // for
}
String& AlertModule::GetAlert (uint8_t idx)
{
  if(idx >= MAX_STORED_ALERTS) 
    idx = MAX_STORED_ALERTS-1;
    
  return strAlerts[idx];
}

void AlertModule::AddAlert(const String& strAlert)
{
  
  if(curAlertIdx >= MAX_STORED_ALERTS) // заворачиваем в начало
    curAlertIdx = 0;
    
    strAlerts[curAlertIdx] = strAlert;
    curAlertIdx++;

    cntAlerts++;
    if(cntAlerts > MAX_STORED_ALERTS)
      cntAlerts = MAX_STORED_ALERTS;
}

bool  AlertModule::ExecCommand(const Command& command)
{
  ModuleController* c = GetController();
  String answer = UNKNOWN_COMMAND;
  bool answerStatus = false; 
  
  if(command.GetType() == ctSET) 
  {
    answer = NOT_SUPPORTED;
    String t = command.GetRawArguments();
    t.toUpperCase();
    if(t == GetID()) // нет аргументов
    {
      answerStatus = false;
      answer = PARAMS_MISSED;
    }
    else
    {
        uint8_t cnt = command.GetArgsCount();
        if(cnt > 0)
        {
          t = command.GetArg(0);
          t.toUpperCase();
          if(t == ADD_RULE)
          {

            t =  command.GetArg(2); // имя модуля
            AbstractModule* m = c->GetModuleByID(t);
            if(m && m != this && AddRule(m,command))
            {
              answerStatus = true;
              answer = REG_SUCC;
            }
          } // ADD_RULE
          else 
          if(t == RULE_STATE) // установить состояние правила - включено или выключено
          {
            if(cnt < 3)
            {
              answer = PARAMS_MISSED;
            } // if
            else
            {
                 String sParam = command.GetArg(1);
                 sParam.toUpperCase();
                 
                 String state = command.GetArg(2);
                 state.toUpperCase();
                 bool bEnabled = (state == STATE_ON) || (state == STATE_ON_ALT);
                        
                if(sParam == ALL)
                 {
                   // все правила
                   for(uint8_t i=0;i<rulesCnt;i++)
                   {
                      AlertRule* rule = alertRules[i];
                      if(rule)
                         rule->SetEnabled(bEnabled);
                   } // for

                   answerStatus = true;
                   answer = RULE_STATE; answer += PARAM_DELIMITER; answer +=  sParam + PARAM_DELIMITER + state;
                 } // if all
                 else // одно правило
                 {
                     uint8_t idx = sParam.toInt();
                     if(idx < rulesCnt)
                     {
                         AlertRule* rule = alertRules[idx];
                         if(rule)
                         {
                            rule->SetEnabled(bEnabled);
    
                            answerStatus = true;
                            answer = RULE_STATE; answer += PARAM_DELIMITER; answer +=  sParam + PARAM_DELIMITER + state;
                         }
                     } // if
                 } // else
            } // else
          } // else RULE_STATE
          else
         if(t == RULE_DELETE) // удалить правило по индексу
          {
            if(cnt < 2)
            {
              answer = PARAMS_MISSED;
            } // if
            else
            {
                 String sParam = command.GetArg(1);
                 sParam.toUpperCase();
                 uint8_t idx = command.GetArg(1).toInt();

                if(sParam == ALL) // удалить все правила
                {
                  for(uint8_t i=0;i<rulesCnt;i++)
                  {
                     AlertRule* rule = alertRules[i];
                     if(rule)
                      delete rule;

                     alertRules[i] = NULL;
                  } // for

                  rulesCnt = 0;
                  nextNoRuleIdx = 0;
                  
                  answerStatus = true;
                  answer = RULE_DELETE; answer += PARAM_DELIMITER; answer +=  sParam + PARAM_DELIMITER + REG_DEL;

                }
                else // только одно правило
                {
                 if(idx < rulesCnt)
                 {
                    String state = command.GetArg(2);
                    state.toUpperCase();
                     AlertRule* rule = alertRules[idx];
                     if(rule)
                     {
                        delete rule; // удаляем правило
                        
                        for(uint8_t i=idx+1;i<rulesCnt;i++) // сдвигаем к голове
                        {
                          alertRules[i-1] = alertRules[i];
                        } // for

                        alertRules[rulesCnt-1] = NULL; // в хвосте обнуляем ячейку
                        
                        rulesCnt--;
                        nextNoRuleIdx--;
                        

                        answerStatus = true;
                        answer = RULE_DELETE; answer += PARAM_DELIMITER; answer +=  command.GetArg(1) + PARAM_DELIMITER + REG_DEL;
                        
                     } // if(rule)
                 } // if
                } // else not ALL
            } // else
          } // else RULE_DELETE
           
        } // if
    } // else
  }
  else
  if(command.GetType() == ctGET) //получить алерты
  {

    String t = command.GetRawArguments();
    t.toUpperCase();
    if(t == GetID()) // нет аргументов
    {
      answerStatus = false;
      answer = PARAMS_MISSED;
    }
    else
    if(t == CNT_COMMAND) // запросили данные о  кол-ве алертов
    {
      answerStatus = true;
      answer = CNT_COMMAND; answer += PARAM_DELIMITER; answer += String(cntAlerts);
    }
    else
    if(t == RULE_CNT) // запросили данные о количестве правил
    {
      answerStatus = true;
      answer = RULE_CNT; answer += PARAM_DELIMITER; answer += String(rulesCnt);
    }
    else
    {
      uint8_t cnt = command.GetArgsCount();
        if(cnt > 0)
        {
          t = command.GetArg(0);
          t.toUpperCase();
          
              if(t == VIEW_ALERT_COMMAND) // запросили данные об алерте
              {
                    
                    if(cnt < 2)
                    {
                        answerStatus = false;
                        answer = PARAMS_MISSED;
                    }
                    else
                    {
                        uint8_t idx = command.GetArg(1).toInt();
                          
                        answerStatus = true;
                        answer = VIEW_ALERT_COMMAND; answer += PARAM_DELIMITER; answer +=  command.GetArg(1) + PARAM_DELIMITER + GetAlert(idx);
                    }
              }
              else if(t == RULE_VIEW) // просмотр правила
              {
                    if(cnt < 2)
                    {
                        answerStatus = false;
                        answer = PARAMS_MISSED;
                    }
                    else
                    {
                        uint8_t idx = command.GetArg(1).toInt();
                        if(idx <= rulesCnt) // норм индекс
                        {
                          AlertRule* rule = alertRules[idx];
                          if(rule) // нашли правило
                          {
                            String ar = rule->GetAlertRule();
                            String tc = rule->GetTargetCommand();
                            if(tc.length())
                              ar += PARAM_DELIMITER;
                              
                            answerStatus = true;
                            answer = RULE_VIEW; answer += PARAM_DELIMITER; answer +=  command.GetArg(1) + PARAM_DELIMITER + ar + tc;
                          }
                        } // if
                    } // else
                
              }
              else if(t == RULE_STATE) // запросили состояние правила
              {
                    if(cnt < 2)
                    {
                        answerStatus = false;
                        answer = PARAMS_MISSED;
                    }
                    else
                    {
                        uint8_t idx = command.GetArg(1).toInt();
                        if(idx <= rulesCnt) // норм индекс
                        {
                          AlertRule* rule = alertRules[idx];
                          if(rule) // нашли правило
                          {
                            String ar = rule->GetEnabled() ? STATE_ON : STATE_OFF;
                            answerStatus = true;
                            answer = RULE_STATE; answer += PARAM_DELIMITER; answer +=  command.GetArg(1) + PARAM_DELIMITER + ar;
                          }
                        } // if
                    } // else
              
              }
              else
              {
                // неизвестная команда
              } // else
        } // if(cnt > 0)

    }
              
  } // if
 
 // отвечаем на команду
    SetPublishData(&command,answerStatus,answer); // готовим данные для публикации
    c->Publish(this);
    
  return answerStatus;
}

