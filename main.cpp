#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <cmath>
#include <util/delay.h>

#include "current.hpp"
#include "device.hpp"
#include "i2c.hpp"
#include "pwm.hpp"
#include "rotation.hpp"

namespace {
constexpr uint8_t ipropi_pin = 0;
constexpr uint8_t divider_pin = 1;
DeviceState state;
auto i2c = I2c([](uint8_t addr, buffer_span &output) {},
               [](uint8_t addr, buffer_span &input) {});
} // namespace

ISR(TWI_vect) {
  I2cStatus stat = static_cast<I2cStatus>(TWSR);
  if (i2c._serve(stat))
    enable_i2c();
  else
    disable_i2c();
}

int main() {
  init_spi();
  init_tmag();
  init_adc();
  init_pwm();
  DeviceState state;
  // i2c_state = &i2c;
  state.update_loc(get_angle());
  state.update_loc(get_angle());
  state.set_current(get_analog(ipropi_pin));
  init_i2c();
  // todo change to timer solution later
  while (true) {
    cli();
    set_motor(state.get_output());
    state.update_loc(get_angle());
    state.set_current(get_analog(ipropi_pin));
    sei();
    _delay_ms(200);
  }
}
