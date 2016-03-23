#include "PushButton.h"
//--------------------------------------------------------------------------------------------------
PushButton::PushButton(uint8_t _pin)
{
  buttonPin = _pin;
  pinMode(buttonPin, INPUT_PULLUP); // подтягиваем к питанию
  
  userData = NULL;
  OnClick = NULL;
  OnPress = NULL;
  OnDoubleClick = NULL;
  OnInactive = NULL;
  OnRetention = NULL;
  
  atLeastOneStateChangesFound = false;

  click_down      = false;
  click_up        = false;
  doubleclick     = false;
  timer           = false;
  retention       = false;

  clickCounter  =     0;
  
  lastBounce  =      false;
  lastDoubleClick =  false;
  lastTimer  =        false;
  lastRetention  =    false;
  
  lastMillis  =      millis();
}
//--------------------------------------------------------------------------------------------------
void PushButton::init(void* _userData  
    , PushButtonEvent _onClick
    , PushButtonEvent _onPress
    , PushButtonEvent _onDoubleClick
    , PushButtonEvent _onInactive
    , PushButtonEvent _onRetention)
{
  userData = _userData;
  OnClick = _onClick;
  OnPress = _onPress;
  OnDoubleClick = _onDoubleClick;
  OnInactive = _onInactive;
  OnRetention = _onRetention;
  
  lastButtonState  = readButtonState(buttonPin);
}
//--------------------------------------------------------------------------------------------------
void PushButton::update()
{

  // обновляем внутреннее состояние
  bool curBounce  = false;
  bool curDoubleClick = false;
  bool curTimer  = false;
  bool curRetention  = false;

  // сбрасываем все флаги
  click_down  = false;
  click_up    = false;
  doubleclick = false;
  timer       = false;
  retention   = false;


  unsigned long curMillis = millis();
  unsigned long millisDelta = curMillis - lastMillis;
  uint8_t curButtonState = readButtonState(buttonPin); // читаем текущее состояние

 if (curButtonState != lastButtonState) // состояние изменилось
  {
    atLeastOneStateChangesFound = true; // было хотя бы одно изменение в состоянии (нужно для того, чтобы не было события "clicked", когда кнопку не нажимали ни разу)
    lastButtonState = curButtonState; // сохраняем его
    lastMillis = curMillis; // и время последнего обновления
  }

  if (millisDelta > BOUNCE_INTERVAL)  // надо проверить на дребезг
    curBounce = true;

  if (millisDelta > DOUBLECLICK_INTERVAL) // надо проверить на даблклик
    curDoubleClick = true;

  if (curDoubleClick != lastDoubleClick) // состояние даблклика с момента последней проверки изменилось
  {
    lastDoubleClick = curDoubleClick; // сохраняем текущее
    if (lastDoubleClick) // проверяем - если кнопка не нажата, то сбрасываем счётчик нажатий 
      clickCounter = 0;
  }

  if (curBounce != lastBounce) // состояние проверки дребезга изменилось
  {
    lastBounce = curBounce; // сохраняем текущее

    if (!lastButtonState && !curBounce) // если кнопка была нажата в момент последнего замера и сейчас - значит, дребезг прошёл и мы можем сохранять состояние
    {
      click_down = true; // выставляем флаг, что кнопка нажата

      if(OnPress)
        OnPress(*this,userData);
      
      ++clickCounter; // увеличиваем счётчик кликов
      
      if (clickCounter == 2) // если кликнули два раза
      {
        clickCounter = 0;  // сбрасываем счётчик кликов
        doubleclick = true; // и выставляем флаг двойного нажатия
        
        if(OnDoubleClick)
          OnDoubleClick(*this,userData);
      }
    }

    click_up = lastButtonState && lastBounce && atLeastOneStateChangesFound; // кнопка отпущена тогда, когда последний замер и текущий - равны 1 (пин подтянут к питанию!), и был хотя бы один клик на кнопке
    
    if(click_up && OnClick)
      OnClick(*this,userData);

  }

  if (millisDelta > INACTIVITY_INTERVAL) // пора проверять неактивность
    curTimer = true;
    
  if (curTimer != lastTimer) // состояние неактивности изменилось с момента последнего замера?
  {
    lastTimer = curTimer; // сохраняем текущее
    timer = lastButtonState && lastTimer && atLeastOneStateChangesFound; // кнопка неактивна тогда, когда не была нажата с момента последнего опроса этого состояния

    if(timer && OnInactive)
      OnInactive(*this,userData);
  }

  if (millisDelta > RETENTION_INTERVAL) // пора проверять удержание
    curRetention = true;

  if (curRetention != lastRetention) // если состояние изменилось
  {
    lastRetention = curRetention; // сохраняем его
    retention = !lastButtonState && lastRetention && atLeastOneStateChangesFound; // и считаем кнопку удерживаемой, когда она нажата сейчас и была нажата до этого
    
    if(retention && OnRetention)
      OnRetention(*this,userData); 
  }
  
}
//--------------------------------------------------------------------------------------------------

