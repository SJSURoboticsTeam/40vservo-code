#include "timer.hpp"
#include <avr/sleep.h>
#include <cstdio>
#include <util/delay.h>

int main() {
  init_timer();
  sei();
  printf("hello world!\n");

  while (true) {
    if (ticks == 66) {
      ticks = 0;
      printf("waiting\n");
    }
    sleep_cpu();
  }
}
