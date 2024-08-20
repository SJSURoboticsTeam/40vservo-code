#include "debug.hpp"
#include "i2c.hpp"
#include "mock/device.hpp"
#include <util/delay.h>

namespace {
auto i2c = I2c(
    [](uint8_t addr, buffer_span &output) {
      debug_print("i2c addressed at 0x%x with bytes", addr);
      while (!output.empty()) {
        printf("0x%x ", output.pop_back());
      }
    },
    [](uint8_t addr, buffer_span &input) { input.push_front(0xff); });
} // namespace

ISR(TWI_vect) {
  I2cStatus stat = static_cast<I2cStatus>(TWSR);
  if (i2c._serve(stat))
    enable_i2c();
  else
    disable_i2c();
}

int main() {
  MockDevice state;
  init_i2c();
  while (true) {
    _delay_ms(10);
  }
}
