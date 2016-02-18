#include "InteropStream.h"
#include "Globals.h"

InteropStream ModuleInterop;


InteropStream::InteropStream() : mainController(NULL)
{
  
}
bool InteropStream::QueryCommand(COMMAND_TYPE cType, const String& command, bool isInternalCommand)
{
  if(!mainController)
    return false;
 
  String fullCommand = CMD_PREFIX;
  fullCommand += cType == ctGET ? CMD_GET : CMD_SET;
  fullCommand += COMMAND_DELIMITER;
  fullCommand += command;
  CommandParser* cParser = mainController->GetCommandParser();

      Command cmd;
      if(cParser->ParseCommand(fullCommand, mainController->GetControllerID(), cmd))
      {
    
        Clear();
        cmd.SetInternal(isInternalCommand); // говорим, что команда - как бы от юзера, контроллер после выполнения команды перейдёт в ручной режим
        cmd.SetIncomingStream(this); // говорим, чтобы модуль плевался ответами в класс взаимодействия между модулями
        mainController->ProcessModuleCommand(cmd,false);
        return true;
      }


  return false;
}

size_t InteropStream::print(const String &s)
{
  data += s;

  return 0;
}
size_t InteropStream::println(const String &s)
{
  data += s;
  data += NEWLINE;
  return 0;
}
size_t InteropStream::write(uint8_t toWr)
{
  data += (char) toWr;
  return 1;
}

BlinkModeInterop::BlinkModeInterop()
{
  lastBlinkInterval = 0xFFFF;
}
void BlinkModeInterop::begin(uint8_t p, const String& lName)
{
  pin = p;
  loopName = F("LOOP|");
  loopName += lName;
  loopName += F("|SET|");
  
  pinCommand = F("|0|PIN|");
  pinCommand += String(pin);
  pinCommand += F("|T");

  lastBlinkInterval = 0xFFFF;
}
void BlinkModeInterop::blink(uint16_t blinkInterval)
{

 if(lastBlinkInterval == blinkInterval)
  // незачем выполнять команду с тем же интервалом
  return;

  lastBlinkInterval = blinkInterval;
  String s;
  
#ifdef USE_LOOP_MODULE 

  s  = loopName;
  s += blinkInterval;
  s += pinCommand;

  ModuleInterop.QueryCommand(ctSET,s,true);
#endif

#ifdef USE_PIN_MODULE 
      if(!blinkInterval) // не надо зажигать диод, принудительно гасим его
      {
        s = F("PIN|");
        s += String(pin);
        s += PARAM_DELIMITER;
        s += F("0");

        ModuleInterop.QueryCommand(ctSET,s,true);
      } // if
 #endif   
}

