#pragma once

#include <array>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <bit>
#include <cstdint>

#include "debug.hpp"
#include "device.hpp"
#include "nonstd/ring_span.hpp"
#include "set_reg.hpp"

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
using buffer_span = nonstd::ring_span_lite::ring_span<uint8_t>;
template <typename WriteCallback, typename ReadCallback> struct I2c {

  I2c(WriteCallback on_write, ReadCallback on_read)
      : in_buffer(in_raw_buffer.begin(), in_raw_buffer.end()),
        out_buffer(out_raw_buffer.begin(), out_raw_buffer.end()),
        write(on_write), read(on_read) {}

public:
  bool _serve(I2cStatus status) noexcept {
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
        address = data;
        return true;
      }
      if (in_buffer.full()) {
        uint8_t _ = TWDR;
        mode = idle;
        return false;
      }
      in_buffer.push_front(data);
      return true;
    }

    case stop_sr: {
      mode = idle;
      write(address, in_buffer);
      return true;
    }

    case start_st: {
      read(address, in_buffer);
      TWDR = in_buffer.pop_back();
      return !in_buffer.empty();
    }
    case ack_st: {
      if (in_buffer.empty()) {
        mode = idle;
        return false;
      }
      TWDR = in_buffer.pop_back();
      return !in_buffer.empty();
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

private:
  enum Mode : uint8_t { idle, addressing };

  Mode mode = idle;
  std::array<uint8_t, 64> in_raw_buffer;
  nonstd::ring_span_lite::ring_span<uint8_t> in_buffer;
  std::array<uint8_t, 64> out_raw_buffer;
  nonstd::ring_span_lite::ring_span<uint8_t> out_buffer;
  uint8_t address{};
  WriteCallback write;
  ReadCallback read;
};

inline void disable_i2c() noexcept { TWCR &= clearmask(TWEA); }

inline void enable_i2c() noexcept { TWCR |= setmask(TWEA); }

inline void init_i2c(uint8_t address = 0x42) noexcept {
  TWCR = setmask(TWEN, TWIE, TWEA);
  // todo actually maybe the thermistor voltage divider can be used to config
  // this
  TWAR = address & 0xfe;
}
