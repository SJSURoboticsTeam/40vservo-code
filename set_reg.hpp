#pragma once

template <typename... Args> auto setmask(Args... values) noexcept {
  return ((1 << values) | ...);
}

template <typename... Args> auto clearmask(Args... values) noexcept {
  return ~((1 << values) | ...);
}
