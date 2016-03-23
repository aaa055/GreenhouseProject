#ifndef _ENCODER_H
#define _ENCODER_H

#include <Arduino.h>

class RotaryEncoder
{
  unsigned int state;
  int pin0, pin1;
  int ppc;
  int change;

  unsigned int readState();


public:
  RotaryEncoder(int p0, int p1, int pulsesPerClick);

  void begin();
  void update();
  int getChange();
};

#endif
