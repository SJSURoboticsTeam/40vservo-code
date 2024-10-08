#include "pwm.hpp"
#include <util/delay.h>

int main() {
  init_pwm();
  while (true) {
    set_motor(0.05);
    _delay_ms(1000);
    set_motor(0.10);
    _delay_ms(1000);
    set_motor(0.05);
    _delay_ms(1000);
    set_motor(0);
    _delay_ms(1000);
    set_motor(-0.05);
    _delay_ms(1000);
    set_motor(-0.10);
    _delay_ms(1000);
    set_motor(-0.05);
    _delay_ms(1000);
    set_motor(0);
    _delay_ms(1000);
  }
}
