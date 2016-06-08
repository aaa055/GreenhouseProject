#include "LCDMenu.h"
#include "InteropStream.h"

#ifdef USE_LCD_MODULE

WaitScreenInfo WaitScreenInfos[] = 
{
   WAIT_SCREEN_SENSORS
  ,{0,0,"",""} // последний элемент пустой, заглушка для признака окончания списка
};


PushButton button(MENU_BUTTON_PIN); // кнопка для управления меню
void ButtonOnClick(const PushButton& Sender, void* UserData) // пришло событие от кнопки - кликнута
{
  UNUSED(Sender);
  
  LCDMenu* menu = (LCDMenu*) UserData;
  menu->enterSubMenu(); // просим войти в подменю
}

IdlePageMenuItem IdleScreen; // экран ожидания

#ifdef USE_TEMP_SENSORS
WindowMenuItem WindowManageScreen; // экран управления окнами
#endif

#ifdef USE_WATERING_MODULE
WateringMenuItem WateringManageScreen; // экран управления поливом
#endif

#ifdef USE_LUMINOSITY_MODULE
LuminosityMenuItem LuminosityManageScreen; // экран управления досветкой
#endif

SettingsMenuItem SettingsManageScreen; // экран настроек


AbstractLCDMenuItem::AbstractLCDMenuItem(const unsigned char* i, const char* c) :
icon(i), caption(c), focused(false), needToDrawCursor(false),cursorPos(-1), itemsCount(0)
{
  
}
void AbstractLCDMenuItem::init(LCDMenu* parent)
{
  parentMenu = parent;
}
void AbstractLCDMenuItem::setFocus(bool f)
{
  focused = f;
 if(!f)
  {
    needToDrawCursor = false;
    cursorPos = -1;
  }  
}
void AbstractLCDMenuItem::OnButtonClicked(LCDMenu* menu)
{
  // кликнули по кнопке, когда наше меню на экране.
  // фокус ввода может быть ещё не установлен (первое нажатие на кнопку),
  // или - установлен (повторные нажатия на кнопку)
  
  bool lastNDC = needToDrawCursor;
  needToDrawCursor = true;
  int8_t lastCP = cursorPos;

  // увеличиваем позицию курсора
  ++cursorPos;
  if(!itemsCount || cursorPos > (itemsCount-1)) // значит, достигли края, и надо выйти из фокуса
  {
    needToDrawCursor = false;
    cursorPos = -1;
    focused = false; // не вызываем setFocus напрямую, т.к. он может быть переопределён
  }

  if(lastNDC != needToDrawCursor || lastCP != cursorPos) // сообщаем, что надо перерисовать экран, т.к. позиция курсора изменилась
    menu->wantRedraw(); 
}


IdlePageMenuItem::IdlePageMenuItem() : AbstractLCDMenuItem(MONITOR_ICON,("Монитор"))
{
  rotationTimer = ROTATION_INTERVAL; // получаем данные с сенсора сразу в первом вызове update
  currentSensorIndex = 0; 
  displayString = NULL;
}
void IdlePageMenuItem::init(LCDMenu* parent)
{
  AbstractLCDMenuItem::init(parent);
  // инициализируем экран ожидания
  
  // получаем данные с сенсора
  RequestSensorData(WaitScreenInfos[currentSensorIndex]);

}
void IdlePageMenuItem::OnButtonClicked(LCDMenu* menu)
{
  AbstractLCDMenuItem::OnButtonClicked(menu);

    rotationTimer = 0; // сбрасываем таймер ротации

    // выбираем следующий сенсор
    SelectNextSensor();

    // получаем данные с сенсора
    RequestSensorData(WaitScreenInfos[currentSensorIndex]);

    // говорим, что мы хотим перерисоваться
    menu->notifyMenuUpdated(this);
    
}
void IdlePageMenuItem::SelectNextSensor()
{
    currentSensorIndex++;
    if(!WaitScreenInfos[currentSensorIndex].sensorType)
    {
      // достигли конца списка, возвращаемся в начало
      currentSensorIndex = 0;
    }  
}
void IdlePageMenuItem::update(uint16_t dt, LCDMenu* menu)
{

  rotationTimer += dt;

  if(rotationTimer >= ROTATION_INTERVAL) // пришла пора крутить показания
  {
    rotationTimer = 0;

    // выбираем следующий сенсор
    SelectNextSensor();

    // получаем данные с сенсора
    RequestSensorData(WaitScreenInfos[currentSensorIndex]);

    // говорим, что мы хотим перерисоваться
    menu->notifyMenuUpdated(this);
    
  } // if(rotationTimer >= ROTATION_INTERVAL)

  
}
void IdlePageMenuItem::RequestSensorData(const WaitScreenInfo& info)
{
  // обновляем показания с датчиков
  sensorData = "";
  displayString = NULL;

  if(!info.sensorType) // нечего показывать
    return;

  AbstractModule* mod = parentMenu->mainController->GetModuleByID(info.moduleName);

  if(!mod) // не нашли такой модуль
  {
    rotationTimer = ROTATION_INTERVAL; // просим выбрать следующий модуль
    return;
  }
  OneState* os = mod->State.GetState((ModuleStates)info.sensorType,info.sensorIndex);
  if(!os)
  {
    // нет такого датчика, просим показать следующие данные
    rotationTimer = ROTATION_INTERVAL;
    return;
  }

  displayString = info.displayName; // запоминаем, чего выводить на экране


   //Тут получаем актуальные данные от датчиков
   switch(info.sensorType)
   {
      case StateTemperature:
      {
        TemperaturePair tp = *os;
        if(tp.Current.Value != NO_TEMPERATURE_DATA)
        {
          sensorData = tp.Current;
          sensorData += F(" C");
        }
        else
          sensorData = NO_DATA;
      } 
      break;

      case StateHumidity:
      case StateSoilMoisture:
      {
        HumidityPair hp = *os;
        if(hp.Current.Value != NO_TEMPERATURE_DATA)
        {
          sensorData = hp.Current;
          sensorData += F("%");
        }
         else
          sensorData = NO_DATA;
       
      }
      break;

      case StatePH: // выводим значение pH
      {
        HumidityPair hp = *os;
        if(hp.Current.Value != NO_TEMPERATURE_DATA)
        {
          sensorData = hp.Current;
          sensorData += F(" pH");
        }
         else
          sensorData = NO_DATA;
       
      }
      break;      

      case StateLuminosity:
      {
        LuminosityPair lp = *os;
        if(lp.Current != NO_LUMINOSITY_DATA)
        {
          sensorData = String(lp.Current);
          sensorData += F(" люкс");
        }
         else
          sensorData = NO_DATA;
     }
      break;
    
   } // switch
  
}
void IdlePageMenuItem::draw(DrawContext* dc)
{
  // рисуем показания с датчиков
  const int frame_width = FRAME_WIDTH - CONTENT_PADDING*2;
  
  if(!sensorData.length() || !displayString) // нечего рисовать
    return;

  // рисуем показания с датчика по центру экрана
  int cur_top = 14 + MENU_BITMAP_SIZE;
  u8g_uint_t strW = dc->getStrWidth(sensorData.c_str());
  int left = (frame_width - strW)/2 + CONTENT_PADDING;

  dc->drawStr(left, cur_top, sensorData.c_str());

  // теперь рисуем строку подписи
  cur_top += HINT_FONT_HEIGHT;
  strW = dc->getStrWidth(displayString);
  left = (frame_width - strW)/2 + CONTENT_PADDING;

  dc->drawStr(left, cur_top, displayString);

     #ifdef USE_DS3231_REALTIME_CLOCK

        cur_top += HINT_FONT_HEIGHT + 6;
        
        DS3231Clock rtc = parentMenu->mainController->GetClock();
        DS3231Time tm = rtc.getTime();

        static char dt_buff[20] = {0};
        sprintf_P(dt_buff,(const char*) F("%02d.%02d.%d %02d:%02d"), tm.dayOfMonth, tm.month, tm.year, tm.hour, tm.minute);
        
        strW = dc->getStrWidth(dt_buff);
        left = (frame_width - strW)/2 + CONTENT_PADDING;
        
        dc->drawStr(left, cur_top, dt_buff);
        
      #endif // USE_DS3231_REALTIME_CLOCK 

}
#ifdef USE_TEMP_SENSORS
WindowMenuItem::WindowMenuItem() : AbstractLCDMenuItem(WINDOW_ICON,("Окна"))
{
  
}
void WindowMenuItem::init(LCDMenu* parent)
{
  AbstractLCDMenuItem::init(parent);
  
  isWindowsOpen = WORK_STATUS.GetStatus(WINDOWS_STATUS_BIT);
  isWindowsAutoMode = WORK_STATUS.GetStatus(WINDOWS_MODE_BIT);

  itemsCount = 3;
}
bool WindowMenuItem::OnEncoderPositionChanged(int dir, LCDMenu* menu)
{
  if(!needToDrawCursor) // курсор не нарисован, значит, нам не надо обрабатывать смену настройки с помощью энкодера
    return false;

    bool lastWO = isWindowsOpen;
    bool lastWAM = isWindowsAutoMode;

    if(dir != 0)
    {
       // есть смена позиции энкодера, смотрим, какой пункт у нас выбран
       switch(cursorPos)
       {
          case 0: // открыть окна
          {
            isWindowsOpen = true;
            //Тут посылаем команду на открытие окон
            ModuleInterop.QueryCommand(ctSET,F("STATE|WINDOW|ALL|OPEN"),false,false);
          }
          break;
          
          case 1: // закрыть окна
          {
            isWindowsOpen = false;
            //Тут посылаем команду на закрытие окон
            ModuleInterop.QueryCommand(ctSET,F("STATE|WINDOW|ALL|CLOSE"),false,false);
          }
          break;
          
          case 2: // поменять режим
          {
            isWindowsAutoMode = !isWindowsAutoMode;
            //Тут посылаем команду на смену режима окон
            if(isWindowsAutoMode)
              ModuleInterop.QueryCommand(ctSET,F("STATE|MODE|AUTO"),false,false);
            else
              ModuleInterop.QueryCommand(ctSET,F("STATE|MODE|MANUAL"),false,false);
          }
          break;
        
       } // switch
    }

    if(lastWO != isWindowsOpen || lastWAM != isWindowsAutoMode) // состояние изменилось, просим меню перерисоваться
      menu->wantRedraw();

    return true; // сами обработали смену позиции энкодера
}
void WindowMenuItem::update(uint16_t dt, LCDMenu* menu)
{
  UNUSED(dt);
  
  // вызывать метод wantRedraw родительского меню можно только, если на экране
  // текущее меню, иначе - слишком частые отрисовки могут быть.
  // поэтому вызываем notifyMenuUpdated только тогда, когда были изменения.
 
  bool lastWO = isWindowsOpen;
  bool lastWAM = isWindowsAutoMode;
  
  isWindowsOpen = WORK_STATUS.GetStatus(WINDOWS_STATUS_BIT);
  isWindowsAutoMode = WORK_STATUS.GetStatus(WINDOWS_MODE_BIT);

  bool anyChangesFound = (isWindowsOpen != lastWO) || (lastWAM != isWindowsAutoMode);
  
  
  if(anyChangesFound)
    menu->notifyMenuUpdated(this);
}
void WindowMenuItem::draw(DrawContext* dc)
{
  // вычисляем, с каких позициях нам рисовать наши иконки
  const int frame_width = FRAME_WIDTH - CONTENT_PADDING*2;
  const int one_icon_box_width = frame_width/itemsCount;
  const int one_icon_left_spacing = (one_icon_box_width-MENU_BITMAP_SIZE)/2;

  static const __FlashStringHelper* captions[] = 
  {
     F("откр")
    ,F("закр")
    ,F("авто")    
  };

 // рисуем три иконки невыбранных чекбоксов  - пока
 for(int i=0;i<itemsCount;i++)
 {
 int cur_top = 20;
  const unsigned char* cur_icon = UNCHECK_ICON;
    if(i == 0)
    {
      if(isWindowsOpen)
        cur_icon = RADIO_CHECK_ICON;
      else
        cur_icon = RADIO_UNCHECK_ICON;
    }
    else
    if(i == 1)
    {
      if(!isWindowsOpen)
        cur_icon = RADIO_CHECK_ICON;
      else
        cur_icon = RADIO_UNCHECK_ICON;
    }
    else
    if(i == 2)
    {
      if(isWindowsAutoMode)
         cur_icon = CHECK_ICON;
    }
  int left = i*CONTENT_PADDING + i*one_icon_box_width + one_icon_left_spacing;
  dc->drawXBMP(left, cur_top, MENU_BITMAP_SIZE, MENU_BITMAP_SIZE, cur_icon);

  // теперь рисуем текст иконки
  u8g_uint_t strW = dc->getStrWidth(captions[i]);

  // вычисляем позицию шрифта слева
  left =  i*CONTENT_PADDING + i*one_icon_box_width + (one_icon_box_width - strW)/2;

  // рисуем заголовок
  cur_top += MENU_BITMAP_SIZE + HINT_FONT_HEIGHT;
  dc->drawStr(left, cur_top, captions[i]);

  if(needToDrawCursor && i == cursorPos)
  {
    // рисуем курсор в текущей позиции
    cur_top += HINT_FONT_BOX_PADDING;
    dc->drawHLine(left,cur_top,strW);
  }
  yield();
 } // for

}
#endif

#ifdef USE_WATERING_MODULE
WateringMenuItem::WateringMenuItem() : AbstractLCDMenuItem(WATERING_ICON,("Полив"))
{
  
}
void WateringMenuItem::init(LCDMenu* parent)
{
  AbstractLCDMenuItem::init(parent);
 
  isWateringOn = WORK_STATUS.GetStatus(WATER_STATUS_BIT);
  isWateringAutoMode = WORK_STATUS.GetStatus(WATER_MODE_BIT);
  
  itemsCount = 3;
}
void WateringMenuItem::draw(DrawContext* dc)
{
  // вычисляем, с каких позициях нам рисовать наши иконки
  const int frame_width = FRAME_WIDTH - CONTENT_PADDING*2;
  const int one_icon_box_width = frame_width/itemsCount;
  const int one_icon_left_spacing = (one_icon_box_width-MENU_BITMAP_SIZE)/2;

  static const __FlashStringHelper* captions[] = 
  {
     F("вкл")
    ,F("выкл")
    ,F("авто")    
  };

 // рисуем три иконки невыбранных чекбоксов  - пока
 for(int i=0;i<itemsCount;i++)
 {
 int cur_top = 20;
  const unsigned char* cur_icon = UNCHECK_ICON;
    if(i == 0)
    {
      if(isWateringOn)
        cur_icon = RADIO_CHECK_ICON;
      else
        cur_icon = RADIO_UNCHECK_ICON;
    }
    else
    if(i == 1)
    {
      if(!isWateringOn)
        cur_icon = RADIO_CHECK_ICON;
      else
        cur_icon = RADIO_UNCHECK_ICON;
    }
    else
    if(i == 2)
    {
      if(isWateringAutoMode)
         cur_icon = CHECK_ICON;
    }
  int left = i*CONTENT_PADDING + i*one_icon_box_width + one_icon_left_spacing;
  dc->drawXBMP(left, cur_top, MENU_BITMAP_SIZE, MENU_BITMAP_SIZE, cur_icon);

  // теперь рисуем текст иконки
  u8g_uint_t strW = dc->getStrWidth(captions[i]);

  // вычисляем позицию шрифта слева
  left =  i*CONTENT_PADDING + i*one_icon_box_width + (one_icon_box_width - strW)/2;

  // рисуем заголовок
  cur_top += MENU_BITMAP_SIZE + HINT_FONT_HEIGHT;
  dc->drawStr(left, cur_top, captions[i]);

  if(needToDrawCursor && i == cursorPos)
  {
    // рисуем курсор в текущей позиции
    cur_top += HINT_FONT_BOX_PADDING;
    dc->drawHLine(left,cur_top,strW);
  }
  yield();
 } // for
}
void WateringMenuItem::update(uint16_t dt, LCDMenu* menu)
{
  UNUSED(dt);
 
  // вызывать метод wantRedraw родительского меню можно только, если на экране
  // текущее меню, иначе - слишком частые отрисовки могут быть.
  // поэтому вызываем notifyMenuUpdated только тогда, когда были изменения.

  bool lastWO = isWateringOn;
  bool lastWAM = isWateringAutoMode;

  isWateringOn = WORK_STATUS.GetStatus(WATER_STATUS_BIT);
  isWateringAutoMode = WORK_STATUS.GetStatus(WATER_MODE_BIT);

  
  bool anyChangesFound = (isWateringOn != lastWO) || (isWateringAutoMode != lastWAM);
 
  if(anyChangesFound)
    menu->notifyMenuUpdated(this);  
}
bool WateringMenuItem::OnEncoderPositionChanged(int dir, LCDMenu* menu)
{
  if(!needToDrawCursor) // курсор не нарисован, значит, нам не надо обрабатывать смену настройки с помощью энкодера
    return false;

    bool lastWO = isWateringOn;
    bool lastWAM = isWateringAutoMode;

    if(dir != 0)
    {
       // есть смена позиции энкодера, смотрим, какой пункт у нас выбран
       switch(cursorPos)
       {
          case 0: // включить полив
          {
            isWateringOn = true;
            //Тут посылаем команду на включение полива
            ModuleInterop.QueryCommand(ctSET,F("WATER|ON"),false,false);
          }
          break;
          
          case 1: // выключить полив
          {
            isWateringOn = false;
            //Тут посылаем команду на выключение полива
            ModuleInterop.QueryCommand(ctSET,F("WATER|OFF"),false,false);
          }
          break;
          
          case 2: // поменять режим
          {
            isWateringAutoMode = !isWateringAutoMode;
            //Тут посылаем команду на смену режима полива
            if(isWateringAutoMode)
              ModuleInterop.QueryCommand(ctSET,F("WATER|MODE|AUTO"),false,false);
            else
              ModuleInterop.QueryCommand(ctSET,F("WATER|MODE|MANUAL"),false,false);
          }
          break;
        
       } // switch
    }

    if(lastWO != isWateringOn || lastWAM != isWateringAutoMode) // состояние изменилось, просим меню перерисоваться
      menu->wantRedraw();

    return true; // сами обработали смену позиции энкодера
  
}

#endif

#ifdef USE_LUMINOSITY_MODULE
LuminosityMenuItem::LuminosityMenuItem() : AbstractLCDMenuItem(LUMINOSITY_ICON,("Досветка"))
{
  
}
void LuminosityMenuItem::init(LCDMenu* parent)
{
  AbstractLCDMenuItem::init(parent);

  isLightOn = WORK_STATUS.GetStatus(LIGHT_STATUS_BIT);
  isLightAutoMode = WORK_STATUS.GetStatus(LIGHT_MODE_BIT);
  
  itemsCount = 3;
}
void LuminosityMenuItem::draw(DrawContext* dc)
{
  // вычисляем, с каких позициях нам рисовать наши иконки
  const int frame_width = FRAME_WIDTH - CONTENT_PADDING*2;
  const int one_icon_box_width = frame_width/itemsCount;
  const int one_icon_left_spacing = (one_icon_box_width-MENU_BITMAP_SIZE)/2;

  static const __FlashStringHelper* captions[] = 
  {
     F("вкл")
    ,F("выкл")
    ,F("авто")    
  };

 // рисуем три иконки невыбранных чекбоксов  - пока
 for(int i=0;i<itemsCount;i++)
 {
 int cur_top = 20;
  const unsigned char* cur_icon = UNCHECK_ICON;
    if(i == 0)
    {
      if(isLightOn)
        cur_icon = RADIO_CHECK_ICON;
      else
        cur_icon = RADIO_UNCHECK_ICON;
    }
    else
    if(i == 1)
    {
      if(!isLightOn)
        cur_icon = RADIO_CHECK_ICON;
      else
        cur_icon = RADIO_UNCHECK_ICON;
    }
    else
    if(i == 2)
    {
      if(isLightAutoMode)
         cur_icon = CHECK_ICON;
    }
  int left = i*CONTENT_PADDING + i*one_icon_box_width + one_icon_left_spacing;
  dc->drawXBMP(left, cur_top, MENU_BITMAP_SIZE, MENU_BITMAP_SIZE, cur_icon);

  // теперь рисуем текст иконки
  u8g_uint_t strW = dc->getStrWidth(captions[i]);

  // вычисляем позицию шрифта слева
  left =  i*CONTENT_PADDING + i*one_icon_box_width + (one_icon_box_width - strW)/2;

  // рисуем заголовок
  cur_top += MENU_BITMAP_SIZE + HINT_FONT_HEIGHT;
  dc->drawStr(left, cur_top, captions[i]);

  if(needToDrawCursor && i == cursorPos)
  {
    // рисуем курсор в текущей позиции
    cur_top += HINT_FONT_BOX_PADDING;
    dc->drawHLine(left,cur_top,strW);
  }
  yield();
 } // for
}
void LuminosityMenuItem::update(uint16_t dt, LCDMenu* menu)
{
 UNUSED(dt);
 
  // вызывать метод wantRedraw родительского меню можно только, если на экране
  // текущее меню, иначе - слишком частые отрисовки могут быть.
  // поэтому вызываем notifyMenuUpdated только тогда, когда были изменения.

  bool lastLO = isLightOn;
  bool lastLAM = isLightAutoMode;
  
  isLightOn = WORK_STATUS.GetStatus(LIGHT_STATUS_BIT);
  isLightAutoMode = WORK_STATUS.GetStatus(LIGHT_MODE_BIT);

  
  bool anyChangesFound = (isLightOn != lastLO) || (isLightAutoMode != lastLAM);
 
  if(anyChangesFound)
    menu->notifyMenuUpdated(this);  
}
bool LuminosityMenuItem::OnEncoderPositionChanged(int dir, LCDMenu* menu)
{
  if(!needToDrawCursor) // курсор не нарисован, значит, нам не надо обрабатывать смену настройки с помощью энкодера
    return false;

    bool lastLO = isLightOn;
    bool lastLAM = isLightAutoMode;

    if(dir != 0)
    {
       // есть смена позиции энкодера, смотрим, какой пункт у нас выбран
       switch(cursorPos)
       {
          case 0: // включить досветку
          {
            isLightOn = true;
            //Тут посылаем команду на включение досветки
            ModuleInterop.QueryCommand(ctSET,F("LIGHT|ON"),false,false);
          }
          break;
          
          case 1: // выключить досветку
          {
            isLightOn = false;
            //Тут посылаем команду на выключение досветки
            ModuleInterop.QueryCommand(ctSET,F("LIGHT|OFF"),false,false);
          }
          break;
          
          case 2: // поменять режим
          {
            isLightAutoMode = !isLightAutoMode;
            //Тут посылаем команду на смену режима досветки
            if(isLightAutoMode)
              ModuleInterop.QueryCommand(ctSET,F("LIGHT|MODE|AUTO"),false,false);
            else
              ModuleInterop.QueryCommand(ctSET,F("LIGHT|MODE|MANUAL"),false,false);
          }
          break;
        
       } // switch
    }

    if(lastLO != isLightOn || lastLAM != isLightAutoMode) // состояние изменилось, просим меню перерисоваться
      menu->wantRedraw();

    return true; // сами обработали смену позиции энкодера
  
}
#endif

SettingsMenuItem::SettingsMenuItem() : AbstractLCDMenuItem(SETTINGS_ICON,("Настройки"))
{
  
}
void SettingsMenuItem::init(LCDMenu* parent)
{
  AbstractLCDMenuItem::init(parent);

  GlobalSettings* s = parentMenu->mainController->GetSettings();
  
  openTemp = s->GetOpenTemp();
  closeTemp = s->GetCloseTemp();
  
  itemsCount = 2;
}
void SettingsMenuItem::draw(DrawContext* dc)
{
  // вычисляем, с каких позициях нам рисовать наши иконки
  const int text_field_width = HINT_FONT_HEIGHT*3 + HINT_FONT_BOX_PADDING*2;
  const int text_field_height = HINT_FONT_HEIGHT + HINT_FONT_BOX_PADDING*3;
  
  const int frame_width = FRAME_WIDTH - CONTENT_PADDING*2;
  const int one_icon_box_width = frame_width/itemsCount;
  const int one_icon_left_spacing = (one_icon_box_width-text_field_width)/2;

  static const __FlashStringHelper* captions[] = 
  {
     F("Тоткр")
    ,F("Тзакр")
   
  };

 // рисуем наши поля ввода
 for(int i=0;i<itemsCount;i++)
 {
 int cur_top = 24;
  int left = i*CONTENT_PADDING + i*one_icon_box_width + one_icon_left_spacing;
  dc->drawFrame(left, cur_top, text_field_width, text_field_height);
  yield();

  // теперь рисуем текст в полях ввода
  String tmp;
  if(i == 1)
    tmp = String(closeTemp);
  else  
    tmp = String(openTemp);
    
  cur_top += HINT_FONT_HEIGHT + HINT_FONT_BOX_PADDING;
  left += HINT_FONT_BOX_PADDING*2;
  dc->drawStr(left,cur_top,tmp.c_str());
  yield();

  // теперь рисуем текст под полем ввода
  u8g_uint_t strW = dc->getStrWidth(captions[i]);

  // вычисляем позицию шрифта слева
  left =  i*CONTENT_PADDING + i*one_icon_box_width + (one_icon_box_width - strW)/2;

  // рисуем заголовок
  cur_top += text_field_height;
  dc->drawStr(left, cur_top, captions[i]);
  yield();

  if(needToDrawCursor && i == cursorPos)
  {
    // рисуем курсор в текущей позиции
    cur_top += HINT_FONT_BOX_PADDING;
    dc->drawHLine(left,cur_top,strW);
  }
  
 } // for
}
void SettingsMenuItem::update(uint16_t dt, LCDMenu* menu)
{
 UNUSED(dt);
 
  // вызывать метод wantRedraw родительского меню можно только, если на экране
  // текущее меню, иначе - слишком частые отрисовки могут быть.
  // поэтому вызываем notifyMenuUpdated только тогда, когда были изменения.
  GlobalSettings* s = parentMenu->mainController->GetSettings();

 uint8_t lastOT = openTemp;
 uint8_t lastCT = closeTemp; 
  
  openTemp = s->GetOpenTemp();
  closeTemp = s->GetCloseTemp();

  
  bool anyChangesFound = (lastOT != openTemp) || (lastCT != closeTemp);

  if(anyChangesFound)
    menu->notifyMenuUpdated(this);  
}
void SettingsMenuItem::setFocus(bool f)
{
  bool lastFocus = focused;
  AbstractLCDMenuItem::setFocus(f);
  
  if(lastFocus && !f)
  {
    // был фокус и мы его потеряли, значит, надо сохранить настройки
    GlobalSettings* s = parentMenu->mainController->GetSettings();
    s->SetOpenTemp(openTemp);
    s->SetCloseTemp(closeTemp);
    s->Save();
  }
}
bool SettingsMenuItem::OnEncoderPositionChanged(int dir, LCDMenu* menu)
{
  if(!needToDrawCursor) // курсор не нарисован, значит, нам не надо обрабатывать смену настройки с помощью энкодера
    return false;

    uint8_t lastOT = openTemp;
    uint8_t lastCT = closeTemp;

    if(dir != 0)
    {
      GlobalSettings* s = parentMenu->mainController->GetSettings();
      
       // есть смена позиции энкодера, смотрим, какой пункт у нас выбран
       switch(cursorPos)
       {
          case 0: // поменять температуру открытия
          {
            openTemp += dir;
            if(openTemp > SCREEN_MAX_TEMP_VALUE)
            {
              openTemp = dir > 0 ? 0 : SCREEN_MAX_TEMP_VALUE;
            }
            s->SetOpenTemp(openTemp);
          }
          break;
          
          case 1: // выключить температуру закрытия
          {
            closeTemp += dir;
            if(closeTemp > SCREEN_MAX_TEMP_VALUE)
            {
              closeTemp = dir > 0 ? 0 : SCREEN_MAX_TEMP_VALUE;
            }
            s->SetCloseTemp(closeTemp);
          }
          break;
          
        
       } // switch
    }

    if(lastOT != openTemp || lastCT != closeTemp) // состояние изменилось, просим меню перерисоваться
      menu->wantRedraw();

    return true; // сами обработали смену позиции энкодера
  
}


LCDMenu::LCDMenu(uint8_t sck, uint8_t mosi, uint8_t cs) :
#ifdef SCREEN_USE_SOFT_SPI
DrawContext(sck,mosi,cs)
#else
DrawContext(cs)
#endif
{ 
#ifndef SCREEN_USE_SOFT_SPI
  UNUSED(sck);
  UNUSED(mosi);
#endif
  
  wantRedraw(); // говорим, что мы хотим перерисоваться

#ifdef FLIP_SCREEN  
  setRot180(); // переворачиваем экран, если нас попросили в настройках
#endif  

  // добавляем экран ожидания
  items.push_back(&IdleScreen);
  
#ifdef USE_TEMP_SENSORS
  // добавляем экран управления окнами
  items.push_back(&WindowManageScreen);
#endif
  
 #ifdef USE_WATERING_MODULE
  // добавляем экран управления поливом
  items.push_back(&WateringManageScreen);
#endif

#ifdef USE_LUMINOSITY_MODULE  
  // добавляем экран управления досветкой
  items.push_back(&LuminosityManageScreen);
#endif
  
  // добавляем экран управления настройками
  items.push_back(&SettingsManageScreen);

  resetTimer(); // сбрасываем таймер ничегонеделания
  selectedMenuItem = 0; // говорим, что выбран первый пункт меню

}
LCDMenu::~LCDMenu()
{
  //чистим за собой
}
void LCDMenu::wantRedraw()
{
  needRedraw = true; // выставляем флаг необходимости перерисовки
}
void LCDMenu::resetTimer()
{
  gotLastCommmandAt = 0; // сбрасываем таймер простоя
}
void LCDMenu::selectNextMenu(int encoderDirection)
{
  if(!encoderDirection) // не было изменений позиции энкодера
    return;

  resetTimer();   // сбрасываем таймер простоя
  backlight(); // включаем подсветку

  uint8_t lastSelMenu = selectedMenuItem; // запоминаем текущий экран

  //Тут проверяем - не захватил ли экран управление (для изменения своих настроек, например)
  AbstractLCDMenuItem* mi = items[selectedMenuItem];
  if(mi->hasFocus())
  {
    // подменю имеет фокус, значит, оно само должно обработать новое положение энкодера.
    // если оно обработало новое положение энкодера - значит, мы ничего обрабатывать не должны.
    // если экран не хочет обрабатывать положение энкодера - то он возвращает false,
    // и мы обрабатываем положение энкодера сами

    if(mi->OnEncoderPositionChanged(encoderDirection,this))
      return;
      
  } // if(mi->hasFocus())
    
  if(encoderDirection > 0) // двигаемся вперёд
  {
    selectedMenuItem++; // двигаемся к следующему пункту меню
    
    if(selectedMenuItem >= items.size()) // если дошли до конца - заворачиваем в начало
      selectedMenuItem = 0;
  }
  else // двигаемся назад
  {
    if(selectedMenuItem) // если мы не дошли до нулевого пункта
      selectedMenuItem--; // доходим до него
    else
      selectedMenuItem = items.size() - 1; // иначе заворачиваем в самый конец меню
  }

  if(lastSelMenu != selectedMenuItem) // если пункт меню на экране изменился
  {
    items[lastSelMenu]->setFocus(false); // сбрасываем фокус с последнего выбранного меню
    wantRedraw(); // просим перерисоваться
  }
  
}
void LCDMenu::backlight(bool en)
{
  backlightIsOn = en;
  analogWrite(SCREEN_BACKLIGHT_PIN, en ? SCREEN_BACKLIGHT_INTENSITY : 0);
 
  backlightCheckingEnabled = false;
  backlightCounter = 0;
}
void LCDMenu::init(ModuleController* c)
{
  // устанавливаем выбранный шрифт
  setFont(SELECTED_FONT);

  mainController = c;
  // инициализируем пин подсветки
  pinMode(SCREEN_BACKLIGHT_PIN,OUTPUT);
  backlight(); // включаем подсветку экрана
  
  // инициализируем кнопку
  button.init(this,ButtonOnClick);

  // инициализируем пункты меню
  size_t cnt = items.size();
  for(size_t i=0;i<cnt;i++)
    items[i]->init(this);
}
void LCDMenu::notifyMenuUpdated(AbstractLCDMenuItem* miUpd)
{
  AbstractLCDMenuItem* mi = items[selectedMenuItem];
  if(mi == miUpd)
    wantRedraw(); // пункт меню, который изменился - находится на экране, надо перерисовать его состояние
}
void LCDMenu::enterSubMenu() // переходим в подменю по клику на кнопке
{
  resetTimer(); // сбрасываем таймер ничегонеделания
  backlight(); // включаем подсветку экрана
  
  AbstractLCDMenuItem* mi = items[selectedMenuItem];
  mi->OnButtonClicked(this); // говорим, что кликнута кнопка, затем устанавливаем фокус на окне.
  // если фокуса раньше не было - окно поймёт, что это первый клик на кнопке,
  // в противном случае - повторный.
  mi->setFocus(); // устанавливаем фокус на текущем экране, после этого все позиции на этом экране 
  // могут листаться энкодером, помимо кнопки
}
void LCDMenu::update(uint16_t dt)
{
  // обновляем кнопку
  button.update();

  // обновляем все экраны
  size_t cnt = items.size();
  for(size_t i=0;i<cnt;i++)
    items[i]->update(dt,this);
  
  // обновляем внутренний таймер ничегонеделания
  gotLastCommmandAt += dt;

  // обновляем таймер выключения подсветки
  if(backlightCheckingEnabled)
  {
    backlightCounter += dt;
    if(backlightCounter >= SCREEN_BACKLIGHT_OFF_DELAY) // надо выключить подсветку
      backlight(false);
  }
  
  if(gotLastCommmandAt >= MENU_RESET_DELAY)
  {
     // ничего не делали какое-то время, надо перейти в экран ожидания
     resetTimer(); // сбрасываем таймер простоя

     //Убираем захват фокуса предыдущим выбранным пунктом меню
     items[selectedMenuItem]->setFocus(false); // сбрасываем фокус с окна
     
     if(selectedMenuItem != 0) // если до этого был выбран не первый пункт меню - просим перерисоваться
      wantRedraw();
      
     selectedMenuItem = 0; // выбираем первый пункт меню
     backlightCheckingEnabled = true; // включаем таймер выключения досветки

  } // if
}
void LCDMenu::draw()
{
if(!needRedraw || !backlightIsOn) // не надо ничего перерисовывать
  return;

#ifdef LCD_DEBUG
Serial.print("LCDMenu::draw() - ");
unsigned long m = millis();
#endif

#define LCD_YIELD yield()
    
 size_t sz = items.size();
 AbstractLCDMenuItem* selItem = items[selectedMenuItem];
 const char* capt = selItem->GetCaption();
 
 firstPage();  
  do 
  {
   LCD_YIELD;
    // рисуем бокс
    drawFrame(0,MENU_BITMAP_SIZE-1,FRAME_WIDTH,FRAME_HEIGHT+1);
    
    // рисуем пункты меню верхнего уровня
    for(size_t i=0;i<sz;i++)
    {
      LCD_YIELD;
      drawXBMP( i*MENU_BITMAP_SIZE, 0, MENU_BITMAP_SIZE, MENU_BITMAP_SIZE, items[i]->GetIcon());
    }
    
    // теперь рисуем фрейм вокруг выбранного пункта меню
    drawFrame(selectedMenuItem*MENU_BITMAP_SIZE,0,MENU_BITMAP_SIZE,MENU_BITMAP_SIZE);
    LCD_YIELD;
    
    // теперь рисуем прямоугольник с заливкой внизу от контента
    drawBox(0,FRAME_HEIGHT + MENU_BITMAP_SIZE - (HINT_FONT_HEIGHT + HINT_FONT_BOX_PADDING),FRAME_WIDTH,HINT_FONT_HEIGHT + HINT_FONT_BOX_PADDING);
    LCD_YIELD;
    
    setColorIndex(0);

    // теперь убираем линию под выбранным пунктом меню
    drawLine(selectedMenuItem*MENU_BITMAP_SIZE+1,MENU_BITMAP_SIZE-1,selectedMenuItem*MENU_BITMAP_SIZE+MENU_BITMAP_SIZE-2,MENU_BITMAP_SIZE-1);
    LCD_YIELD;
    
    // теперь рисуем название пункта меню
    
    #ifdef SCREEN_HINT_AT_RIGHT
          
    // рисуем подсказку, выровненную по правому краю
    u8g_uint_t strW = getStrWidth(capt);    
    drawStr(FRAME_WIDTH - HINT_FONT_BOX_PADDING - strW,FRAME_HEIGHT + MENU_BITMAP_SIZE - HINT_FONT_BOX_PADDING,capt);
    
    #else
    // рисуем подсказку, выровненную по левому краю
    drawStr(HINT_FONT_BOX_PADDING,FRAME_HEIGHT + MENU_BITMAP_SIZE - HINT_FONT_BOX_PADDING,capt);
    
    #endif

    setColorIndex(1);

    // теперь просим пункт меню отрисоваться на экране
    selItem->draw(this);
    LCD_YIELD;  
  
  
  } while( nextPage() ); 

   needRedraw = false; // отрисовали всё, что нам надо - и сбросили флаг необходимости отрисовки
#ifdef LCD_DEBUG
   Serial.println(millis() - m);
#endif   
}

#endif

