#include "LogModule.h"
#include "ModuleController.h"

#ifdef LOGGING_DEBUG_MODE
  #define LOG_DEBUG_WRITE(s) Serial.println((s))
#endif 

#define WRITE_TO_FILE(f,str) f.write((const uint8_t*) str.c_str(),str.length())
#define WRITE_TO_LOG(str) WRITE_TO_FILE(logFile,str)
#define WRITE_TO_ACTION_LOG(str) WRITE_TO_FILE(actionFile,str)

void LogModule::Setup()
{
   lastUpdateCall = 0;
   rtc = mainController->GetClock();
   lastDOW = -1;
#ifdef LOG_ACTIONS_ENABLED   
   lastActionsDOW = -1;
#endif   
   hasSD = mainController->HasSDCard();

   loggingInterval = LOGGING_INTERVAL; // по умолчанию, берём из Globals.h. Позже - будет из настроек.
  // настройка модуля тут
 }
#ifdef LOG_ACTIONS_ENABLED 
void LogModule::CreateActionsFile(const DS3231Time& tm)
{  
  // формат YYYYMMDD.LOG
   String logFileName;
   logFileName += String(tm.year);

   if(tm.month < 10)
    logFileName += F("0");
   logFileName += String(tm.month);

   if(tm.dayOfMonth < 10)
    logFileName += F("0");
   logFileName += String(tm.dayOfMonth);

   logFileName += F(".log");

   String logDirectory = ACTIONS_DIRECTORY; // папка с логами действий

   if(!SD.exists(logDirectory)) // нет папки ACTIONS_DIRECTORY
   {
    #ifdef LOGGING_DEBUG_MODE
    LOG_DEBUG_WRITE(F("Creating the actions directory..."));
    #endif
      
      SD.mkdir(logDirectory); // создаём папку
   }

 if(!SD.exists(logDirectory)) // проверяем её существование, на всякий
  {
    // не удалось создать папку actions
    #ifdef LOGGING_DEBUG_MODE
    LOG_DEBUG_WRITE(F("Unable to access to actions directory!"));
    #endif

    return;
  }

  logFileName = logDirectory + String(F("/")) + logFileName; // формируем полный путь

  if(actionFile)
  {
    // уже есть открытый файл, проверяем, не пытаемся ли мы открыть файл с таким же именем
    if(logFileName.endsWith(actionFile.name())) // такой же файл
      return; 
    else
      actionFile.close(); // закрываем старый
  } // if
  
  actionFile = SD.open(logFileName,FILE_WRITE); // открываем файл
   
}
#endif
void LogModule::WriteAction(const LogAction& action)
{
#ifdef LOG_ACTIONS_ENABLED  
  EnsureActionsFileCreated(); // убеждаемся, что файл создан
  if(!actionFile)
  {
    // что-то пошло не так
    #ifdef LOGGING_DEBUG_MODE
    LOG_DEBUG_WRITE(F("NO ACTIONS FILE AVAILABLE!"));
    #endif

    return;
  }

  #ifdef LOGGING_DEBUG_MODE
  LOG_DEBUG_WRITE( String(F("Write the \"")) + action.Message + String(F("\" action...")));
  #endif

  String comma = COMMA_DELIMITER;
  String rn = NEWLINE;

  DS3231Time tm = rtc.getTime();
  
  String hhmm;
  if(tm.hour < 10)
     hhmm += F("0");
  hhmm += String(tm.hour);

  hhmm += F(":");
  
  if(tm.minute < 10)
    hhmm += F("0");
  hhmm += String(tm.minute);

  WRITE_TO_ACTION_LOG(hhmm);
  WRITE_TO_ACTION_LOG(comma);
  WRITE_TO_ACTION_LOG(action.RaisedModule->GetID());
  WRITE_TO_ACTION_LOG(comma);
  WRITE_TO_ACTION_LOG(csv(action.Message));
  WRITE_TO_ACTION_LOG(rn);

  actionFile.flush(); // сливаем данные на диск
  yield(); // даём поработать другим модулям
#else
UNUSED(action);
#endif
  
}
#ifdef LOG_ACTIONS_ENABLED
void LogModule::EnsureActionsFileCreated()
{
  DS3231Time tm = rtc.getTime();

  if(tm.dayOfWeek != lastActionsDOW)
  {
    // перешли на другой день недели, создаём новый файл
    lastActionsDOW = tm.dayOfWeek;
    
    if(actionFile)
      actionFile.close();
      
    CreateActionsFile(tm); // создаём новый файл
  }
 
}
#endif
void LogModule::CreateNewLogFile(const DS3231Time& tm)
{
    if(logFile) // есть открытый файл
      logFile.close(); // закрываем его

    currentLogFileName = F("");

   // формируем имя нашего нового лог-файла:
   // формат YYYYMMDD.LOG
   String logFileName;

   logFileName += String(tm.year);

   if(tm.month < 10)
    logFileName += F("0");
   logFileName += String(tm.month);
   
   if(tm.dayOfMonth < 10)
    logFileName += F("0");
   logFileName += String(tm.dayOfMonth);

   logFileName += F(".log");

   String logDirectory = LOGS_DIRECTORY; // папка с логами
   if(!SD.exists(logDirectory)) // нет папки LOGS_DIRECTORY
   {
    #ifdef LOGGING_DEBUG_MODE
    LOG_DEBUG_WRITE(F("Creating the logs directory..."));
    #endif
      
      SD.mkdir(logDirectory); // создаём папку
   }
   
  if(!SD.exists(logDirectory)) // проверяем её существование, на всякий
  {
    // не удалось создать папку logs
    #ifdef LOGGING_DEBUG_MODE
    LOG_DEBUG_WRITE(F("Unable to access to logs directory!"));
    #endif

    return;
  } 

   #ifdef LOGGING_DEBUG_MODE
    LOG_DEBUG_WRITE(String(F("Creating the ")) + logFileName + String(F(" log file...")));
   #endif

   // теперь можем создать файл - даже если он существует, он откроется на запись
   logFileName = logDirectory + String(F("/")) + logFileName; // формируем полный путь

   logFile = SD.open(logFileName,FILE_WRITE);

   if(logFile)
   {
   #ifdef LOGGING_DEBUG_MODE
    LOG_DEBUG_WRITE(String(F("File ")) + logFileName + String(F(" successfully created!")));
   #endif
    
   }
   else
   {
   #ifdef LOGGING_DEBUG_MODE
    LOG_DEBUG_WRITE(String(F("Unable to create the ")) + logFileName + String(F(" log file!")));
   #endif
    return;
   }

   // файл создали, можем с ним работать.
   currentLogFileName = logFileName; // сохраняем имя файла, с которым мы сейчас работаем
#ifdef ADD_LOG_HEADER
   TryAddFileHeader(); // пытаемся добавить заголовок в файл
#endif   
      
}
#ifdef ADD_LOG_HEADER
void LogModule::TryAddFileHeader()
{
  uint32_t sz = logFile.size();
  if(!sz) // файл пуст
  {
   #ifdef LOGGING_DEBUG_MODE
    LOG_DEBUG_WRITE(String(F("Adding the header to the ")) + String(logFile.name()) + String(F(" file...")));
   #endif

    // тут можем добавлять свой заголовок в файл

    // сначала опрашиваем все модули в системе, записывая их имена в файл, и попутно сохраняя те типы датчиков, которые есть у модулей
    int statesFound = 0; // какие состояния нашли

    size_t cnt = mainController->GetModulesCount();
    bool anyModuleNameWritten = false;
    String comma = COMMA_DELIMITER;
    String rn = NEWLINE;
    
    for(size_t i=0;i<cnt;i++)
    {
      AbstractModule* m = mainController->GetModule(i);
      if(m == this) // пропускаем себя
        continue;

      // смотрим, есть ли хоть одно интересующее нас состояние, попутно сохраняя найденные нами состояния в список найденных состояний
        bool anyInterestedStatesFound = false;
        if(m->State.HasState(StateTemperature))
        {
          statesFound |= StateTemperature;
          anyInterestedStatesFound = true;
        }
        if(m->State.HasState(StateLuminosity))
        {
          statesFound |= StateLuminosity;
          anyInterestedStatesFound = true;
        }
        if(m->State.HasState(StateHumidity))
        {
          statesFound |= StateHumidity;
          anyInterestedStatesFound = true;
        }
        
        if(anyInterestedStatesFound)
        {
          // есть интересующие нас состояния, можно писать в файл имя модуля и его индекс.
          if(anyModuleNameWritten) // уже было записано имя модуля, надо добавить запятую
          {
            WRITE_TO_LOG(comma);
          }
          
          String mName = m->GetID(); // получаем имя модуля
          mName += COMMAND_DELIMITER;
          mName += String(i);

          // строка вида MODULE_NAME=IDX сформирована, можно писать её в файл
          WRITE_TO_LOG(mName);
          anyModuleNameWritten = true;
        }
    } // for

    if(anyModuleNameWritten) // записали по крайней мере имя одного модуля, надо добавить перевод строки
          WRITE_TO_LOG(rn);

    // теперь можем писать в файл вторую строку, с привязками типов датчиков к индексам
    String secondLine;
 
    if(statesFound & StateTemperature) // есть температура
    {
      if(secondLine.length())
        secondLine += comma;

      secondLine += LOG_TEMP_TYPE;
      secondLine += COMMAND_DELIMITER;
      secondLine += String(StateTemperature);
    }
  
    if(statesFound & StateLuminosity) // есть освещенность
    {
      if(secondLine.length())
        secondLine += comma;
        
      secondLine += LOG_LUMINOSITY_TYPE;
      secondLine += COMMAND_DELIMITER;
      secondLine += String(StateLuminosity);
    }

    if(statesFound & StateHumidity) // есть влажность
    {
      if(secondLine.length())
        secondLine += comma;
        
      secondLine += LOG_HUMIDITY_TYPE;
      secondLine += COMMAND_DELIMITER;
      secondLine += String(StateHumidity);
    }
    
   if(secondLine.length()) // можем записывать в файл вторую строку с привязкой датчиков к типам
   {
    secondLine += rn;
    WRITE_TO_LOG(secondLine);
   }

  // записали, выдыхаем, курим, пьём пиво :)
   #ifdef LOGGING_DEBUG_MODE
    LOG_DEBUG_WRITE(F("File header written successfully!"));
   #endif

   logFile.flush(); // сливаем данные на карту
   
   yield(); // т.к. запись на SD-карту у нас может занимать какое-то время - дёргаем кооперативный режим
    
  } // if(!sz) - файл пуст
}
#endif
void LogModule::GatherLogInfo(const DS3231Time& tm)
{
  // собираем информацию в лог
  
  if(!logFile) // что-то пошло не так
  {
    #ifdef LOGGING_DEBUG_MODE
    LOG_DEBUG_WRITE(F("Current log file not open!"));
    #endif
    return;
  }
  
  logFile.flush(); // сливаем информацию на карту
  
  yield(); // т.к. запись на SD-карту у нас может занимать какое-то время - дёргаем кооперативный режим
 
    #ifdef LOGGING_DEBUG_MODE
    LOG_DEBUG_WRITE(F("Gathering sensors data..."));
    #endif

  // строка с данными у нас имеет вид:
  // HH:MM,MODULE_NAME,SENSOR_TYPE,SENSOR_IDX,SENSOR_DATA
  // и соответствует формату CSV, т.е. если в данных есть "," и другие запрещенные символы, то данные обрамляются двойными кавычками
  String hhmm;
  if(tm.hour < 10)
     hhmm += F("0");
  hhmm += String(tm.hour);

  hhmm += F(":");
  
  if(tm.minute < 10)
    hhmm += F("0");
  hhmm += String(tm.minute);

// формируем типы данных, чтобы не дёргать их каждый раз в цикле
  String temperatureType = 
  #ifdef LOG_CHANGE_TYPE_TO_IDX
  String(StateTemperature);
  #else
  LOG_TEMP_TYPE;
  #endif

  String humidityType = 
  #ifdef LOG_CHANGE_TYPE_TO_IDX
  String(StateHumidity);
  #else
  LOG_HUMIDITY_TYPE;
  #endif

  String luminosityType = 
  #ifdef LOG_CHANGE_TYPE_TO_IDX
  String(StateLuminosity);
  #else
  LOG_LUMINOSITY_TYPE;
  #endif
  
  // он сказал - поехали
  size_t cnt = mainController->GetModulesCount();
  // он махнул рукой
    for(size_t i=0;i<cnt;i++)
    {
      AbstractModule* m = mainController->GetModule(i);
      if(m == this) // пропускаем себя
        continue;

        // смотрим, чего там есть у модуля
        String moduleName = 
        #ifdef LOG_CNANGE_NAME_TO_IDX
        String(i);
        #else
        m->GetID();
        #endif

        // обходим температуру
        uint8_t stateCnt = m->State.GetStateCount(StateTemperature);
        if(stateCnt > 0)
        {
          // о, температуру нашли, да? Так и запишем.
          for(uint8_t stateIdx = 0; stateIdx < stateCnt;stateIdx++)
          {
            OneState* os = m->State.GetState(StateTemperature,stateIdx);
            String sensorIdx = String(os->Index);
            Temperature* t = (Temperature*) os->Data;
            String sensorData = *t;
            if(
              #ifdef WRITE_ABSENT_SENSORS_DATA
              true
              #else
              t->Value != NO_TEMPERATURE_DATA // только если датчик есть на линии
              #endif
              ) 
            {
                // пишем строку с данными
                WriteLogLine(hhmm,moduleName,temperatureType,sensorIdx,sensorData);
            } // if
            
            
          } // for
          
        } // if(stateCnt > 0)

         // температуру обошли, обходим влажность
        stateCnt = m->State.GetStateCount(StateHumidity);
        if(stateCnt > 0)
        {
          // нашли влажность
          for(uint8_t stateIdx = 0; stateIdx < stateCnt;stateIdx++)
          {
            OneState* os = m->State.GetState(StateHumidity,stateIdx);
            String sensorIdx = String(os->Index);
            Humidity* h = (Humidity*) os->Data;
            String sensorData = *h;
            if(
              #ifdef WRITE_ABSENT_SENSORS_DATA
              true
              #else
              h->Value != NO_TEMPERATURE_DATA // только если датчик есть на линии
              #endif
              ) 
            {
                // пишем строку с данными
                WriteLogLine(hhmm,moduleName,humidityType,sensorIdx,sensorData);
            } // if
            
            
          } // for
          
        } // if(stateCnt > 0)

        // влажность обошли, обходим освещенность
        stateCnt = m->State.GetStateCount(StateLuminosity);
        if(stateCnt > 0)
        {
          // нашли освещенность
          for(uint8_t stateIdx = 0; stateIdx < stateCnt;stateIdx++)
          {
            OneState* os = m->State.GetState(StateLuminosity,stateIdx);
            String sensorIdx = String(os->Index);
            long* l = (long*) os->Data;
            long dt = *l;
            String sensorData = String(dt);
            if(
              #ifdef WRITE_ABSENT_SENSORS_DATA
              true
              #else
              dt != NO_LUMINOSITY_DATA // только если датчик есть на линии
              #endif
              ) 
            {
                // пишем строку с данными
                WriteLogLine(hhmm,moduleName,luminosityType,sensorIdx,sensorData);
            } // if
            
            
          } // for
          
        } // if(stateCnt > 0)

        
    } // for

  
    // записали, выдохнули, расслабились.
    #ifdef LOGGING_DEBUG_MODE
    LOG_DEBUG_WRITE(F("Sensors data gathered."));
    #endif
  
}
void LogModule::WriteLogLine(const String& hhmm, const String& moduleName, const String& sensorType, const String& sensorIdx, const String& sensorData)
{
  // пишем строку с данными в лог
  String comma = COMMA_DELIMITER;
  String rn = NEWLINE;
  // HH:MM,MODULE_NAME,SENSOR_TYPE,SENSOR_IDX,SENSOR_DATA\r\n
  WRITE_TO_LOG(hhmm);             WRITE_TO_LOG(comma);
  WRITE_TO_LOG(moduleName);       WRITE_TO_LOG(comma);
  WRITE_TO_LOG(sensorType);       WRITE_TO_LOG(comma);
  WRITE_TO_LOG(sensorIdx);        WRITE_TO_LOG(comma);
  WRITE_TO_LOG(csv(sensorData));  WRITE_TO_LOG(rn);

  logFile.flush(); // сливаем данные на карту

  yield(); // т.к. запись на SD-карту у нас может занимать какое-то время - дёргаем кооперативный режим

}
String LogModule::csv(const String& src)
{
  String fnd = F("\"");
  String rpl = fnd + fnd;
  String input = src;
  input.replace(fnd,rpl); // заменяем кавычки на двойные
  
 if(input.indexOf(COMMA_DELIMITER) != -1 ||
    input.indexOf(F("\"")) != -1 ||
    input.indexOf(F(";")) != -1 ||
    input.indexOf(F(",")) != -1 || // прописываем запятую принудительно, т.к. пользователь может переопределить COMMA_DELIMITER
    input.indexOf(NEWLINE) != -1
 )
 { // нашли запрещённые символы - надо обрамить в двойные кавычки строку
  
  String s; s.reserve(input.length() + 2);
  s += fnd;
  s += input;
  s += fnd;
  
  return s;
 }

  return input;
}
void LogModule::Update(uint16_t dt)
{ 
  lastUpdateCall += dt;
  if(lastUpdateCall < loggingInterval) // не надо обновлять ничего - не пришло время
    return;
  else
    lastUpdateCall = 0;

  if(!hasSD) // нет карты или карту не удалось инициализировать
  {
    #ifdef LOGGING_DEBUG_MODE
    LOG_DEBUG_WRITE(F("NO SD CARD PRESENT!"));
    #endif
    return;
  }

  DS3231Time tm = rtc.getTime();
  if(lastDOW != tm.dayOfWeek) // наступил следующий день недели, надо создать новый лог-файл
  {
   lastDOW = tm.dayOfWeek;
   CreateNewLogFile(tm); // создаём новый файл
#ifdef LOG_ACTIONS_ENABLED
   EnsureActionsFileCreated(); // создаём новый файл действий, если он ещё не был создан
#endif
  }

  GatherLogInfo(tm); // собираем информацию в лог

    
  // обновление модуля тут

}

bool LogModule::ExecCommand(const Command& command)
{
UNUSED(command);
 

  return true;
}

