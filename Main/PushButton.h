#ifndef _PUSHBUTTON_H
#define _PUSHBUTTON_H
//--------------------------------------------------------------------------------------------------
#include <Arduino.h>
//--------------------------------------------------------------------------------------------------
#define BOUNCE_INTERVAL  70 // длительность отслеживания дребезга.
#define DOUBLECLICK_INTERVAL 200 // длительность отслеживания двойного клика.
#define INACTIVITY_INTERVAL 5000 // длительность отслеживания неактивности.
#define RETENTION_INTERVAL 2000 // длительность отслеживания нажатия и удержания.
//--------------------------------------------------------------------------------------------------
class PushButton;
typedef void (*PushButtonEvent)(const PushButton& Sender, void* UserData);
//--------------------------------------------------------------------------------------------------
class PushButton
{
 public:
  
    PushButton(uint8_t _pin);

    // инициализируем, в параметрах можем передать указатель на любые пользовательские данные,
    // плюс указатели на функции-обработчики событий, если это необходимо
    void init(void* _userData = NULL
    , PushButtonEvent _onClick = NULL
    , PushButtonEvent _onPress = NULL
    , PushButtonEvent _onDoubleClick = NULL
    , PushButtonEvent _onInactive = NULL
    , PushButtonEvent _onRetention = NULL
    );    
    void update(); // обновляем внутреннее состояние
    
    bool isPressed() { return click_down; }
    bool isClicked() { return click_up; }
    bool isDoubleClicked() { return doubleclick; }
    bool isInactive() { return timer; }
    bool isRetention() { return retention; }


 private:

  void* userData;
  PushButtonEvent OnClick; // событие - кнопка нажата и отпущена
  PushButtonEvent OnPress; // событие - кнопка нажата
  PushButtonEvent OnDoubleClick; // событие - кнопка нажата дважды
  PushButtonEvent OnInactive; // событие - кнопка неактивна в течение настроенного интервала
  PushButtonEvent OnRetention; // событие - кнопка нажата и удерживается определённое время
  
  
  unsigned long lastMillis;
  uint8_t  lastButtonState;
  bool  lastBounce;
  bool lastDoubleClick;
  uint8_t     clickCounter;
  bool  lastTimer;
  bool  lastRetention;
  bool atLeastOneStateChangesFound;

  byte buttonPin; // пин, на котором висит кнопка

  bool click_down; // нажата?
  bool click_up; // нажата и отпущена?
  bool doubleclick; // два раза нажата и отпущена?
  bool timer; // неактивна в течение установленного интервала?
  bool retention; // нажата и удерживается в течение установленного интервала?


 protected:   
    
    uint8_t readButtonState(uint8_t pin)
    {
      return digitalRead(pin);
    }
    
 };
//--------------------------------------------------------------------------------------------------
#endif
