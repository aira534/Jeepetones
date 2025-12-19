#include "ultrasonido.h"

UltraSoundClass::UltraSoundClass(int _echo, int _trg) {
  pin_echo = _echo;
  pin_trg = _trg;

  pinMode(pin_trg, OUTPUT);
  pinMode(pin_echo, INPUT);
}


float UltraSoundClass::get_dist() {
  digitalWrite(pin_trg, LOW);
  delayMicroseconds(2);

  digitalWrite(pin_trg, HIGH);
  delayMicroseconds(10);
  digitalWrite(pin_trg, LOW);

  long duration = pulseIn(pin_echo, HIGH, 200000);

  float distance = duration * 0.034 / 2; // distance in cm

  return distance;
}