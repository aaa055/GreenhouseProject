#ifndef _LCD_MODULE_H
#define _LCD_MODULE_H

#include "AbstractModule.h"
#include "Encoder.h"



class LCDModule : public AbstractModule // модуль поддержки экрана 128х64 на чипе ST7920
{
  private:
  public:
    LCDModule() : AbstractModule(F("LCD")) {}

    bool ExecCommand(const Command& command, bool wantAnswer);
    void Setup();
    void Update(uint16_t dt);

};

extern RotaryEncoder rotaryEncoder;

#endif
