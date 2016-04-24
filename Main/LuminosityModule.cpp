#include "LuminosityModule.h"
#include "ModuleController.h"

static uint8_t LAMP_RELAYS[] = { LAMP_RELAYS_PINS }; // объявляем массив пинов реле

BH1750Support::BH1750Support()
{

}
void BH1750Support::begin(BH1750Address addr, BH1750Mode mode)
{
  deviceAddress = addr;
  Wire.begin();
  writeByte(BH1750PowerOn); // включаем датчик
  ChangeMode(mode); 
}
void BH1750Support::ChangeMode(BH1750Mode mode) // смена режима работы
{
   currentMode = mode; // сохраняем текущий режим опроса
   writeByte((uint8_t)currentMode);
  _delay_ms(10);
}
void BH1750Support::ChangeAddress(BH1750Address newAddr)
{
  if(newAddr != deviceAddress) // только при смене адреса включаем датчик
  { 
    deviceAddress = newAddr;
    
    writeByte(BH1750PowerOn); // включаем датчик  
    ChangeMode(currentMode); // меняем режим опроса на текущий
  } // if
}
void BH1750Support::writeByte(uint8_t toWrite) 
{
  Wire.beginTransmission(deviceAddress);
  BH1750_WIRE_WRITE(toWrite);
  Wire.endTransmission();
}
long BH1750Support::GetCurrentLuminosity() 
{

  long curLuminosity = NO_LUMINOSITY_DATA;

  Wire.beginTransmission(deviceAddress); // начинаем опрос датчика освещенности
 if(Wire.requestFrom(deviceAddress, 2) == 2)// ждём два байта
 {
  // читаем два байта
  curLuminosity = BH1750_WIRE_READ();
  curLuminosity <<= 8;
  curLuminosity |= BH1750_WIRE_READ();
  curLuminosity = curLuminosity/1.2; // конвертируем в люксы
 }

  Wire.endTransmission();


  return curLuminosity;
}


void LuminosityModule::Setup()
{

 #if LIGHT_SENSORS_COUNT > 0
  for(uint8_t i=0;i<LIGHT_SENSORS_COUNT;i++)
  {
    State.AddState(StateLuminosity,i); // добавляем в состояние нужные индексы датчиков
  } // for
  #endif
  
  #if LIGHT_SENSORS_COUNT > 0
  lightMeter.begin(); // запускаем первый датчик освещенности
  #endif

  #if LIGHT_SENSORS_COUNT > 1
  lightMeter2.begin(BH1750Address2); // запускаем второй датчик освещённости
  #endif

  
  // настройка модуля тут
  settings = mainController->GetSettings();

  workMode = lightAutomatic; // автоматический режим работы
  bRelaysIsOn = false; // все реле выключены
  bLastRelaysIsOn = false; // состояние не изменилось
  
  SAVE_STATUS(LIGHT_STATUS_BIT,0); // сохраняем, что досветка выключена
  SAVE_STATUS(LIGHT_MODE_BIT,1); // сохраняем, что мы в автоматическом режиме работы
  
#ifdef USE_LIGHT_MANUAL_MODE_DIODE
  blinker.begin(DIODE_LIGHT_MANUAL_MODE_PIN,F("LX")); // настраиваем блинкер на нужный пин
#endif

  #ifdef SAVE_RELAY_STATES
   uint8_t relayCnt = LAMP_RELAYS_COUNT/8; // устанавливаем кол-во каналов реле
   if(LAMP_RELAYS_COUNT > 8 && LAMP_RELAYS_COUNT % 8)
    relayCnt++;

  if(LAMP_RELAYS_COUNT < 9)
    relayCnt = 1;
    
   for(uint8_t i=0;i<relayCnt;i++) // добавляем состояния реле (каждый канал - 8 реле)
    State.AddState(StateRelay,i);
  #endif  


 // выключаем все реле
  for(uint8_t i=0;i<LAMP_RELAYS_COUNT;i++)
  {
    pinMode(LAMP_RELAYS[i],OUTPUT);
    digitalWrite(LAMP_RELAYS[i],RELAY_OFF);

    #ifdef SAVE_RELAY_STATES
    uint8_t idx = i/8;
    uint8_t bitNum1 = i % 8;
    OneState* os = State.GetState(StateRelay,idx);
    if(os)
    {
      RelayPair rp = *os;
      uint8_t curRelayStates = rp.Current;
      bitWrite(curRelayStates,bitNum1, bRelaysIsOn);
      os->Update((void*)&curRelayStates);
    }
    #endif
  } // for
    
       
 }
void LuminosityModule::Update(uint16_t dt)
{ 
  // обновление модуля тут
#ifdef USE_LIGHT_MANUAL_MODE_DIODE
    blinker.update();
#endif

 // обновляем состояние всех реле управления досветкой
 if(bLastRelaysIsOn != bRelaysIsOn) // только если состояние с момента последнего опроса изменилось
 {
    bLastRelaysIsOn = bRelaysIsOn; // сохраняем текущее
    
    for(uint8_t i=0;i<LAMP_RELAYS_COUNT;i++)
    {
      digitalWrite(LAMP_RELAYS[i],bRelaysIsOn ? RELAY_ON : RELAY_OFF); // пишем в пин нужное состояние
  
      #ifdef SAVE_RELAY_STATES
      uint8_t idx = i/8;
      uint8_t bitNum1 = i % 8;
      OneState* os = State.GetState(StateRelay,idx);
      if(os)
      {
        RelayPair rp = *os;
        uint8_t curRelayStates = rp.Current;
        bitWrite(curRelayStates,bitNum1, bRelaysIsOn);
        os->Update((void*)&curRelayStates);
      }
      #endif
      
    } // for 
 } // if


  lastUpdateCall += dt;
  if(lastUpdateCall < LUMINOSITY_UPDATE_INTERVAL) // обновляем согласно настроенному интервалу
    return;
  else
    lastUpdateCall = 0;

  long lum = NO_LUMINOSITY_DATA;

  #if LIGHT_SENSORS_COUNT > 0
    lum = lightMeter.GetCurrentLuminosity();
    State.UpdateState(StateLuminosity,0,(void*)&lum);
  #endif
    
  #if LIGHT_SENSORS_COUNT > 1
    lum = lightMeter2.GetCurrentLuminosity();
    State.UpdateState(StateLuminosity,1,(void*)&lum);
  #endif   

}

bool  LuminosityModule::ExecCommand(const Command& command, bool wantAnswer)
{
  if(wantAnswer) 
    PublishSingleton = UNKNOWN_COMMAND;
  
  uint8_t argsCnt = command.GetArgsCount();
  
  if(command.GetType() == ctSET) 
  {
      if(wantAnswer) 
        PublishSingleton = PARAMS_MISSED;

      if(argsCnt > 0)
      {
         String s = command.GetArg(0);
         if(s == STATE_ON) // CTSET=LIGHT|ON
         {
          
          // попросили включить досветку
          if(command.IsInternal() // если команда пришла от другого модуля
          && workMode == lightManual)  // и мы в ручном режиме, то
          {
            // просто игнорируем команду, потому что нами управляют в ручном режиме
          } // if
           else
           {
              if(!command.IsInternal()) // пришла команда от пользователя,
              {
                workMode = lightManual; // переходим на ручной режим работы
                #ifdef USE_LIGHT_MANUAL_MODE_DIODE
                // мигаем светодиодом на 8 пине
                blinker.blink(WORK_MODE_BLINK_INTERVAL);
                #endif
              }

            if(!bRelaysIsOn)
            {
              // значит - досветка была выключена и будет включена, надо записать в лог событие
              mainController->Log(this,s); 
            }

            bRelaysIsOn = true; // включаем реле досветки
            
            PublishSingleton.Status = true;
            if(wantAnswer) 
              PublishSingleton = STATE_ON;


            
           } // else
 
            SAVE_STATUS(LIGHT_STATUS_BIT,bRelaysIsOn ? 1 : 0); // сохраняем состояние досветки
            SAVE_STATUS(LIGHT_MODE_BIT,workMode == lightAutomatic ? 1 : 0); // сохраняем режим работы досветки
          
         } // STATE_ON
         else
         if(s == STATE_OFF) // CTSET=LIGHT|OFF
         {
          // попросили выключить досветку
          if(command.IsInternal() // если команда пришла от другого модуля
          && workMode == lightManual)  // и мы в ручном режиме, то
          {
            // просто игнорируем команду, потому что нами управляют в ручном режиме
           } // if
           else
           {
              if(!command.IsInternal()) // пришла команда от пользователя,
              {
                workMode = lightManual; // переходим на ручной режим работы
                #ifdef USE_LIGHT_MANUAL_MODE_DIODE
                // мигаем светодиодом на 8 пине
                blinker.blink(WORK_MODE_BLINK_INTERVAL);
                #endif
              }

            if(bRelaysIsOn)
            {
              // значит - досветка была включена и будет выключена, надо записать в лог событие
              mainController->Log(this,s); 
            }

            bRelaysIsOn = false; // выключаем реле досветки
            
            PublishSingleton.Status = true;
            if(wantAnswer) 
              PublishSingleton = STATE_OFF;

            
           } // else

            SAVE_STATUS(LIGHT_STATUS_BIT,bRelaysIsOn ? 1 : 0); // сохраняем состояние досветки
            SAVE_STATUS(LIGHT_MODE_BIT,workMode == lightAutomatic ? 1 : 0); // сохраняем режим работы досветки

         } // STATE_OFF
         else
         if(s == WORK_MODE) // CTSET=LIGHT|MODE|AUTO, CTSET=LIGHT|MODE|MANUAL
         {
           // попросили установить режим работы
           if(argsCnt > 1)
           {
              s = command.GetArg(1);
              if(s == WM_MANUAL)
              {
                // попросили перейти в ручной режим работы
                workMode = lightManual; // переходим на ручной режим работы
                #ifdef USE_LIGHT_MANUAL_MODE_DIODE
                 // мигаем светодиодом на 8 пине
               blinker.blink(WORK_MODE_BLINK_INTERVAL);
               #endif
              }
              else
              if(s == WM_AUTOMATIC)
              {
                // попросили перейти в автоматический режим работы
                workMode = lightAutomatic; // переходим на автоматический режим работы
                #ifdef USE_LIGHT_MANUAL_MODE_DIODE
                 // гасим диод на 8 пине
                blinker.blink();
                #endif
              }

              PublishSingleton.Status = true;
              if(wantAnswer)
              {
                PublishSingleton = WORK_MODE; 
                PublishSingleton << PARAM_DELIMITER << (workMode == lightAutomatic ? WM_AUTOMATIC : WM_MANUAL);
              }

            SAVE_STATUS(LIGHT_STATUS_BIT,bRelaysIsOn ? 1 : 0); // сохраняем состояние досветки
            SAVE_STATUS(LIGHT_MODE_BIT,workMode == lightAutomatic ? 1 : 0); // сохраняем режим работы досветки
              
           } // if (argsCnt > 1)
         } // WORK_MODE
         
      } // if(argsCnt > 0)
  }
  else
  if(command.GetType() == ctGET) //получить данные
  {

    if(!argsCnt) // нет аргументов, попросили дать показания с датчика
    {
      
      PublishSingleton.Status = true;
      if(wantAnswer) 
      {
         PublishSingleton = "";
        // запросили показания с датчиков. У нас должно выводиться минимум 2 показания,
        // для обеспечения нормальной работы конфигуратора. Поэтому мы добавляем недостающие показания
        // как показания NO_LUMINOSITY_DATA для тех датчиков, которых нет в прошивке.

         uint8_t _cnt = State.GetStateCount(StateLuminosity);
         uint8_t _written = 0;
         
         for(uint8_t i=0;i<_cnt;i++)
         {
            OneState* os = State.GetStateByOrder(StateLuminosity,i);
            if(os)
            {
              LuminosityPair lp = *os;

              if(_written > 0)
                PublishSingleton << PARAM_DELIMITER;

              PublishSingleton << lp.Current;
                
              _written++;
              
            } // if(os)
         } // for

         // добиваем до двух датчиков минимум
         for(uint8_t i=_written; i<2;i++)
         {
           PublishSingleton << PARAM_DELIMITER;
           PublishSingleton << NO_LUMINOSITY_DATA;
         } // for
        
      }
    }
    else // есть аргументы
    {
       String s = command.GetArg(0);
       if(s == WORK_MODE) // запросили режим работы
       {
          PublishSingleton.Status = true;
          if(wantAnswer) 
          {
            PublishSingleton = WORK_MODE;
            PublishSingleton << PARAM_DELIMITER << (workMode == lightAutomatic ? WM_AUTOMATIC : WM_MANUAL);
          }
          
       } // if(s == WORK_MODE)
       else
       if(s == LIGHT_STATE_COMMAND) // CTGET=LIGHT|STATE
       {
          if(wantAnswer)
          {
            PublishSingleton = LIGHT_STATE_COMMAND;
            PublishSingleton << PARAM_DELIMITER << (bRelaysIsOn ? STATE_ON : STATE_OFF) << PARAM_DELIMITER << (workMode == lightAutomatic ? WM_AUTOMATIC : WM_MANUAL);
          }
          
          PublishSingleton.Status = true;
             
       } // LIGHT_STATE_COMMAND
        
      // разбор других команд

      
    } // if(argsCnt > 0)
    
  } // if
 
 // отвечаем на команду
    mainController->Publish(this,command);
    
  return true;
}

