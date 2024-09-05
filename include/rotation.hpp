#pragma once

#include "crc4.hpp"
#include "set_reg.hpp"

#include <array>
#include <avr/io.h>
#include <avr/sfr_defs.h>
#include <bit>
#include <cstdint>
#include <cstdio>

inline void init_spi() noexcept {
  PORTB |= _BV(PORT2);
  DDRB |= setmask(DDB2, DDB3, DDB4, DDB5);

  // CPOL, CPHA
  SPCR = setmask(SPE, MSTR);
}

// enum struct Address:uint8_t{};

struct TmagPacket {
  uint8_t addr : 7 = 0;
  bool is_read : 1 = 0;
  uint16_t data = 0;
  uint8_t crc : 4 = 0;
  uint8_t command : 4 = 0;
};

inline TmagPacket make_write(uint8_t addr, auto data) {
  TmagPacket packet{.addr = addr, .data = std::bit_cast<uint16_t>(data)};
  packet.crc = crc4(packet);
  return packet;
}

struct TmagReturn {
  uint8_t status2;
  uint16_t data;
  uint8_t status1 : 4;
  uint8_t crc : 4;
};

constexpr uint32_t byteswap(uint32_t i) {
  return ((i & 0x000000ff) << 24) | ((i & 0x0000ff00) << 8) |
         ((i & 0x00ff0000) >> 8) | ((i & 0xff000000) >> 24);
}

constexpr uint16_t byteswap(uint16_t i) {
  return ((i & 0x00ff) << 8) | ((i & 0xff00) >> 8);
}

inline TmagReturn spi_transaction(auto input) noexcept {
  std::array<uint8_t, 4> result = {};
  std::array<uint8_t, 4> in = std::bit_cast<std::array<uint8_t, 4>>(input);
  // I hate spinlock implementations. I am too lazy to bother trying something
  // better
  auto output = result.begin();
  PORTB &= ~_BV(PORT2);
  for (auto c : in) {
    SPDR = c;
    loop_until_bit_is_set(SPSR, SPIF);
    *output++ = SPDR;
  }
  PORTB |= _BV(PORT2);
  return std::bit_cast<TmagReturn>(result);
}

inline TmagReturn read_raw(uint8_t addr) noexcept {
  TmagPacket packet{
      .addr = addr,
      .is_read = true,
  };
  packet.crc = crc4(packet);
  auto value = std::bit_cast<TmagReturn>(spi_transaction(packet));
  return value;
}

inline uint16_t get_sys_stat() noexcept { return read_raw(0xe).data; }

inline uint16_t get_angle() noexcept { return read_raw(0x13).data; }

inline uint16_t get_mag() noexcept { return read_raw(0x14).data; }

enum struct MagnetType : uint8_t { none, NdBFe, SmCo, Ceramic };
enum struct OpMode : uint8_t {
  config,
  standby,
  continuous,
  trigger,
  duty_cycle,
  sleep,
  deep_sleep
};
/* Note to self
 * Bit and byte layout of a struct is such that
 * the member's byte is submitted up to down, but
 * the bits are submitted from down to up.
 * This is weird, and honestly I could change it
 * But patching GCC more than I already do seems annoying,
 * so I'll leave it like this for now
 */
struct TmagDeviceConfig {
  // submitted first
  MagnetType magnet_tempco : 2;
  uint8_t : 2;
  uint8_t conv_avg : 3 = 5;
  uint8_t : 1;

  // submitted second
  uint8_t : 1;
  bool temp_halt_en : 1;
  bool temp_rate : 1;
  bool temp_ch_enable : 1;
  OpMode op_mode : 3;
  uint8_t : 1;
};

enum struct Axis : uint8_t { none, xy, yz, xz };

struct [[gnu::packed]] TmagSensorConfig {
  uint8_t magnet_ch_en_hi : 2;
  uint8_t sleeptime : 4;
  Axis angle_en : 2;

  uint8_t x_range : 2;
  uint8_t y_range : 2;
  uint8_t z_range : 2;
  uint8_t magnet_ch_en_lo : 2;

  constexpr void set_magnet_ch(uint8_t i) noexcept {
    magnet_ch_en_lo = i & 0x3;
    magnet_ch_en_hi = (i & 0xc) >> 2;
  }
  // See datasheet about values for this
};

inline void init_tmag() noexcept {
  TmagDeviceConfig settings{
      .magnet_tempco = MagnetType::NdBFe,
      .temp_ch_enable = true,
      .op_mode = OpMode::continuous,
  };
  // runs a conversion every 50 ms
  TmagSensorConfig sensor{
      .sleeptime = 6,
      .angle_en = Axis::xy,
  };
  sensor.set_magnet_ch(0xf);
  spi_transaction(byteswap(uint32_t(0x0f000407)));
  auto result0 = spi_transaction(TmagPacket{.addr = 0xd, .is_read = true});
  auto result1 = spi_transaction(make_write(1, sensor));
  auto result2 = spi_transaction(make_write(0, settings));
  printf("r0 data: 0x%x s2: %x", result0.data, result0.status2);
  printf("r1 s2: 0x%x\n", result1.status2);
  printf("r2 s2: 0x%x\n", result2.status2);
}
