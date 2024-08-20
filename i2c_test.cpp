#include "i2c.hpp"
#include "mock/device.hpp"
#include <util/delay.h>

namespace {
I2cState<MockDevice> *i2c_state = nullptr;
} // namespace

ISR(TWI_vect) {
  I2cStatus stat = static_cast<I2cStatus>(TWSR);
  if (i2c_state->_serve(stat))
    enable_i2c();
  else
    disable_i2c();
}

int main() {
  MockDevice state;
  auto i2c = I2cState(state);
  i2c_state = &i2c;
  init_i2c();
  while (true) {
    _delay_ms(10);
  }
}
