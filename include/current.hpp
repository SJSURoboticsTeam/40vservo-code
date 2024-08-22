#pragma once

#include <avr/io.h>
#include <avr/sfr_defs.h>

#include "set_reg.hpp"

inline void init_adc() {
  ADMUX |= setmask(REFS0);
  ADCSRA |= setmask(ADEN);
}

inline uint16_t get_analog_raw(uint8_t pin) {
  ADMUX &= ~0x0f;
  ADMUX |= (pin & 0x0f);

  // prescaler is 1/32
  ADCSRA |= setmask(ADSC, ADPS2, ADPS0);
  loop_until_bit_is_set(ADCSRA, ADIF);
  uint16_t result = ADCL | (ADCH << 8);
  return result;
}

inline float get_analog(uint8_t pin) {
  return float(get_analog_raw(pin) * 5) / 1024;
}
