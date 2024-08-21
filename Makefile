.PHONEY: all clean sim

all: main.hex i2c_test.hex timer_test.hex

CC:=avr-gcc
CXX:=avr-g++
AS:=avr-as

MCU:=atmega328p

ASFLAGS := -mmcu=$(MCU)
FLAGS :=   -maccumulate-args -ffunction-sections  -mmcu=$(MCU) -Oz -g -I include --param=min-pagesize=0 \
           -Werror=array-bounds -mcall-prologues -flto
CFLAGS := $(FLAGS)
CXXFLAGS := $(FLAGS) -std=c++20
CPPFLAGS := -MMD -DF_CPU=16000000
LDFLAGS :=  -mmcu=$(MCU) -Wl,--gc-sections

FILES := pid.cpp main.cpp print.cpp timer_test.cpp
BASENAMES := $(basename $(FILES))
OBJ := $(addsuffix .o, $(BASENAMES))
DEPS := $(addsuffix .d, $(BASENAMES))

clean:
	rm -f *.elf *.o *.hex *.map *.txt *.d

%.hex: %.elf
	avr-objcopy -j .text -j .data -O ihex $< $@

main.elf: main.o pid.o
	$(CXX) -o  $@ $^ $(LDFLAGS)

i2c_test.elf: i2c_test.o pid.o print.o
	$(CXX) -o  $@ $^ $(LDFLAGS)

timer_test.elf: timer_test.o print.o
	$(CXX) -o  $@ $^ $(LDFLAGS)

sim: test.elf
	simavr test.elf -m $(MCU)

gdb: test.elf
	simavr test.elf -m $(MCU) -g

-include $(DEPS)