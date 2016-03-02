#ifndef _LOG_MODULE_H
#define _LOG_MODULE_H
#include "AbstractModule.h"
#include "Globals.h"

#ifndef USE_DS3231_REALTIME_CLOCK
#error Define the USE_DS3231_REALTIME_CLOCK in Globals.h, please, or undef the USE_LOG_MODULE.
#endif

#include "DS3231Support.h"
#include <SD.h>

class LogModule : public AbstractModule // модуль логгирования данных с датчиков
{
  private:

  unsigned long lastUpdateCall;
  DS3231Clock rtc;
  byte lastDOW;

  bool hasSD;
  File logFile; // текущий файл для логгирования
  String currentLogFileName; // текущее имя файла, с которым мы работаем сейчас 

  void CreateNewLogFile(const DS3231Time& tm);
  void GatherLogInfo(const DS3231Time& tm); 
#ifdef ADD_LOG_HEADER  
  void TryAddFileHeader();
#endif  

  String csv(const String& input);

  // HH:MM,MODULE_NAME,SENSOR_TYPE,SENSOR_IDX,SENSOR_DATA\r\n
  void WriteLogLine(const String& hhmm, const String& moduleName, const String& sensorType, const String& sensorIdx, const String& sensorData);
  
  public:
    LogModule() : AbstractModule(F("LOG")) {}

    bool ExecCommand(const Command& command);
    void Setup();
    void Update(uint16_t dt);

};


#endif
