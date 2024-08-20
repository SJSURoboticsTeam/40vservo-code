#pragma once

#include "debug.hpp"

struct MockDevice {
  enum Mode : char { position, velocity, current };

  float get_angle() const noexcept { return 0; }
  float get_vel() const noexcept { return 0; }
  float get_current() const noexcept { return 0; }
  float get_setpoint() const noexcept { return 0; }
  float get_P() const noexcept { return 0; }
  float get_I() const noexcept { return 0; }
  float get_D() const noexcept { return 0; }
  float get_F() const noexcept { return 0; }
  void set_P(float p) noexcept { debug_print("setting P to %f", p); }
  void set_I(float i) noexcept { debug_print("setting I to %f", i); }
  void set_D(float d) noexcept { debug_print("setting D to %f", d); }
  void set_F(float f) noexcept { debug_print("setting F to %f", f); }

  void update_loc(uint16_t loc) noexcept { debug_print("location at %d", loc); }
  void set_current(float f) noexcept { debug_print("set current to %f", f); }

  float get_output() noexcept { return 0; }
  Mode get_mode() const noexcept { return position; }

  MockDevice() = default;
  MockDevice(MockDevice &&) = delete;
  MockDevice(MockDevice const &) = delete;
  ~MockDevice() = default;

  void transition_state(Mode new_state, float setpoint) noexcept {
    debug_print("transitioning to %d with setpoint %f", new_state, setpoint);
  }
  void set_setpoint(float setpoint) noexcept {
    debug_print("setpoint set to %f", setpoint);
  }
};