# 40 V Servo

This is the repo for the 40V servo firmware.

## I2C interface

All registers are written in ASCII values. 

| Register letter| R/W | Register Type | Register Description |
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