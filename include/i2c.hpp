#pragma once

#include <array>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <bit>
#include <cstdint>

#include "device.hpp"
#include "nonstd/ring_span.hpp"
#include "set_reg.hpp"

inline DeviceState *device_state = nullptr;

enum struct I2cStatus : uint8_t {
  start_sr = 0x60,
  start_gen = 0x70,
  ack_sr = 0x80,
  nack_sr = 0x88,
  ack_gen = 0x90,
  nack_gen = 0x98,
  stop_sr = 0xa0,
  start_st = 0xa8,
  ack_st = 0xb8,
  nack_st = 0xc0,
  stop_st_err = 0xc8
};

struct I2cState {

  I2cState() : buffer(raw_buffer.begin(), raw_buffer.end()) {}

  enum Mode : uint8_t {
    idle,
    addressing = 0x01,
    change_mode = 0x02,
    change_pid = 0x03,
    change_setpoint = 0x04,
    read_mode = 0x04,
    read_pid = 0x80,
    read_pos = 0x81,
    read_vel = 0x82,
    read_current = 0x83,
    read_setpoint = 0x84,
  };

  bool is_write_mode() const noexcept { return mode < 0x80; }
  bool is_read_mode() const noexcept {
    if (mode == idle || mode == addressing)
      return false;
    return mode >= 0x80;
  }

  Mode mode = idle;
  std::array<uint8_t, 64> raw_buffer;
  nonstd::ring_span_lite::ring_span<uint8_t> buffer;

  bool serve(I2cStatus status) noexcept {
    using enum I2cStatus;
    switch (status) {
    case start_sr:
    case start_gen:
      if (!(status == start_gen || status == start_sr)) {
        return false;
      }
      mode = addressing;
      return true;
    case ack_sr:
    case ack_gen: {
      uint8_t data = TWDR;
      if (mode == addressing) {
        return set_addr(data);
      }
      if (!is_write_mode() || buffer.full()) {
        uint8_t _ = TWDR;
        mode = idle;
        return false;
      }
      buffer.push_front(data);
      return true;
    }

    case stop_sr: {
      mode = idle;
      output_data();
      return true;
    }

    case start_st: {
      if (!is_read_mode() || buffer.full()) {
        TWDR = 0xff;
        mode = idle;
        return false;
      }
      load_data();
      uint8_t last_byte = buffer.back();
      TWDR = last_byte;
      buffer.pop_back();
      return !buffer.empty();
    }
    case ack_st: {
      if (buffer.empty()) {
        mode = idle;
        return false;
      }
      uint8_t last_byte = buffer.back();
      TWDR = last_byte;
      buffer.pop_back();
      return !buffer.empty();
    }
    case nack_st: {
      mode = idle;
      return true;
    }

    case nack_sr: {
      return mode == addressing;
    }

    case stop_st_err:
    case nack_gen:
    default:
      mode = idle;
      return true;
    }
  }

  bool set_addr(char addr) {
    switch (addr) {
    case 'm':
      mode = change_mode;
      return true;
    case 't':
      mode = change_pid;
      return true;
    case 's':
      mode = change_setpoint;
      return true;
    case 'w':
      mode = read_mode;
      return true;
    case 'T':
      mode = read_pid;
      return true;
    case 'P':
      mode = read_pos;
      return true;
    case 'V':
      mode = read_vel;
      return true;
    case 'A':
      mode = read_current;
      return true;
    case 'S':
      mode = read_setpoint;
      return true;
    default:
      return false;
    }
  }

  float pop_float() {
    std::array<uint8_t, 4> result;
    std::copy(buffer.begin(), buffer.end(), result.data());
    return std::bit_cast<float>(result);
  }

  void output_data() {
    if (device_state == nullptr)
      return;
    switch (mode) {
    case change_pid: {
      device_state->pid().setF(pop_float());
      device_state->pid().setD(pop_float());
      device_state->pid().setI(pop_float());
      device_state->pid().setP(pop_float());
      return;
    }
    case change_mode: {
      const float setpoint = pop_float();
      const char mode = static_cast<char>( buffer.back());
      buffer.pop_back();
      DeviceState::Mode m = DeviceState::Mode::position;
      switch (mode) {
      case 'p':
        m = DeviceState::Mode::position;
        break;
      case 'v':
        m = DeviceState::Mode::velocity;
        break;
      case 'a':
        m = DeviceState::Mode::current;
        break;
      default:;
      }
      device_state->transition_state(m, setpoint);
      break;
    }
    case change_setpoint:
      device_state->setpoint = pop_float();
      break;
    default:
      return;
    }
  }

  void push_back_float(float f) {
    auto p = std::bit_cast<std::array<uint8_t, 4>>(f);
    for (auto c : p) {
      buffer.push_front(c);
    }
  }

  void load_data() {
    switch (mode) {
    case read_mode:
      buffer.push_front(device_state->mode);
      break;
    case read_pid: {
      auto &pid = device_state->pid();
      push_back_float(pid.P);
      push_back_float(pid.I);
      push_back_float(pid.D);
      push_back_float(pid.F);
      break;
    }
    case read_pos: {
      push_back_float(device_state->get_angle());
      break;
    }
    case read_vel: {
      push_back_float(device_state->get_vel());
      break;
    }
    case read_current: {
      push_back_float(device_state->dev_current);
      break;
    }
    case read_setpoint:
      push_back_float(device_state->setpoint);
      break;

    default:
      break;
    }
  }
} inline i2c_state;

inline void disable_i2c() noexcept { TWCR &= clearmask(TWEA); }

inline void enable_i2c() noexcept { TWCR |= setmask(TWEA); }

inline void init_i2c() noexcept {
  TWCR = setmask(TWEN, TWIE, TWEA);
  // todo actually maybe the thermistor voltage divider can be used to config
  // this
  TWAR = 0x42;
}

ISR(TWI_vect) {
  I2cStatus stat = static_cast<I2cStatus>(TWSR);
  if (i2c_state.serve(stat))
    enable_i2c();
  else
    disable_i2c();
}