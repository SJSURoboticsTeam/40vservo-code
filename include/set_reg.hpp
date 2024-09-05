#pragma once

#include <cstdint>
template <typename... Args> uint8_t setmask(Args... values) noexcept {
  return ((1 << values) | ...);
}

template <typename... Args> uint8_t clearmask(Args... values) noexcept {
  return ~((1 << values) | ...);
}
