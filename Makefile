.PHONEY: all clean sim gdb

CC:=avr-gcc
CXX:=avr-g++
AS:=avr-as

MCU:=atmega328p

ASFLAGS := -mmcu=$(MCU)
FLAGS :=   -maccumulate-args -ffunction-sections  -mmcu=$(MCU) -Oz -g -I include --param=min-pagesize=0 \
           -Werror=array-bounds -mcall-prologues -flto
CFLAGS := $(FLAGS)
CXXFLAGS := $(FLAGS) -std=c++20
CPPFLAGS := -MMD -DF_CPU=12000000
LDFLAGS :=  -mmcu=$(MCU) -Wl,--gc-sections -Wl,-u,vfprintf -lprintf_flt

MAIN_FILES := main.cpp i2c_test.cpp timer_test.cpp pwm_test.cpp analog_test.cpp tmag_test.cpp
LIB_FILES := pid.cpp print.cpp
FILES := $(MAIN_FILES) $(LIB_FILES)
BASENAMES := $(basename $(FILES))
OBJ := $(addsuffix .o, $(BASENAMES))
DEPS := $(addsuffix .d, $(BASENAMES))

all: $(addsuffix .hex, $(basename $(MAIN_FILES))) $(addsuffix .elf, $(basename $(MAIN_FILES)))

clean:
	rm -f *.elf *.o *.hex *.map *.txt *.d

%.hex: %.elf
	avr-objcopy -j .text -j .data -O ihex $< $@

main.elf: main.o pid.o
	$(CXX) -o  $@ $^ $(LDFLAGS)

%_test.elf: %_test.o print.o pid.o
	$(CXX) -o  $@ $^ $(LDFLAGS)

sim: test.elf
	simavr test.elf -m $(MCU)

gdb: test.elf
	simavr test.elf -m $(MCU) -g

-include $(DEPS)