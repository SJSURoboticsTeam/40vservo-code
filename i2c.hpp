#pragma once

#include <array>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <bit>
#include <cstdint>

#include "inplace_vector.hpp"
#include "pid.hpp"
#include "set_reg.hpp"

struct DeviceState {
  enum Mode : char { position = 'p', velocity = 'v', current = 'a' };
  Mode mode = DeviceState::position;
  MiniPID p_pid = {0, 0, 0, 0}, v_pid = {0, 0, 0, 0}, i_pid = {0, 0, 0, 0};
  uint16_t prev_loc = 0;
  uint16_t current_loc = 0;
  float dev_current = 0;

  float get_angle() { return current_loc / float(1 << 4); }
  float get_vel() { return (current_loc - prev_loc) / float(1 << 4); }

  DeviceState() {}
  DeviceState(DeviceState &&) = delete;
  DeviceState(DeviceState const &) = delete;
  ~DeviceState() = default;

  MiniPID &pid() noexcept {
    switch (mode) {
    case position:
      return p_pid;
    case velocity:
      return v_pid;
    case current:
      return i_pid;
    }
  }
  void transition_state(Mode new_state) noexcept {
    if (new_state == mode)
      return;
    mode = new_state;
    pid().reset();
  }
};
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
  enum Mode : uint8_t {
    idle,
    addressing = 0x01,
    change_mode = 0x02,
    change_pid = 0x03,
    read_mode = 0x04,
    read_pid = 0x80,
    read_pos = 0x81,
    read_vel = 0x82,
    read_current = 0x83,
  };

  bool is_write_mode() const noexcept {
    return mode == change_mode || mode == change_pid;
  }
  bool is_read_mode() const noexcept {
    if (mode == idle || mode == addressing)
      return false;
    return mode >= 0x80;
  }

  Mode mode = idle;
  ext::inplace_vector<uint8_t, 16> buffer;
  uint8_t last_byte = 0;

  bool serve(I2cStatus status) {
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
      char data = TWDR;
      if (mode == addressing) {
        return set_addr(data);
      }
      if (!is_write_mode() || buffer.full()) {
        char _ = TWDR;
        mode = idle;
        return false;
      }
      buffer.push_back(data);
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
      last_byte = buffer.back();
      TWDR = last_byte;
      buffer.pop_back();
      return !buffer.empty();
    }
    case ack_st: {
      if (buffer.empty()) {
        mode = idle;
        return false;
      }
      last_byte = buffer.back();
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
    default:
      return false;
    }
  }

  float pop_float() {
    float value = *reinterpret_cast<float *>(buffer.data() + buffer.size() - 4);
    for (int i = 0; i != 4; i++) {
      buffer.pop_back();
    }
    return value;
  }

  void output_data() {
    if (device_state == nullptr)
      return;
    switch (mode) {
    case change_pid: {
      device_state->pid().setP(pop_float());
      device_state->pid().setI(pop_float());
      device_state->pid().setD(pop_float());
      device_state->pid().setF(pop_float());
      return;
    }
    case change_mode: {
      const char mode = buffer.back();
      switch (mode) {
      case 'p':
      case 'v':
      case 'a':
        device_state->mode = static_cast<DeviceState::Mode>(mode);
      default:;
      }
      buffer.pop_back();
      break;
    }
    default:
      return;
    }
  }

  void push_back_float(float f) {
    auto p = std::bit_cast<std::array<uint8_t, 4>>(f);
    for (auto c : p) {
      buffer.push_back(c);
    }
  }

  void load_data() {
    switch (mode) {
    case read_mode:
      buffer.push_back(device_state->mode);
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