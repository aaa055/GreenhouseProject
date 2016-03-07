#ifndef _COMMAND_PARSER_H
#define _COMMAND_PARSER_H

#include <Arduino.h>
#include <WString.h>
#include "Globals.h"
#include "TinyVector.h"


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
 * CTSET=PIN|13|T - инвертировать текущее состояние модуля PIN для пина номер 13
 * CTGET=PIN|13 - получить текущее состояние модуля PIN для пина номер 13
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

typedef Vector<char*> CommandArgsVec;

class Command
{
  private:


    Stream* IncomingStream; // поток, из которого пришла команда
    CommandArgsVec arguments; // аргументы команды
    
    bool bIsInternal; // флаг того, что команда получена от другого зарегистрированного модуля
    uint8_t Type; // тип команды
    uint8_t Destination; // кому команда
    String ModuleID; // ID модуля
    uint8_t GetCommandType(const String& command);

    void Clear();
    void Construct(const String& moduleID,const String& rawArgs, uint8_t ct,uint8_t dest); // конструирует команду из переданных аргументов

 public:

    static String _CMD_GET;
    static String _CMD_SET;

    // устанавливает/возвращает поток для работы с командой
    void SetIncomingStream(Stream* s) {IncomingStream = s;}
    Stream* GetIncomingStream() const {return IncomingStream;}

    // флаг, что команда внутренняя, т.е. от одного модуля другому
    bool IsInternal() const {return bIsInternal;}
    void SetInternal(bool i) {bIsInternal = i;}
    
    void Construct(const String& moduleID,const String& rawArgs, const String& commandType,uint8_t dest); // конструирует команду из переданных аргументов

    // возвращает тип адресации команды: контроллеру или дочернему модулю (отдельной железной коробочке со своим МК)
    uint8_t GetDestination() const { return Destination;}

    // возвращает тип команды
    uint8_t GetType() const {return Type;}
    // возвращает тип команды в виде строки
    String GetStringType() const;

    // возвращает все аргументы в виде строки
   // String GetRawArguments() const  {return Arg;}

    // возвращает ID программного модуля, которому адресована команда
    String GetTargetModuleID() const {return ModuleID;}

    // возвращает количество переданных аргументов
    size_t GetArgsCount() const;

    // возвращает аргумент по индексу
    const char* GetArg(size_t idx) const;
    
    Command();
    ~Command();
};


// парсер команд
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
