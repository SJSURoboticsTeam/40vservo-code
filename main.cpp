#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <bit>
#include <cmath>
#include <util/delay.h>

#include "current.hpp"
#include "device.hpp"
#include "i2c.hpp"
#include "pwm.hpp"
#include "rotation.hpp"
#include "timer.hpp"

namespace {
constexpr uint8_t ipropi_pin = 0;
constexpr uint8_t divider_pin = 1;
constexpr float get_float(auto &buffer) {
  std::array<uint8_t, 4> data;
  for (uint8_t i = 0; i != 4; i++) {
    data[i] = buffer[i];
  }
  return std::bit_cast<float>(data);
}

bool checkFloat(auto &buf) { return buf.size() != sizeof(float); }

constexpr void push_float(auto &buffer, float f) {
  auto data = std::bit_cast<std::array<uint8_t, 4>>(f);
  for (uint8_t b : data) {
    buffer.push_back(b);
  }
}

DeviceState state;
auto i2c = I2c(
    [](uint8_t addr, auto &output) {
      switch (addr) {
      case 'p':
        if (checkFloat(output))
          break;
        state.set_P(get_float(output));
        break;
      case 'i':
        if (checkFloat(output))
          break;
        state.set_I(get_float(output));
        break;
      case 'd':
        if (checkFloat(output))
          break;
        state.set_D(get_float(output));
        break;
      case 'f':
        if (checkFloat(output))
          break;
        state.set_F(get_float(output));
        break;
      case 's':
        if (checkFloat(output))
          break;
        state.set_setpoint(get_float(output));
        break;
      case 'm': {
        if (output.size() != sizeof(float) + 1)
          break;
        uint8_t mode = output.pop_back();
        float setpoint = get_float(output);
        state.transition_state(DeviceState::Mode(mode), setpoint);
        break;
      }
      default:;
      }
      while (!output.empty()) {
        output.pop_back();
      }
    },
    [](uint8_t addr, auto &input) {
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
        input.push_back(state.get_mode());
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
    i2c_ack();
  else
    i2c_nack();
}

int main() {
  init_spi();
  init_tmag();
  init_adc();
  init_pwm();
  init_timer();
  state.update_loc(get_angle());
  init_i2c();
  sei();
  while (true) {
    sleep_ms(20);
    cli();
    state.update_loc(get_angle());
    state.set_current(get_analog(ipropi_pin));
    set_motor(state.get_output());
    sei();
  }
}
