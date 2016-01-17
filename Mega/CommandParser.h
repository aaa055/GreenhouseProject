#ifndef _COMMAND_PARSER_H
#define _COMMAND_PARSER_H

#include <Arduino.h>
#include <WString.h>
#include "Globals.h"


/*
 * Структура команд:
 * CT - префикс
 * SET или GET = тип команды
 * = после знака равно начинается текст команды
 * 
 * Каждая команда передается выбранному модулю.
 * Каждый аргумент в команде разделен через символ '|'.
 * Если символа '|' нет в Command.Arg - считается, что всё, переданное в Command.Arg  - это ID модуля.
 * Первый аргумент в Command.Arg  - это ID модуля (регистрозависимо!).
 * Следующие n аргументов - специфичны для каждого модуля.
 * 
 * Пример команды:
 * CTSET=PIN13|T - инвертировать текущее состояние модуля PIN13
 * CTGET=PIN13 - получить текущее состояние модуля PIN13
 */


typedef enum 
{ 
ctUNKNOWN, 
ctGET, 
ctSET,
} COMMAND_TYPE; // тип команды

typedef enum
{
  cdUNKNOWN,
  cdCONTROLLER, // контроллеру
  cdCHILDMODULE // дочернему модулю
  
} COMMAND_DESTINATION; // кому адресована команда

class Command
{
  private:

    Stream* IncomingStream; // поток, из которого пришла команда
    String ArgsSplitted[MAX_ARGS_IN_LIST];
    uint8_t ArgsCount; // кол-во аргументов, максимально - 255

    bool bIsInternal; // флаг того, что команда получена от другого зарегистрированного модуля
    String StringType;
    COMMAND_TYPE Type; // тип команды
    COMMAND_DESTINATION Destination; // кому команда
    String ModuleID; // ID модуля
    String Arg; // аргументы команды
    COMMAND_TYPE GetCommandType(const String& command);

    void Clear();
    void Construct(const String& moduleID,const String& rawArgs, COMMAND_TYPE ct,COMMAND_DESTINATION dest); // конструирует команду из переданных аргументов

 public:

    void SetIncomingStream(Stream* s) {IncomingStream = s;}
    Stream* GetIncomingStream() const {return IncomingStream;}

    bool IsInternal() const {return bIsInternal;}
    void SetInternal(bool i) {bIsInternal = i;}
    
    void Construct(const String& moduleID,const String& rawArgs, const String& commandType,COMMAND_DESTINATION dest); // конструирует команду из переданных аргументов
    COMMAND_DESTINATION GetDestination() const { return Destination;}
    COMMAND_TYPE GetType() const {return Type;}
    String GetRawArguments() const  {return Arg;}
    String GetTargetModuleID() const {return ModuleID;}
    String GetStringType() const {return StringType;};
    uint8_t GetArgsCount() const {return ArgsCount;}
    String GetArg(uint16_t idx) const {return idx >= ArgsCount ? F("") : ArgsSplitted[idx];}
    Command() : IncomingStream(NULL), bIsInternal(false) {}
};



class CommandParser
{
  private:
    String commandBuf;
 

  public:
    CommandParser();

    void Clear();
    bool ParseCommand(const String& command, const String& ourID, Command& outCommand);
    const String& GetRawCommand() {return commandBuf;}
};


#endif
