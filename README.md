# 40 V Servo

This is the repo for the 40V servo firmware.

## I2C interface

All registers addresses are ASCII values. The default I2C address is 0xfe, but can be configured upon request.

| Register Address | R/W | Register Type | Register Description |
| ---------------|---- | ------------- | -------------------- |
| p              | R/W | float         | The proportional constant |
| i              | R/W | float         | The integral constant|
| d              | R/W | float         | The derivative constant|
| f              | R/W | float         | The feed-forward constant|
| s              | R/W | float         | The setpoint         |
| m              | R/W | {mode, float} / mode | The mode, then the new setpoint.|
| x              |  R  | float         | Angle in degrees     |
| v              |  R  | float         | Change in degrees in 20 ms |
| a              |  R  | float         | Current measured by analog pin |

### Mode Register

| Mode value | Description |
| ---------- | ----------- |
| 0x0        | Position Control |
| 0x1        | Velocity Control |
| 0x2        | Force Control |

Note that current is used as a proxy for the force experienced by the motor.

The mode register is a bit unusual in that it has a different type depending
on if it's written to or read from. When writing to it, you must also write
a set-point in the same I2C write as the mode. When reading to it, you only
get the current mode.

## Internal Architecture

Internally, the 40 V servo is controlled with an Atmega328PB. The Atmega interfaces
externally with I2C, and internally with a [TMAG5170-Q1](https://www.ti.com/product/TMAG5170-Q1)
magnetic encoder and a [DRV8251](https://www.ti.com/product/DRV8251A) motor driver.
The magnetic encoder is used to sense position, and by extension velocity. The motor
driver's current output is used to sense drawn current, and by extension acceleration/force
applied.

The servo itself operates a very simple loop. First it updates all sensor measurements, then it
calculates the PID and outputs it to the motor driver. In between updates, I2C can be serviced via
interrupts.

## Building

Building this requires a AVR-GCC toolchain that contains the standard library.
To my knowledge only [my own toolchain](https://github.com/DolphinGui/std-avr-gcc) has this,
so you need to download the relevant version and add it to your path to make it work.

After getting the toolchain, just run the makefile to compile all programs.

## External Libraries

PID: <https://github.com/tekdemo/MiniPID>  
Used in [pid.cpp](pid.cpp) and [pid.hpp](include/pid.hpp). Licensed under GPL3.

Ring Span Lite: <https://github.com/martinmoene/ring-span-lite>  
Used in [ring_span.hpp](include/nonstd/ring_span.hpp). Licensed under Boost Software License, relicensed here under GPL3.

TODO: 
Verify that one update takes less than 120 cycles, the amount of cycles that 1 I2C bit takes.
Alternatively figure out speed of a single cycle, then calculate probability of trying in the middle
of a update.
