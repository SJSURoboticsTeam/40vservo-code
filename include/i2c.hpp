#pragma once

#include <array>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <cstdint>
#include <vector>

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

inline uint8_t g = 0;
template <typename WriteCallback, typename ReadCallback> struct I2c {

  I2c(WriteCallback on_write, ReadCallback on_read)
      : in_buf(in_buf_raw.begin(), in_buf_raw.end()),
        out_buf(out_buf_raw.begin(), out_buf_raw.end()), write(on_write),
        read(on_read) {}

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
        mode = writing;
        return true;
      }
      out_buf.push_back(data);
      return true;
    }

    case stop_sr: {
      mode = idle;
      if (!out_buf.empty())
        write(address, out_buf);
      while (!out_buf.empty()) {
        out_buf.pop_front();
      }
      return true;
    }

    case start_st: {
      g++;
      read(address, in_buf);
      TWDR = in_buf.pop_front();
      return !in_buf.empty();
    }
    case ack_st: {
      if (in_buf.empty()) {
        mode = idle;
        TWDR = 0xff;
        return false;
      }
      TWDR = in_buf.pop_front();
      return !in_buf.empty();
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

  // private:
  enum Mode : uint8_t { idle, addressing, writing };

  Mode mode = idle;
  std::array<uint8_t, 64> in_buf_raw{}, out_buf_raw{};
  nonstd::ring_span_lite::ring_span<uint8_t> in_buf;
  nonstd::ring_span_lite::ring_span<uint8_t> out_buf;
  uint8_t address{};
  WriteCallback write;
  ReadCallback read;
};

inline void i2c_nack() noexcept {
  TWCR &= clearmask(TWEA);
  TWCR |= setmask(TWINT);
}

inline void i2c_ack() noexcept { TWCR |= setmask(TWEA, TWINT); }

inline void init_i2c(uint8_t address = 0xfe) noexcept {
  TWCR = setmask(TWEN, TWIE, TWEA);
  // todo actually maybe the thermistor voltage divider can be used to config
  // this
  TWAR = address & 0xfe;
}
