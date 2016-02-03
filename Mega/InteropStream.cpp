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
}
