#ifndef _COMMAND_BUFFER_H
#define _COMMAND_BUFFER_H

#include <Stream.h>



class CommandBuffer
{
private:
  Stream* pStream;
  String strBuff;
public:
  CommandBuffer(Stream* s);

  bool HasCommand();
  const String& GetCommand() {return strBuff;}
  void ClearCommand() {strBuff = "";}
  Stream* GetStream() {return pStream;}

};

#endif
