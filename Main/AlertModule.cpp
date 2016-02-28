#include "AlertModule.h"
#include "ModuleController.h"
#include <EEPROM.h>

AlertRule::AlertRule()
{
  linkedModule = NULL;
  bEnabled = true;
  whichTime = 0;
  workTime = 0;
  linkedRulesCnt = 0;
  canWork = true;
  dataAlertLong = 0;
  
}
void AlertRule::Update(uint16_t dt
  #ifdef USE_DS3231_REALTIME_CLOCK 
     ,uint8_t currentHour // текущий час
    , uint8_t currentMinute // текущая минута
  #endif
 )
{

  UNUSED(dt);
  
  canWork = true; // считаем, что мы можем работать

  if(whichTime == 0  && workTime == 0) // работаем всегда
  {
     return;
  }


  #ifdef USE_DS3231_REALTIME_CLOCK

  // создаём диапазон для проверки
  uint16_t startDia = whichTime*60;
  uint16_t stopDia = startDia + workTime;

  // если мы находимся между этим диапазоном, то мы можем работать в это время,
  // иначе - не можем, и просто выставляем флаг работы в false.
  // надо отразить текущее время в этот диапазон. Существует одна особенность диапазона:
  // если он полностью попадает в текущие сутки, то мы просто смотрим, попадает ли
  // текущее время в этот диапазон. Иначе (например, час начала 23, продолжительность - 
  // 120 минут, т.е. работа закончится на следующие сутки, в час ночи) нам надо отразить
  // текущий час на этот диапазон, т.е. виртуально продлить сутки. Для этого к текущему 
  // времени прибавляем кол-во минут в сутках.

  const uint16_t mins_in_day = 1440; // кол-во минут в сутках
  uint16_t checkMinutes = currentHour*60 + currentMinute; // текущее время в минутах

  if(stopDia >= mins_in_day)
  {
    // правая граница диапазона перешагнула на следующие сутки,
    // отражаем диапазон текущего часа на следующие сутки
    // только в том случае, если текущий час меньше, чем час начала работы
    if(currentHour < whichTime)
      checkMinutes += mins_in_day;
  }

    // проверяем попадание в диапазон
    canWork = checkMinutes >= startDia && checkMinutes <= stopDia;
     
  #endif  
  
}
bool AlertRule::HasAlert()
{
  if(!linkedModule || !bEnabled || !canWork)
    return false;


  switch(target)
  {
    case rtTemp: // проверяем температуру
    {
     if(!linkedModule->State.HasState(StateTemperature))  // не поддерживаем температуру
        return false;

     if(!linkedModule->State.IsStateChanged(StateTemperature,sensorIdx)) // ничего не изменилось
      {
        return false;
      }
       OneState* os = linkedModule->State.GetState(StateTemperature,sensorIdx);
       if(!os)
        return false;
        
       Temperature* t = (Temperature*) os->Data;
       int8_t curTemp = t->Value;

       if(curTemp == NO_TEMPERATURE_DATA) // нет датчика на линии
        return false;

       int8_t tAlert = dataAlert; // следим за переданной температурой
       switch(dataSource)
       {
          case tsOpenTemperature: // попросили подставить температуру открытия из настроек
            tAlert = linkedModule->GetController()->GetSettings()->GetOpenTemp();
          break;

          case tsCloseTemperature: // попросили подставить температуру закрытия из настроек
            tAlert = linkedModule->GetController()->GetSettings()->GetCloseTemp();
          break;

          case tsPassed:
          break;
       }

       switch(operand)
       {
          case roLessThan: return curTemp < tAlert;
          case roLessOrEqual: return curTemp <= tAlert;
          case roGreaterThan: return curTemp > tAlert;
          case roGreaterOrEqual: return curTemp >= tAlert;
          default: return false;
       } // switch
    }  
    break; // rtTemp

    case rtLuminosity: // следим за освещенностью
    {
      if(dataAlertLong == -2) 
      {
        // специальное значение, означающее "работать без датчика освещённости"
        return true; // в этом случае считаем, что работать мы можем при любом раскладе
      } // if
      
     if(!linkedModule->State.HasState(StateLuminosity))  // не поддерживаем освещенность
        return false;

     if(!linkedModule->State.IsStateChanged(StateLuminosity,sensorIdx)) // ничего не изменилось
      {
        return false;
      }
       OneState* os = linkedModule->State.GetState(StateLuminosity,sensorIdx);
       if(!os)
        return false;
        
       long* lum = (long*) os->Data;

       if(*lum == -1) // нет датчика на линии
        return false;

       long mappedLum = dataAlertLong;//map(dataAlert,0,0xFF,0,0xFFFF);

       switch(operand)
       {
          case roLessThan: return *lum < mappedLum;
          case roLessOrEqual: return *lum <= mappedLum;
          case roGreaterThan: return *lum > mappedLum;
          case roGreaterOrEqual: return *lum >= mappedLum;
          default: return false;
       } // switch
      
    }
    break;

    case rtHumidity: // следим за влажностью
    {
     if(!linkedModule->State.HasState(StateHumidity))  // не поддерживаем влажность
        return false;

     if(!linkedModule->State.IsStateChanged(StateHumidity,sensorIdx)) // ничего не изменилось
      {
        return false;
      }
       OneState* os = linkedModule->State.GetState(StateHumidity,sensorIdx);
       if(!os)
        return false;
        
       Humidity* h = (Humidity*) os->Data;
       int8_t curHumidity = h->Value;

       if(curHumidity == NO_TEMPERATURE_DATA) // нет датчика на линии
        return false;


       switch(operand)
       {
          case roLessThan: return curHumidity < dataAlert;
          case roLessOrEqual: return curHumidity <= dataAlert;
          case roGreaterThan: return curHumidity > dataAlert;
          case roGreaterOrEqual: return curHumidity >= dataAlert;
          default: return false;
       } // switch
      
    }
    break;

    case rtUnknown:
    break;
  } // switch

  return false;
}
uint8_t AlertRule::Save(uint16_t writeAddr) // сохраняем себя в EEPROM, возвращаем кол-во записанных байт
{
  uint16_t curWriteAddr = writeAddr;
  uint8_t written = 0;

  EEPROM.write(curWriteAddr++,target); written++;// записали, за чем следим
  EEPROM.write(curWriteAddr++,dataAlert); written++;// записали установку, за которой следим
  EEPROM.write(curWriteAddr++,sensorIdx); written++;// записали индекс датчика, за которым следим
  EEPROM.write(curWriteAddr++,operand); written++;// записали оператор сравнения
  EEPROM.write(curWriteAddr++,bEnabled); written++;// записали флаг - активно правило или нет?
  EEPROM.write(curWriteAddr++,dataSource); written++;// записали источник, с которого надо брать установку
  EEPROM.write(curWriteAddr++,whichTime); written++;// записали, когда работаем

  // записали, сколько времени работать
  const byte* readAddr = (const byte*) &workTime;
  EEPROM.write(curWriteAddr++,*readAddr++); written++;
  EEPROM.write(curWriteAddr++,*readAddr++); written++;
  EEPROM.write(curWriteAddr++,*readAddr++); written++;
  EEPROM.write(curWriteAddr++,*readAddr++); written++;

  // записали имя нашего правила
  uint8_t namelen = ruleName.length();
  const char* nameptr = ruleName.c_str();
  EEPROM.write(curWriteAddr++,namelen); written++;// записали длину имени нашего правила
  for(uint8_t i=0;i<namelen;i++)
  {
    EEPROM.write(curWriteAddr++,*nameptr++); written++; // записываем имя посимвольно
  }
 
  // записали имя связанного модуля, за показаниями которого мы следим
  String lmName = linkedModule->GetID();
  namelen = lmName.length();
  nameptr = lmName.c_str();
  EEPROM.write(curWriteAddr++,namelen); written++;// записали длину имени связанного модуля
  for(uint8_t i=0;i<namelen;i++)
  {
    EEPROM.write(curWriteAddr++,*nameptr++); written++; // записываем имя посимвольно
  }

  // записали длину команды, которую надо отправить другому модулю при срабатывании правила
  namelen = targetCommand.length();
  nameptr = targetCommand.c_str();
  EEPROM.write(curWriteAddr++,namelen); written++;

  // записали саму команду
  for(uint8_t i=0;i<namelen;i++)
  {
    EEPROM.write(curWriteAddr++,*nameptr++); written++; // записываем посимвольно
  }

  // записываем кол-во связанных правил
   EEPROM.write(curWriteAddr++,linkedRulesCnt); written++;

   // записываем имена связанных правил, одно за другим
   for(uint8_t i=0;i<linkedRulesCnt;i++)
   {
      // записываем длину имени
      namelen = linkedRuleNames[i].length();
      nameptr = linkedRuleNames[i].c_str();
      EEPROM.write(curWriteAddr++,namelen); written++;

      // записываем имя посимвольно
      for(uint8_t j=0;j<namelen;j++)
      {
        EEPROM.write(curWriteAddr++,*nameptr++); written++; // записываем посимвольно
      }
      
     
   } // for
 
  readAddr = (const byte*) &dataAlertLong;
  EEPROM.write(curWriteAddr++,*readAddr++); written++;
  EEPROM.write(curWriteAddr++,*readAddr++); written++;
  EEPROM.write(curWriteAddr++,*readAddr++); written++;
  EEPROM.write(curWriteAddr++,*readAddr++); written++;

  // записали всё, оставим заглушку в несколько байт, вдруг что ещё будет в правиле?
  
  return (written + 6); // оставляем 6 байт на будущее
}
uint8_t AlertRule::Load(uint16_t readAddr, ModuleController* controller)
{
  uint8_t readed = 0;
  uint16_t curReadAddr = readAddr;


  target = (RuleTarget) EEPROM.read(curReadAddr++); readed++;// прочитали, за чем следим
  dataAlert = EEPROM.read(curReadAddr++); readed++;// прочитали установку, за которой следим
  sensorIdx = EEPROM.read(curReadAddr++); readed++;// прочитали индекс датчика, за которым следим
  operand = (RuleOperand) EEPROM.read(curReadAddr++); readed++;// прочитали оператор сравнения
  bEnabled = EEPROM.read(curReadAddr++); readed++;// прочитали флаг - активно правило или нет?
  dataSource = (RuleDataSource) EEPROM.read(curReadAddr++); readed++;// прочитали источник, с которого надо брать установку
  whichTime = EEPROM.read(curReadAddr++); readed++;// прочитали, когда работаем

  // прочитали, сколько времени работать
   byte* writeAddr = (byte*) &workTime;
  *writeAddr++ = EEPROM.read(curReadAddr++); readed++;
  *writeAddr++ = EEPROM.read(curReadAddr++); readed++;
  *writeAddr++ = EEPROM.read(curReadAddr++); readed++;
  *writeAddr++ = EEPROM.read(curReadAddr++); readed++;

  // прочитали имя нашего правила
  uint8_t namelen = EEPROM.read(curReadAddr++); readed++;// прочитали длину имени нашего правила
  ruleName = F("");
  
  for(uint8_t i=0;i<namelen;i++)
  {
    ruleName += (char) EEPROM.read(curReadAddr++); readed++; // читаем имя посимвольно
  }
  
 
  // читаем имя связанного модуля, за показаниями которого мы следим
  String lmName;
  namelen =  EEPROM.read(curReadAddr++); readed++;// читаем длину имени связанного модуля
  
  for(uint8_t i=0;i<namelen;i++)
  {
    lmName += (char) EEPROM.read(curReadAddr++); readed++; // читаем имя посимвольно
  }
 
  // ищем связанный модуль
  linkedModule = controller->GetModuleByID(lmName);

 
  // читаем длину команды, которую надо отправить другому модулю при срабатывании правила
  targetCommand = "";
  namelen = EEPROM.read(curReadAddr++); readed++;//targetCommand.length();
 
  // читаем саму команду
  for(uint8_t i=0;i<namelen;i++)
  {
    targetCommand += (char) EEPROM.read(curReadAddr++); readed++; // читаем посимвольно
  }


  // читаем кол-во связанных правил
   linkedRulesCnt = EEPROM.read(curReadAddr++); readed++;

   // читаем имена связанных правил, одно за другим
   for(uint8_t i=0;i<linkedRulesCnt;i++)
   {
      // читаем длину имени
      namelen = EEPROM.read(curReadAddr++); readed++;
      String curName;
        

      // читаем имя посимвольно
      for(uint8_t j=0;j<namelen;j++)
      {
       curName += (char) EEPROM.read(curReadAddr++); readed++; // читаем посимвольно
      }

      linkedRuleNames[i] = curName;
      
     
   } // for
   
   writeAddr = (byte*) &dataAlertLong;
  *writeAddr++ = EEPROM.read(curReadAddr++); readed++;
  *writeAddr++ = EEPROM.read(curReadAddr++); readed++;
  *writeAddr++ = EEPROM.read(curReadAddr++); readed++;
  *writeAddr++ = EEPROM.read(curReadAddr++); readed++;

  writeAddr = (byte*) &dataAlertLong;
  if(*writeAddr == 0xFF) // расширенной настройки не сохранено для правила
    dataAlertLong = dataAlert; // применяем короткую настройку



   // теперь конструируем правило, это нужно для запроса просмотра правила
    alertRule = ruleName + PARAM_DELIMITER;
    alertRule += lmName + PARAM_DELIMITER;
    switch(target)
    {
      case rtTemp:
      alertRule += PROP_TEMP;
      break;
      
      case rtLuminosity:
      alertRule += PROP_LIGHT;
      break;

      case rtHumidity:
      alertRule += PROP_HUMIDITY;
      break;

      case rtUnknown:
      break;
      
    }
    alertRule += String(PARAM_DELIMITER) + String(sensorIdx) + PARAM_DELIMITER;

    switch(operand)
    {
      case roLessThan:
        alertRule += LESS_THAN;
      break;
      case roGreaterThan:
        alertRule += GREATER_THAN;
      break;
      case roLessOrEqual:
        alertRule += LESS_OR_EQUAL_THAN;
      break;
      case roGreaterOrEqual:
        alertRule += GREATER_OR_EQUAL_THAN;
      break;
    }
    
    alertRule += PARAM_DELIMITER;

    switch(dataSource)
    {
      case tsOpenTemperature:
        alertRule += T_OPEN_MACRO;
      break;
      case tsCloseTemperature:
        alertRule += T_CLOSE_MACRO;
      break;
      case tsPassed:
        alertRule += dataAlertLong;
      break;
    }
    alertRule += PARAM_DELIMITER;
    
  alertRule += String(whichTime) + PARAM_DELIMITER;
  alertRule += String((uint16_t)workTime) + PARAM_DELIMITER;

  String lRulesNames;
  for(uint8_t i=0;i<linkedRulesCnt;i++)
  {
    if(lRulesNames.length())
      lRulesNames += F(",");
      
    lRulesNames += linkedRuleNames[i];
    
  } // for

  if(!lRulesNames.length())
    lRulesNames = F("_");
 
  alertRule += lRulesNames;


  
  return (readed+6); // оставляем в хвосте 6 свободных байт на будущее
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
  else
  if(ruleTargetStr == PROP_LIGHT) // следим за освещенностью
    target = rtLuminosity;
  else
  if(ruleTargetStr == PROP_HUMIDITY) // следим за влажностью
    target = rtHumidity;

  alertRule += ruleTargetStr + PARAM_DELIMITER;

  alertRule += command.GetArg(curArgIdx) + PARAM_DELIMITER;
  sensorIdx = command.GetArg(curArgIdx++).toInt();
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

  String strTempAlert = command.GetArg(curArgIdx++);

  // выясняем, за какой температурой следим
  if(strTempAlert == T_OPEN_MACRO)
    dataSource = tsOpenTemperature;
  else if(strTempAlert == T_CLOSE_MACRO)
    dataSource = tsCloseTemperature;
  else
    dataSource = tsPassed;
    
  
  dataAlert = strTempAlert.toInt();
  dataAlertLong = strTempAlert.toInt();
  
  // дошли до температуры, после неё - настройки срабатывания

  // следом идёт час начала работы
  alertRule += command.GetArg(curArgIdx) + PARAM_DELIMITER;
  whichTime = command.GetArg(curArgIdx++).toInt();

  
  // дальше идёт продолжительность работы, в минутах
  alertRule += command.GetArg(curArgIdx) + PARAM_DELIMITER;
  workTime = command.GetArg(curArgIdx++).toInt(); // переводим в миллисекунды

  
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
 
  return true;
}
void AlertModule::LoadRules() // читаем настройки из EEPROM
{
  ModuleController* c = GetController();
  
  for(uint8_t i=0;i<rulesCnt;i++)
  {
    AlertRule* r = alertRules[i];
    delete r;
  }
  InitRules(); // инициализируем массив

 uint16_t readAddr = EEPROM_RULES_START_ADDR; // пишем с этого смещения

  // сначала читаем заголовок
  uint8_t h1, h2;
  h1 = EEPROM.read(readAddr++);
  h2 = EEPROM.read(readAddr++);

  if(!(h1 == SETT_HEADER1 && h2 == SETT_HEADER2)) // ничего не записано
    return;
  
  // потом читаем количество правил
 rulesCnt = EEPROM.read(readAddr++);
 
  // потом читаем правила
  for(uint8_t i=0;i<rulesCnt;i++)
  {
    AlertRule* r = new AlertRule;
    alertRules[i] = r;
    readAddr += r->Load(readAddr,c); // просим правило прочитать своё внутреннее состояние
  } // for
  
}
void AlertModule::SaveRules() // сохраняем настройки в EEPROM
{
  uint16_t writeAddr = EEPROM_RULES_START_ADDR; // пишем с этого смещения

  // сначала пишем заголовок
  EEPROM.write(writeAddr++,SETT_HEADER1);
  EEPROM.write(writeAddr++,SETT_HEADER2);
  
  // потом пишем количество правил
  EEPROM.write(writeAddr++,rulesCnt);

  // потом пишем правила
  for(uint8_t i=0;i<rulesCnt;i++)
  {
    AlertRule* r = alertRules[i];
    if(r)
      writeAddr += r->Save(writeAddr); // просим правило записать своё внутреннее состояние
  } // for

}
bool AlertModule::AddRule(AbstractModule* m, const Command& c)
{

// сперва ищем правило с таким же именем, как у переданное
 String rName = c.GetArg(1);
 for(uint8_t i= 0;i<rulesCnt;i++)
 {
  AlertRule* r = alertRules[i];
  if(r && r->GetName() == rName)
  {
     // нашли такое правило, просто модифицируем его
     return r->Construct(m,c);
  }
 } // for
  
  if(rulesCnt >=  MAX_ALERT_RULES)
    return false;

   AlertRule* ar = new AlertRule;
   if(!ar)
    return false;

   if(!ar->Construct(m,c))
   {
    delete ar;
    return false;
   }
   alertRules[rulesCnt] = ar;

    rulesCnt++;
    return true;
}

void AlertModule::Setup()
{
  // настройка модуля алертов тут
  #if MAX_STORED_ALERTS > 0
  curAlertIdx = 0; // нет событий пока
  cntAlerts = 0;

  
  for(uint8_t i = 0;i<MAX_STORED_ALERTS;i++)
  {
    strAlerts[i] = F(""); // резервируем события
  } // for
#endif

  // загружаем правила
  LoadRules();

  lastUpdateCall = 0;

  controller = GetController();
  cParser = controller->GetCommandParser();
  controllerID = controller->GetControllerID();  
}
void AlertModule::InitRules()
{
  rulesCnt = 0; // кол-вo правил
  for(uint8_t i=0;i<MAX_ALERT_RULES;i++)
  {
    alertRules[i] = NULL;
  } // for
  
}
void AlertModule::Update(uint16_t dt)
{ 
  // обновление модуля алертов тут

  lastUpdateCall += dt;
    
  if(lastUpdateCall < ALERT_UPD_INTERVAL) // обновляем согласно настроенному интервалу
    return;
  

 
#ifdef USE_DS3231_REALTIME_CLOCK
  DS3231Clock rtc = controller->GetClock();
  DS3231Time tm = rtc.getTime();
#endif


  Vector<AlertRule*> raisedAlerts;
  
  for(uint8_t i=0;i<rulesCnt;i++)
  {
    AlertRule* r = alertRules[i];
    if(!r)
      break;

      // сначала обновляем состояние правила
      r->Update(lastUpdateCall
        //dt
#ifdef USE_DS3231_REALTIME_CLOCK
,tm.hour, tm.minute
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
        controller->ProcessModuleCommand(cmd,false); // не проверяем адресата, т.к. он может быть удаленной коробочкой    
      } // if  

      
    } // if(tc.length())
   #if MAX_STORED_ALERTS > 0 
   AddAlert(r->GetAlertRule());
   #endif   
  } // for

  lastUpdateCall = lastUpdateCall - ALERT_UPD_INTERVAL;
  
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
#if MAX_STORED_ALERTS > 0
String& AlertModule::GetAlert(uint8_t idx)
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
#endif

bool  AlertModule::ExecCommand(const Command& command)
{
  ModuleController* c = GetController();
  String answer; answer.reserve(RESERVE_STR_LENGTH);
  answer = UNKNOWN_COMMAND;
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
          if(t == SAVE_RULES) // запросили сохранение правил
          {
            SaveRules();
            answerStatus = true;
            answer = SAVE_RULES;
          }
          else 
          if(t == RULE_STATE) // установить состояние правила - включено или выключено
          {
            if(cnt < 2)
            {
              answer = PARAMS_MISSED;
            } // if
            else
            {
                 String sParam = command.GetArg(1);                 
                 String state = command.GetArg(2);

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
                      // ищем правило по имени
                      String rName = command.GetArg(1);
                      for(uint8_t i=0;i<rulesCnt;i++)
                      {
                         AlertRule* rule = alertRules[i];
                         if(rule && rule->GetName() == rName)
                         {
                          rule->SetEnabled(bEnabled);
                          answerStatus = true;
                          answer = RULE_STATE; answer += PARAM_DELIMITER; answer +=  sParam + PARAM_DELIMITER + state;
                          break;
                         }
                      } // for
                
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
                  
                  answerStatus = true;
                  answer = RULE_DELETE; answer += PARAM_DELIMITER; answer +=  sParam + PARAM_DELIMITER + REG_DEL;

                }
                else // только одно правило, удаляем по имени правила
                {
                   uint8_t deletedIdx = 0;
                   bool bDeleted = false;
                   for(uint8_t i=0;i<rulesCnt;i++)
                   {
                      AlertRule* rule = alertRules[i];
                      if(rule && rule->GetName() == sParam) // нашли правило
                      {
                        delete rule;
                        bDeleted = true;
                        deletedIdx = i; 
                        break;
                      }
                   } // for
                   if(bDeleted)
                   {
                      for(uint8_t i=deletedIdx+1;i<rulesCnt;i++) // сдвигаем массив
                      {
                        alertRules[i-1] = alertRules[i];
                      } // for

                    rulesCnt--;
 
                    answerStatus = true;
                    answer = RULE_DELETE; answer += PARAM_DELIMITER; answer +=  sParam + PARAM_DELIMITER + REG_DEL;
                   } // if(bDeleted)
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
    #if MAX_STORED_ALERTS > 0
    else
    if(t == CNT_COMMAND) // запросили данные о  кол-ве алертов
    {
      answerStatus = true;
      answer = CNT_COMMAND; answer += PARAM_DELIMITER; answer += String(cntAlerts);
    }
    #endif
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

              #if MAX_STORED_ALERTS > 0
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
              else
              #endif
               
              if(t == RULE_VIEW) // просмотр правила
              {
                    if(cnt < 2)
                    {
                        answerStatus = false;
                        answer = PARAMS_MISSED;
                    }
                    else
                    {
                        uint8_t idx = command.GetArg(1).toInt();
                        if(idx < rulesCnt) // норм индекс
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

