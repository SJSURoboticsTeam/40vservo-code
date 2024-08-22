#include "current.hpp"
#include <cstdio>
#include <util/delay.h>

int main() {
  init_adc();
  printf("hello world!\n");
  while (true) {
    printf("analog value: %d\n", get_analog_raw(0));
    _delay_ms(1000);
  }
}
