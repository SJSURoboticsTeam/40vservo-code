#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <cmath>
#include <util/delay.h>

#include "current.hpp"
#include "i2c.hpp"
#include "pwm.hpp"
#include "rotation.hpp"

void get_rot();
void get_i();

namespace {
bool initial_rot = true;

constexpr uint8_t ipropi_pin = 0;
constexpr uint8_t divider_pin = 1;
} // namespace

int main() {
  init_spi();
  init_tmag();
  init_adc();
  init_pwm();
  DeviceState state;
  device_state = &state;
  state.current_loc = get_angle();
  state.prev_loc = state.current_loc;
  state.dev_current = get_analog(ipropi_pin);
  init_i2c();
  // todo change to timer solution later
  while (true) {
    cli();
    set_motor(state.get_output());
    state.update_loc(get_angle());
    state.dev_current = get_analog(0);
    sei();
    _delay_ms(200);
  }
}
