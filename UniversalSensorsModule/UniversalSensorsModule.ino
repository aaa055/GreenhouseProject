/*
Прошивка для универсального модуля, предназначена для подключения
любого типа поддерживаемых датчиков и передачи с них показаний по шине 1-Wire.
*/
//----------------------------------------------------------------------------------------------------------------
#include <avr/io.h>
#include <avr/interrupt.h>
#include <OneWire.h>
#include "BH1750.h"
#include "UniGlobals.h"
#include "Si7021Support.h"
//----------------------------------------------------------------------------------------------------------------
typedef enum
{
  mstNone, // ничего нету
  mstDS18B20, // температурный DS18B20
  mstBH1750, // цифровой освещённости BH1750
  mstSi7021 // цифровой влажности Si7021
  
} ModuleSensorType; // тип датчика, кодключенного к модулю
//----------------------------------------------------------------------------------------------------------------
typedef struct
{
  byte Type; // тип датчика
  byte Pin; // пин, на котором висит датчик
  
} SensorSettings; // настройки датчиков, подключённых к модулю
//----------------------------------------------------------------------------------------------------------------
// настройки
//----------------------------------------------------------------------------------------------------------------
#define RF_MODULE_ID 202 // уникальный ID модуля
#define ROM_ADDRESS (void*) 4 // по какому адресу у нас настройки?
//----------------------------------------------------------------------------------------------------------------
// настройки датчиков для модуля, МЕНЯТЬ ЗДЕСЬ!
const SensorSettings Sensors[3] = {

{mstBH1750,BH1750Address1}, // датчик освещённости BH1750 на шине I2C
{mstNone,0}, // ничего нету
{mstNone,0} // ничего нету

};
//----------------------------------------------------------------------------------------------------------------
//Синонимы регистров управления прерываниями. Разные для ATTINY и ATMEGA
//----------------------------------------------------------------------------------------------------------------
#define GIMSK EIMSK
#define GIFR EIFR
//----------------------------------------------------------------------------------------------------------------
//Порт 1-wire  вывод PD2 он же PIN2 Ардуино
//----------------------------------------------------------------------------------------------------------------
#define OW_PORT PORTD //1 Wire Port
#define OW_PIN PIND //1 Wire Pin as number
#define OW_PORTN (1<<PIND2)  //Pin as bit in registers
#define OW_PINN (1<<PIND2)
#define OW_DDR DDRD  //pin direction register
//----------------------------------------------------------------------------------------------------------------
inline void OneWireSetLow()
{
  //set 1-Wire line to low
  OW_DDR|=OW_PINN;
  OW_PORT&=~OW_PORTN;
}
//----------------------------------------------------------------------------------------------------------------
inline void OneWireSendAck()
{
  OW_DDR&=~OW_PINN;
}
//----------------------------------------------------------------------------------------------------------------
inline void OneWireEnableInterrupt()
{
  GIMSK|=(1<<INT0);GIFR|=(1<<INTF0);
}
//----------------------------------------------------------------------------------------------------------------
inline void OneWireDisableInterrupt()
{
  GIMSK&=~(1<<INT0);
}
//----------------------------------------------------------------------------------------------------------------
inline void OneWireInterruptAtRisingEdge()
{
  MCUCR=(1<<ISC01)|(1<<ISC00);
}
//----------------------------------------------------------------------------------------------------------------
inline void OneWireInterruptAtFallingEdge()
{
  MCUCR=(1<<ISC01);
}
//----------------------------------------------------------------------------------------------------------------
inline bool OneWireIsInterruptEnabled()
{
  return (GIMSK&(1<<INT0))==(1<<INT0); 
}
//----------------------------------------------------------------------------------------------------------------
//Timer Interrupt
//----------------------------------------------------------------------------------------------------------------
// Используем 16 разрядный таймер. Остальные не отдала Ардуино.
//Делитель - 64. То есть каждый тик таймера - 1/4 микросекунды
//----------------------------------------------------------------------------------------------------------------
inline void TimerEnable()
{
  TIMSK1  |= (1<<TOIE1); 
  TIFR1|=(1<<TOV1);
}
//----------------------------------------------------------------------------------------------------------------
inline void TimerDisable()
{
  TIMSK1  &= ~(1<<TOIE1);
}
//----------------------------------------------------------------------------------------------------------------
inline void TimerSetTimeout(uint8_t tmio)
{
  TCNT1 = ~tmio;
}
//----------------------------------------------------------------------------------------------------------------
#define OnTimer ISR(TIMER1_OVF_vect) // процедура обработки прерывания по таймеру
//----------------------------------------------------------------------------------------------------------------
inline void PreInit()
{
//Initializations of AVR
  CLKPR=(1<<CLKPCE);
  CLKPR=0;/*9.6Mhz*/
  TIMSK1=0;
  GIMSK=(1<<INT0);/*set direct GIMSK register*/
  TCCR1A = 0;
  TCCR1B = 0;
  TCCR1B = (1 << CS10) | (1 << CS11);
}
//----------------------------------------------------------------------------------------------------------------
//Интервалы для 1-Wire все в формате Микросекунды/4
//----------------------------------------------------------------------------------------------------------------
#define OWT_MIN_RESET  400/4   // Минимальная длительность Reset
#define OWT_RESET_PRESENCE 30/4  //Сколько ждать от фронта завершения Reset до начала Present
#define OWT_PRESENCE 200/4  // Длительность имульса Present
#define OWT_READLINE 30/4        //  ждать от спада мастера до момента чтения линии 1-Wire line
#define OWT_LOWTIME 15/4         // Сколько удерживать LOW
//----------------------------------------------------------------------------------------------------------------
typedef enum
{
  ptSensorsData = 1, // данные с датчиков
  ptNextionDisplay = 2, // дисплей Nextion, подключённый по шине 1-Wire
  ptExecuteModule = 3 // исполнительный модуль 
  
} PacketTypes;
//----------------------------------------------------------------------------------------------------------------
//Структура передаваемая мастеру и обратно
//----------------------------------------------------------------------------------------------------------------
struct sensor
{
    byte index;
    byte type;
    byte data[4];
    
};
//----------------------------------------------------------------------------------------------------------------
typedef struct
{
    byte packet_type;
    byte packet_subtype;
    byte config;
    byte controller_id;
    byte rf_id;
    byte battery_status;
    byte calibration_factor1;
    byte calibration_factor2;
    byte query_interval;
    byte reserved[2];

    sensor sensor1,sensor2,sensor3;

    byte crc8;
} t_scratchpad;
//----------------------------------------------------------------------------------------------------------------
typedef enum
{
  uniNone = 0, // ничего нет
  uniTemp = 1, // только температура, значащие - два байта
  uniHumidity = 2, // влажность (первые два байта), температура (вторые два байта) 
  uniLuminosity = 3, // освещённость, 4 байта
  uniSoilMoisture = 4, // влажность почвы (два байта)
  uniPH = 5 // показания pH (два байта)
  
} UniSensorType; // тип датчика
//----------------------------------------------------------------------------------------------------------------
//States / Modes
//----------------------------------------------------------------------------------------------------------------
typedef enum
{
  stateSleep,
  stateReset,
  statePresence,
  stateReadCommand,
  stateReadScratchpad,
  stateWriteScratchpad,
  stateCheckReset,
  stateMeasure
    
} MachineStates;
//----------------------------------------------------------------------------------------------------------------
const int sensePin = 2; // пин, на котором висит 1-Wire
t_scratchpad scratchpadS;
volatile char* scratchpad = (char *)&scratchpadS; //что бы обратиться к scratchpad как к линейному массиву

volatile uint8_t crcHolder; //CRC calculation

volatile uint8_t commandBuffer; //Входной буфер команды

volatile uint8_t bitPointer;  //pointer to current Bite
volatile uint8_t bytePointer; //pointer to current Byte

volatile MachineStates machineState; //state
volatile uint8_t workMode; //if 0 next bit that send the device is  0
volatile uint8_t actualBit; //current

volatile bool scratchpadReceivedFromMaster = false; // флаг, что мы получили данные с мастера
volatile bool needToMeasure = false; // флаг, что мы должны запустить конвертацию
volatile unsigned long sensorsUpdateTimer = 0; // таймер получения информации с датчиков и обновления данных в скратчпаде
volatile bool measureTimerEnabled = false; // флаг, что мы должны прочитать данные с датчиков после старта измерений
#define MEASURE_MIN_TIME 1000 // через сколько минимум можно читать с датчиков после запуска конвертации
unsigned long query_interval = MEASURE_MIN_TIME; // тут будет интервал опроса
//----------------------------------------------------------------------------------------------------------------
//Write a bit after next falling edge from master
//its for sending a zero as soon as possible
#define OWW_NO_WRITE 2
#define OWW_WRITE_0 0
//----------------------------------------------------------------------------------------------------------------
byte GetSensorType(const SensorSettings& sett)
{
  switch(sett.Type)
  {
    case mstNone:
      return uniNone;
    
    case mstDS18B20:
      return uniTemp;
      
    case mstBH1750:
      return uniLuminosity;

    case mstSi7021:
      return uniHumidity;
  }

  return uniNone;
}
//----------------------------------------------------------------------------------------------------------------
void SetDefaultValue(const SensorSettings& sett, byte* data)
{
  switch(sett.Type)
  {
    case mstNone:
      *data = 0xFF;
    break;
    
    case mstDS18B20:
      *data = NO_TEMPERATURE_DATA;
    break;
      
    case mstBH1750:
    {
    long lum = NO_LUMINOSITY_DATA;
    memcpy(data,&lum,sizeof(lum));
    }
    break;

    case mstSi7021:
    {
    *data = NO_TEMPERATURE_DATA;
    data++; data++;
    *data = NO_TEMPERATURE_DATA;
    }
    break;
  }
}
//----------------------------------------------------------------------------------------------------------------
void* InitSensor(const SensorSettings& sett)
{
  switch(sett.Type)
  {
    case mstNone:
      return NULL;
    
    case mstDS18B20:
      return InitDS18B20(sett);
      
    case mstBH1750:
      return InitBH1750(sett);

    case mstSi7021:
      return InitSi7021(sett);
  }

  return NULL;  
}
//----------------------------------------------------------------------------------------------------------------
void ReadROM()
{
    memset((void*)&scratchpadS,0,sizeof(scratchpadS));
    eeprom_read_block((void*)&scratchpadS, ROM_ADDRESS, 29);

    // пишем свой уникальный ID
    scratchpadS.rf_id = RF_MODULE_ID; 
    scratchpadS.packet_type = ptSensorsData; // говорим, что это тип пакета - данные с датчиками
    scratchpadS.packet_subtype = 0;

    // говорим, что никакой калибровки не поддерживаем
    scratchpadS.config &= ~2; // второй бит убираем по-любому

    // если интервала опроса не сохранено - выставляем по умолчанию
    if(scratchpadS.query_interval == 0xFF)
      scratchpadS.query_interval =  MEASURE_MIN_TIME/1000;

    // вычисляем интервал опроса
    query_interval = ((scratchpadS.query_interval & 0xF0)*60 + (scratchpadS.query_interval & 0x0F))*1000;
      

    scratchpadS.sensor1.type = GetSensorType(Sensors[0]);
    scratchpadS.sensor2.type = GetSensorType(Sensors[1]);
    scratchpadS.sensor3.type = GetSensorType(Sensors[2]);

    SetDefaultValue(Sensors[0],scratchpadS.sensor1.data);
    SetDefaultValue(Sensors[1],scratchpadS.sensor2.data);
    SetDefaultValue(Sensors[2],scratchpadS.sensor3.data);

}
//----------------------------------------------------------------------------------------------------------------
void* InitSi7021(const SensorSettings& sett) // инициализируем датчик влажности Si7021
{
  UNUSED(sett);
  Si7021* si = new Si7021();
  si->begin();

  return si;
}
//----------------------------------------------------------------------------------------------------------------
void* InitBH1750(const SensorSettings& sett) // инициализируем датчик освещённости
{
  BH1750Support* bh = new BH1750Support();
  bh->begin((BH1750Address)sett.Pin);
  return bh;
}
//----------------------------------------------------------------------------------------------------------------
void* InitDS18B20(const SensorSettings& sett) // инициализируем датчик температуры
{
  if(!sett.Pin)
    return NULL;

   OneWire ow(sett.Pin);

  if(!ow.reset()) // нет датчика
    return NULL;  

   ow.write(0xCC); // пофиг на адреса (SKIP ROM)
   ow.write(0x4E); // запускаем запись в scratchpad

   ow.write(0); // верхний температурный порог 
   ow.write(0); // нижний температурный порог
   ow.write(0x7F); // разрешение датчика 12 бит

   ow.reset();
   ow.write(0xCC); // пофиг на адреса (SKIP ROM)
   ow.write(0x48); // COPY SCRATCHPAD
   delay(10);
   ow.reset();

   return NULL;
    
}
//----------------------------------------------------------------------------------------------------------------
void* SensorDefinedData[3] = {NULL}; // данные, определённые датчиками при инициализации
//----------------------------------------------------------------------------------------------------------------
void InitSensors()
{
  // инициализируем датчики
  SensorDefinedData[0] = InitSensor(Sensors[0]);
  SensorDefinedData[1] = InitSensor(Sensors[1]);
  SensorDefinedData[2] = InitSensor(Sensors[2]);
     
}
//----------------------------------------------------------------------------------------------------------------
 void ReadDS18B20(const SensorSettings& sett, struct sensor* s) // читаем данные с датчика температуры
{ 
  s->data[0] = NO_TEMPERATURE_DATA;
  s->data[1] = 0;
  
  if(!sett.Pin)
    return;

   OneWire ow(sett.Pin);
    
    if(!ow.reset()) // нет датчика на линии
      return; 

  static byte data[9] = {0};
  
  ow.write(0xCC); // пофиг на адреса (SKIP ROM)
  ow.write(0xBE); // читаем scratchpad датчика на пине

  for(uint8_t i=0;i<9;i++)
    data[i] = ow.read();


 if (OneWire::crc8( data, 8) != data[8]) // проверяем контрольную сумму
      return;
  
  int loByte = data[0];
  int hiByte = data[1];

  int temp = (hiByte << 8) + loByte;
  
  bool isNegative = (temp & 0x8000);
  
  if(isNegative)
    temp = (temp ^ 0xFFFF) + 1;

  int tc_100 = (6 * temp) + temp/4;
   
  s->data[0] = tc_100/100;
  s->data[1] = tc_100 % 100;
    
}
//----------------------------------------------------------------------------------------------------------------
void ReadBH1750(const SensorSettings& sett, void* sensorDefinedData, struct sensor* s) // читаем данные с датчика освещённости
{
  UNUSED(sett);
  BH1750Support* bh = (BH1750Support*) sensorDefinedData;
  long lum = bh->GetCurrentLuminosity();
  memcpy(s->data,&lum,sizeof(lum));
}
//----------------------------------------------------------------------------------------------------------------
void ReadSi7021(const SensorSettings& sett, void* sensorDefinedData, struct sensor* s) // читаем данные с датчика влажности Si7021
{
  UNUSED(sett);
  Si7021* si = (Si7021*) sensorDefinedData;
  HumidityAnswer ha = si->read();

  s->data[0] = ha.Humidity;
  s->data[1] = ha.HumidityDecimal;
  s->data[2] = ha.Temperature;
  s->data[3] = ha.TemperatureDecimal;

}
//----------------------------------------------------------------------------------------------------------------
void ReadSensor(const SensorSettings& sett, void* sensorDefinedData, struct sensor* s)
{
  switch(sett.Type)
  {
    case mstNone:
      
    break;

    case mstDS18B20:
    ReadDS18B20(sett,s);
    break;

    case mstBH1750:
    ReadBH1750(sett,sensorDefinedData,s);
    break;

    case mstSi7021:
    ReadSi7021(sett,sensorDefinedData,s);
    break;
  }
}
//----------------------------------------------------------------------------------------------------------------
void ReadSensors()
{
  // читаем информацию с датчиков
  ReadSensor(Sensors[0],SensorDefinedData[0],&scratchpadS.sensor1);
  ReadSensor(Sensors[1],SensorDefinedData[1],&scratchpadS.sensor2);
  ReadSensor(Sensors[2],SensorDefinedData[2],&scratchpadS.sensor3);
}
//----------------------------------------------------------------------------------------------------------------
void MeasureDS18B20(const SensorSettings& sett)
{
  if(!sett.Pin)
    return;

   OneWire ow(sett.Pin);
    
    if(!ow.reset()) // нет датчика на линии
      return; 

    ow.write(0xCC);
    ow.write(0x44); // посылаем команду на старт измерений
    
    ow.reset();    
  
}
//----------------------------------------------------------------------------------------------------------------
void MeasureSensor(const SensorSettings& sett) // запускаем конвертацию с датчика, если надо
{
  switch(sett.Type)
  {
    case mstNone:    
    break;

    case mstDS18B20:
    MeasureDS18B20(sett);
    break;

    case mstBH1750:
    break;

    case mstSi7021:
    break;
  }  
}
//----------------------------------------------------------------------------------------------------------------
void StartMeasure()
{
  // запускаем конвертацию
  MeasureSensor(Sensors[0]);
  MeasureSensor(Sensors[1]);
  MeasureSensor(Sensors[2]);
}
//----------------------------------------------------------------------------------------------------------------
void WriteROM()
{
    scratchpadS.rf_id = RF_MODULE_ID;

    scratchpadS.sensor1.type = GetSensorType(Sensors[0]);
    scratchpadS.sensor2.type = GetSensorType(Sensors[1]);
    scratchpadS.sensor3.type = GetSensorType(Sensors[2]);
  
    eeprom_write_block( (void*)scratchpad,ROM_ADDRESS,29);

}
//----------------------------------------------------------------------------------------------------------------
void setup()
{
  //  Serial.begin(9600);
  //  byte dummy[100] = {0xFF};
   //  eeprom_write_block( (void*)dummy,ROM_ADDRESS,100);
  
    ReadROM();

    InitSensors(); // инициализируе датчики
    StartMeasure(); // запускаем конвертацию с датчиков при старте

    pinMode(sensePin, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(sensePin), OW_Process, CHANGE);


    machineState = stateSleep;
    workMode = OWW_NO_WRITE;
    OW_DDR &= ~OW_PINN;

    OneWireInterruptAtFallingEdge();

    // Select clock source: internal I/O clock 
    ASSR &= ~(1<<AS2);

    PreInit();
    TimerEnable();

  
}
//----------------------------------------------------------------------------------------------------------------
// обработчик прерывания на пине
//----------------------------------------------------------------------------------------------------------------
void OW_Process()
{
    // копируем переменные в регистры
    uint8_t thisWorkMode = workMode;  
    MachineStates thisMachineState = machineState;

    // попросили отправить нолик
    if ((thisWorkMode == OWW_WRITE_0))
    {
        OneWireSetLow();    // если попросили отправить нолик - жмём линию к земле
        thisWorkMode = OWW_NO_WRITE;
    }
    // если надо отправить единицу - ничего специально не делаем.

    // выключаем прерывание на пине, оно должно быть активно только
    // тогда, когда состояние автомата - stateSleep.
    OneWireDisableInterrupt();

    // чего делаем?
    switch (thisMachineState)
    {

     case stateMeasure:
     case statePresence:
     case stateReset:
     break;

    // ничего не делаем
    case stateSleep:

        // просим таймер проснуться через некоторое время,
        // чтобы проверить, есть ли импульс RESET
        TimerSetTimeout(OWT_MIN_RESET);
        
        // включаем прерывание на пине, ждём других фронтов
        OneWireEnableInterrupt();
        
        break;
        
    // начинаем читать на спадающем фронте от мастера,
    // чтение закрывается в обработчике таймера.
    case stateWriteScratchpad: // ждём приёма
    case stateReadCommand:

        // взводим таймер на чтение из линии
        TimerSetTimeout(OWT_READLINE);
        
        break;
        
    case stateReadScratchpad:  // нам послали бит

        // взводим таймер, удерживая линию в LOW
        TimerSetTimeout(OWT_LOWTIME);
        
        break;
        
    case stateCheckReset:  // нарастающий фронт или импульс RESET

        // включаем прерывание по спадающему фронту
        OneWireInterruptAtFallingEdge();
        
        // проверяем по таймеру - это импульс RESET?
        TimerSetTimeout(OWT_RESET_PRESENCE);

        // говорим конечному автомату, что мы ждём импульса RESET
        thisMachineState = stateReset;
        
        break;
    } // switch

    // включаем таймер
    TimerEnable();

    // сохраняем состояние работы
    machineState = thisMachineState;
    workMode = thisWorkMode;

}
//----------------------------------------------------------------------------------------------------------------
void loop()
{
   MachineStates thisMachineState = machineState;

  if(scratchpadReceivedFromMaster)
  {
    // скратч был получен от мастера, тут можно что-то делать

    // вычисляем новый интервал опроса
    query_interval = ((scratchpadS.query_interval & 0xF0)*60 + (scratchpadS.query_interval & 0x0F))*1000;
    scratchpadReceivedFromMaster = false;
      
  } // scratchpadReceivedFromMaster

  unsigned long curMillis = millis();

  // только если ничего не делаем на линии 1-Wire и запросили конвертацию
  if(needToMeasure && thisMachineState == stateSleep)
  {
    needToMeasure = false;
    StartMeasure();
    sensorsUpdateTimer = curMillis; // сбрасываем таймер обновления
    measureTimerEnabled = true; // включаем флаг, что мы должны прочитать данные с датчиков
  }

  // если линия 1-Wire спит, таймер получения данных с датчиков взведён и отсчёт кончился - получаем данные с датчиков
  
  if(thisMachineState == stateSleep && measureTimerEnabled && (curMillis - sensorsUpdateTimer) > query_interval)
  {
     sensorsUpdateTimer = curMillis;
     measureTimerEnabled = false;
     // можно читать информацию с датчиков
     ReadSensors();
  }

}
//----------------------------------------------------------------------------------------------------------------
// проверяет - в высоком ли уровне линия 1-Wire
//----------------------------------------------------------------------------------------------------------------
inline bool OneWireIsLineHigh() 
{
  return ((OW_PIN&OW_PINN) == OW_PINN);
}
//----------------------------------------------------------------------------------------------------------------
OnTimer
{
    // копируем все переменные в регистры
    uint8_t thisWorkMode = workMode;
    MachineStates thisMachineState = machineState;
    uint8_t thisBytePointer = bytePointer;
    uint8_t thisBitPointer = bitPointer;
    uint8_t thisActualBit = actualBit;
    uint8_t thisCrcHolder = crcHolder;


    // смотрим, в высоком ли уровне линия 1-Wire?
    bool isLineHigh = OneWireIsLineHigh();

    // прерывание активно?
    if (OneWireIsInterruptEnabled())
    {
        // это может быть импульс RESET
        if (!isLineHigh)   // линия всё ещё прижата к земле
        {
            // будем ждать нарастающий фронт импульса
            thisMachineState = stateCheckReset;
            OneWireInterruptAtRisingEdge();
        }
        
        // выключаем таймер
        TimerDisable();
        
    } // if
    else // прерывание неактивно
    {

        // чего делаем?
        switch (thisMachineState)
        {
         case stateCheckReset:
         case stateSleep:
         break;

         case stateMeasure: // измеряем
        
          break;

        case stateReset:  //импульс RESET закончился, надо послать Presence

            // будем ждать окончания отработки Presence
            thisMachineState = statePresence;

            // кидаем линию на землю
            OneWireSetLow();

            // и взводим таймер, чтобы удержать её нужное время
            TimerSetTimeout(OWT_PRESENCE);

            // никаких прерываний на линии, пока не отработаем Presence
            OneWireDisableInterrupt();
            
            break;

        case statePresence: // посылали импульс Presence

            OneWireSendAck();  // импульс Presence послан, теперь надо ждать команды

            // всё сделали, переходим в ожидание команды
             thisMachineState = stateReadCommand;
            commandBuffer = 0;
            thisBitPointer = 1;
            break;

        case stateReadCommand: // ждём команду
        
            if (isLineHigh)    // если линия в высоком уровне - нам передают единичку
            {
                // запоминаем её в текущей позиции
                commandBuffer |= thisBitPointer;
            }

            // сдвигаем позицию записи
            thisBitPointer = (thisBitPointer<<1);
            
            if (!thisBitPointer)   // прочитали 8 бит
            {
                thisBitPointer = 1; // переходим опять на первый бит

                // чего нам послали за команду?
                switch (commandBuffer)
                {
                
                case 0x4E: // попросили записать скратчпад, следом пойдёт скратчпад
                
                    thisMachineState = stateWriteScratchpad;
                    thisBytePointer = 0; //сбрасываем указатель записи на начало данных
                    scratchpad[0] = 0; // обнуляем данные
                    break;
                    
                case 0x25: // попросили сохранить скратчпад в EEPROM
                
                    WriteROM(); // сохранили

                    // и засыпаем
                    thisMachineState = stateSleep;

                    // сбросили данные в исходные значения
                    commandBuffer = 0;
                    thisBitPointer = 1; 
                    
                    break;
                    
                case 0x44:  // попросили запустить конвертацию
                case 0x64:  // и такая команда приходит для конвертации
                      needToMeasure = true; // выставили флаг, что надо запустить конвертацию
                      thisMachineState = stateSleep; // спим
                    break;
                    
                case 0xBE: // попросили отдать скратчпад мастеру
                
                    // запоминаем, чего мы будем делать дальше

                    thisMachineState = stateReadScratchpad;
                    thisBytePointer = 0;
                    thisCrcHolder = 0;

                    // запоминаем первый бит, который надо послать
                    thisActualBit = (thisBitPointer & scratchpad[0]) == thisBitPointer;
                    thisWorkMode = thisActualBit; // запоминаем, какой бит послать
                    
                    break;

               case 0xCC:
                    // ждём команды, она пойдёт следом
                    thisMachineState = stateReadCommand;
                    commandBuffer = 0;
                    thisBitPointer = 1;  //Command buffer have to set zero, only set bits will write in

               break;
                    
                default:

                    // по умолчанию - спим
                    thisMachineState = stateSleep;
                } // switch
            }
            break;

        case stateWriteScratchpad: // пишем в скратчпад данные, принятые от мастера

            if (isLineHigh) // если линия поднята - послали единичку
            {
                // запоминаем в текущей позиции
                scratchpad[thisBytePointer] |= thisBitPointer;

            }

            // передвигаем позицию записи
            thisBitPointer = (thisBitPointer << 1);
            
            if (!thisBitPointer) // прочитали байт
            {
                // сдвигаем указатель записи байтов
                thisBytePointer++;
                thisBitPointer = 1;

                
                if (thisBytePointer>=30) // если прочитали 30 байт - переходим на сон
                {
                   thisMachineState = stateSleep;
                   commandBuffer = 0;
                   thisBitPointer = 1;  //Command buffer have to set zero, only set bits will write in 

                   // говорим, что мы прочитали скратчпад c мастера
                   scratchpadReceivedFromMaster = true;       
                  break;
                }
                else // сбрасываем следующий байт в 0, в последующем мы будем туда писать.
                  scratchpad[thisBytePointer]=0;
            }
            break;
            
        case stateReadScratchpad: // посылаем данные скратчпада мастеру
        
            OneWireSendAck(); // подтверждаем, что готовы передавать

            // по ходу считаем CRC
            if ((thisCrcHolder & 1)!= thisActualBit) 
              thisCrcHolder = (thisCrcHolder>>1)^0x8c;
            else 
              thisCrcHolder >>=1;

            // передвигаем позицию чтения
            thisBitPointer = (thisBitPointer<<1);
            
            if (!thisBitPointer) // прочитали байт
            {
                // переходим на следующий байт
                thisBytePointer++;
                thisBitPointer = 1;
                
                if (thisBytePointer>=30) // если послали весь скратчпад, то вываливаемся в сон
                {
                    thisMachineState = stateSleep;
                    break;
                }
                else 
                  if (thisBytePointer==29) // если следующий байт - последний, то пишем туба подсчитанную контрольную сумму
                    scratchpad[29] = thisCrcHolder;
            }

            // вычисляем, какой бит послать
            thisActualBit = (thisBitPointer & scratchpad[thisBytePointer])== thisBitPointer;
            thisWorkMode = thisActualBit; // запоминаем, чего надо послать
            
            break;
        } // switch
    } // else прерывание выключено
    
    if (thisMachineState == stateSleep) // если спим, то выключаем таймер
    {
        TimerDisable();
    }

    if ( (thisMachineState != statePresence) && (thisMachineState != stateMeasure) )
    {
        TimerSetTimeout((OWT_MIN_RESET-OWT_READLINE));
        OneWireEnableInterrupt();
    }


    machineState = thisMachineState;
    workMode = thisWorkMode;
    bytePointer = thisBytePointer;
    bitPointer = thisBitPointer;
    actualBit = thisActualBit;
    crcHolder = thisCrcHolder;
}
//----------------------------------------------------------------------------------------------------------------


