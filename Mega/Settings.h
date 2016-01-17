#ifndef _SETTINGS_H
#define _SETTINGS_H

#include <Arduino.h>

// класс настроек, которые сохраняются и читаются в/из EEPROM
// здесь будут всякие настройки, типа уставок срабатывания и пр. лабуды
class GlobalSettings
{
  private:

  uint8_t tempOpen; // температура открытия
  uint8_t tempClose; // температура закрытия
  unsigned long openInterval; // интервал для открытия окон

  public:
    GlobalSettings();

    void Load();
    void Save();
    void ResetToDefault();

    uint8_t GetOpenTemp() {return tempOpen;}
    void SetOpenTemp(uint8_t val) {tempOpen = val;}

    uint8_t GetCloseTemp() {return tempClose;}
    void SetCloseTemp(uint8_t val) {tempClose = val;}

    unsigned long GetOpenInterval() {return openInterval;}
    void SetOpenInterval(unsigned long val) {openInterval = val;}
    
};

#endif
