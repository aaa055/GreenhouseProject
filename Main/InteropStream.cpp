#include "InteropStream.h"
#include "Globals.h"

InteropStream ModuleInterop;


InteropStream::InteropStream() : Stream(), mainController(NULL)
{
  
}
bool InteropStream::QueryCommand(COMMAND_TYPE cType, const String& command, bool isInternalCommand,bool wantAnwer)
{
 
  if(!mainController)
    return false; 
 
 CHECK_PUBLISH_CONSISTENCY; // проверяем структуру публикации на предмет того, что там ничего нет

  //TODO: тут налицо оверхед, т.к. мы вынуждены собирать строку полной команды,
  // а это совершенно ни к чему.
  String fullCommand = CMD_PREFIX;
  fullCommand += cType == ctGET ? CMD_GET : CMD_SET;
  fullCommand += COMMAND_DELIMITER;
  fullCommand += command;
  CommandParser* cParser = mainController->GetCommandParser();

  Command cmd;
  if(cParser->ParseCommand(fullCommand, mainController->GetControllerID(), cmd))
  {

    cmd.SetInternal(isInternalCommand); // устанавливаем флаг команды
    data = F("");    
    if(wantAnwer)
    {
      cmd.SetIncomingStream(this); // просим контроллер опубликовать ответ в нас - мы сохраним ответ в data
    }
    else
      cmd.SetIncomingStream(NULL);

    mainController->ProcessModuleCommand(cmd,NULL,false);

    return true;
  }


  return false;
}

size_t InteropStream::write(uint8_t toWr)
{
  data += (char) toWr;
  return 1;
}
BlinkModeInterop::BlinkModeInterop()
{
  lastBlinkInterval = 0xFFFF;
  needUpdate = false;
}
void BlinkModeInterop::update()
{
  
  if(!needUpdate)
    return;

 needUpdate = false;   

  String s;
  
#ifdef USE_LOOP_MODULE 

  s  = loopName;
  s += lastBlinkInterval;
  s += pinCommand;

  ModuleInterop.QueryCommand(ctSET,s,false,false);
#endif

#ifdef USE_PIN_MODULE 
      if(!lastBlinkInterval) // не надо зажигать диод, принудительно гасим его
      {
        s = F("PIN|");
        s += String(pin);
        s += PARAM_DELIMITER;
        s += F("0");

        ModuleInterop.QueryCommand(ctSET,s,false,false);
 
      } // if
 #endif   

    
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

  needUpdate = true;
  lastBlinkInterval = blinkInterval;
  

}

