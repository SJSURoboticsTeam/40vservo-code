#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <bit>
#include <cmath>
#include <util/delay.h>

#include "current.hpp"
#include "mock/device.hpp"
#include "i2c.hpp"
#include "pwm.hpp"
#include "rotation.hpp"
#include "timer.hpp"

namespace {
constexpr uint8_t ipropi_pin = 0;
constexpr uint8_t divider_pin = 1;
constexpr float get_float(buffer_span &buffer) {
  std::array<uint8_t, 4> data;
  for (uint8_t i = 0; i != 4; i++) {
    data[i] = buffer.pop_back();
  }
  return std::bit_cast<float>(data);
}
constexpr void push_float(buffer_span &buffer, float f) {
  auto data = std::bit_cast<std::array<uint8_t, 4>>(f);
  for (uint8_t b : data) {
    buffer.push_front(b);
  }
}

MockDevice state;
auto i2c = I2c(
    [](uint8_t addr, buffer_span &output) {
      switch (addr) {
      case 'p':
        state.set_P(get_float(output));
        break;
      case 'i':
        state.set_I(get_float(output));
        break;
      case 'd':
        state.set_D(get_float(output));
        break;
      case 'f':
        state.set_F(get_float(output));
        break;
      case 's':
        state.set_setpoint(get_float(output));
        break;
      case 'm': {
        uint8_t mode = output.pop_back();
        float setpoint = get_float(output);
        state.transition_state(MockDevice::Mode(mode), setpoint);
        break;
      }
      default:
        while (!output.empty()) {
          output.pop_back();
        }
      }
    },
    [](uint8_t addr, buffer_span &input) {
      switch (addr) {
      case 'p':
        push_float(input, state.get_P());
        break;
      case 'i':
        push_float(input, state.get_I());
        break;
      case 'd':
        push_float(input, state.get_D());
        break;
      case 'f':
        push_float(input, state.get_F());
        break;
      case 's':
        push_float(input, state.get_setpoint());
        break;
      case 'm': {
        input.push_front(state.get_mode());
        break;
      }
      case 'x': {
        push_float(input, state.get_angle());
      }
      case 'v': {
        push_float(input, state.get_vel());
      }
      case 'a': {
        push_float(input, state.get_current());
      }
      default:
        input.push_back(0xff);
      }
    });
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
  init_timer();
  state.update_loc(get_angle());
  state.update_loc(get_angle());
  state.set_current(get_analog(ipropi_pin));
  init_i2c();
  sei();
  while (true) {
    sleep_ms(500);
    cli();
    set_motor(state.get_output());
    state.update_loc(get_angle());
    state.set_current(get_analog(ipropi_pin));
    sei();
  }
}
