#pragma once

#include <array>
#include <bit>
#include <cstdint>

constexpr uint8_t get(auto word, uint32_t bit) noexcept {
  return bool(word & (1 << bit));
}

constexpr uint8_t set(uint8_t byte, uint8_t bit, uint8_t value) noexcept {
  if (value)
    return byte |= 1 << bit;
  else
    return byte &= ~(1 << bit);
}

constexpr uint8_t crc4_byte_slow(uint8_t input, uint8_t seed = 15) noexcept {
  for (int i = 7; i >= 0; --i) {
    uint8_t inv = get(input, i) ^ get(seed, 3);
    seed <<= 1;
    seed = set(seed, 1, inv ^ get(seed, 1));
    seed = set(seed, 0, inv);
  }
  return seed & 0x0f;
}

consteval std::array<uint8_t, 128> gen_table() noexcept {
  std::array<uint8_t, 128> result;
  for (int i = 0; i != 128; i++) {
    result[i] = (crc4_byte_slow(i * 2, 0) & 0x0f) |
                ((crc4_byte_slow(i * 2 + 1, 0) << 4) & 0xf0);
  }
  return result;
}

constexpr auto table = gen_table();

constexpr uint8_t crc_fast(uint8_t byte) noexcept {
  if (byte % 2 == 0)
    return table[byte / 2] & 0x0f;
  else
    return (table[byte / 2] & 0xf0) >> 4;
}

constexpr uint8_t crc4(auto input) noexcept {
  uint8_t seed = 15;
  auto bytes = std::bit_cast<std::array<uint8_t, sizeof(input)>>(input);
  for (uint8_t byte : bytes) {
    seed = crc_fast(byte ^ seed);
  }
  return seed;
}
