#include "Globals.h"

#ifdef _DEBUG
#include "PDUClasses.h"
#endif

#include "CommandBuffer.h"
#include "CommandParser.h"
#include "ModuleController.h"

#ifdef AS_CONTROLLER
#include "AlertModule.h"
#include "ZeroStreamListener.h"
#endif

#ifdef USE_PIN_MODULE
#include "PinModule.h"
#endif

#ifdef USE_LOOP_MODULE
#include "LoopModule.h"
#endif

#ifdef USE_STAT_MODULE
#include "StatModule.h"
#endif

#ifdef USE_TEMP_SENSORS
#include "TempSensors.h"
#endif

#ifdef USE_SMS_MODULE
#include "SMSModule.h"
#endif

#ifdef USE_WATERING_MODULE
#include "WateringModule.h"
#endif

#ifdef USE_LUMINOSITY_MODULE
#include "LuminosityModule.h"
#endif

#ifdef USE_HUMIDITY_MODULE
#include "HumidityModule.h"
#endif

#ifdef USE_WIFI_MODULE
#include "WiFiModule.h"
#endif

#ifdef USE_LOG_MODULE
#include "LogModule.h"
#endif

#ifdef USE_DELTA_MODULE
#include "DeltaModule.h"
#endif

#ifdef USE_LCD_MODULE
#include "LCDModule.h"
#endif

#ifdef USE_NEXTION_MODULE
#include "NextionModule.h"
#endif

#ifdef USE_WATERFLOW_MODULE
#include "WaterflowModule.h"
#endif

#ifdef USE_COMPOSITE_COMMANDS_MODULE
#include "CompositeCommandsModule.h"
#endif

#ifdef USE_SOIL_MOISTURE_MODULE
#include "SoilMoistureModule.h"
#endif

// КОМАНДЫ ИНИЦИАЛИЗАЦИИ ПРИ СТАРТЕ
//const char init_0[] PROGMEM = "CTSET=PIN|13|0";// ВЫКЛЮЧИМ ПРИ СТАРТЕ СВЕТОДИОД
#ifdef USE_READY_DIODE
const char init_1[] PROGMEM = "CTSET=LOOP|SD|SET|80|7|PIN|6|T";// помигаем 5 раз диодом для проверки
#endif
const char init_STUB[] PROGMEM = ""; // ЗАГЛУШКА, НЕ ТРОГАТЬ!


// команды инициализации при старте контроллера
const char* const  INIT_COMMANDS[] PROGMEM  = 
{
 //  init_0,
#ifdef USE_READY_DIODE
init_1,
#endif
  init_STUB // ЗАГЛУШКА, НЕ ТРОГАТЬ!
};


// таймер
unsigned long lastMillis = 0;


// Ждем команды из сериала
CommandBuffer commandsFromSerial(&Serial);

// Парсер команд
CommandParser commandParser;

// Контроллер модулей - в указанном режиме работы - как главный контроллер или дочерний модуль
ModuleController controller(
  
  #ifdef AS_CONTROLLER
  cdCONTROLLER
  #else
  cdCHILDMODULE
  #endif
    
  ,OUR_ID);


#ifdef USE_PIN_MODULE
//  Модуль управления цифровыми пинами
PinModule pinModule;
#endif

#ifdef USE_LOOP_MODULE
// Модуль поддержки периодически повторяемых операций
LoopModule loopModule;
#endif

#ifdef USE_STAT_MODULE
// Модуль вывода статистики
StatModule statModule;
#endif

#ifdef USE_TEMP_SENSORS
// модуль опроса температурных датчиков и управления фрамугами
TempSensors tempSensors;
#endif

#ifdef USE_SMS_MODULE
// модуль управления по SMS
 SMSModule smsModule;
 String smsReceiveBuff;
 
void NEOWAY_EVENT_FUNC()
{
  char ch;
  while(NEOWAY_SERIAL.available())
  {
    ch = NEOWAY_SERIAL.read();

    if(ch == '\r')
      continue;
    
    if(ch == '\n')
    {
      smsModule.ProcessAnswerLine(smsReceiveBuff);
      smsReceiveBuff = F("");
    }
    else
    {
        
        if(smsModule.WaitForSMSWelcome && ch == '>') // ждут команду >
        {
          smsModule.WaitForSMSWelcome = false;
          smsModule.ProcessAnswerLine(F(">"));
        }
        else
          smsReceiveBuff += ch;
    }

    
  } // while

}
#endif

#ifdef USE_WATERING_MODULE
// модуль управления поливом
WateringModule wateringModule;
#endif

#ifdef USE_LUMINOSITY_MODULE
// модуль управления досветкой и получения значений освещённости
LuminosityModule luminosityModule;
#endif

#ifdef USE_HUMIDITY_MODULE
// модуль работы с датчиками влажности DHT
HumidityModule humidityModule;
#endif

#ifdef USE_LOG_MODULE
// модуль логгирования информации
LogModule logModule;
#endif

#ifdef USE_DELTA_MODULE
// модуль сбора дельт с датчиков
DeltaModule deltaModule;
#endif

#ifdef USE_LCD_MODULE
// модуль LCD
LCDModule lcdModule;
#endif

#ifdef USE_NEXTION_MODULE
// модуль Nextion
NextionModule nextionModule;
#endif

#ifdef USE_WATERFLOW_MODULE
// модуль учёта расхода воды
WaterflowModule waterflowModule;
#endif

#ifdef USE_COMPOSITE_COMMANDS_MODULE
// модуль составных команд
CompositeCommandsModule compositeCommands;
#endif

#ifdef USE_SOIL_MOISTURE_MODULE
// модуль датчиков влажности почвы
SoilMoistureModule soilMoistureModule;
#endif

#ifdef USE_WIFI_MODULE
// модуль работы по Wi-Fi
WiFiModule wifiModule;
String wiFiReceiveBuff;

void WIFI_EVENT_FUNC()
{
  char ch;
  while(WIFI_SERIAL.available())
  {
    ch = WIFI_SERIAL.read();

   
    if(ch == '\r')
      continue;
    
    if(ch == '\n')
    {    
        if(wiFiReceiveBuff.startsWith(F("+IPD")))
        {
          // Не убираем переводы строки, когда пришёл пакет с данными, поскольку \r\n может придти прямо в пакете данных.
          // Т.к. у нас \r\n служит признаком окончания команды - значит, мы должны учитывать эти символы в пакете,
          // и не можем самовоизвольно их отбрасывать.
          wiFiReceiveBuff += NEWLINE; 
        }
          
        wifiModule.ProcessAnswerLine(wiFiReceiveBuff);
        wiFiReceiveBuff = F("");
    }
    else
    {     
        if(wifiModule.WaitForDataWelcome && ch == '>') // ждут команду >
        {
          wifiModule.WaitForDataWelcome = false;
          wifiModule.ProcessAnswerLine(F(">"));
        }
        else
          wiFiReceiveBuff += ch;
    }
  
    
  } // while
   
}

#endif


#ifdef AS_CONTROLLER
// Модуль поддержки регистрации сторонних коробочек - только в режиме работы контроллера
ZeroStreamListener remoteRegistrator;
// модуль обработки правил тоже только в режиме контроллера
AlertModule alerts;
#endif
   
String ReadProgmemString(const char* c)
{
  String s;
  int len = strlen_P(c);
  
  for (int i = 0; i < len; i++)
    s += (char) pgm_read_byte_near(c + i);

  return s;
}
// ДОБАВЛЯЕМ КОМАНДЫ ИНИЦИАЛИЗАЦИИ В ОБРАБОТКУ
void ProcessInitCommands()
{
  int curIdx = 0;
  while(true)
  {
    const char* c = (const char*) pgm_read_word(&(INIT_COMMANDS[curIdx]));
    String command = ReadProgmemString(c);

    if(!command.length())
      break;

     Command cmd;
    if(commandParser.ParseCommand(command, OUR_ID, cmd))
    {
      // КОМАНДЫ ИНИЦИАЛИЗАЦИИ НЕ ДЕЛАЮТ ВЫВОД В СЕРИАЛ
      //cmd.SetIncomingStream(commandsFromSerial.GetStream());
      controller.ProcessModuleCommand(cmd);    
    } // if

    curIdx++;
  } // while
}

void setup() 
{ 
  Serial.begin(SERIAL_BAUD_RATE); // запускаем Serial на нужной скорости

  // настраиваем все железки
  controller.Setup();
   
  // устанавливаем провайдера команд для контроллера
  controller.SetCommandParser(&commandParser);


  // регистрируем модули  
  #ifdef USE_PIN_MODULE  
  controller.RegisterModule(&pinModule);
  #endif
  
  #ifdef USE_LOOP_MODULE
  controller.RegisterModule(&loopModule);
  #endif

  #ifdef USE_STAT_MODULE
  controller.RegisterModule(&statModule);
  #endif

  #ifdef USE_TEMP_SENSORS
  controller.RegisterModule(&tempSensors);
  #endif

  #ifdef USE_SMS_MODULE
  smsReceiveBuff.reserve(100);
  controller.RegisterModule(&smsModule);
  #endif

  #ifdef USE_WATERING_MODULE
  controller.RegisterModule(&wateringModule);
  #endif

  #ifdef USE_LUMINOSITY_MODULE
  controller.RegisterModule(&luminosityModule);
  #endif

  #ifdef USE_HUMIDITY_MODULE
  controller.RegisterModule(&humidityModule);
  #endif

  #ifdef USE_DELTA_MODULE
  controller.RegisterModule(&deltaModule);
  #endif
  
  #ifdef USE_LCD_MODULE
  controller.RegisterModule(&lcdModule);
  #endif

  #ifdef USE_NEXTION_MODULE
  controller.RegisterModule(&nextionModule);
  #endif

  #ifdef USE_WATERFLOW_MODULE
  controller.RegisterModule(&waterflowModule);
  #endif

  #ifdef USE_COMPOSITE_COMMANDS_MODULE
  controller.RegisterModule(&compositeCommands);
  #endif
  
  #ifdef USE_SOIL_MOISTURE_MODULE
  controller.RegisterModule(&soilMoistureModule);
  #endif

  #ifdef USE_LOG_MODULE
  controller.RegisterModule(&logModule);
  controller.SetLogWriter(&logModule); // задаём этот модуль как модуль, который записывает события в лог
  #endif

  #ifdef USE_WIFI_MODULE
  wiFiReceiveBuff.reserve(100);
  controller.RegisterModule(&wifiModule);
  #endif 

 // модуль алертов регистрируем последним, т.к. он должен вычитать зависимости с уже зарегистрированными модулями
  #ifdef AS_CONTROLLER
    controller.RegisterModule(&remoteRegistrator);
    controller.RegisterModule(&alerts);
  #endif

  controller.begin(); // начинаем работу

  ProcessInitCommands(); // запускаем на обработку команды инициализации

  // Печатаем в Serial готовность
  Serial.print(READY);

  #ifdef USE_LOG_MODULE
    controller.Log(&logModule,READY); // печатаем в файл действий строчку Ready, которая скажет нам, что мега стартовала
  #endif

  // тест часов реального времени
  #ifdef USE_DS3231_REALTIME_CLOCK
  
   DS3231Clock rtc = controller.GetClock();
   DS3231Time tm = rtc.getTime();

   Serial.print(F(", "));
   Serial.print(rtc.getDayOfWeekStr(tm));
   Serial.print(F(" "));
   Serial.print(rtc.getDateStr(tm));
   Serial.print(F(" - "));
   Serial.print(rtc.getTimeStr(tm));

   
  #endif 

    Serial.println(F(""));

#ifdef _DEBUG
  Serial.println("Command: \t " + String(sizeof(Command)));
  Serial.println("AlertRule: \t " + String(sizeof(AlertRule)));
  Serial.println("AlertModule: \t " + String(sizeof(AlertModule)));
  Serial.println("HumidityModule: \t " + String(sizeof(HumidityModule)));
  Serial.println("InteropStream: \t " + String(sizeof(InteropStream)));
  Serial.println("LogModule: \t " + String(sizeof(LogModule)));
  Serial.println("LoopModule: \t " + String(sizeof(LoopModule)));
  Serial.println("LuminosityModule: \t " + String(sizeof(LuminosityModule)));
  Serial.println("ModuleController: \t " + String(sizeof(ModuleController)));
  Serial.println("PinModule: \t " + String(sizeof(PinModule)));
  Serial.println("GlobalSettings: \t " + String(sizeof(GlobalSettings)));
  Serial.println("SMSModule: \t " + String(sizeof(SMSModule)));
  Serial.println("StatModule: \t " + String(sizeof(StatModule)));
  Serial.println("TempSensors: \t " + String(sizeof(TempSensors)));
  Serial.println("WindowState: \t " + String(sizeof(WindowState)));
  Serial.println("WateringModule: \t " + String(sizeof(WateringModule)));
  Serial.println("WiFiModule: \t " + String(sizeof(WiFiModule)));
  Serial.println("ZeroStreamListener: \t " + String(sizeof(ZeroStreamListener)));
  Serial.println("HTTPClient: \t " + String(sizeof(HTTPClient)));
  Serial.println("DeltaModule: \t " + String(sizeof(DeltaModule)));
  Serial.println("File: \t " + String(sizeof(File)));
  Serial.println("LCDModule: \t " + String(sizeof(LCDModule)));
  Serial.println(F(""));
  Serial.print(F("FREERAM HERE: "));
  Serial.println(freeRam());
#endif


}
// эта функция вызывается после обновления состояния каждого модуля.
// передаваемый параметр - указатель на обновлённый модуль.
// если модулю предстоит долгая работа - помимо этого инструмента
// модуль должен дёргать функцию yield, если чем-то долго занят!
void ModuleUpdateProcessed(AbstractModule* module)
{
  UNUSED(module);

  // используем её, чтобы проверить состояние порта UART для WI-FI-модуля - вдруг надо внеочередно обновить
    #ifdef USE_WIFI_MODULE
    // модуль Wi-Fi обновляем каждый раз после обновления очередного модуля
    WIFI_EVENT_FUNC(); // вызываем функцию проверки данных в порту
    #endif

   #ifdef USE_SMS_MODULE
   // и модуль GSM тоже тут обновим
   NEOWAY_EVENT_FUNC();
   #endif     
}
void loop() 
{
// отсюда можно добавлять любой сторонний код

// до сюда можно добавлять любой сторонний код

  
    // вычисляем время, прошедшее с момента последнего вызова
    unsigned long curMillis = millis();
    uint16_t dt = curMillis - lastMillis;
    
    lastMillis = curMillis; // сохраняем последнее значение вызова millis()
    

  // смотрим, есть ли входящие команды
   if(commandsFromSerial.HasCommand())
   {
    // есть новая команда
    Command cmd;
    if(commandParser.ParseCommand(commandsFromSerial.GetCommand(),OUR_ID, cmd))
    {
       Stream* answerStream = commandsFromSerial.GetStream();
      // разобрали, назначили поток, с которого пришла команда
        cmd.SetIncomingStream(answerStream);

      // запустили команду в обработку
       controller.ProcessModuleCommand(cmd);
 
    } // if
    else
    {
      // что-то пошло не так, игнорируем команду
    } // else
    
    commandsFromSerial.ClearCommand(); // очищаем полученную команду
   } // if
    
    // обновляем состояние всех зарегистрированных модулей
   controller.UpdateModules(dt,ModuleUpdateProcessed);


   
// отсюда можно добавлять любой сторонний код


// до сюда можно добавлять любой сторонний код

}
// обработчик простоя, используем и его. Если сторонняя библиотека устроена правильно - она будет
// вызывать yield периодически - этим грех не воспользоваться, чтобы избежать потери данных
// в портах UART. 
void yield()
{
// отсюда можно добавлять любой сторонний код, который надо вызывать, когда МК чем-то долго занят (например, чтобы успокоить watchdog)


// до сюда можно добавлять любой сторонний код


   #ifdef USE_WIFI_MODULE
    // модуль Wi-Fi обновляем каждый раз при вызове функции yield
    WIFI_EVENT_FUNC(); // вызываем функцию проверки данных в порту
    #endif

   #ifdef USE_SMS_MODULE
   // и модуль GSM тоже тут обновим
   NEOWAY_EVENT_FUNC();
   #endif 

   #ifdef USE_LCD_MODULE
    rotaryEncoder.update(); // обновляем энкодер меню
   #endif

// отсюда можно добавлять любой сторонний код, который надо вызывать, когда МК чем-то долго занят (например, чтобы успокоить watchdog)


// до сюда можно добавлять любой сторонний код

}

