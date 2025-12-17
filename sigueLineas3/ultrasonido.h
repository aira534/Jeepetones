#ifndef ULTRASOUND_CLASS_H
#define ULTRASOUND_CLASS_H

#include <Arduino.h>


class UltraSoundClass {
public:
  UltraSoundClass(int _echo, int _trg);

  float get_dist();

private:
  int pin_echo;
  int pin_trg;
};

#endif