#pragma once

#include <cstdint>

#include "pid.hpp"

struct DeviceState {
  enum Mode : char { position, velocity, current };

  float get_angle() const noexcept { return current_loc / float(1 << 4); }
  float get_vel() const noexcept {
    return (current_loc - prev_loc) / float(1 << 4);
  }
  float get_current() const noexcept { return dev_current; }
  float get_setpoint() const noexcept { return pid().getSetpoint(); }

  void update_loc(uint16_t value) noexcept {
    prev_loc = current_loc;
    current_loc = value;
  }
  void set_current(float f) noexcept { dev_current = f; }

  float get_output() noexcept {
    float metric = 0;
    switch (mode) {
    case position:
      metric = get_angle();
    case velocity:
      metric = get_vel();
    case current:
      metric = dev_current;
    default:;
    }
    return pid().getOutput(metric);
  }
  Mode get_mode() const noexcept { return mode; }

  DeviceState() = default;
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
    __builtin_unreachable();
  }
  const MiniPID &pid() const noexcept {
    switch (mode) {
    case position:
      return p_pid;
    case velocity:
      return v_pid;
    case current:
      return i_pid;
    }
    __builtin_unreachable();
  }

  void transition_state(Mode new_state, float setpoint) noexcept {
    if (new_state == mode)
      return;
    mode = new_state;
    pid().reset();
    pid().setSetpoint(setpoint);
  }
  void set_setpoint(float setpoint) noexcept { pid().setSetpoint(setpoint); }

private:
  Mode mode = DeviceState::position;
  MiniPID p_pid = {0, 0, 0, 0}, v_pid = {0, 0, 0, 0}, i_pid = {0, 0, 0, 0};
  uint16_t prev_loc = 0;
  uint16_t current_loc = 0;
  float dev_current = 0;
};