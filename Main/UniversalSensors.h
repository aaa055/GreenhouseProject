#ifndef _UNUVERSAL_SENSORS_H
#define _UNUVERSAL_SENSORS_H
#include <Arduino.h>
#include "ModuleController.h"

/*
структура скратчпада датчика:
controller_id - 1 байт, идентификатор контроллера, к которому привязан датчик
rf_id - 1 байт, уникальный идентификатор датчика
battery_status - 1 байт, статус заряда батареи
calibration_factor1 - 1 байт, фактор калибровки
calibration_factor2 - 1 байт, фактор калибровки
reserved - 6 байт, резерв
index1 - 1 байт, индекс первого датчика в системе
type1 - 1 байт, тип первого датчика
data1 - 4 байта, данные первого датчика
index2 - 1 байт
type2 - 1 байт
data2 - 4 байта
index3 - 1 байт
type3 - 1 байт
data3 - 4 байта
crc8 - 1 байт, контрольная сумма скратчпада

*/

// итого, на скратчпад - имеем 30 байт, что вписывается в максимальную длину пакета для радиопередачи в 32 байта.
#define UNI_SCRATCH_SIZE 30 // размер скратчпада к вычитке
#define CONTROLLER_ID_IDX 0 // индекс ID контроллера в скратчпаде
#define RF_ID_IDX 1 // индекс уникального идентификатора модуля
#define DATA_START_IDX 11 // с какого индекса начинаются данные с датчиков в скратчпаде
#define NO_SENSOR_REGISTERED 0xFF // значение, которое сообщает нам, что датчик в системе не зарегистрирован (например, в прошивке нет модуля, в который помещать данные)
#define UNI_SENSORS_COUNT 3 // сколько датчиков максимум может быть внутри одного универсального

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


typedef struct
{
 OneState* State1; // первое внутреннее состояние в контроллере
 OneState* State2; // второе внутреннее состояние в контроллере  
 
} UniSensorState; // состояние для датчика, максимум два (например, для влажности надо ещё и температуру тянуть)

// класс поддержки регистрации универсальных датчиков в системе
 class UniRegDispatcher 
 {
  public:
    UniRegDispatcher();

    void Setup(ModuleController* controller); // настраивает диспетчер регистрации перед работой

    // регистрирует датчик, возвращает состояние для датчика в системе и индекс, который датчик должен запомнить у себя
    bool RegisterSensor(UniSensorType type, UniSensorState& resultStates, uint8_t& assignedIndex);

    // возвращает состояния для ранее зарегистрированного датчика
    bool GetRegisteredStates(UniSensorType type, uint8_t sensorIndex, UniSensorState& resultStates);

    // возвращает кол-во жёстко прописанных в прошивке датчиков того или иного типа
    uint8_t GetHardCodedSensorsCount(UniSensorType type); 

    uint8_t GetControllerID(); // возвращает уникальный ID контроллера

    void SaveState(); // сохраняет текущее состояние


 private:

    void ReadState(); // читает последнее запомненное состояние
    void RestoreState(); // восстанавливает последнее запомненное состояние


    ModuleController* mainController; // главный контроллер

    // модули разного типа, для быстрого доступа к ним
    AbstractModule* temperatureModule; // модуль температуры
    AbstractModule* humidityModule; // модуль влажности
    AbstractModule* luminosityModule; // модуль освещенности
    AbstractModule* soilMoistureModule; // модуль влажности почвы

    // жёстко указанные в прошивке датчики
    uint8_t hardCodedTemperatureCount;
    uint8_t hardCodedHumidityCount;
    uint8_t hardCodedLuminosityCount;
    uint8_t hardCodedSoilMoistureCount;


    // последние выданные индексы для универсальных датчиков
    uint8_t currentTemperatureCount;
    uint8_t currentHumidityCount;
    uint8_t currentLuminosityCount;
    uint8_t currentSoilMoistureCount;
  
 };

extern UniRegDispatcher UniDispatcher; // экземпляр класса диспетчера, доступный отовсюда

// абстрактный класс обслуживания универсального сенсора
class AbstractUniSensor 
{
  public:
  
    AbstractUniSensor(uint8_t* scratchAddress);

    uint8_t GetRegistrationID(); // возвращает сохранённый ID контроллера в скратчпаде
    uint8_t GetID(); // возвращает уникальный индекс датчика

    bool GetSensorInfo(uint8_t scratchIndex,uint8_t& sensorType,uint8_t& sensorIndex); // возвращает информацию о сенсоре у датчика

    virtual void Update(uint16_t dt) = 0; // функция обновления внутреннего состояния

  protected:

    void SetPin(uint8_t pinNumber); // привязывает датчик к пину
    bool SetRFState(bool enabled); // включает или выключает радиопередатчик, если это необходимо
    
    bool ReadScratchpad(); // читает скратчпад
    bool WriteScratchpad(); // пишет скратчпад

    bool IsRegistered(); // проверяет, зарегистрирован ли сенсор у нас?
    
    bool SensorSetup(); // настраивает датчик по ранее сохранённым данным
    bool SensorRegister(bool rfTransmitterEnabled); // регистрирует датчик в системе

    void ReBindSensor(uint8_t scratchIndex,uint8_t newSensorIndex); // переназначает привязки для сенсоров в скратчпаде
    
    void UpdateData(bool isSensorOnline); // обновляет данные в контроллере, если isSensorOnline == false, то вместо актуальных данных показывает <нет данных>
    
  
  private:
  
  

    // функции обновления показаний в контроллере
    void UpdateStateData(UniSensorState* state,uint8_t idx, bool isSensorOnline);
    void UpdateOneState(OneState* os, uint8_t* data, bool isSensorOnline);

    UniSensorState States[UNI_SENSORS_COUNT]; // состояния датчиков в системе
    uint8_t pin; // пин, на котором висит сенсор

    uint8_t* scratchpadAddress; // адрес скратчпада, куда мы будем вычитывать слепок состояния датчика
    bool rfEnabled; // флаг состояния радиопередатчика
    bool isScratchpadReaded; // флаг, что мы прочитали скратчпад
    
};

 // класс датчика, постоянно висящего на пине
 class UniPermanentSensor : public AbstractUniSensor
 {
  public:
    UniPermanentSensor(uint8_t pinNumber);

    void Update(uint16_t dt);

  private:
  
    unsigned long timer; // таймер обновления
    bool isSensorInited; // флаг инициализации
    bool canWork; // флаг, что мы можем работать
  
 };

 // класс поддержки регистрации датчиков на выделенном пине.
 // может работать в автоматическом режиме, в Update регистрируя
 // датчики. В ручном режиме можно переназначить индексы
 // любому датчику, присоединённому к линии.
 class UniRegistrationLine : public AbstractUniSensor
 {
  public:
    UniRegistrationLine(uint8_t pinNumber, bool autoRegistrationMode);

    void SetAutoRegistrationMode(bool autoreg); // устанавливает режим авторегистрации

    void Update(uint16_t dt);

    bool IsSensorPresent(); // читает скратчпад и если сенсор есть на линии регистрации - возвращает true
    void SetSensorIndex(uint8_t scrathIndex, uint8_t sensorIndex); // назначает новый внутренний индекс сенсору в скратчпаде
    
    void SaveConfiguration(); // сохраняет новую конфигурацию в датчик

  private:

    int16_t lastCheckedSensor; // последний проверенный сенсор
    unsigned long timer;
    bool autoRegistrationMode;

    uint8_t rebindedSensorIndicies[UNI_SENSORS_COUNT]; // переназначенные индексы сенсоров
  
 }; 

#endif
