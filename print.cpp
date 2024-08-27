#include <array>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdio.h>

#ifndef BAUD
#define BAUD 9600
#endif
#include <util/delay.h>
#include <util/setbaud.h>

/* http://www.cs.mun.ca/~rod/Winter2007/4723/notes/serial/serial.html */

void uart_init(void) {
  UBRR0H = UBRRH_VALUE;
  UBRR0L = UBRRL_VALUE;

#if USE_2X
  UCSR0A |= _BV(U2X0);
#else
  UCSR0A &= ~(_BV(U2X0));
#endif

  UCSR0C = _BV(UCSZ01) | _BV(UCSZ00);        /* 8-bit data */
  UCSR0B = _BV(RXEN0) | _BV(TXEN0) | UDRIE0; /* Enable RX and TX */
}

int uart_putchar(char c, FILE *);

int uart_getchar(FILE *stream) {
  loop_until_bit_is_set(UCSR0A, RXC0);
  return UDR0;
}

FILE uart_output = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);
FILE uart_input = FDEV_SETUP_STREAM(NULL, uart_getchar, _FDEV_SETUP_READ);

// no buffer is set up for input since it is not typically expected that is
// necessary.

struct Initializer {
  std::array<char, 32> raw_buffer;
  Initializer() {
    uart_init();
    stdout = &uart_output;
    stdin = &uart_input;
  }

};
Initializer i;

int uart_putchar(char c, FILE *) {
  if (c == '\n')
    uart_putchar('\r', nullptr);
  UDR0 = c;
  loop_until_bit_is_set(UCSR0A, UDRE0);
  return 0;
}

// ISR(UART0_UDRE_vect) {
//   UDR0 = i.buffer.pop_front();
//   if (i.buffer.empty())
//     UCSR0B &= ~(_BV(UDRIE0));
// }
