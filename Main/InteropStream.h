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


public:

   InteropStream();
  virtual ~InteropStream() {}

    bool QueryCommand(COMMAND_TYPE cType, const String& command, bool isInternalCommand,bool wantAnwer=true); // вызывает команду для зарегистрированного модуля

    const String& GetData() {return data;}
    
    virtual int available(){ return false; };
    virtual int read(){ return -1;};
    virtual int peek(){return -1;};
    virtual void flush(){};

 
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
    bool needUpdate;
  
  public:
    BlinkModeInterop();

    void begin(uint8_t pin, const String& loopName); // запоминаем настройки
    void blink(uint16_t interval=0); // мигаем диодом
    void update(); // обновляем состояние
};

#endif

