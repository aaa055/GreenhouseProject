#ifndef _SETTINGS_H
#define _SETTINGS_H

#include <Arduino.h>

// класс настроек, которые сохраняются и читаются в/из EEPROM
// здесь будут всякие настройки, типа уставок срабатывания и пр. лабуды

enum WateringOption // какая опция управления поливом выбрана
{
  wateringOFF = 0, // автоматическое управление поливом выключено
  wateringWeekDays = 1 // управление поливом по дням недели
  
};

class GlobalSettings
{
  private:


  uint8_t tempOpen; // температура открытия
  uint8_t tempClose; // температура закрытия
  unsigned long openInterval; // интервал для открытия окон

  String smsPhoneNumber; // номер телефона для управления по SMS

  WateringOption wateringOption; // какая опция управления выбрана?
  uint8_t wateringWeekDays; // в какие дни недели управляем поливом?
  uint16_t wateringTime; // время полива
  uint8_t startWateringTime; // время начала полива
 
  public:
    GlobalSettings();

    void Load();
    void Save();
    void ResetToDefault();

    WateringOption GetWateringOption() {return wateringOption; }
    void SetWateringOption(WateringOption val) {wateringOption = val; }

     uint8_t GetWateringWeekDays() {return wateringWeekDays; }
     void SetWateringWeekDays(uint8_t val) {wateringWeekDays = val;}

     uint16_t GetWateringTime() {return wateringTime;}
     void SetWateringTime(uint16_t val) {wateringTime = val;}

     uint8_t GetStartWateringTime() {return startWateringTime;}
     void SetStartWateringTime(uint8_t val) {startWateringTime = val;}

    uint8_t GetOpenTemp() {return tempOpen;}
    void SetOpenTemp(uint8_t val) {tempOpen = val;}

    uint8_t GetCloseTemp() {return tempClose;}
    void SetCloseTemp(uint8_t val) {tempClose = val;}

    unsigned long GetOpenInterval() {return openInterval;}
    void SetOpenInterval(unsigned long val) {openInterval = val;}

    String GetSmsPhoneNumber() {return smsPhoneNumber; }
    void SetSmsPhoneNumber(const String& v) {smsPhoneNumber = v;}

    
    
};

#endif
