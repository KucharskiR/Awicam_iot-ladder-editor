#pragma once
#include <Arduino.h>

class Ladder2Pin
{
  uint8_t _pinIndex;
  uint16_t* _states;
  public:
  Ladder2Pin(uint16_t pinIndex, uint16_t* states)
  {
    _states = states;
    _pinIndex = pinIndex-1;
  }
  Ladder2Pin operator=(uint32_t a)
  {
    (*_states) = ((*_states) & ~(1<<_pinIndex)) | (((a > 0 ? 1 : 0) & 0x01) << _pinIndex);
    return *this;
  }
  Ladder2Pin operator()(uint32_t a)
  {
    (*_states)= ((*_states) & ~(1<<_pinIndex)) | (((a > 0 ? 1 : 0) & 0x01) << _pinIndex);
    return *this;
  }
  operator int() const
  {
    return (((*_states)>>11) & 0x0001);
  }
};