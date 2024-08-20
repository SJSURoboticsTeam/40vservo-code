.PHONEY: all clean sim

all: main.hex i2c.hex

CC:=avr-gcc
CXX:=avr-g++
AS:=avr-as

MCU:=atmega328p

ASFLAGS := -mmcu=$(MCU)
FLAGS :=   -maccumulate-args -ffunction-sections  -mmcu=$(MCU) -Oz -g -I include --param=min-pagesize=0 -Werror=array-bounds -mcall-prologues
CFLAGS := $(FLAGS)
CXXFLAGS := $(FLAGS) -std=c++20
CPPFLAGS := -MMD -DF_CPU=16000000
LDFLAGS :=  -mmcu=$(MCU) -Wl,--gc-sections

FILES := pid.cpp main.cpp print.cpp
BASENAMES := $(basename $(FILES))
OBJ := $(addsuffix .o, $(BASENAMES))
DEPS := $(addsuffix .d, $(BASENAMES))

clean:
	rm -f *.elf *.o *.hex *.map *.txt *.d

%.hex: %.elf
	avr-objcopy -j .text -j .data -O ihex $< $@

main.elf: main.o pid.o
	$(CXX) -o  $@ $^ $(LDFLAGS)

i2c.elf: i2c_test.o pid.o
	$(CXX) -o  $@ $^ $(LDFLAGS)

sim: test.elf
	simavr test.elf -m $(MCU)

gdb: test.elf
	simavr test.elf -m $(MCU) -g

-include $(DEPS)