#include "crc4.hpp"
#include "rotation.hpp"
#include <avr/interrupt.h>
#include <avr/sfr_defs.h>
#include <bit>
#include <cstdint>
#include <cstdio>
#include <util/delay.h>

constexpr TmagDeviceConfig settings{
    .magnet_tempco = MagnetType::NdBFe,
    .temp_ch_enable = true,
    .op_mode = OpMode::continuous,
};
TmagSensorConfig sensor{
    .sleeptime = 6,
    .angle_en = Axis::xy,
    .x_range = 1,
    .y_range = 1,
    .z_range = 1,
};

int main() {
  sensor.set_magnet_ch(0x7);
  _delay_ms(2000);
  printf("hello world!\n");
  init_spi();
  sei();
  init_tmag();
  while (true) {
    int16_t mag = get_mag();
    uint16_t data = get_angle();
    float angle = ((float)(data >> 4)) + (((float)(data & 0x000f)) / 16);
    printf("mag: %.02f, angle: %.02f\n", (float)mag / 654, angle);
    _delay_ms(300);
  }
}