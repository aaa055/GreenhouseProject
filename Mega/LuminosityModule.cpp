#include "LuminosityModule.h"
#include "ModuleController.h"

BH1750Support::BH1750Support()
{

}
void BH1750Support::begin(BH1750Mode mode)
{
  Wire.begin();
  writeByte(BH1750PowerOn); // включаем датчик
  ChangeMode(mode); 
}
void BH1750Support::ChangeMode(BH1750Mode mode) // смена режима работы
{
   writeByte((uint8_t)mode);
  _delay_ms(10);
}

void BH1750Support::writeByte(uint8_t toWrite) 
{
  Wire.beginTransmission(BH1750Address);
  BH1750_WIRE_WRITE(toWrite);
  Wire.endTransmission();
}
uint16_t BH1750Support::GetCurrentLuminosity() 
{

  uint16_t curLuminosity;

  Wire.beginTransmission(BH1750Address); // начинаем опрос датчика освещенности
  Wire.requestFrom(BH1750Address, 2); // ждём два байта

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
  lightMeter.begin(); // запускаем датчик освещенности
  // настройка модуля тут
  
 }

void LuminosityModule::Update(uint16_t dt)
{ 
  // обновление модуля тут

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
      answer = String(lightMeter.GetCurrentLuminosity());
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

