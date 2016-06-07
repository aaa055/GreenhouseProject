#include "AlertModule.h"
#include "ModuleController.h"
#include <EEPROM.h>

AlertRule::AlertRule(AlertModule* am)
{
  parent = am;
  linkedModule = NULL;
  startTime = 0;
  workTime = 0;
  dayMask = 0xFF; // все дни недели работаем
  dataAlertLong = 0;
/*
  canWork = true;
  bEnabled = true;
  bFirstCall = true;
*/
  bitWrite(flags,RULE_ENABLED_BIT,1);  
  bitWrite(flags,RULE_CAN_WORK_BIT,1);  
  bitWrite(flags,RULE_FIRST_CALL_BIT,1);  
}
const char* AlertRule::GetName()
{
  return parent->GetParam(ruleNameIdx);
}
void AlertRule::Update(uint16_t dt
  #ifdef USE_DS3231_REALTIME_CLOCK 
     ,uint8_t currentHour // текущий час
    , uint8_t currentMinute // текущая минута
    ,uint8_t currentDOW // текущий день недели
  #endif
 )
{

  UNUSED(dt);
  
  // считаем, что мы можем работать, если попадаем в текущий день недели
  #ifdef USE_DS3231_REALTIME_CLOCK 
    bitWrite(flags,RULE_CAN_WORK_BIT,bitRead(dayMask,currentDOW-1));
  #else
    bitWrite(flags,RULE_CAN_WORK_BIT,1); // без часов реального времени считаем, что работаем всегда
  #endif  

  if(startTime == 0  && workTime == 0) // работаем всегда
  {
     return;
  }


  #ifdef USE_DS3231_REALTIME_CLOCK

  // создаём диапазон для проверки
  uint16_t startDia = startTime;
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
  bool haveOverflow = false; // флаг переноса работы на следующие сутки

  if(stopDia >= mins_in_day)
  {
    // правая граница диапазона перешагнула на следующие сутки,
    // отражаем диапазон текущего часа на следующие сутки
    // только в том случае, если текущее кол-во минут от начала суток меньше, чем время начала работы
    if(checkMinutes < startTime)
    {
      checkMinutes += mins_in_day;
      haveOverflow = true;
    }
  }

    // проверяем попадание в диапазон
    bool canWeWork = (checkMinutes >= startDia && checkMinutes <= stopDia);
    if(canWeWork)
    {
      // в диапазон попали, надо проверить попадание в дни недели.
      // считаем, что мы попали в день недели, если он выставлен
      // в флагах или у нас был перенос работы на следующие сутки.
      canWeWork = haveOverflow || bitRead(dayMask,currentDOW-1);
    }
    
    bitWrite(flags,RULE_CAN_WORK_BIT,(canWeWork ? 1 : 0));  


  #endif  
  
}
bool AlertRule::HasAlert()
{
  if(!linkedModule || !bitRead(flags,RULE_ENABLED_BIT) || !bitRead(flags,RULE_CAN_WORK_BIT))
    return false;


  switch(target)
  {
    case rtTemp: // проверяем температуру
    {
     if(!linkedModule->State.HasState(StateTemperature))  // не поддерживаем температуру
        return false;

     OneState* os = linkedModule->State.GetState(StateTemperature,sensorIdx);
       
     if(!os) // не срослось
      return false;

     if(!os->IsChanged() && !bitRead(flags,RULE_FIRST_CALL_BIT))//!bFirstCall) // ничего не изменилось
        return false;
      
       //bFirstCall = false;
       bitWrite(flags,RULE_FIRST_CALL_BIT,0);
            

       TemperaturePair tp = *os; 
       int8_t curTemp = tp.Current.Value;

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

       OneState* os = linkedModule->State.GetState(StateLuminosity,sensorIdx);
       
       if(!os) // не срослось
        return false;

     if(!os->IsChanged() && !bitRead(flags,RULE_FIRST_CALL_BIT))//!bFirstCall) // ничего не изменилось
        return false;

       //bFirstCall = false;
       bitWrite(flags,RULE_FIRST_CALL_BIT,0);

       LuminosityPair lp = *os; 
       long lum = lp.Current;

       if(lum == NO_LUMINOSITY_DATA) // нет датчика на линии
        return false;

       switch(operand)
       {
          case roLessThan: return lum < dataAlertLong;
          case roLessOrEqual: return lum <= dataAlertLong;
          case roGreaterThan: return lum > dataAlertLong;
          case roGreaterOrEqual: return lum >= dataAlertLong;
          default: return false;
       } // switch
      
    }
    break;

    case rtHumidity: // следим за влажностью
    {
     if(!linkedModule->State.HasState(StateHumidity))  // не поддерживаем влажность
        return false;

       OneState* os = linkedModule->State.GetState(StateHumidity,sensorIdx);
       if(!os) // не срослось
        return false;

     if(!os->IsChanged() && !bitRead(flags,RULE_FIRST_CALL_BIT))//!bFirstCall) // ничего не изменилось
        return false;

       //bFirstCall = false;
       bitWrite(flags,RULE_FIRST_CALL_BIT,0);

       HumidityPair hp = *os;
       int8_t curHumidity = hp.Current.Value;

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

   case rtSoilMoisture: // следим за влажностью почвы
    {
     if(!linkedModule->State.HasState(StateSoilMoisture))  // не поддерживаем влажность
        return false;

       OneState* os = linkedModule->State.GetState(StateSoilMoisture,sensorIdx);
       if(!os) // не срослось
        return false;

     if(!os->IsChanged() && !bitRead(flags,RULE_FIRST_CALL_BIT))//!bFirstCall) // ничего не изменилось
        return false;

       //bFirstCall = false;
       bitWrite(flags,RULE_FIRST_CALL_BIT,0);

       HumidityPair hp = *os;
       int8_t curHumidity = hp.Current.Value;

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

    case rtPinState: // следим за статусом пина
    {
       pinMode(sensorIdx,INPUT);
       int pinState = digitalRead(sensorIdx); // читаем из пина его значение
       // dataAlert у нас может принимать одно значение: 1, поскольку мы сравниваем
       // это значение с HIGH и LOW на пине. Поэтому не имеют смысла операнды > и <=,
       // вместо них мы принудительно используем операнды >= и <.
     
       switch(operand)
       {
          case roLessThan: return pinState < dataAlert; 
          case roLessOrEqual: return pinState < dataAlert;
          case roGreaterThan: return pinState >= dataAlert;
          case roGreaterOrEqual: return pinState >= dataAlert;
          default: return false;
         
       } // switch
    }
    break;

    case rtUnknown:
     // нет того, за чем следим, считаем, что мы сработали по времени
     return true;
  } // switch

  return false;
}
String AlertRule::GetAlertRule() // конструируем правило, когда запрашивают его просмотр
{
    String result; 
    result = GetName();
    result += PARAM_DELIMITER;
    result += (linkedModule ? linkedModule->GetID() : F("") ) + PARAM_DELIMITER;
      
    switch(target)
    {
      case rtTemp:
      result += PROP_TEMP;
      break;
      
      case rtLuminosity:
      result += PROP_LIGHT;
      break;

      case rtHumidity:
      result += PROP_HUMIDITY;
      break;

      case rtPinState:
      result += PROP_PIN;
      break;

      case rtSoilMoisture:
      result += PROP_SOIL;
      break;

      case rtUnknown:
      result += PROP_NONE; // нет у нас установки, за чем следить
      break;
      
    }
    result += String(PARAM_DELIMITER) + String(sensorIdx) + PARAM_DELIMITER;

    switch(operand)
    {
      case roLessThan:
        result += LESS_THAN;
      break;
      case roGreaterThan:
        result += GREATER_THAN;
      break;
      case roLessOrEqual:
        result += LESS_OR_EQUAL_THAN;
      break;
      case roGreaterOrEqual:
        result += GREATER_OR_EQUAL_THAN;
      break;
    }
    
    result += PARAM_DELIMITER;

    switch(dataSource)
    {
      case tsOpenTemperature:
        result += T_OPEN_MACRO;
      break;
      case tsCloseTemperature:
        result += T_CLOSE_MACRO;
      break;
      case tsPassed:
        result += dataAlertLong;
      break;
    }
    result += PARAM_DELIMITER;
    
  result += String((uint16_t)startTime) + PARAM_DELIMITER;
  result += String((uint16_t)workTime) + PARAM_DELIMITER;
  result += String((uint8_t)dayMask) + PARAM_DELIMITER;

  size_t sz = linkedRulesIndices.size();
    
  if(!sz)
    result += F("_");
  else
  {
    for(size_t i=0;i<sz;i++)
    {
      if(i > 0)
        result += F(",");
        
      result += parent->GetParam(linkedRulesIndices[i]);
      
    } // for
  } // else
  
  return result;
}
uint8_t AlertRule::Save(uint16_t writeAddr) // сохраняем себя в EEPROM, возвращаем кол-во записанных байт
{
  uint16_t curWriteAddr = writeAddr;
  uint8_t written = 0;

  EEPROM.write(curWriteAddr++,target); written++;// записали, за чем следим
  EEPROM.write(curWriteAddr++,dataAlert); written++;// записали установку, за которой следим
  EEPROM.write(curWriteAddr++,sensorIdx); written++;// записали индекс датчика, за которым следим
  EEPROM.write(curWriteAddr++,operand); written++;// записали оператор сравнения
  EEPROM.write(curWriteAddr++,/*bEnabled*/ bitRead(flags,RULE_ENABLED_BIT)); written++;// записали флаг - активно правило или нет?
  EEPROM.write(curWriteAddr++,dataSource); written++;// записали источник, с которого надо брать установку
  EEPROM.write(curWriteAddr++,dayMask); written++;// записали маску дней недели

  // записали, сколько времени работать
  const byte* readAddr = (const byte*) &workTime;
  EEPROM.write(curWriteAddr++,*readAddr++); written++;
  EEPROM.write(curWriteAddr++,*readAddr++); written++;
  EEPROM.write(curWriteAddr++,*readAddr++); written++;
  EEPROM.write(curWriteAddr++,*readAddr++); written++;

  // записали имя нашего правила
  const char* nameptr = GetName();
  uint8_t namelen = strlen(nameptr);
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
   size_t sz = linkedRulesIndices.size();
   EEPROM.write(curWriteAddr++,sz); written++;


   // записываем имена связанных правил, одно за другим
   for(size_t i=0;i<sz;i++)
   {
      // записываем длину имени
      const char* lrnm = parent->GetParam(linkedRulesIndices[i]);

      namelen = strlen(lrnm);
      nameptr = lrnm;
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

  // записываем время начала работы
  readAddr = (const byte*) &startTime;
  EEPROM.write(curWriteAddr++,*readAddr++); written++;
  EEPROM.write(curWriteAddr++,*readAddr++); written++;
  
  // записали всё, оставим заглушку в несколько байт, вдруг что ещё будет в правиле?
  
  return (written + 4); // оставляем 4 байта на будущее
}
uint8_t AlertRule::Load(uint16_t readAddr, ModuleController* controller)
{
  uint8_t readed = 0;
  uint16_t curReadAddr = readAddr;

  linkedRulesIndices.Clear();


  target = (RuleTarget) EEPROM.read(curReadAddr++); readed++;// прочитали, за чем следим
  dataAlert = EEPROM.read(curReadAddr++); readed++;// прочитали установку, за которой следим
  sensorIdx = EEPROM.read(curReadAddr++); readed++;// прочитали индекс датчика, за которым следим
  operand = (RuleOperand) EEPROM.read(curReadAddr++); readed++;// прочитали оператор сравнения
  bool bEnabled = EEPROM.read(curReadAddr++); readed++;// прочитали флаг - активно правило или нет?
  bitWrite(flags,RULE_ENABLED_BIT, (bEnabled ? 1 : 0));
  dataSource = (RuleDataSource) EEPROM.read(curReadAddr++); readed++;// прочитали источник, с которого надо брать установку
  dayMask = EEPROM.read(curReadAddr++); readed++;// прочитали маску дней недели


  // прочитали, сколько времени работать
   byte* writeAddr = (byte*) &workTime;
  *writeAddr++ = EEPROM.read(curReadAddr++); readed++;
  *writeAddr++ = EEPROM.read(curReadAddr++); readed++;
  *writeAddr++ = EEPROM.read(curReadAddr++); readed++;
  *writeAddr++ = EEPROM.read(curReadAddr++); readed++;

  // прочитали имя нашего правила
  uint8_t namelen = EEPROM.read(curReadAddr++); readed++;// прочитали длину имени нашего правила
  //ruleName = F("");
  char* ruleName = new char[namelen+1];
  char* wrPtr = ruleName;
  
  for(uint8_t i=0;i<namelen;i++)
  {
    *wrPtr++ = (char) EEPROM.read(curReadAddr++); readed++; // читаем имя посимвольно
  }
  *wrPtr = '\0';
  bool added;
  ruleNameIdx = parent->AddParam(ruleName,added);
  if(!added) // такое правило уже было у родителя
    delete[] ruleName;
  
 
  // читаем имя связанного модуля, за показаниями которого мы следим
  String strReaded;
  namelen =  EEPROM.read(curReadAddr++); readed++;// читаем длину имени связанного модуля
  
  for(uint8_t i=0;i<namelen;i++)
  {
    strReaded += (char) EEPROM.read(curReadAddr++); readed++; // читаем имя посимвольно
  }
 
  // ищем связанный модуль
  linkedModule = controller->GetModuleByID(strReaded);

 
  // читаем длину команды, которую надо отправить другому модулю при срабатывании правила
  targetCommand = F("");
  namelen = EEPROM.read(curReadAddr++); readed++;
 
  // читаем саму команду
  for(uint8_t i=0;i<namelen;i++)
  {
    targetCommand += (char) EEPROM.read(curReadAddr++); readed++; // читаем посимвольно
  }


  // читаем кол-во связанных правил
   uint8_t lrCnt = EEPROM.read(curReadAddr++); readed++;


   // читаем имена связанных правил, одно за другим
   for(uint8_t i=0;i<lrCnt;i++)
   {
      // читаем длину имени
      namelen = EEPROM.read(curReadAddr++); readed++;

     char* nm = new char[namelen+1];
     char* nmPtr = nm;
        

      // читаем имя посимвольно
      for(uint8_t j=0;j<namelen;j++)
      {
       *nmPtr++ = (char) EEPROM.read(curReadAddr++); readed++; // читаем посимвольно
      }
      *nmPtr = '\0';

      bool added;
      linkedRulesIndices.push_back(parent->AddParam(nm,added)); // привязываем имя связанного правила к его индексу
      
      if(!added) // такое имя уже существовало, освобождаем память
        delete [] nm;
     
   } // for
   
   writeAddr = (byte*) &dataAlertLong;
  *writeAddr++ = EEPROM.read(curReadAddr++); readed++;
  *writeAddr++ = EEPROM.read(curReadAddr++); readed++;
  *writeAddr++ = EEPROM.read(curReadAddr++); readed++;
  *writeAddr++ = EEPROM.read(curReadAddr++); readed++;

  writeAddr = (byte*) &dataAlertLong;
  if(*writeAddr == 0xFF) // расширенной настройки не сохранено для правила
    dataAlertLong = dataAlert; // применяем короткую настройку

  // читаем время начала работы
   writeAddr = (byte*) &startTime;
  *writeAddr++ = EEPROM.read(curReadAddr++); readed++;
  *writeAddr++ = EEPROM.read(curReadAddr++); readed++;

  if(startTime == 0xFFFF) // ничего не было сохранено
    startTime = 0; // сбрасываем на 0 часов
  
  return (readed+4); // оставляем в хвосте 4 свободных байта на будущее
}
const char* AlertRule::GetLinkedRuleName(uint8_t idx)
{

 if(idx < linkedRulesIndices.size())
  return parent->GetParam(linkedRulesIndices[idx]);

 return NULL;
  
}
size_t AlertRule::GetLinkedRulesCount()
{
  return linkedRulesIndices.size();
}
bool AlertRule::Construct(AbstractModule* lm, const Command& command)
{
  // конструируем команду
  linkedModule = lm;

  // чистим имена связанных правил, об удалении памяти имён заботится родитель
  linkedRulesIndices.Clear();

  uint8_t argsCnt = command.GetArgsCount();
  if(argsCnt < 11) // мало аргументов
    return false;

  uint8_t curArgIdx = 1;
  
  // ищем имя правила
  String curArg = command.GetArg(curArgIdx++);
  // сохраняем имя у родителя
  char* rName = new char[curArg.length()+1];
  strcpy(rName,curArg.c_str());
  bool added;
  ruleNameIdx = parent->AddParam(rName,added);
  if(!added) // имя уже было у родителя
    delete[] rName;
 
  // записываем имя связанного модуля
  curArgIdx++; // пропускаем имя связанного модуля, нам его уже дали в параметрах функции
  
  curArg = command.GetArg(curArgIdx++);

  target = rtUnknown; // да ни за чем не следим
  
  if(curArg == PROP_TEMP) // следим за температурой
    target = rtTemp;
  else
  if(curArg == PROP_LIGHT) // следим за освещенностью
    target = rtLuminosity;
  else
  if(curArg == PROP_HUMIDITY) // следим за влажностью
    target = rtHumidity;
  else
  if(curArg == PROP_PIN)
    target = rtPinState; // следим за состоянием пина
  else
  if(curArg == PROP_SOIL)
    target = rtSoilMoisture; // следим за влажностью почвы

  sensorIdx = (uint8_t) atoi(command.GetArg(curArgIdx++));
  curArg = command.GetArg(curArgIdx++);

  
  if(curArg == GREATER_THAN)
    operand = roGreaterThan;
  else if(curArg == LESS_THAN)
    operand = roLessThan;
  else if(curArg == LESS_OR_EQUAL_THAN)
    operand = roLessOrEqual;
  else if(curArg == GREATER_OR_EQUAL_THAN)
    operand = roGreaterOrEqual;


  curArg = command.GetArg(curArgIdx++);

  // выясняем, за какой температурой следим
  if(curArg == T_OPEN_MACRO)
    dataSource = tsOpenTemperature;
  else if(curArg == T_CLOSE_MACRO)
    dataSource = tsCloseTemperature;
  else
    dataSource = tsPassed;
    
  
  dataAlertLong = curArg.toInt();
  dataAlert = dataAlertLong;
  
  // дошли до температуры, после неё - настройки срабатывания

  // следом идёт час начала работы
  startTime = (uint16_t) atoi(command.GetArg(curArgIdx++));
 
  // дальше идёт продолжительность работы
  workTime = (unsigned long) atol(command.GetArg(curArgIdx++));

  // дальше идёт маска дней недели
  dayMask = (uint8_t) atoi(command.GetArg(curArgIdx++));
  
  // далее идут правила, при срабатывании которых данное правило работать не будет
  curArg = command.GetArg(curArgIdx++);

  // парсим имена связанных правил
  if(curArg != F("_")) // есть связанные правила
  {
        int curNameIdx = 0;
 
        while(curNameIdx != -1)
        {
          curNameIdx = curArg.indexOf(F(",")); // парсим по запятой
          if(curNameIdx == -1)
          {
           if(curArg.length() > 0)
           {
              char* lrnm = new char[curArg.length()+1];
              strcpy(lrnm,curArg.c_str());
              bool added;
              linkedRulesIndices.push_back(parent->AddParam(lrnm,added));
              if(!added) // имя правила уже существовало
                delete[] lrnm;
           }
              
            break;
          } // if
          String param = curArg.substring(0,curNameIdx);
          curArg = curArg.substring(curNameIdx+1,curArg.length());
          if(param.length() > 0)
          {
              char* lrnm = new char[param.length()+1];
              strcpy(lrnm,param.c_str());
              bool added;
              linkedRulesIndices.push_back(parent->AddParam(lrnm,added));
              if(!added) // имя правила уже существовало
                delete[] lrnm;
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
    AlertRule* r = new AlertRule(this);
    alertRules[i] = r;
    readAddr += r->Load(readAddr,mainController); // просим правило прочитать своё внутреннее состояние
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
char* AlertModule::GetParam(size_t idx)
{
  if(idx < paramsArray.size())
    return paramsArray[idx];

  return NULL;
}
bool AlertModule::AddRule(AbstractModule* m, const Command& c)
{

// сперва ищем правило с таким же именем, как и переданное
 String rName = c.GetArg(1);
 for(uint8_t i= 0;i<rulesCnt;i++)
 {
  AlertRule* r = alertRules[i];
  if(r && !strcmp(r->GetName(),rName.c_str()))
  {
     // нашли такое правило, просто модифицируем его
     return r->Construct(m,c);
  }
 } // for
  
  if(rulesCnt >=  MAX_ALERT_RULES)
    return false;

   AlertRule* ar = new AlertRule(this);
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

  cParser = mainController->GetCommandParser();
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
    
  if(lastUpdateCall < ALERT_UPDATE_INTERVAL) // обновляем согласно настроенному интервалу
    return;
     
#ifdef USE_DS3231_REALTIME_CLOCK
  DS3231Clock rtc = mainController->GetClock();
  DS3231Time tm = rtc.getTime();
#endif


  RulesVector raisedAlerts;
  
  for(uint8_t i=0;i<rulesCnt;i++)
  {
    AlertRule* r = alertRules[i];
    if(!r)
      break;

      // сначала обновляем состояние правила
      r->Update(lastUpdateCall
#ifdef USE_DS3231_REALTIME_CLOCK
,tm.hour, tm.minute, tm.dayOfWeek
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
      if(cParser->ParseCommand(tc, cmd))
      {
         cmd.SetInternal(true); // говорим, что команда - от одного модуля к другому

        // НЕ БУДЕМ НИКУДА ПЛЕВАТЬСЯ ОТВЕТОМ ОТ МОДУЛЯ
        //cmd.SetIncomingStream(pStream);
        mainController->ProcessModuleCommand(cmd,NULL);

        // дёргаем функцию обновления других вещей - типа, кооперативная работа
        yield();
      } // if  

      
    } // if(tc.length())
   #if MAX_STORED_ALERTS > 0 
   AddAlert(r->GetAlertRule());
   #endif   
  } // for

  lastUpdateCall = lastUpdateCall - ALERT_UPDATE_INTERVAL;
  
}
bool AlertModule::CanWorkWithRule(RulesVector& checkedRules, AlertRule* rule, RulesVector& raisedAlerts)
{

  yield(); // дёргаем многозадачность за хвост
  
  size_t cnt = rule->GetLinkedRulesCount();
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
    const char* linkedRuleName = rule->GetLinkedRuleName(i);
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
size_t AlertModule::AddParam(char* nm, bool& added)
{
  added = false;
  size_t sz = paramsArray.size();
  for(size_t i=0;i<sz;i++)
  {
    if(!strcmp(nm,paramsArray[i]))
      return i;
  } // for

  added = true;
  paramsArray.push_back(nm);
  return (paramsArray.size()-1);
}
AlertRule* AlertModule::GetLinkedRule(const char* linkedRuleName,RulesVector& raisedAlerts)
{
  size_t sz = raisedAlerts.size();
  for(size_t i=0;i<sz;i++)
  {
      AlertRule* r = raisedAlerts[i];
      if(!strcmp(r->GetName(),linkedRuleName))
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

bool  AlertModule::ExecCommand(const Command& command, bool wantAnswer)
{
  if(wantAnswer) 
    PublishSingleton = UNKNOWN_COMMAND;

  size_t argsCount = command.GetArgsCount();
    
  if(command.GetType() == ctSET) 
  {
    PublishSingleton = NOT_SUPPORTED;
   
    if(!argsCount) // нет аргументов
    {
      PublishSingleton = PARAMS_MISSED;
    }
    else
    {
      String t = command.GetArg(0);
      t.toUpperCase();
 
          if(t == ADD_RULE)
          {
            t =  command.GetArg(2); // имя модуля
            AbstractModule* m = mainController->GetModuleByID(t);
            if(m && m != this && AddRule(m,command))
            {
              PublishSingleton.Status = true;
              PublishSingleton = REG_SUCC;
            }
          } // ADD_RULE
          else 
          if(t == SAVE_RULES) // запросили сохранение правил
          {
            SaveRules();
            PublishSingleton.Status = true;
            PublishSingleton = SAVE_RULES;
          }
          else 
          if(t == RULE_STATE) // установить состояние правила - включено или выключено
          {
            if(argsCount < 2)
            {
              PublishSingleton = PARAMS_MISSED;
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

                   PublishSingleton.Status = true;
                   PublishSingleton = RULE_STATE; 
                   PublishSingleton << PARAM_DELIMITER <<  sParam << PARAM_DELIMITER << state;
                 } // if all
                 else // одно правило
                 {
                      // ищем правило по имени
                      String rName = command.GetArg(1);
                      for(uint8_t i=0;i<rulesCnt;i++)
                      {
                         AlertRule* rule = alertRules[i];
                         if(rule && !strcmp(rule->GetName(),rName.c_str()))
                         {
                          rule->SetEnabled(bEnabled);
                          PublishSingleton.Status = true;
                          PublishSingleton = RULE_STATE; 
                          PublishSingleton << PARAM_DELIMITER <<  sParam << PARAM_DELIMITER << state;
                          break;
                         }
                      } // for
                
                 } // else
            } // else
          } // else RULE_STATE
          else
         if(t == RULE_DELETE) // удалить правило по индексу
          {
            if(argsCount < 2)
            {
             PublishSingleton = PARAMS_MISSED;
            } // if
            else
            {
                 String sParam = command.GetArg(1);
                 //sParam.toUpperCase();
 
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
                  
                  PublishSingleton.Status = true;
                  PublishSingleton = RULE_DELETE; 
                  PublishSingleton << PARAM_DELIMITER <<  sParam << PARAM_DELIMITER << REG_DEL;

                }
                else // только одно правило, удаляем по имени правила
                {
                   uint8_t deletedIdx = 0;
                   bool bDeleted = false;
                   for(uint8_t i=0;i<rulesCnt;i++)
                   {
                      AlertRule* rule = alertRules[i];
                      if(rule && !strcmp(rule->GetName(),sParam.c_str())) // нашли правило
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
 
                    PublishSingleton.Status = true;
                    PublishSingleton = RULE_DELETE; 
                    PublishSingleton << PARAM_DELIMITER <<  sParam << PARAM_DELIMITER << REG_DEL;
                   } // if(bDeleted)
                } // else not ALL
            } // else
          } // else RULE_DELETE
           

    } // else
  }
  else
  if(command.GetType() == ctGET) //получить алерты
  {
    if(!argsCount) // нет аргументов
    {
      PublishSingleton = PARAMS_MISSED;
    }
    else
    {
        String t = command.GetArg(0);
        
        #if MAX_STORED_ALERTS > 0
        if(t == CNT_COMMAND) // запросили данные о  кол-ве алертов
        {
          PublishSingleton.Status = true;
          PublishSingleton = CNT_COMMAND; 
          PublishSingleton << PARAM_DELIMITER << cntAlerts;
        }
        else
        #endif
        if(t == RULE_CNT) // запросили данные о количестве правил
        {
          PublishSingleton.Status = true;
          PublishSingleton = RULE_CNT; 
          PublishSingleton << PARAM_DELIMITER << rulesCnt;
        }
        else
        {
              #if MAX_STORED_ALERTS > 0
              if(t == VIEW_ALERT_COMMAND) // запросили данные об алерте
              {
                    
                    if(argsCount < 2)
                    {
                        PublishSingleton.Status = false;
                        PublishSingleton = PARAMS_MISSED;
                    }
                    else
                    {
                        uint8_t idx = (uint8_t) atoi(command.GetArg(1));
                          
                        PublishSingleton.Status = true;
                        PublishSingleton = VIEW_ALERT_COMMAND; 
                        PublishSingleton << PARAM_DELIMITER << (command.GetArg(1)) << PARAM_DELIMITER << (GetAlert(idx));
                    }
              }
              else
              #endif
               
              if(t == RULE_VIEW) // просмотр правила
              {
                    if(argsCount < 2)
                    {
                        PublishSingleton = PARAMS_MISSED;
                    }
                    else
                    {
                        uint8_t idx = (uint8_t) atoi(command.GetArg(1));
                        if(idx < rulesCnt) // норм индекс
                        {
                          AlertRule* rule = alertRules[idx];
                          if(rule) // нашли правило
                          {
                            String ar = rule->GetAlertRule();
                            String tc = rule->GetTargetCommand();
                            if(tc.length())
                              ar += PARAM_DELIMITER;
                              
                            PublishSingleton.Status = true;
                            PublishSingleton = RULE_VIEW; 
                            PublishSingleton << PARAM_DELIMITER <<  (command.GetArg(1)) << PARAM_DELIMITER << ar << tc;
                          }
                        } // if
                    } // else
                
              }
              else if(t == RULE_STATE) // запросили состояние правила
              {
                    if(argsCount < 2)
                    {
                        PublishSingleton = PARAMS_MISSED;
                    }
                    else
                    {
                        uint8_t idx = (uint8_t) atoi(command.GetArg(1));
                        if(idx <= rulesCnt) // норм индекс
                        {
                          AlertRule* rule = alertRules[idx];
                          if(rule) // нашли правило
                          {
                            
                            PublishSingleton.Status = true;
                            PublishSingleton = RULE_STATE; 
                            PublishSingleton << PARAM_DELIMITER << (command.GetArg(1)) << PARAM_DELIMITER
                             << (rule->GetEnabled() ? STATE_ON : STATE_OFF);
                          }
                        } // if
                    } // else
              
              }
              else
              {
                // неизвестная команда
              } // else
  
        } 

    } // else have args
              
  } // if ctGET
 
 // отвечаем на команду
  mainController->Publish(this,command);
  return true;
}

