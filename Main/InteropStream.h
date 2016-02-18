#ifndef _INTEROP_STREAM_H
#define _INTEROP_STREAM_H

#include <Arduino.h>
#include <WString.h>
#include "ModuleController.h"
#include "CommandParser.h"

// класс поддержки коммуникации между модулями
class InteropStream : public Stream
{
private:
  String data;

  ModuleController* mainController;

public:

  InteropStream();

    void SetController(ModuleController* c) {mainController = c;}

    bool QueryCommand(COMMAND_TYPE cType, const String& command, bool isInternalCommand); // вызывает команду для зарегистрированного модуля
    
    void Clear() { data = F("");}
    const String& GetData() { return data;}
    
    virtual int available(){ return false; };
    virtual int read(){ return -1;};
    virtual int peek(){return -1;};
    virtual void flush(){};

    virtual size_t print(const String &s);
    virtual size_t println(const String &s);
    virtual size_t write(uint8_t toWr);  
  
};

extern InteropStream ModuleInterop;

// класс-хелпер мигания информационным диодом
class BlinkModeInterop
{
  private:
    uint16_t lastBlinkInterval; // последний интервал, с которым мигаем
    uint8_t pin; // пин, на котором диод
    String loopName; // имя периодически выполняемой операции
    String pinCommand;
  
  public:
    BlinkModeInterop();

    void begin(uint8_t pin, const String& loopName); // запоминаем настройки
    void blink(uint16_t interval=0); // мигаем диодом
};

#endif

