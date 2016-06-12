#ifndef LCD_MENU_H
#define LCD_MENU_H

#include <Arduino.h>
#include "Globals.h"

#ifdef USE_LCD_MODULE

#include "U8glib.h"
#include "rus6x10.h" // подключаем нужный шрифт
#include "TinyVector.h"
#include "PushButton.h"
#include "ModuleController.h"

#define FRAME_WIDTH 128 // ширина фрейма с контентом
#define MENU_BITMAP_SIZE 16 // размер иконки в пункте меню
#define FRAME_HEIGHT (64 - MENU_BITMAP_SIZE) // высота фрейма с контентом
#define SELECTED_FONT rus6x10 // выбранный шрифт
#define HINT_FONT_HEIGHT 8 // высота шрифта подсказки, в пикселах
#define HINT_FONT_BOX_PADDING 1 // сколько пространства оставлять вокруг шрифта в боксе подсказки
#define CONTENT_PADDING 4 // сколько пикселей с каждой стороны бокса отдавать под padding
#define SCREEN_MAX_TEMP_VALUE 50 // какая температура максимально может быть выставлена на экране (0-127)? 

const unsigned char RADIO_CHECK_ICON[] U8G_PROGMEM = {
0x00,0x00,
0x00,0x00,
0xC0,0x03,
0xF0,0x0F,
0x38,0x1C,
0x18,0x18,
0x8C,0x31,
0xCC,0x33,
0xCC,0x33,
0x8C,0x31,
0x18,0x18,
0x38,0x1C,
0xF0,0x0F,
0xC0,0x03,
0x00,0x00,
0x00,0x00
};

const unsigned char RADIO_UNCHECK_ICON[] U8G_PROGMEM = {
0x00,0x00,
0x00,0x00,
0xC0,0x03,
0xF0,0x0F,
0x38,0x1C,
0x18,0x18,
0x0C,0x30,
0x0C,0x30,
0x0C,0x30,
0x0C,0x30,
0x18,0x18,
0x38,0x1C,
0xF0,0x0F,
0xC0,0x03,
0x00,0x00,
0x00,0x00
};


const unsigned char CHECK_ICON[] U8G_PROGMEM = {
0x00,0x00,
0x00,0x40,
0xFC,0x3F,
0xFC,0x3F,
0x0C,0x38,
0x0C,0x34,
0x0C,0x32,
0x2C,0x33,
0xFC,0x31,
0xCC,0x30,
0xCC,0x30,
0x0C,0x30,
0xFC,0x3F,
0xFC,0x3F,
0x00,0x00,
0x00,0x00
};

const unsigned char UNCHECK_ICON[] U8G_PROGMEM = {
0x00,0x00,
0x00,0x00,
0xFC,0x3F,
0xFC,0x3F,
0x0C,0x30,
0x0C,0x30,
0x0C,0x30,
0x0C,0x30,
0x0C,0x30,
0x0C,0x30,
0x0C,0x30,
0x0C,0x30,
0xFC,0x3F,
0xFC,0x3F,
0x00,0x00,
0x00,0x00
};

const unsigned char MONITOR_ICON[] U8G_PROGMEM = {
0x00,0x00,
0x00,0x00,
0xC0,0x01,
0xC0,0x03,
0xE0,0x03,
0xC0,0x01,
0x00,0x00,
0xE0,0x03,
0xC0,0x03,
0xC0,0x03,
0xC0,0x03,
0xC0,0x03,
0xC0,0x03,
0x00,0x00,
0x00,0x00,
0x00,0x00
 };

#ifdef USE_TEMP_SENSORS
const unsigned char WINDOW_ICON[] U8G_PROGMEM = {
0x00,0x00,
0x00,0x00,
0xFC,0x3F,
0x04,0x22,
0x04,0x22,
0x04,0x22,
0x04,0x3E,
0x04,0x22,
0x04,0x22,
0x04,0x22,
0x04,0x22,
0x04,0x22,
0x04,0x22,
0xFC,0x3F,
0x00,0x00,
0x00,0x00
 };
#endif
#ifdef USE_WATERING_MODULE
const unsigned char WATERING_ICON[] U8G_PROGMEM = {
0x00,0x00,
0x00,0x00,
0x80,0x01,
0x80,0x01,
0x80,0x01,
0xC0,0x03,
0xE0,0x06,
0xE0,0x05,
0xF0,0x0D,
0xF0,0x0B,
0xF0,0x0D,
0xF0,0x0D,
0xE0,0x07,
0xC0,0x03,
0x00,0x00,
0x00,0x00
 };
#endif

#ifdef USE_LUMINOSITY_MODULE
const unsigned char LUMINOSITY_ICON[] U8G_PROGMEM = {
0x00,0x00,
0x00,0x00,
0xE0,0x03,
0x10,0x04,
0x08,0x08,
0x08,0x08,
0x48,0x09,
0x88,0x08,
0x90,0x04,
0xA0,0x02,
0xA0,0x02,
0xE0,0x03,
0xE0,0x03,
0xC0,0x01,
0x00,0x00,
0x00,0x00
 };
#endif

const unsigned char SETTINGS_ICON[] U8G_PROGMEM = {
0x00,0x00,
0x00,0x00,
0xFC,0x3F,
0x14,0x28,
0x14,0x28,
0x14,0x28,
0x14,0x28,
0x14,0x28,
0xF4,0x2F,
0xE4,0x27,
0x24,0x27,
0x24,0x27,
0x24,0x27,
0xF8,0x3F,
0x00,0x00,
0x00,0x00
 };

 typedef struct
 {
  uint8_t sensorType;
  uint8_t sensorIndex;
  const char* moduleName;
  const char* displayName;
  
 } WaitScreenInfo; // структура для хранения информации, которую необходимо показывать на экране ожидания

typedef U8GLIB_ST7920_128X64_1X DrawContext; // приводим типы для удобства
class LCDMenu; // forward declaration

 class AbstractLCDMenuItem // абстрактный класс для пунктов меню
 {
  protected:
  
    const unsigned char* icon; // иконка, которая отображается для меню верхнего уровня
    const char* caption; // текст пункта меню

    bool focused; // флаг наличия фокуса на окне
    bool needToDrawCursor;
    int8_t cursorPos;
    uint8_t itemsCount; // сколько иконок в меню экрана

    LCDMenu* parentMenu;

    
    
  public:
    const unsigned char* GetIcon() {return icon;}
    const char* GetCaption() {return caption;}

    AbstractLCDMenuItem(const unsigned char* i, const char* c);

    virtual void init(LCDMenu* parent); // инициализируем, что нужно

    virtual void draw(DrawContext* dc) = 0; // отрисовывает контент пункта меню
    virtual void update(uint16_t dt, LCDMenu* menu) = 0; // обновляет внутреннее состояние
    
    virtual void setFocus(bool f=true); // устанавливает фокус ввода на пункте меню
    virtual bool hasFocus() { return focused; } // проверяет, есть ли фокус на пункте меню

    virtual bool OnEncoderPositionChanged(int dir, LCDMenu* menu) // вызывается при смене позиции энкодера
    {
      UNUSED(dir);
      UNUSED(menu);
      // если мы отработали новое положение энкодера - возвращаем true.
      // если хотим, чтобы новое положение энкодера было обработано по умолчанию - возвращаем false.
      return false;
    }

    virtual void OnButtonClicked(LCDMenu* menu); // обрабатываем нажатие кнопки в меню

    
 };

class IdlePageMenuItem : public AbstractLCDMenuItem // класс экрана ожидания
{
  private:

    unsigned long rotationTimer;
    int8_t currentSensorIndex;
    String sensorData; // данные с текущего сенсора
    const char* displayString; // что писать на экране для расшифровки показаний

    void RequestSensorData(const WaitScreenInfo& info); // получаем данные с датчика
    void SelectNextSensor(); // выбираем следующий сенсор

   public:
    IdlePageMenuItem();
    virtual void draw(DrawContext* dc);
    virtual void init(LCDMenu* parent);
    virtual bool hasFocus()
    {
      return false; // меню ожидания не имеет фокуса ввода
    }
    virtual void update(uint16_t dt, LCDMenu* menu);
    virtual void OnButtonClicked(LCDMenu* menu);
    
};
#ifdef USE_TEMP_SENSORS
class WindowMenuItem : public AbstractLCDMenuItem // класс меню управления окнами
{
  private:
    bool isWindowsOpen;
    bool isWindowsAutoMode;
  
   public:
    WindowMenuItem();
    virtual void draw(DrawContext* dc);
    virtual void init(LCDMenu* parent);
    virtual bool OnEncoderPositionChanged(int dir, LCDMenu* menu);
    virtual void update(uint16_t dt, LCDMenu* menu);

  
};
#endif
#ifdef USE_WATERING_MODULE
class WateringMenuItem : public AbstractLCDMenuItem // класс меню управления поливом
{
  private:
  
    bool isWateringOn;
    bool isWateringAutoMode;

   public:
    WateringMenuItem();
    virtual void draw(DrawContext* dc);
    virtual void init(LCDMenu* parent);
    virtual bool OnEncoderPositionChanged(int dir, LCDMenu* menu);
    virtual void update(uint16_t dt, LCDMenu* menu);
  
};
#endif

#ifdef USE_LUMINOSITY_MODULE
class LuminosityMenuItem : public AbstractLCDMenuItem // класс меню управления досветкой
{
  private:
  
    bool isLightOn;
    bool isLightAutoMode;

   public:
    LuminosityMenuItem();
    virtual void draw(DrawContext* dc);
    virtual void init(LCDMenu* parent);
    virtual bool OnEncoderPositionChanged(int dir, LCDMenu* menu);
    virtual void update(uint16_t dt, LCDMenu* menu);
  
}; 
#endif

class SettingsMenuItem : public AbstractLCDMenuItem // класс меню управления настройками
{
  private:

    uint8_t openTemp;
    uint8_t closeTemp;
  
   public:
    SettingsMenuItem();
    virtual void draw(DrawContext* dc);
    virtual void init(LCDMenu* parent);
    virtual bool OnEncoderPositionChanged(int dir, LCDMenu* menu);
    virtual void update(uint16_t dt, LCDMenu* menu);
    virtual void setFocus(bool f=true);
  
}; 

 typedef Vector<AbstractLCDMenuItem*> MenuItems;
 
class LCDMenu : public DrawContext
{
  public:
    LCDMenu(uint8_t sck, uint8_t mosi, uint8_t cs);
    ~LCDMenu();

    void init(); // инициализируем

    void draw(); // рисует меню
    void update(uint16_t dt); // обновляем меню
    void selectNextMenu(int encoderDirection); // выбирает следующее меню в списке
    void enterSubMenu(); // входим внутрь выбранного экрана

  protected:

    // даём этим классам доступ к защищённым методам, которые не надо вытаскивать в public, во избежание лишней путаницы
    friend class AbstractLCDMenuItem;
    friend class IdlePageMenuItem;
    friend class SettingsMenuItem;
    
#ifdef USE_TEMP_SENSORS
    friend class WindowMenuItem;
#endif    
#ifdef USE_WATERING_MODULE
    friend class WateringMenuItem;
#endif    
#ifdef USE_LUMINOSITY_MODULE
    friend class LuminosityMenuItem;
#endif    

   void wantRedraw(); // ставим флаг необходимости перерисовки 
   void resetTimer(); // сбрасываем таймер перехода в меню ожидания
    void notifyMenuUpdated(AbstractLCDMenuItem* mi); // пункт меню уведомляет, что он изменил своё внутреннее состояние

   private:

   bool backlightIsOn;
   void backlight(bool en=true); // управление подсветкой
   uint16_t backlightCounter; // выключаем досветку, когда ничего не делается и накопили в этой переменной нужный интервал
   bool backlightCheckingEnabled;

   size_t selectedMenuItem; // какой пункт меню выбран?
   MenuItems items;
   bool needRedraw; // флаг, что нам надо перерисовать экран

   uint16_t gotLastCommmandAt; // время с момента получения последней команды
};

#endif

#endif
