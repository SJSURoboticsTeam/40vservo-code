#pragma once

#include <cstdio>
#ifndef NDEBUG
template <typename... Args>
inline void _debug_print(const char *str, Args... args) {
  std::printf(str, args...);
}
#define debug_print(format_str, ...) _debug_print("%d:%d DEBUG: " format_str "\n\r" __VA_OPT__(,) __VA_ARGS__)
#endif