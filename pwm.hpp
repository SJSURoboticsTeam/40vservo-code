#pragma once

#include <avr/io.h>
#include <limits>

#include "set_reg.hpp"

inline void init_pwm() {
  DDRD = setmask(DD5, DD6);
  // 256 prescaler for pwm clock
  TCCR1A = setmask(COM1A1, WGM11);
  TCCR1B = setmask(CS12, WGM12, WGM13);
}

inline void pwm_set(float f) {
  uint16_t value = f * std::numeric_limits<uint16_t>::max();
  TCNT1L = value & 0x00ff;
  TCNT1H = value & 0xff00;
}

inline void set_A(bool value) {
  if (value)
    TCCR1A |= setmask(COM1A1);
  else
    TCCR1A &= clearmask(COM1A1);
}
inline void set_B(bool value) {
  if (value)
    TCCR1A |= setmask(COM1B1);
  else
    TCCR1A &= clearmask(COM1B1);
}
