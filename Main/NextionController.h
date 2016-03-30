#ifndef _NEXTION_CONTROLLER_H
#define _NEXTION_CONTROLLER_H
#include <Arduino.h>
#include "Globals.h"
#include "AbstractModule.h"

#define NEXTION_COMMAND_BUFFER_LENGTH 50 // длина буфера для команд, 50 байт должно хватить с запасом
#define NEXTION_CHAR_PLACES 7 // сколько у нас позиций под надпись
#define MINUS_START_ADDR 97 // стартовый адрес минуса
#define DOT_START_ADDRESS 104 // стартовый адрес запятой
#define CELSIUS_START_ADDRESS 118 // стартовый адрес знака градуса
#define PERCENT_START_ADDRESS 125 // стартовый адрес знака процента
#define LUX_START_ADDRESS 132 // стартовый адрес знака освещенности
#define DIGITS_START_ADDRESS 27 // нули начинаются с адреса 27, группами по 7 далее идут все цифры
#define EMPTY_CELLS_START_ADDRESS 20 // стартовый адрес пустых ячеек для цифр

typedef enum
{
  emReturnNothing, // No return
  emReturnSuccessfull, // Only return the successful data
  emReturnFailed, // Only return the failed data
  emAlwaysReturn // Always return

} NextionEchoMode; // режим эха в ответ на посланные команды

typedef enum
{
  etCommandFinished, // событие "Команда обработана успешно"

  etInvalidCommand, // событие "Неправильная команда"
  etInvalidComponentID, // событие "Неправильный ID сомпонента"
  etInvalidPageID, // событие "Неправильный ID страницы"
  etInvalidPictureID, // событие "Неправильный ID рисунка"
  etInvalidFontID, // событие "Неправильный ID шрифта"
  etInvalidBaudRate, // событие "Неправильные установки скорости"
  
  etCurrentPageID, // событие "Получен номер текущей страницы"

  etButtonTouch, // событие "Нажата кнопка"
  etTouchPosition, // событие "Получены координаты тача"
  etTouchInSleep, // событие "Получены координаты тача в режиме сна"

  etStringReceived, // событие "Получена строка" 
  etNumberReceived, // событие "Получен номер"

  etSleep, // событие "Дисплей заснул"
  etWakeUp, // событие "Дисплей проснулся"
  
  etStartup, // событие "Дисплей стартовал"
  etUpgrade // событие "Дисплей обновляет прошивку"
  
  
} NextionEventType; // тип события от Nextion

class NextionAbstractController; // forward declaration

// событие получения ошибки от Nextion
typedef void (*OnNextionEvent)(NextionAbstractController* Sender);
typedef void (*OnNextionError)(NextionAbstractController* Sender, NextionEventType errorCode);

typedef void (*OnNextionButtonTouchEvent)(NextionAbstractController* Sender, uint8_t pageID, uint8_t buttonID, bool pressed);
typedef void (*OnNextionTouchEvent)(NextionAbstractController* Sender, uint16_t x, uint16_t y, bool pressed, bool pressedInSleep);

typedef void (*OnNextionNumberEvent)(NextionAbstractController* Sender, uint32_t num);
typedef void (*OnNextionStringEvent)(NextionAbstractController* Sender, const char* str);

struct NextionSubscribeStruct
{
  OnNextionError OnError;
  OnNextionEvent OnSuccess;
  
  OnNextionButtonTouchEvent OnButtonTouch;
  OnNextionTouchEvent OnTouch;

  OnNextionNumberEvent OnPageIDReceived;
  OnNextionNumberEvent OnNumberReceived;
  OnNextionStringEvent OnStringReceived;

  OnNextionEvent OnSleep;
  OnNextionEvent OnWakeUp;
  OnNextionEvent OnLaunch;
  OnNextionEvent OnUpgrade;  
};

class NextionAbstractController
{
  public:

    void begin(Stream* s,void* userData = NULL); // начинаем работу
    void update(); // обновляем внутреннее состояние
    
    void sendCommand(const char* cmd); // посылает команду дисплею

    // всякие настроечные команды
    void setSleepDelay(uint8_t seconds=NEXTION_SLEEP_DELAY); // через сколько секунд, если ничего не нажато, переходить в режим сна
    void setWakeOnTouch(bool awake=true); // просыпаться после нажатия на тач?
    void setEchoMode(NextionEchoMode mode=emReturnNothing); // установить режим эха в ответ на посланные команды
    void setBaudRate(uint16_t baud, bool setAsDefault=false); // устанавливает скорость соединения
    void setBrightness(uint8_t bright, bool setAsDefault=false); // устанавливает яркость подсветки (0-100)
    void setFontXSpacing(uint8_t spacing=0); // устанавливает x-spacing для шрифта
    void setFontYSpacing(uint8_t spacing=0); // устанавливает y-spacing для шрифта
    void setSendXY(bool shouldSend=false); // если true - Nextion будет посылать координаты тача в порт при каждом таче
    void sleep(bool enterSleep=false); // если true - Nextion переходит в спящий режим, иначе - выходит из него
    void setSysVariableValue(uint8_t sysVarNumber,uint32_t val); // устанавливает значение системных переменных. Номер - от 0 до 2, т.е. или 0, или 1, или 2.  

    // переход по страницам
    void goToPage(uint8_t pageNum=0);

    void* getUserData() {return _userData;} // возвращает пользовательские данные
  
    NextionAbstractController(); // конструктор

    // подписка на события
    void subscribe(const NextionSubscribeStruct& ss);


protected:

  static char command_buff[NEXTION_COMMAND_BUFFER_LENGTH]; // буфер для команд
  Stream* workStream; // поток для общения с Nextion

  void* _userData; // пользовательские данные
  
  OnNextionError _onError;
  OnNextionEvent _onSuccess;
  
  OnNextionButtonTouchEvent _onButtonTouch;
  OnNextionTouchEvent _onTouch;

  OnNextionNumberEvent _onPageIDReceived;
  OnNextionNumberEvent _onNumberReceived;
  OnNextionStringEvent _onStringReceived;

  OnNextionEvent _onSleep;
  OnNextionEvent _onWakeUp;
  OnNextionEvent _onLaunch;
  OnNextionEvent _onUpgrade;

 private:

 String recvBuff; // буфер для приёма команд
 void recvAnswer(); // получает ответ от Nextion
 bool gotCommand(); // проверяет, есть ли полная команда в буфере
 void processCommand(); // обрабатывает команду


  
};

class NextionController : public NextionAbstractController
{
  public:
    NextionController();

    // всякие специфические команды
    void setWaitTimerInterval(uint16_t val=NEXTION_WAIT_TIMER); // установить интервал таймера переключения на экран ожидания
    
    void notifyWindowState(bool isWindowsOpen); // сообщает, открыты или закрыты окна экрану, чтобы тот сменил состояние кнопки
    void notifyWindowMode(bool isAutoMode); // сообщает, какой режим работы окон сейчас - автоматический или ручной

    void notifyWaterState(bool isWaterOn); // сообщает, включен ли полив
    void notifyWaterMode(bool isAutoMode); // сообщает режим работы полива - автоматический или ручной

    void notifyLightState(bool iLightOn); // сообщает, включена ли досветка
    void notifyLightMode(bool isAutoMode); // сообщает режим работы досветки - автоматический или ручной

    // управление показаниями датчиков
    void showTemperature(const Temperature& t); // показывает температуру
    void showHumidity(const Humidity& h); // показывает влажность
    void showLuminosity(long lum); // показывает освещенность
    
    void showOpenTemp(uint8_t temp); // показывает температуру открытия
    void showCloseTemp(uint8_t temp); // показывает температуру закрытия
    


 private:

  void setSegmentInfo(uint8_t segNum,uint8_t charStartAddress); // устанавливает символ для нужного сегмента, начиная с переданного смещения
  // показывает номер на дисплее в указанной позиции, 
  // при необходимости - добавляет ведущий 0.
  // возвращает кол-во записанных позиций.
  uint8_t showNumber(long num,uint8_t segNum=0,bool addLeadingZero=false);
  uint8_t fillEmptySpaces(uint8_t pos_written);

  void doShowSettingsTemp(uint8_t temp,const char* which="open",uint8_t offset=0);
    
};

#endif
