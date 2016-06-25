
/*

Модуль поддержки дисплея Nextion по шине 1-Wire

*/
#include <avr/io.h>
#include <avr/interrupt.h>
#include "NextionController.h"
//----------------------------------------------------------------------------------------------------------------
#define UNUSED(expr) do { (void)(expr); } while (0)
//----------------------------------------------------------------------------------------------------------------
// уникальный ID модуля
//----------------------------------------------------------------------------------------------------------------
#define RF_MODULE_ID 200
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
//Структура передаваемая мастеру и обратно
//----------------------------------------------------------------------------------------------------------------
struct sensorData
{
    byte type;
    byte data[2];
};
//----------------------------------------------------------------------------------------------------------------
typedef enum
{
  StateTemperature = 1, // есть температурные датчики
  StateLuminosity = 4, // есть датчики освещенности
  StateHumidity = 8 // есть датчики влажности
  
} ModuleStates; // вид состояния
//----------------------------------------------------------------------------------------------------------------
typedef struct
{
  byte packet_type; // тип пакета
  byte packet_subtype; // подтип пакета
  byte config; // конфигурация
  byte controller_id; // ID контроллера, к которому привязан модуль
  byte rf_id; // уникальный идентификатор модуля
  byte reserved[3]; // резерв, добитие до 24 байт
  byte controllerStatus;
  byte nextionStatus1;
  byte nextionStatus2;  
  byte openTemperature; // температура открытия окон
  byte closeTemperature; // температура закрытия окон
  byte dataCount; // кол-во записанных показаний с датчиков 
  sensorData data[5];

  byte crc8;
    
} t_scratchpad;
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
t_scratchpad scratchpadS, savedScratchpad;
volatile char* scratchpad = (char *)&scratchpadS; //что бы обратиться к scratchpad как к линейному массиву

volatile uint8_t crcHolder; //CRC calculation

volatile uint8_t commandBuffer; //Входной буфер команды

volatile uint8_t bitPointer;  //pointer to current Bite
volatile uint8_t bytePointer; //pointer to current Byte

volatile MachineStates machineState; //state
volatile uint8_t workMode; //if 0 next bit that send the device is  0
volatile uint8_t actualBit; //current

volatile bool scratchpadReceivedFromMaster = false; // флаг, что мы должны обновить данные в Nextion

//----------------------------------------------------------------------------------------------------------------
//Write a bit after next falling edge from master
//its for sending a zero as soon as possible
#define OWW_NO_WRITE 2
#define OWW_WRITE_0 0
//----------------------------------------------------------------------------------------------------------------
void ReadROM()
{

    memset((void*)&scratchpadS,0,sizeof(scratchpadS));

    // пишем свой уникальный ID
    scratchpadS.rf_id = RF_MODULE_ID; 
    scratchpadS.packet_type = 2; // говорим, что это тип пакета - дисплей Nextion

    memcpy((void*)&savedScratchpad,(void*)&scratchpadS,sizeof(scratchpadS));


}
//----------------------------------------------------------------------------------------------------------------
void WriteROM()
{

}
//----------------------------------------------------------------------------------------------------------------
NextionController nextion;
volatile unsigned long rotationTimer = 0; // таймер авторотации
const unsigned int ROTATION_INTERVAL = 7000; // интервал авторотации
volatile bool isDisplaySleep = false;
volatile int8_t currentSensorIndex = -1;
//----------------------------------------------------------------------------------------------------------------
void displayNextSensorData(int8_t dir)
{
  if(isDisplaySleep)
    return;  

  if(!scratchpadS.dataCount) // нет данных с датчиков
    return;

  currentSensorIndex += dir; // прибавляем направление
  if(currentSensorIndex < 0)
  {
     // надо искать последний элемент
     currentSensorIndex = scratchpadS.dataCount-1;  
  }

  if(currentSensorIndex >= scratchpadS.dataCount)
    currentSensorIndex = 0;

  if(currentSensorIndex < 0)
    currentSensorIndex = 0;

  byte type = scratchpadS.data[currentSensorIndex].type;
  switch(type)
  {
      case StateTemperature:
      {
        Temperature t; t.Value = scratchpadS.data[currentSensorIndex].data[1]; t.Fract = scratchpadS.data[currentSensorIndex].data[0];
        nextion.showTemperature(t);
      }
      break;

      case StateHumidity:
      {
        Temperature t; t.Value = scratchpadS.data[currentSensorIndex].data[1]; t.Fract = scratchpadS.data[currentSensorIndex].data[0];
        nextion.showHumidity(t);
      }
      break;
    
      case StateLuminosity:
      {
        long lum = 0;
        memcpy(&lum,scratchpadS.data[currentSensorIndex].data,2); 
        nextion.showLuminosity(lum);

      }
      break;

  } // switch
}
//----------------------------------------------------------------------------------------------------------------
void nSleep(NextionAbstractController* Sender)
{
  UNUSED(Sender);
  scratchpadS.controllerStatus |= 64;
  isDisplaySleep = true;
}
//----------------------------------------------------------------------------------------------------------------
void nWake(NextionAbstractController* Sender)
{
  UNUSED(Sender);
  scratchpadS.controllerStatus &= ~64;
  isDisplaySleep = false;
}
//----------------------------------------------------------------------------------------------------------------
void nString(NextionAbstractController* Sender, const char* str)
{
  UNUSED(Sender);

  if(!strcmp_P(str,(const char*)F("w_open")))
  {
    // попросили открыть окна
    scratchpadS.nextionStatus1 |= 2;
    return;
  }
  
  if(!strcmp_P(str,(const char*)F("w_close")))
  {
    // попросили закрыть окна
    scratchpadS.nextionStatus1 |= 1;
    return;
  }
  
  if(!strcmp_P(str,(const char*)F("w_auto")))
  {
    // попросили перевести в автоматический режим окон
    scratchpadS.nextionStatus1 |= 4;
    return;
  }
  
  if(!strcmp_P(str,(const char*)F("w_manual")))
  {
    // попросили перевести в ручной режим работы окон
    scratchpadS.nextionStatus1 |= 8;
    return;
  }
  
  if(!strcmp_P(str,(const char*)F("wtr_on")))
  {
    // попросили включить полив
    scratchpadS.nextionStatus1 |= 16;
    return;
  }
  
  if(!strcmp_P(str,(const char*)F("wtr_off")))
  {
    // попросили выключить полив
    scratchpadS.nextionStatus1 |= 32;
    return;
  }
  
  if(!strcmp_P(str,(const char*)F("wtr_auto")))
  {
    // попросили перевести в автоматический режим работы полива
    scratchpadS.nextionStatus1 |= 64;
    return;
  }
  
  if(!strcmp_P(str,(const char*)F("wtr_manual")))
  {
    // попросили перевести в ручной режим работы полива
    scratchpadS.nextionStatus1 |= 128;
    return;
  }
  
  if(!strcmp_P(str,(const char*)F("lht_on")))
  {
    // попросили включить досветку
    scratchpadS.nextionStatus2 |= 1;
    return;
  }
  
  if(!strcmp_P(str,(const char*)F("lht_off")))
  {
    // попросили выключить досветку
    scratchpadS.nextionStatus2 |= 2;
    return;
  }
  
  if(!strcmp_P(str,(const char*)F("lht_auto")))
  {
    // попросили перевести досветку в автоматический режим
    scratchpadS.nextionStatus2 |= 4;
    return;
  }
  
  if(!strcmp_P(str,(const char*)F("lht_manual")))
  {
    // попросили перевести досветку в ручной режим
    scratchpadS.nextionStatus2 |= 8;
    return;
  }
  
 if(!strcmp_P(str,(const char*)F("topen_down")))
 {
    // листают температуру открытия вниз
    scratchpadS.nextionStatus2 |= 32;    
    return;
  }  

 if(!strcmp_P(str,(const char*)F("topen_up")))
  {
    // листают температуру открытия вверх
    scratchpadS.nextionStatus2 |= 16;
    
    return;
  }

 if(!strcmp_P(str,(const char*)F("tclose_down")))
  {
    // листают температуру закрытия вниз
   scratchpadS.nextionStatus2 |= 128;
    return;
  }  

 if(!strcmp_P(str,(const char*)F("tclose_up")))
  {
    // листают температуру закрытия вверх
    scratchpadS.nextionStatus2 |= 64;
    return;
  }

 if(!strcmp_P(str,(const char*)F("prev")))
  {
    rotationTimer = millis();
    displayNextSensorData(-1);
    return;
  }

 if(!strcmp_P(str,(const char*)F("next")))
  {
    rotationTimer = millis();
    displayNextSensorData(1);
    return;
  }
  
}
//----------------------------------------------------------------------------------------------------------------
void setup()
{
    Serial.begin(9600);

    NextionSubscribeStruct ss;
    ss.OnStringReceived = nString;
    ss.OnSleep = nSleep;
    ss.OnWakeUp = nWake;
    nextion.subscribe(ss);
     
    nextion.begin(&Serial,NULL);
    
    nextion.setWaitTimerInterval();
    nextion.setSleepDelay();
    nextion.setWakeOnTouch();
    nextion.setEchoMode();
  
    ReadROM();

    pinMode(sensePin, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(sensePin), OW_Process, CHANGE);


    machineState = stateSleep;
    workMode = OWW_NO_WRITE;
    OW_DDR &= ~OW_PINN;
   // DDRD |= R_BIT;

    OneWireInterruptAtFallingEdge();

  //  PORTB &= ~R_BIT;

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

  // проверяем, надо ли обновить Nextion
  if(scratchpadReceivedFromMaster)
  {

      if((savedScratchpad.controllerStatus & 1) != (scratchpadS.controllerStatus & 1) )
      {
        nextion.notifyWindowState(scratchpadS.controllerStatus & 1);
      }

      if((savedScratchpad.controllerStatus & 2) != (scratchpadS.controllerStatus & 2))
      {
        nextion.notifyWindowMode(scratchpadS.controllerStatus & 2);
      }

      if((savedScratchpad.controllerStatus & 4) != (scratchpadS.controllerStatus & 4))
      {
        nextion.notifyWaterState(scratchpadS.controllerStatus & 4);
      }

      if((savedScratchpad.controllerStatus & 8) != (scratchpadS.controllerStatus & 8) )
      {
        nextion.notifyWaterMode(scratchpadS.controllerStatus & 8);
      }

      if((savedScratchpad.controllerStatus & 16) != (scratchpadS.controllerStatus & 16))
      {
        nextion.notifyLightState(scratchpadS.controllerStatus & 16);
      }

      if((savedScratchpad.controllerStatus & 32) != (scratchpadS.controllerStatus & 32))
      {
        nextion.notifyLightMode(scratchpadS.controllerStatus & 32);
      }

      if(savedScratchpad.openTemperature != scratchpadS.openTemperature)
      {
        nextion.showOpenTemp(scratchpadS.openTemperature);
      }

      if(savedScratchpad.closeTemperature != scratchpadS.closeTemperature)
      {
        nextion.showCloseTemp(scratchpadS.closeTemperature);
      }

      memcpy((void*)&savedScratchpad,(void*)&scratchpadS,sizeof(scratchpadS));
      scratchpadReceivedFromMaster = false;
      
  } // scratchpadReceivedFromMaster

  // крутим показания на экране ожидания
  unsigned long curMillis = millis();
  if( (curMillis - rotationTimer) > NEXTION_ROTATION_INTERVAL)
  {
    rotationTimer = curMillis;
    displayNextSensorData(1);
  }  

  nextion.update();

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
        
        // включаем таймер
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

            // настраиваем всё добро на ожидание команды
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

                    // и ждём команды
                    thisMachineState = stateReadCommand;

                    // сбросили данные в исходные значения
                    commandBuffer = 0;
                    thisBitPointer = 1; 
                    
                    break;
                    
                case 0x44:  // попросили запустить конвертацию
                case 0x64:  // и такая команда приходит для конвертации
                    
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
                    
                default:

                    // по умолчанию ждём команды
                    thisMachineState = stateReadCommand;
                    commandBuffer = 0;
                    thisBitPointer = 1;  //Command buffer have to set zero, only set bits will write in
                    //lmode=OWM_SLEEP;  //all other commands do nothing
                }
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

                
                if (thisBytePointer>=30) // если прочитали 30 байт - переходим на ожидание команды
                {
                   thisMachineState = stateReadCommand;
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
                
                if (thisBytePointer>=30) // если послали весь скратчпад, то вываливаемся в ожидание команды
                {
                    thisMachineState = stateReadCommand;
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


