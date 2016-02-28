#include "Globals.h"

#include "CommandBuffer.h"
#include "CommandParser.h"
#include "ModuleController.h"
#include "Menu.h"

#ifdef AS_CONTROLLER
#include "AlertModule.h"
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

#include "ZeroStreamListener.h"

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

// КОМАНДЫ ИНИЦИАЛИЗАЦИИ ПРИ СТАРТЕ
const char init_0[] PROGMEM = "CTSET=PIN|13|0";// ВЫКЛЮЧИМ ПРИ СТАРТЕ СВЕТОДИОД
const char init_1[] PROGMEM = "CTSET=LOOP|SD|SET|100|11|PIN|6|T";// помигаем 5 раз диодом для проверки

const char init_STUB[] PROGMEM = ""; // ЗАГЛУШКА, НЕ ТРОГАТЬ!


// команды инициализации при старте контроллера
const char* const  INIT_COMMANDS[] PROGMEM  = 
{
   init_0
  ,init_1
  ,init_STUB // ЗАГЛУШКА, НЕ ТРОГАТЬ!
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


// паблишер для вывода ответов в сериал
//SerialPublisher serialPublisher;

// паблишер вывода на экран
//DisplayPublisher displayPublisher;

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
// модуль регистрации алертов тоже только в режиме контроллера
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

   
  // устанавливаем провайдера команд для контроллера
  controller.SetCommandParser(&commandParser);

  Serial.begin(SERIAL_BAUD_RATE); // запускаем Serial на нужной скорости

  // назначаем поток вывода по умолчанию для контроллера
  controller.SetCurrentStream(commandsFromSerial.GetStream());

  /*
   * ЛОГИКА РАБОТЫ ПУБЛИКАТОРОВ: ОНИ МОГУТ ВЫВОДИТЬ ОТВЕТ ТУДА, КУДА ХОТЯТ.
   * ПО УМОЛЧАНИЮ ОТВЕТ НА ЗАПРОС ПРИХОДИТ В ТОТ ПОТОК, С КОТОРОГО БЫЛ ПОСЛАН ЗАПРОС.
   * НАПРИМЕР, ЕСЛИ ЗАПРОС БЫЛ ИЗ SERIAL, ТО ТУДА И УЙДЁТ ОТВЕТ.
   * ЕСЛИ МЫ ПОДПИШЕМ МОДУЛЬ, ПРИПИСАВ ЕМУ ПОДПИСЧИКА НА ОТВЕТ, И ЭТОТ ПОДПИСЧИК
   * ВЫВОДИТ ДАННЫЕ В ЭТОТ ЖЕ ПОТОК - ВЫВОД ПО УМОЛЧАНИЮ ПРОИГНОРИРУЕТСЯ,
   * ВМЕСТО ЭТОГО БУДЕТ ВЫЗВАН МЕТОД ПОДПИСЧИКА.
   * 
   * ТО ЕСТЬ - ОДИН И ТОТ ЖЕ ОТВЕТ НЕ ПОПАДЁТ НЕСКОЛЬКО РАЗ В ПОТОК ДЛЯ ВЫВОДА ОТВЕТА.
   */
#ifdef USE_PIN_MODULE   
  // подписываем ответы от модуля на сериал
  //pinModule.AddPublisher(&serialPublisher);
  // для модуля управления диодом дублируем надпись на дисплей 
  //pinModule.AddPublisher(&displayPublisher);
#endif

#ifdef USE_STAT_MODULE
// подписываем ответы от модуля статистики на дисплей
  //statModule.AddPublisher(&serialPublisher);
#endif

 // подписываем ответы от регистратора сторонних модулей на сериал
 #ifdef AS_CONTROLLER
 //remoteRegistrator.AddPublisher(&serialPublisher);
 #endif
 /*
  * Как видно - строчка выше закомментирована. Это значит, что при отсутствии
  * подписчиков на вывод куда-либо информации от модуля - возвращаемая модулем
  * информация публикуется в текущий поток контроллера, т.е. в поток, от которого
  * и пришёл запрос на выполнение команды. В нашем случае - это Serial.
  */
 
  
  // регистрируем модули
  #ifdef USE_WIFI_MODULE
  wiFiReceiveBuff.reserve(100);
  controller.RegisterModule(&wifiModule);
  #endif
  
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

 // модуль алертов регистрируем последним, т.к. он должен вычитать зависимости с уже зарегистрированными модулями
  #ifdef AS_CONTROLLER
    controller.RegisterModule(&remoteRegistrator);
    controller.RegisterModule(&alerts);
  #endif

  controller.begin(); // начинаем работу

  ProcessInitCommands(); // запускаем на обработку команды инициализации

  // Печатаем в Serial готовность
  Serial.print(READY);

  // тест часов реального времени
  #ifdef USE_DS3231_REALTIME_CLOCK
  
   DS3231Clock rtc = controller.GetClock();
   DS3231Time tm = rtc.getTime();
   
   String s = F(", ");
   
   s += rtc.getDayOfWeekStr(tm);
   s += F(" ");
   s += rtc.getDateStr(tm);
   s += F(" - ");
   s += rtc.getTimeStr(tm);
   Serial.print(s);
   
  #endif 

   Serial.println(F(""));

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
        // сохранили поток и для контроллера
        controller.SetCurrentStream(answerStream);
        
      // запустили команду в обработку
       controller.ProcessModuleCommand(cmd);

    
    } // if
    else
    {
      // что-то пошло не так, игнорируем команду
    } // else
    
    commandsFromSerial.ClearCommand(); // очищаем полученную команду
   } // if

    #ifdef USE_WIFI_MODULE
    // модуль Wi-Fi обновляем каждый раз при вызове loop
    WIFI_EVENT_FUNC(); // вызываем функцию проверки данных в порту
    #endif
    
    // обновляем состояние всех зарегистрированных модулей, по одному за каждый вызов loop
   controller.UpdateModules(dt);


   
// отсюда можно добавлять любой сторонний код


// до сюда можно добавлять любой сторонний код

}

#ifdef USE_SMS_MODULE
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
