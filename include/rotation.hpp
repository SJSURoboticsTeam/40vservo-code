#pragma once

#include "crc4.hpp"
#include "set_reg.hpp"

#include <array>
#include <avr/io.h>
#include <avr/sfr_defs.h>
#include <bit>
#include <cstdint>

inline void init_spi() noexcept {
  // I have no idea if clock polarity and phase is correct or not
  SPCR = setmask(SPE, MSTR, CPOL, CPHA);
}

union TmagPacket {
  struct {
    bool is_read : 1;
    uint8_t addr : 7;
    uint16_t data;
    uint8_t command : 4;
    uint8_t crc : 4;
  };
  uint32_t raw;
};

inline TmagPacket make_write(uint8_t addr, uint16_t data) {
  TmagPacket packet{.addr = addr, .data = data};
  packet.crc = crc4(packet);
  return packet;
}

union TmagReturn {
  struct {
    uint8_t status2;
    uint16_t data;
    uint8_t status1 : 4;
    uint8_t crc : 4;
  };
  uint32_t raw;
};

inline TmagReturn spi_transaction(uint32_t input) noexcept {
  std::array<uint8_t, 4> result = {};
  std::array<uint8_t, 4> in = std::bit_cast<std::array<uint8_t, 4>>(input);
  // I hate spinlock implementations. I am too lazy to bother trying something
  // better
  auto output = result.begin();
  for (auto c : in) {
    SPDR = c;
    loop_until_bit_is_set(SPSR, SPIF);
    *output++ = SPDR;
  }
  return std::bit_cast<TmagReturn>(result);
}

inline uint16_t get_angle() noexcept {
  TmagPacket packet{.is_read = true, .addr = 0x13, .command = 1};
  packet.crc = crc4(packet);
  auto value = std::bit_cast<TmagReturn>(spi_transaction(packet.raw));
  return value.data;
}

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

union TmagDeviceConfig {
  struct {
    uint8_t : 1;
    uint8_t conv_avg : 3;
    uint8_t : 2;
    MagnetType magnet_tempco : 2;
    uint8_t : 1;
    OpMode op_mode : 3;
    bool temp_ch_enable : 1;
    bool temp_rate : 1;
    bool temp_halt_en : 1;
    bool : 1;
  };
  uint16_t raw;
};

enum struct Axis : uint8_t { none, xy, yz, xz };

union TmagSensorConfig {
  struct [[gnu::packed]] {
    Axis angle_en : 2;
    uint8_t sleeptime : 4;
    // See datasheet about values for this
    uint8_t magnet_ch_en : 4;
    uint8_t z_range : 2;
    uint8_t y_range : 2;
    uint8_t x_range : 2;
  };
  uint16_t raw;
};

inline void init_tmag() noexcept {
  TmagDeviceConfig settings{.magnet_tempco = MagnetType::NdBFe,
                            .op_mode = OpMode::continuous,
                            .temp_ch_enable = true};
  // runs a conversion every 50 ms
  TmagSensorConfig sensor{.angle_en = Axis::xy, .sleeptime = 6};
  spi_transaction(make_write(1, sensor.raw).raw);
  spi_transaction(make_write(0, settings.raw).raw);
}
