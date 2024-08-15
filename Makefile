.PHONEY: all clean sim

all: test.hex dump.txt filtered.map

CC:=avr-gcc
CXX:=avr-g++
AS:=avr-as

MCU:=atmega328p

ASFLAGS := -mmcu=$(MCU)
FLAGS :=   -maccumulate-args -ffunction-sections  -mmcu=$(MCU) -Oz -g -I . --param=min-pagesize=0 -Werror=array-bounds -mcall-prologues
CFLAGS := $(FLAGS)
CXXFLAGS := $(FLAGS) -std=c++20
CPPFLAGS := -DNDEBUG -MMD
LDFLAGS :=  -mmcu=$(MCU) -Xlinker -Map=output.map -Wl,--gc-sections

FILES := pid.cpp test.cpp
BASENAMES := $(basename $(FILES))
OBJ := $(addsuffix .o, $(BASENAMES))
DEPS := $(addsuffix .d, $(BASENAMES))

clean:
	rm -f *.elf *.o *.hex *.map *.txt *.d

test.hex: test.elf
	avr-objcopy -j .text -j .data -O ihex test.elf test.hex

dump.txt: test.elf
	avr-objdump -Cd test.elf > dump.txt

test.elf output.map: $(OBJ)
	$(CXX) -o  $@ $^ $(LDFLAGS)

sim: test.elf
	simavr test.elf -m $(MCU)

gdb: test.elf
	simavr test.elf -m $(MCU) -g

filtered.map: output.map
	cat output.map | c++filt > filtered.map

-include $(DEPS)