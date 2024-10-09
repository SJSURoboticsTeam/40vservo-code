#include "i2c.hpp"
#include <avr/interrupt.h>
#include <cstdio>
#include <util/delay.h>

namespace {
uint8_t out_addr = 0, in_addr = 0;
std::array<uint8_t, 16> out{};
bool has_written = false;
std::vector<I2cStatus> statuses;
auto i2c = I2c(
    [](uint8_t addr, auto &output) {
      out_addr = addr;
      std::copy(output.begin(), output.end(), out.data());
      has_written = true;
    },
    [](uint8_t addr, auto &input) {
      input.push_back(0x42);
      input.push_back(0x21);
      input.push_back(0x8);
      in_addr = addr;
    });
} // namespace

ISR(TWI_vect) {
  I2cStatus stat = static_cast<I2cStatus>(TWSR);
  statuses.push_back(stat);
  if (i2c._serve(stat))
    i2c_ack();
  else
    i2c_nack();
}

int main() {
  statuses.reserve(128);
  init_i2c();
  printf("Hello world!\n");
  while (true) {
    _delay_ms(1000);
    printf("g: %d\n", g);
    if (has_written) {
      has_written = false;
      printf("Addressed at 0x%x with bytes:\n", out_addr);
      for (uint8_t b : out) {
        printf("0x%x\n", b);
      }
    }
    if (in_addr != 0) {
      printf("Addressed at 0x%x\n", in_addr);
      in_addr = 0;
    }
    sei();
  }
}
