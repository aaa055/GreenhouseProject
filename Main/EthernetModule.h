#ifndef _ETHERNET_MODULE_H
#define _ETHERNET_MODULE_H

#include "AbstractModule.h"

#define MAX_LAN_CLIENTS 4 // максимальное кол-во клиентов

class EthernetModule : public AbstractModule // модуль поддержки W5100
{
  private:

    bool bInited;
    String clientCommands[MAX_LAN_CLIENTS]; // наши команды с клиентов
  
  public:
    EthernetModule() : AbstractModule("LAN") {}

    bool ExecCommand(const Command& command, bool wantAnswer);
    void Setup();
    void Update(uint16_t dt);

};


#endif
