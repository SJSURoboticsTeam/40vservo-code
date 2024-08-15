.PHONEY: all clean sim

all: test.hex dump.txt map.txt

CC:=avr-gcc
CXX:=avr-g++
AS:=avr-as

MCU:=atmega328p

ASFLAGS := -mmcu=$(MCU)
FLAGS :=   -maccumulate-args -funwind-tables -ffunction-sections  -mmcu=$(MCU) -Oz -g -I . --param=min-pagesize=0 -Werror=array-bounds -mcall-prologues
CFLAGS := $(FLAGS)
CXXFLAGS := $(FLAGS) -frtti -std=c++20
CPPFLAGS := -DNDEBUG
LDFLAGS :=  -mmcu=$(MCU) -funwind-tables -Xlinker -Map=output.map -Wl,--gc-sections

clean:
	rm -f *.elf *.o *.hex dump.txt output.map *.txt

test.hex: test.elf
	avr-objcopy -j .text -j .data -O ihex test.elf test.hex

dump.txt: test.elf
	avr-objdump -Cd test.elf > dump.txt

test.elf output.map: test.o pid.o
	$(CXX) -o  $@ $^ $(LDFLAGS)

sim: test.elf
	simavr test.elf -m $(MCU)

gdb: test.elf
	simavr test.elf -m $(MCU) -g

map.txt: output.map
	cat output.map | c++filt > map.txt
