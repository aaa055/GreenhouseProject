#ifndef _UNUVERSAL_SENSORS_H
#define _UNUVERSAL_SENSORS_H

#include <Arduino.h>
#include "ModuleController.h"

// настроечные параметры
/*
структура скратчпада датчика:
controller_id - 1 байт
rf_id - 1 байт
battery_status - 1 байт
calibration_factor1 - 1 байт
calibration_factor2 - 1 байт
reserved - 6 байт
index1 - 1 байт
type1 - 1 байт
data1 - 4 байта
index2 - 1 байт
type2 - 1 байт
data2 - 4 байта
index3 - 1 байт
type3 - 1 байт
data3 - 4 байта
crc8 - 1 байт

*/
// итого, на скратчпад - имеем 30 байт
#define UNI_SCRATCH_SIZE 30 // размер скратчпада к вычитке
#define CONTROLLER_ID_IDX 0 // индекс ID контроллера в скратчпаде
#define RF_ID_IDX 1 // индекс уникального идентификатора модуля
#define DATA_START_IDX 11 // с какого индекса начинаются данные с датчиков в скратчпаде
#define NO_SENSOR_REGISTERED 0xFF // значение, которое сообщает нам, что датчик в системе не зарегистрирован (например, в прошивке нет модуля, в который помещать данные)


// команды
#define UNI_START_MEASURE 0x44 // запустить конвертацию
#define UNI_READ_SCRATCHPAD 0x4E // прочитать скратчпад
#define UNI_WRITE_SCRATCHPAD 0xBE // записать скратчпад

typedef enum
{
  uniNone = 0, // ничего нет
  uniTemp = 1, // только температура, значащие - два байта
  uniHumidity = 2, // влажность (первые два байта), температура (вторые два байта) 
  uniLuminosity = 3, // освещённость, 4 байта
  uniSoilMoisture = 4 // влажность почвы (два байта)
  
} UniSensorType; // тип датчика



class UniSensor1Wire // класс поддержки универсального сенсора по шине 1-Wire
{
  public:
    UniSensor1Wire();
    
    void begin(unsigned long queryInterval, bool permanentConnection, uint8_t pin, ModuleController* controller); // начинаем работу, следим за переданным пином
    uint8_t getPin() { return _pin; } // возвращаем номер пина, с которым работаем
    
    void update(uint16_t dt); // обновляем внутреннее состояние
    
  private:

    bool present(); // определяет, есть ли датчик на линии
    bool readScratchpad(); // читает скратчпад датчика
    void writeScratchpad(); // пишет настроенный скратчпад датчика
    
    bool needToConfigure(); // проверяем, надо ли сконфигурировать датчик
    void configure(bool disableRF); // конфигурируем его
    void updateData(); // обновляем данные с датчика в контроллере
    void offLine(); // заполняем скратчпад данными, говорящими, что датчика нет на линии

    bool isPermanentConnection; // флаг, что датчик работает как проводной
    uint8_t _pin; // пин, на котором висит или конфигурируется сенсор
    int16_t lastCheckedSensor; // ID последнего проверенного датчика, нужно для работы в режиме прослушки линии, для регистрации датчиков
    
    uint8_t scratch[UNI_SCRATCH_SIZE]; // скратчпад

    ModuleController* mainController; // главный контроллер
    unsigned long timer; // таймер для периодического опроса
    unsigned long queryInterval; // через какое время опрашивать датчик
    
    bool bInited; // флаг, что мы проинициализировали модули
    
    // модули разного типа, для быстрого доступа к ним
    AbstractModule* temperatureModule; // модуль температуры
    AbstractModule* humidityModule; // модуль влажности
    AbstractModule* luminosityModule; // модуль освещенности
    AbstractModule* soilMoistureModule; // модуль влажности почвы
    
    uint8_t registerTemperatureSensor(); // регистрируем датчик температуры
    uint8_t registerHumiditySensor(); // регистрируем датчик влажности
    uint8_t registerLuminositySensor(); // регистрируем датчик освещенности
    uint8_t registerSoilMoistureSensor(); // регистрируем датчик влажности почвы

};

#endif
