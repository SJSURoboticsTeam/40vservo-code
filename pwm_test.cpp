#include "pwm.hpp"
#include <util/delay.h>

int main() {
  init_pwm();
  while (true) {
    set_motor(0.30);
    _delay_ms(1500);
    set_motor(0.60);
    _delay_ms(1500);
    set_motor(1.0);
    _delay_ms(1500);
  }
}
