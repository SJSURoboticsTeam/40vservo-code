#pragma once

#include "set_reg.hpp"
#include <avr/interrupt.h>
#include <avr/io.h>
#include <cstdint>

inline uint8_t ticks = 0;

// triggers every 256 * 1024 cycles, or about every 1.6 ms
ISR(TIMER0_OVF_vect) { ++ticks; }

inline void init_timer() {
  TCCR0B |= setmask(CS02, CS00);
  TIMSK0 |= setmask(TOIE0);
}
