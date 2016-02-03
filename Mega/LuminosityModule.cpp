#include "LuminosityModule.h"
#include "ModuleController.h"

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
uint16_t BH1750Support::GetCurrentLuminosity() 
{

  uint16_t curLuminosity;

  Wire.beginTransmission(deviceAddress); // начинаем опрос датчика освещенности
  Wire.requestFrom(deviceAddress, 2); // ждём два байта

  // читаем два байта
  curLuminosity = BH1750_WIRE_READ();
  curLuminosity <<= 8;
  curLuminosity |= BH1750_WIRE_READ();

  Wire.endTransmission();

  curLuminosity = curLuminosity/1.2; // конвертируем в люксы

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
  
 }

void LuminosityModule::Update(uint16_t dt)
{ 
  // обновление модуля тут
  lastUpdateCall += dt;
  if(lastUpdateCall < 2000) // не будем обновлять чаще, чем раз в две секунды
  {
    return;
  }
  lastUpdateCall = 0;

  #if LIGHT_SENSORS_COUNT > 0
    uint16_t lum = lightMeter.GetCurrentLuminosity();
    State.UpdateState(StateLuminosity,0,(void*)&lum);
  #endif
    
  #if LIGHT_SENSORS_COUNT > 1
    lum = lightMeter2.GetCurrentLuminosity();
    State.UpdateState(StateLuminosity,1,(void*)&lum);
  #endif

}

bool  LuminosityModule::ExecCommand(const Command& command)
{
  ModuleController* c = GetController();
  GlobalSettings* settings = c->GetSettings();
  
  String answer = UNKNOWN_COMMAND;
  
  bool answerStatus = false; 
  
  if(command.GetType() == ctSET) 
  {
      answerStatus = false;
      answer = NOT_SUPPORTED;      
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
    {
      // разбор других команд
    } // else
    
  } // if
 
 // отвечаем на команду
    SetPublishData(&command,answerStatus,answer); // готовим данные для публикации
    c->Publish(this);
    
  return answerStatus;
}

