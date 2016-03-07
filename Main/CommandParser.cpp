#include <Arduino.h>
#include "CommandParser.h"

String Command::_CMD_GET;
String Command::_CMD_SET;

Command::Command()
{

  Clear();  
}
Command::~Command()
{
  Clear();
}
uint8_t Command::GetCommandType(const String& command)
{
  if(command == Command::_CMD_SET)
  return ctSET;
  else if(command == Command::_CMD_GET)
    return ctGET;

  return ctUNKNOWN;
}
String Command::GetStringType() const
{
  switch(Type)
  {
    case ctGET:
      return Command::_CMD_GET;
    case ctSET:
      return Command::_CMD_SET;
    default:
    case ctUNKNOWN:
      return F("");
  }
}
size_t Command::GetArgsCount() const
{ 
  return arguments.size();
}
 const char* Command::GetArg(size_t idx) const
{
  if(idx < arguments.size())
    return arguments[idx];

 return NULL;
}
void Command::Construct(const String& id,const String& arg, uint8_t ct,uint8_t dest)
{
  Clear(); // сбрасываем все настройки
  
    Type = ct;
   // Arg = arg;
    ModuleID = id;
    Destination = dest;

    // разбиваем на аргументы
        int curIdx = 0;
        String tmpStr = arg;
        String param;
        
        while(curIdx != -1)
        {
          curIdx = tmpStr.indexOf(PARAM_DELIMITER);
          if(curIdx == -1)
          {
           if(tmpStr.length() > 0)
           {
              char* newArg = new char[tmpStr.length() + 1];
              strcpy(newArg,tmpStr.c_str());
    
              arguments.push_back(newArg);
           }
              
            break;
          } // if
          param = tmpStr.substring(0,curIdx);
          tmpStr = tmpStr.substring(curIdx+1,tmpStr.length());
          if(param.length() > 0)
          {
              char* newArg = new char[param.length() + 1];
              strcpy(newArg,param.c_str());

              arguments.push_back(newArg);
          }

         if( arguments.size() >= MAX_ARGS_IN_LIST)
          break;
            
        } // while

}
void Command::Construct(const String& id,const String& arg, const String& ct,uint8_t dest)
{
 //  StringType = ct;
   Construct(id, arg,GetCommandType(ct),dest);
}

void Command::Clear()
{
 // ArgsCount = 0;
  Type = ctUNKNOWN;
  ModuleID = F("");
//  Arg = F("");
  IncomingStream = NULL;
  bIsInternal = false;
  Destination = cdUNKNOWN;

  size_t sz = arguments.size();
  for(size_t i=0;i<sz;i++)
  {
    char* ch = arguments[i];
    delete[] ch;
  } // for
  arguments.Clear();

}
  
CommandParser::CommandParser()
{  
  Clear();
}

void CommandParser::Clear()
{
commandBuf = F("");
}

bool CommandParser::ParseCommand(const String& command,const String& ourID,Command& outCommand)
{
  Clear(); // clear first

  //TODO: тяжёлая функция, много локальных переменных и дёрганья substring - переписать!
  
  commandBuf = command;
  
  if(commandBuf.length() < MIN_COMMAND_LENGTH)
    return false;

  String tmpBuf = commandBuf.substring(0,CMD_PREFIX_LEN);
  String commandType,commandArg,commandModuleId;
  
  if(!(tmpBuf == CMD_PREFIX || tmpBuf == CHILD_PREFIX) ) // not right command
    return false;

  uint8_t dest = tmpBuf == CMD_PREFIX ? cdCONTROLLER : cdCHILDMODULE;

  tmpBuf = commandBuf.substring(CMD_PREFIX_LEN,CMD_PREFIX_LEN+CMD_TYPE_LEN);

  if(!(tmpBuf == Command::_CMD_SET || tmpBuf == Command::_CMD_GET))
    return false;

    commandType = tmpBuf;
    commandArg = commandBuf.substring(CMD_PREFIX_LEN+CMD_TYPE_LEN+1);

    if(commandArg.length() > 0)
    {
      // ищем ID модуля
      int idx = commandArg.indexOf(PARAM_DELIMITER);
      if(idx != -1)
      {
        // нашли, сохраняем
        commandModuleId = commandArg.substring(0,idx);
        commandArg = commandArg.substring(idx+1);

        if(dest == cdCHILDMODULE)// работаем как дочерний модуль
        {
          // значит, первым параметром пришло наше имя, а всё остальное - уже команда

              // проверяем, наш ли?
              if(commandModuleId != ourID) // не нам пришла команда, игнорируем!
                return false;

              // теперь ищем правильное имя модуля, оно идёт сразу за параметрами
              idx = commandArg.indexOf(PARAM_DELIMITER);
              if(idx != -1)
              {
                commandModuleId = commandArg.substring(0,idx);
                commandArg = commandArg.substring(idx+1); // выщемляем остальную часть параметров
              }
              else
                commandModuleId = commandArg;
 
        } // if(dest == cdCHILDMODULE)
        
      } // if
      else
      {
        // Не нашли ID модуля, считаем, что весь переданный аргумент - это ID модуля
        commandModuleId = commandArg;
        // сбрасываем аргументы, потому что нам передали команду без аргументов
        commandArg = F("");
        
        if(dest == cdCHILDMODULE && commandModuleId != ourID) // не нам пришла команда, игнорируем!
          return false;
      } // else
      
    } // if

    outCommand.Construct(commandModuleId,commandArg,commandType,dest);
  
    return true;
}

