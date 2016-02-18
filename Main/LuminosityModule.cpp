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

  long curLuminosity = -1;

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
  lightMeter.begin(); // запускаем первый датчик освещенности
  State.AddState(StateLuminosity,0); // добавляем в состояние модуля флаг, что мы поддерживаем освещенность, и у нас есть датчик с индексов 0
  #endif

  #if LIGHT_SENSORS_COUNT > 1
  lightMeter2.begin(BH1750Address2);
  State.AddState(StateLuminosity,1); // добавляем в состояние модуля флаг, что мы поддерживаем освещенность, и у нас есть датчик с индексов 1
  #endif

  
  // настройка модуля тут
  controller = GetController();
  settings = controller->GetSettings();

  workMode = lightAutomatic; // автоматический режим работы
  bRelaysIsOn = false; // все реле выключены

  blinker.begin(DIODE_LIGHT_MANUAL_MODE_PIN,F("LX")); // настраиваем блинкер на нужный пин
  
   uint8_t relayCnt = LAMP_RELAYS_COUNT/8; // устанавливаем кол-во каналов реле
   if(LAMP_RELAYS_COUNT > 8 && LAMP_RELAYS_COUNT % 8)
    relayCnt++;

  if(LAMP_RELAYS_COUNT < 9)
    relayCnt = 1;
    
   for(uint8_t i=0;i<relayCnt;i++) // добавляем состояния реле (каждый канал - 8 реле)
    State.AddState(StateRelay,i);  


 // выключаем все реле
  for(uint8_t i=0;i<LAMP_RELAYS_COUNT;i++)
  {
    pinMode(LAMP_RELAYS[i],OUTPUT);
    digitalWrite(LAMP_RELAYS[i],RELAY_OFF);
  
    uint8_t idx = i/8;
    uint8_t bitNum1 = i % 8;
    OneState* os = State.GetState(StateRelay,idx);
    if(os)
    {
      uint8_t curRelayStates = *((uint8_t*) os->Data);
      bitWrite(curRelayStates,bitNum1, bRelaysIsOn);
      State.UpdateState(StateRelay,idx,(void*)&curRelayStates);
    }
  } // for
    
       
 }
void LuminosityModule::Update(uint16_t dt)
{ 
  // обновление модуля тут  

 // обновляем состояние всех реле управления досветкой
  for(uint8_t i=0;i<LAMP_RELAYS_COUNT;i++)
  {
    digitalWrite(LAMP_RELAYS[i],bRelaysIsOn ? RELAY_ON : RELAY_OFF);
  
    uint8_t idx = i/8;
    uint8_t bitNum1 = i % 8;
    OneState* os = State.GetState(StateRelay,idx);
    if(os)
    {
      uint8_t curRelayStates = *((uint8_t*) os->Data);
      bitWrite(curRelayStates,bitNum1, bRelaysIsOn);
      State.UpdateState(StateRelay,idx,(void*)&curRelayStates);
    }
  } // for 


  lastUpdateCall += dt;
  if(lastUpdateCall < 2000) // не будем обновлять чаще, чем раз в две секунды
  {
    return;
  }
  lastUpdateCall = 0;

  long lum = -1;

  #if LIGHT_SENSORS_COUNT > 0
    lum = lightMeter.GetCurrentLuminosity();
    State.UpdateState(StateLuminosity,0,(void*)&lum);
  #endif
    
  #if LIGHT_SENSORS_COUNT > 1
    lum = lightMeter2.GetCurrentLuminosity();
    State.UpdateState(StateLuminosity,1,(void*)&lum);
  #endif   

}

bool  LuminosityModule::ExecCommand(const Command& command)
{
  
  String answer = UNKNOWN_COMMAND;
  
  bool answerStatus = false; 
  
  uint8_t argsCnt = command.GetArgsCount();
  
  if(command.GetType() == ctSET) 
  {
      answerStatus = false;
      answer = PARAMS_MISSED;

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
                // мигаем светодиодом на 8 пине
                blinker.blink(WORK_MODE_BLINK_INTERVAL);
              }

            bRelaysIsOn = true; // включаем реле досветки
            
            answerStatus = true;
            answer = STATE_ON;
            
           } // else
 
          
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
                // мигаем светодиодом на 8 пине
                blinker.blink(WORK_MODE_BLINK_INTERVAL);
              }

            bRelaysIsOn = false; // выключаем реле досветки
            
            answerStatus = true;
            answer = STATE_OFF;
            
           } // else
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
                 // мигаем светодиодом на 8 пине
               blinker.blink(WORK_MODE_BLINK_INTERVAL);
              }
              else
              if(s == WM_AUTOMATIC)
              {
                // попросили перейти в автоматический режим работы
                workMode = lightAutomatic; // переходим на автоматический режим работы
                 // гасим диод на 8 пине
                blinker.blink();
              }

              answerStatus = true;
              answer = WORK_MODE; answer += PARAM_DELIMITER;
              answer += workMode == lightAutomatic ? WM_AUTOMATIC : WM_MANUAL;
              
           } // if (argsCnt > 1)
         } // WORK_MODE
         
      } // if(argsCnt > 0)
  }
  else
  if(command.GetType() == ctGET) //получить данные
  {

    String t = command.GetRawArguments();
    t.toUpperCase();
    if(t == GetID()) // нет аргументов, попросили дать показания с датчика
    {
      
      answerStatus = true;
      answer = 
     #if LIGHT_SENSORS_COUNT > 0
      String(lightMeter.GetCurrentLuminosity());
      #else
        String(F("-1"));
      #endif
      
      answer += PARAM_DELIMITER;
      answer += 
      #if LIGHT_SENSORS_COUNT > 1
        String(lightMeter2.GetCurrentLuminosity());
      #else
        String(F("-1"));
      #endif
    }
    else
    if(argsCnt > 0)
    {
       String s = command.GetArg(0);
       if(s == WORK_MODE) // запросили режим работы
       {
          String wm = workMode == lightAutomatic ? WM_AUTOMATIC : WM_MANUAL;
          answerStatus = true;
          answer = String(WORK_MODE) + PARAM_DELIMITER + wm;
          
       } // if(s == WORK_MODE)
       else
       if(s == LIGHT_STATE_COMMAND) // CTGET=LIGHT|STATE
       {
          answer = LIGHT_STATE_COMMAND;
          answer += PARAM_DELIMITER;
          answer += bRelaysIsOn ? STATE_ON : STATE_OFF;
          answer += PARAM_DELIMITER;
          answer += workMode == lightAutomatic ? WM_AUTOMATIC : WM_MANUAL;
          
          answerStatus = true;
             
       } // LIGHT_STATE_COMMAND
        
      // разбор других команд

      
    } // if(argsCnt > 0)
    
  } // if
 
 // отвечаем на команду
    SetPublishData(&command,answerStatus,answer); // готовим данные для публикации
    controller->Publish(this);
    
  return answerStatus;
}

