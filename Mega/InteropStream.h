#ifndef _INTEROP_STREAM_H
#define _INTEROP_STREAM_H

#include <Arduino.h>
#include <WString.h>

class InteropStream : public Stream
{
private:
  String data;

public:

  InteropStream();
    
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

#endif

