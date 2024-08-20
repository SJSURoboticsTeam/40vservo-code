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

inline void setA(bool value) {
  if (value)
    TCCR1A |= setmask(COM1A1);
  else
    TCCR1A &= clearmask(COM1A1);
}

inline void setB(bool value) {
  if (value)
    TCCR1A |= setmask(COM1B1);
  else
    TCCR1A &= clearmask(COM1B1);
}

inline void pinA(bool value) {
  if (value)
    PORTD |= setmask(PORTD6);
  else
    PORTD &= clearmask(PORTD6);
}

inline void pinB(bool value) {
  if (value)
    PORTD |= setmask(PORTD5);
  else
    PORTD &= clearmask(PORTD5);
}

inline void set_motor(float value) {
  if (value == 0) {
    setA(false);
    setB(false);
    pinA(false);
    pinB(false);
    return;
  }
  if (value > 0) {
    pwm_set(value);
    setA(true);
    setB(false);
    pinA(false);
    pinB(true);

  } else {
    pwm_set(-value);
    setA(false);
    setB(true);
    pinA(true);
    pinB(false);
  }
}
