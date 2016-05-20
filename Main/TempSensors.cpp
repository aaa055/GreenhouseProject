#include "TempSensors.h"
#include "ModuleController.h"

#if SUPPORTED_SENSORS > 0
static TempSensorSettings TEMP_SENSORS[] = { TEMP_SENSORS_PINS };
#endif

#ifndef USE_WINDOWS_SHIFT_REGISTER
static uint8_t WINDOWS_RELAYS[] = { WINDOWS_RELAYS_PINS };
#endif

void WindowState::Setup(TempSensors* parent,ModuleState* state,  uint8_t relayChannel1, uint8_t relayChannel2)
{
  RelayStateHolder = state;
  Parent = parent;

  // запоминаем, какие каналы модуля реле мы используем (в случае со сдвиговым регистром - это номера битов)
  RelayChannel1 = relayChannel1;
  RelayChannel2 = relayChannel2;

}

bool WindowState::ChangePosition(uint8_t dir, unsigned long newPos)
{
  bool bRet = false;
  
  if(IsBusy()) // заняты сменой позиции
    return bRet;

  if(CurrentPosition == newPos) // та же самая позиция запрошена, ничего не делаем
    return bRet;

  
  if(dir == dirOPEN)
  {
      if(newPos < CurrentPosition) // запросили открыть назад - невозможно.
      {
       //  do Nothing();
      }
      else
      {  
       // открываем тут
       TimerInterval = newPos - CurrentPosition;
       TimerTicks = 0;
       RequestedPosition = newPos;
       Direction = dir;
       OnMyWay = true; // поогнали!
       bRet = true;

       //Serial.println("OPEN FROM POSITION " + String(CurrentPosition) + " to " + String(newPos));
      }
  }
  else
  if(dir == dirCLOSE)
  {
      if(newPos > CurrentPosition) // запросили закрыть вперёд - невозможно
      {
       // do Nothing();
      }
      else
       {
        TimerInterval = CurrentPosition - newPos;
        TimerTicks = 0;
        RequestedPosition = newPos;
        Direction = dir;
        OnMyWay = true; // поогнали!
        bRet = true;

       // Serial.println("CLOSE FROM POSITION " + String(CurrentPosition) + " to " + String(newPos));
       }
  }
    return bRet;
}
void WindowState::SwitchRelays(uint8_t rel1State, uint8_t rel2State)
{

  // уведомляем родителя, что такой-то канал имеет такое-то состояние, он сам разберётся, что с этим делать
  Parent->SaveChannelState(RelayChannel1,rel1State);
  Parent->SaveChannelState(RelayChannel2,rel2State);

  #ifdef SAVE_RELAY_STATES
  if(RelayStateHolder) // сообщаем, что реле мы выключили или включили
  {
   uint8_t idx = RelayChannel1/8; // выясняем, какой индекс

   // теперь мы должны выяснить, в какой бит писать
   uint8_t bitNum1 = RelayChannel1 % 8;
   uint8_t bitNum2 = RelayChannel2 % 8;
   

   OneState* os = RelayStateHolder->GetState(StateRelay,idx);
   if(os)
   {
     RelayPair rp = *os;
     uint8_t curRelayStates = rp.Current; // получаем текущую маску состояния реле

     // устанавливаем нужные биты
     bitWrite(curRelayStates,bitNum1, (rel1State == RELAY_ON));
     bitWrite(curRelayStates,bitNum2, (rel2State == RELAY_ON));
     
     // записываем новую маску состояния реле
     os->Update((void*)&curRelayStates);
     
   } // if(os)
      
  } // if
  #endif
  
}
void WindowState::UpdateState(uint16_t dt)
{
  
    if(!OnMyWay) // ничего не делаем
    {
      SwitchRelays(); // держим реле выключенными
      return;
    }

   uint8_t bRelay1State, bRelay2State; // состояние выходов реле
   
   switch(Direction)
   {
      case dirOPEN:
        bRelay1State = RELAY_ON; // крутимся в одну сторону
        bRelay2State = RELAY_OFF;
        
      break;

      case dirCLOSE:
        bRelay1State = RELAY_OFF; // или в другую
        bRelay2State = RELAY_ON;
        
      break;

      case dirNOTHING:
      default:

        bRelay1State = SHORT_CIRQUIT_STATE; // накоротко, мотор не крутится
        bRelay2State = SHORT_CIRQUIT_STATE;
        
      break;
   } // switch

    TimerTicks += dt;
    if(TimerTicks >= TimerInterval) // отработали, выключаем
    {
        CurrentPosition = RequestedPosition; // сохранили текущую позицию
        TimerInterval = 0; // обнуляем интервал
        TimerTicks = 0; // и таймер
        Direction = dirNOTHING; // уже никуда не движемся

        //ВЫКЛЮЧАЕМ РЕЛЕ
        SwitchRelays();
        
        OnMyWay = false;

        return;
        
    } // if

    // продолжаем работу, включаем реле в нужное состояние
    SwitchRelays(bRelay1State,bRelay2State);
  
}
#ifdef USE_WINDOWS_SHIFT_REGISTER
void TempSensors::WriteToShiftRegister() // ПИШЕМ В СДВИГОВЫЙ РЕГИСТР
{
  // сперва проверяем, были ли изменения
  bool hasChanges = false;
  for(uint8_t i=0;i<shiftRegisterDataSize;i++)
  {
    if(shiftRegisterData[i] != lastShiftRegisterData[i])
    {
      hasChanges = true;
      break;
    }
  } // for

  if(!hasChanges)
    return;
/*
  Serial.print("Writing to shift register: ");
  
  for(uint8_t i=0;i<shiftRegisterDataSize;i++)
    Serial.print(shiftRegisterData[i],BIN);
    
  Serial.println("");
*/
   if(shiftRegisterDataSize > 0)
   {
    
    //Тут пишем в сдвиговый регистр
    
    // Отключаем вывод на регистре
    digitalWrite(WINDOWS_SHIFT_LATCH_PIN, LOW);

    // проталкиваем все байты один за другим, начиная со старшего к младшему
    
      for(uint8_t i=shiftRegisterDataSize-1;i>=0;i++)
      {
        // проталкиваем байт в регистр
        shiftOut(WINDOWS_SHIFT_DATA_PIN, WINDOWS_SHIFT_CLOCK_PIN, MSBFIRST, shiftRegisterData[i]);
      } // for

      // "защелкиваем" регистр, чтобы байт появился на его выходах
      digitalWrite(WINDOWS_SHIFT_LATCH_PIN, HIGH);
    
   } // if
  

  // теперь сохраняем последнее запомненное состояние
   for(uint8_t i=0;i<shiftRegisterDataSize;i++)
    lastShiftRegisterData[i] = shiftRegisterData[i];
}
#endif
void TempSensors::SaveChannelState(uint8_t channel, uint8_t state)
{
  #ifdef USE_WINDOWS_SHIFT_REGISTER
    //TODO: Сохраняем состояние каналов для сдвигового регистра тут!!!

     uint8_t idx = channel/8; // выясняем, какой индекс в массиве байт
   // теперь мы должны выяснить, в какой бит писать
    uint8_t bitNum = channel % 8;

    // пишем в нужный байт и в нужный бит нужное состояние
    uint8_t bt = shiftRegisterData[idx];
    bitWrite(bt,bitNum, state);
    shiftRegisterData[idx] = bt;
    
    
  #else
    // просто управляем пинами, поэтому напрямую пишем в пины
    digitalWrite(WINDOWS_RELAYS[channel],state);
  #endif
}
void TempSensors::SetupWindows()
{
  // настраиваем фрамуги
  for(uint8_t i=0, j=0;i<SUPPORTED_WINDOWS;i++, j+=2)
  {
      // раздаём каналы реле: первому окну - 0,1, второму - 2,3 и т.д.
      Windows[i].Setup(this, &State,j,j+1);

      #ifdef USE_WINDOWS_SHIFT_REGISTER // если используем сдвиговые регистры
        // ничего не делаем, поскольку у нас все реле будут выключены после первоначальной настройки
      #else
        // просто настраиваем пины
          uint8_t pin1 = WINDOWS_RELAYS[j];
          uint8_t pin2 = WINDOWS_RELAYS[j+1];
        
          pinMode(pin1,OUTPUT);
          pinMode(pin2, OUTPUT);
        
          // выключаем реле
          digitalWrite(pin1,RELAY_OFF);
          digitalWrite(pin2,RELAY_OFF);        
     #endif
      
  } // for
}

void TempSensors::Setup()
{
  // настройка модуля тут
   workMode = wmAutomatic; // автоматический режим работы по умолчанию
   
#ifdef USE_WINDOWS_MANUAL_MODE_DIODE
  blinker.begin(DIODE_WINDOWS_MANUAL_MODE_PIN,F("SM"));  // настраиваем блинкер на нужный пин
#endif  


  lastUpdateCall = 0;
  smallSensorsChange = 0;
  
   // добавляем датчики температуры
   #if SUPPORTED_SENSORS > 0
   tempData.Whole = 0;
   tempData.Fract = 0;
   for(uint8_t i=0;i<SUPPORTED_SENSORS;i++)
   {
    State.AddState(StateTemperature,i);
    // запускаем конвертацию с датчиков при старте, через 2 секунды нам вернётся измеренная температура
    tempSensor.begin(TEMP_SENSORS[i].pin);

    tempSensor.setResolution(temp12bit); // устанавливаем разрешение датчика
    
    tempSensor.readTemperature(&tempData,(DSSensorType)TEMP_SENSORS[i].type);
   }
   #endif

   #ifdef SAVE_RELAY_STATES   
   // добавляем N восьмиканальных состояний реле
   uint8_t relayCnt = (SUPPORTED_WINDOWS*2)/8;
   if((SUPPORTED_WINDOWS*2) > 8 && (SUPPORTED_WINDOWS*2) % 8)
    relayCnt++;

   if((SUPPORTED_WINDOWS*2) < 9)
    relayCnt = 1;
    
   for(uint8_t i=0;i<relayCnt;i++)
    State.AddState(StateRelay,i);
   #endif  

  
   SetupWindows(); // настраиваем фрамуги

   #ifdef USE_WINDOWS_SHIFT_REGISTER

    // настраиваем пины для сдвигового регистра на выход
    pinMode(WINDOWS_SHIFT_LATCH_PIN,OUTPUT);
    pinMode(WINDOWS_SHIFT_DATA_PIN,OUTPUT);
    pinMode(WINDOWS_SHIFT_CLOCK_PIN,OUTPUT);
   
    // настраиваем кол-во байт, в котором мы будем держать состояние каналов для сдвигового регистра.
    // у нас для каждого окна - два канала, соответственно, общее кол-во бит - это
    // SUPPORTED_WINDOWS*2. Исходя из этого - легко посчитать кол-во байт, необходимых
    // для хранения данных.
    shiftRegisterDataSize =  (SUPPORTED_WINDOWS*2)/8;
    if((SUPPORTED_WINDOWS*2) > 8 && (SUPPORTED_WINDOWS*2) % 8)
      shiftRegisterDataSize++;

    shiftRegisterData = new uint8_t[shiftRegisterDataSize];
    lastShiftRegisterData = new uint8_t[shiftRegisterDataSize];
    // теперь в каждый бит этих байт записываем значение RELAY_OFF для shiftRegisterData,
    // и значение RELAY_ON для lastShiftRegisterData.
    // надо именно побитово, т.к. значение RELAY_OFF может быть 1, и в этом случае
    // все биты должны быть установлены в 1.

      uint8_t bOff = 0;
      uint8_t bOn = 0;
      for(uint8_t j=0;j<8;j++)
      {
        bOff |= (RELAY_OFF << j);
        bOn |= (RELAY_ON << j);
      }

    for(uint8_t i=0;i<shiftRegisterDataSize;i++)
    {      
      // сохранили разные значения первоначально, поскольку мы хотим записать их впервые
      shiftRegisterData[i] = bOff;
      lastShiftRegisterData[i] = bOn;
      
    } // for
      
    WriteToShiftRegister(); // пишем первоначальное состояние реле в сдвиговый регистр
    
   #endif // USE_WINDOWS_SHIFT_REGISTER


   SAVE_STATUS(WINDOWS_MODE_BIT,1); // сохраняем режим работы окон
   

 }
void TempSensors::Update(uint16_t dt)
{ 
#ifdef USE_WINDOWS_MANUAL_MODE_DIODE
  blinker.update();
#endif  

  for(uint8_t i=0;i<SUPPORTED_WINDOWS;i++) // обновляем каналы управления фрамугами
  {
      Windows[i].UpdateState(dt);
  } // for 

 #ifdef USE_WINDOWS_SHIFT_REGISTER
  // пишем в сдвиговый регистр, если есть изменения
  WriteToShiftRegister();
 #endif 


  lastUpdateCall += dt;
  if(lastUpdateCall < TEMP_UPDATE_INTERVAL) // обновляем согласно настроенному интервалу
    return;
  else
    lastUpdateCall = 0;

  // опрашиваем наши датчики
  #if SUPPORTED_SENSORS > 0
  Temperature t;
  for(uint8_t i=0;i<SUPPORTED_SENSORS;i++)
  {
    t.Value = NO_TEMPERATURE_DATA;
    t.Fract = 0;
    
    tempSensor.begin(TEMP_SENSORS[i].pin);
    if(tempSensor.readTemperature(&tempData,(DSSensorType)TEMP_SENSORS[i].type))
    {
      t.Value = tempData.Whole;
    
      if(tempData.Negative)
        t.Value = -t.Value;

      t.Fract = tempData.Fract + smallSensorsChange;
      
    }
    State.UpdateState(StateTemperature,i,(void*)&t); // обновляем состояние температуры, индексы датчиков у нас идут без дырок, поэтому с итератором цикла вызывать можно
  } // for
  #endif

  smallSensorsChange = 0;


}
bool  TempSensors::ExecCommand(const Command& command, bool wantAnswer)
{
  GlobalSettings* sett = mainController->GetSettings();
  if(wantAnswer) 
    PublishSingleton = PARAMS_MISSED;
      
  String commandRequested;

  if(command.GetType() == ctSET) // напрямую запись в датчики запрещена, пишем только в состояние каналов
  {
    uint8_t argsCnt = command.GetArgsCount();
    
    if(argsCnt > 2)
    {
      commandRequested = command.GetArg(0);
      commandRequested.toUpperCase();
      if(commandRequested == PROP_WINDOW) // надо записать состояние окна, от нас просят что-то сделать
      {
        if(command.IsInternal() // если команда пришла от другого модуля
        && workMode == wmManual) // и мы в ручном режиме, то
        {
          // просто игнорируем команду, потому что нами управляют в ручном режиме
          // мигаем светодиодом на 6 пине
          
         }
        else
        {
          if(!command.IsInternal()) // пришла команда от пользователя,
          {
            workMode = wmManual; // переходим на ручной режим работы
            #ifdef USE_WINDOWS_MANUAL_MODE_DIODE
            // мигаем светодиодом на 6 пине
             blinker.blink(WORK_MODE_BLINK_INTERVAL);
            #endif 
          }

          String token = command.GetArg(1);
          token.toUpperCase();

          String whichCommand = command.GetArg(2); // какую команду запросили?
          whichCommand.toUpperCase();
          bool bOpen = (whichCommand == STATE_OPEN); // запросили открытие фрамуг?
          
          bool bAll = (token == ALL); // на все окна распространяется запрос?
          bool bIntervalAsked = token.indexOf("-") != -1; // запросили интервал каналов?
          uint8_t channelIdx = token.toInt();
          unsigned long interval = sett->GetOpenInterval();
          
          if(command.GetArgsCount() > 3)
            interval = (unsigned long) atol(command.GetArg(3));//String(command.GetArg(3)).toInt(); // получили интервал для работы реле

 
          PublishSingleton.Status = true;
          // откуда до куда шаримся
          uint8_t from = 0;
          uint8_t to = SUPPORTED_WINDOWS;


          if(bIntervalAsked)
          {
             // парсим интервал
             int delim = token.indexOf("-");
             from = token.substring(0,delim).toInt();
             to = token.substring(delim+1,token.length()).toInt();
             
          }

          // правильно расставляем шаги - от меньшего к большему
          uint8_t tmp = min(from,to);
          to = max(from,to);
          from = tmp;

             to++; // включаем to в интервал, это надо, если пришла команда интервала, например, 2-3, тогда в этом случае опросятся третий и четвертый каналы
             if(to >= SUPPORTED_WINDOWS)
              to = SUPPORTED_WINDOWS;
          
          if(bAll || bIntervalAsked)
          {
            // по всем каналам шаримся
            bool bAnyPosChanged = false;
            
            for(uint8_t i=from;i<to;i++)
            {
              if(Windows[i].ChangePosition(bOpen? dirOPEN : dirCLOSE,bOpen ? interval : 0))
              {
                if(wantAnswer) 
                  PublishSingleton = (bOpen ? STATE_OPENING : STATE_CLOSING);
                bAnyPosChanged = true;
              } 
            } // for
            
            if(!bAnyPosChanged) // позицию окон не сменили, значит, они либо в этой позиции, либо в процессе смены позиции
            {
              // проверяем, заняты ли окна чем-то
              if(Windows[from].IsBusy())
               {
                // окно занято сменой позиции
                if(wantAnswer) 
                  PublishSingleton = (Windows[from].GetDirection() == dirOPEN ? STATE_OPENING : STATE_CLOSING);

                SAVE_STATUS(WINDOWS_STATUS_BIT,Windows[from].GetDirection() == dirOPEN ? 1 : 0); // сохраняем состояние окон
               }
               else
               {
                // окно не сменяет позицию
                if(wantAnswer) 
                  PublishSingleton =  (bOpen ? STATE_OPEN : STATE_CLOSED);

                SAVE_STATUS(WINDOWS_STATUS_BIT,bOpen ? 1 : 0); // сохраняем состояние окон  
               }
               
              SAVE_STATUS(WINDOWS_MODE_BIT,workMode == wmAutomatic ? 1 : 0); // сохраняем режим работы окон

            } // не смогли сменить позицию
            else
            {
              // сменили позицию, пишем в лог действие
              mainController->Log(this,commandRequested + String(PARAM_DELIMITER) + whichCommand);

              SAVE_STATUS(WINDOWS_STATUS_BIT,bOpen ? 1 : 0); // сохраняем состояние окон
              SAVE_STATUS(WINDOWS_MODE_BIT,workMode == wmAutomatic ? 1 : 0); // сохраняем режим работы окон

            } // else

          }
          else
          { 
            
              if(Windows[channelIdx].ChangePosition( bOpen ? dirOPEN : dirCLOSE, interval) ) // смогли сменить позицию окна
              {
                  // сменили позицию, пишем в лог действие
                  mainController->Log(this,commandRequested + String(PARAM_DELIMITER) + whichCommand);
                  if(wantAnswer) 
                    PublishSingleton = (bOpen ? STATE_OPENING : STATE_CLOSING);

              SAVE_STATUS(WINDOWS_STATUS_BIT,bOpen ? 1 : 0); // сохраняем состояние окон
              SAVE_STATUS(WINDOWS_MODE_BIT,workMode == wmAutomatic ? 1 : 0); // сохраняем режим работы окон
                    
              }
               else
               {
                // позицию окна не сменили, смотрим - занято ли оно?
                
                    if(Windows[channelIdx].IsBusy()) // занято, возвращаем состояние - открывается или закрывается
                    {
                       if(wantAnswer) 
                        PublishSingleton = (Windows[channelIdx].GetDirection() == dirOPEN ? STATE_OPENING : STATE_CLOSING);
    
                       SAVE_STATUS(WINDOWS_STATUS_BIT,Windows[channelIdx].GetDirection() == dirOPEN ? 1 : 0); // сохраняем состояние окон  
                        
                    }   
                    else // окно ничем не занято, возвращаем положение - открыто или закрыто
                    {
                      if(wantAnswer) 
                          PublishSingleton =  (bOpen ? STATE_OPEN : STATE_CLOSED);
    
                      SAVE_STATUS(WINDOWS_STATUS_BIT,bOpen ? 1 : 0); // сохраняем состояние окон
                    }
    
                    SAVE_STATUS(WINDOWS_MODE_BIT,workMode == wmAutomatic ? 1 : 0); // сохраняем режим работы окон
                
               } // else не сменили позицию
          }

        } // else can process
        
      } // if PROP_WINDOW
      else
      if(commandRequested == TEMP_SETTINGS) // установить температуры закрытия/открытия
      {
        uint8_t tOpen = (uint8_t) atoi(command.GetArg(1));//String(command.GetArg(1)).toInt();
        uint8_t tClose = (uint8_t) atoi(command.GetArg(2));//String(command.GetArg(2)).toInt();

        sett->SetOpenTemp(tOpen);
        sett->SetCloseTemp(tClose);
        sett->Save();
        
        PublishSingleton.Status = true;
        if(wantAnswer) 
        {
          PublishSingleton = TEMP_SETTINGS;
          PublishSingleton << PARAM_DELIMITER << REG_SUCC;
        }
      } // TEMP_SETTINGS
      
    } // if(argsCnt > 2)
    else if(argsCnt > 1)
    {
      commandRequested = command.GetArg(0);
      commandRequested.toUpperCase();

      if (commandRequested == WORK_MODE)
      {
        // запросили установить режим работы
        commandRequested = command.GetArg(1);
        commandRequested.toUpperCase();


        if(commandRequested == WM_AUTOMATIC)
        {
          PublishSingleton.Status = true;
          if(wantAnswer) 
          {
            PublishSingleton = WORK_MODE;
            PublishSingleton << PARAM_DELIMITER << commandRequested;
          }
          workMode = wmAutomatic;
          smallSensorsChange = 1;
#ifdef USE_WINDOWS_MANUAL_MODE_DIODE        
          blinker.blink();
#endif          
        }
        else if(commandRequested == WM_MANUAL)
        {
          PublishSingleton.Status = true;
          if(wantAnswer) 
          {
            PublishSingleton = WORK_MODE;
            PublishSingleton << PARAM_DELIMITER << commandRequested;
          }
          workMode = wmManual;
          smallSensorsChange = 1;
#ifdef USE_WINDOWS_MANUAL_MODE_DIODE
          blinker.blink(WORK_MODE_BLINK_INTERVAL);
#endif          
        }
        
        SAVE_STATUS(WINDOWS_MODE_BIT,workMode == wmAutomatic ? 1 : 0); // сохраняем режим работы окон
        
      } // WORK_MODE
      else if(commandRequested == WM_INTERVAL) // запросили установку интервала
      {
              unsigned long newInt = (unsigned long) atol(command.GetArg(1));//String(command.GetArg(1)).toInt();
              if(newInt > 0)
              {
                //СОХРАНЕНИЕ ИНТЕРВАЛА В НАСТРОЙКАХ
                sett->SetOpenInterval(newInt);
                sett->Save();
                
                PublishSingleton.Status = true;
                if(wantAnswer) 
                {
                  PublishSingleton = WM_INTERVAL;
                  PublishSingleton << PARAM_DELIMITER << REG_SUCC;
                }
              } // if
      } // WM_INTERVAL
    } // argsCnt > 1
  } // SET
  else
  if(command.GetType() == ctGET) // запросили показание датчика
  {
      uint8_t argsCnt = command.GetArgsCount();
       
      if(argsCnt > 1)
      {
        // параметров хватает
        // проверяем, есть ли там запрос на кол-во, или просто индекс?
        commandRequested = command.GetArg(0);
        commandRequested.toUpperCase();

          if(commandRequested == PROP_TEMP) // обращение по температуре
          {
              commandRequested = command.GetArg(1);
              commandRequested.toUpperCase();

              if(commandRequested == PROP_TEMP_CNT) // кол-во датчиков
              {
                 PublishSingleton.Status = true;
                 if(wantAnswer) 
                 {
                  uint8_t _tempCnt = State.GetStateCount(StateTemperature);
                  PublishSingleton = PROP_TEMP_CNT;
                  PublishSingleton << PARAM_DELIMITER << _tempCnt;
                 }
              } // if
              else // запросили по индексу или запрос ALL
              {
                if(commandRequested == ALL)
                {
                  // все датчики
                  PublishSingleton.Status = true;
                  if(wantAnswer)
                  { 
                   PublishSingleton = PROP_TEMP;
                    
                    // получаем значение всех датчиков
                    uint8_t _tempCnt = State.GetStateCount(StateTemperature);
                    
                    for(uint8_t i=0;i<_tempCnt;i++)
                    {
  
                       OneState* os = State.GetStateByOrder(StateTemperature,i);
                       if(os)
                       {
                          TemperaturePair tp = *os;
                          PublishSingleton << PARAM_DELIMITER << (tp.Current);
                       } // if(os)
                    } // for
                  } // want answer
                  
                }
                else
                {
                   // по индексу
                uint8_t sensorIdx = commandRequested.toInt();
                if(sensorIdx >= State.GetStateCount(StateTemperature) )
                {
                   if(wantAnswer)
                      PublishSingleton = NOT_SUPPORTED; // неверный индекс
                }
                 else
                  {
                    // получаем текущее значение датчика
                    PublishSingleton.Status = true;

                    if(wantAnswer)
                    {
                        OneState* os = State.GetStateByOrder(StateTemperature,sensorIdx);
                        if(os)
                        {
                          TemperaturePair tp = *os;
                          PublishSingleton = PROP_TEMP;
                          PublishSingleton << PARAM_DELIMITER  << sensorIdx << PARAM_DELIMITER << (tp.Current);
                        }
                    } // if(wantAnswer)
                  }
                } // else
              } // else
              
          } // if
          else if(commandRequested == PROP_WINDOW) // статус окна
          {
             commandRequested = command.GetArg(1);
             commandRequested.toUpperCase();

             if(commandRequested == PROP_WINDOW_CNT)
             {
                    PublishSingleton.Status = true;
                    if(wantAnswer)
                    {
                      PublishSingleton = PROP_WINDOW_CNT;
                      PublishSingleton << PARAM_DELIMITER  << SUPPORTED_WINDOWS;
                    }

             }
            else // запросили по индексу
            {
              //TODO: Тут может быть запрос ALL, а не только индекс!!!
              
             uint8_t windowIdx = commandRequested.toInt();
             if(windowIdx >= SUPPORTED_WINDOWS)
             {
                if(wantAnswer)
                  PublishSingleton = NOT_SUPPORTED; // неверный индекс
             }
              else
              {
                WindowState* ws = &(Windows[windowIdx]);
                String sAdd;
                if(ws->IsBusy())
                {
                  //куда-то едем
                  sAdd = ws->GetDirection() == dirOPEN ? STATE_OPENING : STATE_CLOSING;
                  
                } // if
                else
                {
                    // никуда не едем
                    if(ws->GetCurrentPosition() > 0)
                      sAdd = STATE_OPEN;
                    else
                      sAdd = STATE_CLOSED;
                } // else
                
                
                PublishSingleton.Status = true;
                if(wantAnswer)
                {
                  PublishSingleton = PROP_WINDOW;
                  PublishSingleton << PARAM_DELIMITER << commandRequested << PARAM_DELIMITER << sAdd;
                }
              }
            } // else
          } // else
         
        
      } // if
      else if(argsCnt > 0)
      {
        commandRequested = command.GetArg(0);
        commandRequested.toUpperCase();

        if(commandRequested == WORK_MODE) // запросили режим работы
        {
          
          PublishSingleton.Status = true;
          if(wantAnswer)
          {
            PublishSingleton = WORK_MODE;
            PublishSingleton << PARAM_DELIMITER << (workMode == wmAutomatic ? WM_AUTOMATIC : WM_MANUAL);
          }
          
        } // if
        else
        if(commandRequested == WM_INTERVAL) // запросили интервал срабатывания форточек
        {
          PublishSingleton.Status = true;
          if(wantAnswer)
          {
            PublishSingleton = WM_INTERVAL;
            PublishSingleton << PARAM_DELIMITER  << (sett->GetOpenInterval());
          }
        } // WM_INTERVAL
        else
        if(commandRequested == TEMP_SETTINGS) // запросили температуры открытия и закрытия
        {
          PublishSingleton.Status = true;
          
          if(wantAnswer)
          {
            PublishSingleton = TEMP_SETTINGS;
            PublishSingleton << PARAM_DELIMITER << (sett->GetOpenTemp()) << PARAM_DELIMITER << (sett->GetCloseTemp());
          }
        }
        
      } // else if(argsCnt > 0)
  } // if GET
  
 // отвечаем на команду
    mainController->Publish(this,command);

  return PublishSingleton.Status;
}


