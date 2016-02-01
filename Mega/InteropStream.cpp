#include "InteropStream.h"
#include "Globals.h"

InteropStream ModuleInterop;


InteropStream::InteropStream()
{
  
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
