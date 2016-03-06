#include <Arduino.h>
#include "CommandParser.h"


COMMAND_TYPE Command::GetCommandType(const String& command)
{
  if(command == CMD_SET)
  return ctSET;
  else if(command == CMD_GET)
    return ctGET;

  return ctUNKNOWN;
}
void Command::Construct(const String& id,const String& arg, COMMAND_TYPE ct,COMMAND_DESTINATION dest)
{
  Clear(); // сбрасываем все настройки
    
    Type = ct;
    Arg = arg;
    ModuleID = id;
    Destination = dest;

    // разбиваем на аргументы
        int curIdx = 0;
        String tmpStr = Arg;
        String param;
        
        while(curIdx != -1)
        {
          curIdx = tmpStr.indexOf(PARAM_DELIMITER);
          if(curIdx == -1)
          {
           if(tmpStr.length() > 0)
           {
              ArgsSplitted[ArgsCount++] = tmpStr;
           }
              
            break;
          } // if
          param = tmpStr.substring(0,curIdx);
          tmpStr = tmpStr.substring(curIdx+1,tmpStr.length());
          if(param.length() > 0)
          {
             ArgsSplitted[ArgsCount++] = param;
          }

         if( ArgsCount >= MAX_ARGS_IN_LIST)
          break;
            
        } // while

}
void Command::Construct(const String& id,const String& arg, const String& ct,COMMAND_DESTINATION dest)
{
   StringType = ct;
   Construct(id, arg,GetCommandType(ct),dest);
}

void Command::Clear()
{
  ArgsCount = 0;
  Type = ctUNKNOWN;
  ModuleID = F("");
  Arg = F("");
  IncomingStream = NULL;
  bIsInternal = false;
  Destination = cdUNKNOWN;
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

  COMMAND_DESTINATION dest = tmpBuf == CMD_PREFIX ? cdCONTROLLER : cdCHILDMODULE;

  tmpBuf = commandBuf.substring(CMD_PREFIX_LEN,CMD_PREFIX_LEN+CMD_TYPE_LEN);

  if(!(tmpBuf == CMD_SET || tmpBuf == CMD_GET))
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
        
        if(dest == cdCHILDMODULE && commandModuleId != ourID) // не нам пришла команда, игнорируем!
          return false;
      } // else
      
    } // if

    outCommand.Construct(commandModuleId,commandArg,commandType,dest);
  
    return true;
}

