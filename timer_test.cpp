#include "timer.hpp"
#include <avr/sleep.h>
#include <cstdio>
#include <util/delay.h>

int main() {
  init_timer();
  sei();
  printf("hello world!\n");

  while (true) {
    sleep_ms(500);
    printf("waiting\n");
  }
}
