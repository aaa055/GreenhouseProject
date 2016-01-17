#ifndef _PINMODULE_H
#define _PINMODULE_H

#include "Globals.h"
#include "AbstractModule.h"

typedef struct
{
  uint8_t pinNumber;
  uint8_t pinState;
  bool isActive;
  
} PIN_STATE;

typedef Vector<PIN_STATE> PIN_STATES;

class PinModule : public AbstractModule
{
  private:

    PIN_STATES pinStates;
    void UpdatePinStates();

    uint8_t GetPinState(uint8_t pinNumber);
    bool AddPin(uint8_t pinNumber,uint8_t currentState);
    bool PinExist(uint8_t pinNumber);
   
  public:
    PinModule(const String& id) : AbstractModule(id) {}

    bool ExecCommand(const Command& command);
    void Setup();
    void Update(uint16_t dt);
};

#endif
