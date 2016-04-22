#ifndef _SETTINGS_H
#define _SETTINGS_H

#include <Arduino.h>
#include "Globals.h"

// класс настроек, которые сохраняются и читаются в/из EEPROM
// здесь будут всякие настройки, типа уставок срабатывания и пр. лабуды

enum WateringOption // какая опция управления поливом выбрана
{
  wateringOFF = 0, // автоматическое управление поливом выключено
  wateringWeekDays = 1, // управление поливом по дням недели, все каналы одновременно
  wateringSeparateChannels = 2 // раздельное управление каналами по дням недели  
};

typedef struct
{
  uint8_t wateringWeekDays; // в какие дни недели управляем поливом на этом канале?
  uint16_t wateringTime; // время полива на этом канале
  uint8_t startWateringTime; // время начала полива для этого канала
  
} WateringChannelOptions; // настройки для отдельного канала полива

// функция, которая вызывается при чтении/записи установок дельт - чтобы не хранить их в классе настроек.
// При чтении настроек класс настроек вызывает функцию OnDeltaRead, передавая прочитанные значения вовне.
// При записи настроек класс настроек вызывает функцию OnDeltaWrite.
typedef void (*DeltaReadWriteFunction)(uint8_t& sensorType, String& moduleName1,uint8_t& sensorIdx1, String& moduleName2, uint8_t& sensorIdx2);

// функция, которая вызывается при чтении/записи установок дельт. Класс настроек вызывает OnDeltaGetCount, чтобы получить кол-во записей, которые следует сохранить,
// и OnDeltaSetCount - чтобы сообщить подписчику - сколько записей он передаст в вызове OnDeltaRead.
typedef void (*DeltaCountFunction)(uint8_t& count);

class GlobalSettings
{
  private:

  uint8_t controllerID;

  uint8_t tempOpen; // температура открытия
  uint8_t tempClose; // температура закрытия
  unsigned long openInterval; // интервал для открытия окон

  String smsPhoneNumber; // номер телефона для управления по SMS

  uint8_t wateringOption; // какая опция управления выбрана?
  uint8_t wateringWeekDays; // в какие дни недели управляем поливом на всех каналах?
  uint16_t wateringTime; // время полива на всех каналах
  uint8_t startWateringTime; // время начала полива для всех каналов
  uint8_t turnOnPump; // включать ли определённый канал реле при включенном поливе на любом из каналов (может использоваться для насоса)?

  WateringChannelOptions wateringChannelsOptions[WATER_RELAYS_COUNT]; // настройки каналов полива

  // настройки Wi-Fi
  uint8_t wifiState; // первый бит установлен - коннектиться к домашнему роутеру
  String routerID; // название точки доступа домашнего роутера
  String routerPassword; // пароль к точке доступа домашнего роутера
  String stationID; // название точки доступа у модуля ESP
  String stationPassword; // пароль к точке доступа модуля ESP 
 
  public:
    GlobalSettings();

    void Load();
    void Save();
    void ResetToDefault();

    uint8_t GetControllerID() {return controllerID;}
    void SetControllerID(uint8_t val);

    void ReadDeltaSettings(DeltaCountFunction OnDeltaSetCount, DeltaReadWriteFunction OnDeltaRead); // читаем настройки дельт 
    void WriteDeltaSettings(DeltaCountFunction OnDeltaGetCount, DeltaReadWriteFunction OnDeltaWrite); // пишем настройки дельт 

    uint8_t GetWateringOption() {return wateringOption; }
    void SetWateringOption(uint8_t val) {wateringOption = val; }

    uint8_t GetChannelWateringWeekDays(uint8_t idx) {return wateringChannelsOptions[idx].wateringWeekDays;};
    void SetChannelWateringWeekDays(uint8_t idx, uint8_t val) {wateringChannelsOptions[idx].wateringWeekDays = val;};

     uint16_t GetChannelWateringTime(uint8_t idx) {return wateringChannelsOptions[idx].wateringTime;}
     void SetChannelWateringTime(uint8_t idx,uint16_t val) {wateringChannelsOptions[idx].wateringTime = val;}

     uint8_t GetChannelStartWateringTime(uint8_t idx) {return wateringChannelsOptions[idx].startWateringTime;}
     void SetChannelStartWateringTime(uint8_t idx,uint8_t val) {wateringChannelsOptions[idx].startWateringTime = val;}

     uint8_t GetWateringWeekDays() {return wateringWeekDays; }
     void SetWateringWeekDays(uint8_t val) {wateringWeekDays = val;}

     uint16_t GetWateringTime() {return wateringTime;}
     void SetWateringTime(uint16_t val) {wateringTime = val;}

     uint8_t GetStartWateringTime() {return startWateringTime;}
     void SetStartWateringTime(uint8_t val) {startWateringTime = val;}

    uint8_t GetTurnOnPump() {return turnOnPump;}
    void SetTurnOnPump(uint8_t val) {turnOnPump = val;}

    uint8_t GetOpenTemp() {return tempOpen;}
    void SetOpenTemp(uint8_t val) {tempOpen = val;}

    uint8_t GetCloseTemp() {return tempClose;}
    void SetCloseTemp(uint8_t val) {tempClose = val;}

    unsigned long GetOpenInterval() {return openInterval;}
    void SetOpenInterval(unsigned long val) {openInterval = val;}

    String GetSmsPhoneNumber() const {return smsPhoneNumber; }
    void SetSmsPhoneNumber(const String& v) {smsPhoneNumber = v;}

    uint8_t GetWiFiState() {return wifiState;}
    void SetWiFiState(uint8_t st) {wifiState = st;}
    
    String GetRouterID() const {return routerID;}
    void SetRouterID(const String& val) {routerID = val;}
    String GetRouterPassword() const {return routerPassword;}
    void SetRouterPassword(const String& val) {routerPassword = val;}
    
    String GetStationID() const {return stationID;}
    void SetStationID(const String& val) {stationID = val;}
    String GetStationPassword() const {return stationPassword;}
    void SetStationPassword(const String& val) {stationPassword = val;}


    
    
};

#endif
