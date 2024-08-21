#pragma once

#include "set_reg.hpp"
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <cstdint>

namespace timer_impl {
inline uint8_t ticks = 0;
inline uint8_t remainder = 0;

struct tick_count {
  uint16_t ticks;
  uint8_t remainder;
};

inline constexpr tick_count calculate_ticks(uint32_t ms) {
  float ticks = float(ms) / 1000 * 16'000'000 / 256 / 1024;
  uint16_t integer_count = ticks;
  uint8_t remainder = (ticks - integer_count) * 256;
  return tick_count{integer_count, remainder};
}

} // namespace timer_impl

// triggers every 256 * 1024 cycles, or 16 ms at 16Mhz
ISR(TIMER0_COMPA_vect) {
  ++timer_impl::ticks;
  TCNT0 = 0;
}

// works until about 1074790.4 ms aka about 12 days
// surely you would not delay for over 12 days right?
inline void sleep_ms(uint32_t ms) {
  auto [tick_time, remainder] = timer_impl::calculate_ticks(ms);
  timer_impl::ticks = 0;
  OCR0A = 0xff;
  TIMSK0 |= setmask(OCIE0A);
  while (true) {
    cli();
    if (timer_impl::ticks >= tick_time) {
      break;
    }
    sleep_enable();
    sei();
    sleep_cpu();
    sleep_disable();
  }
  OCR0A = remainder;
  TCNT0 = 0;
  sleep_enable();
  sei();
  sleep_cpu();
  sleep_disable();
  TIMSK0 &= clearmask(OCIE0A);
}

inline void init_timer() { TCCR0B |= setmask(CS02, CS00); }
