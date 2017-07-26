TOOLCHAIN=/usr/gcc-arm-none-eabi-5_4-2016q3
ARM_CMSIS_PATH=
STM32_DEVICE_PATH=

CC = $(TOOLCHAIN)/bin/arm-none-eabi-gcc
LD = $(TOOLCHAIN)/bin/arm-none-eabi-gcc
OBJCP = $(TOOLCHAIN)/bin/arm-none-eabi-objcopy -O binary

OPTIONS=-DUSE_FULL_ASSERT -D_DEBUG
CPU_OPT=-mcpu=cortex-m3 # works for all cores
LD_SCRIPT=stm32-min.ld

CFLAGS = $(CPU_OPT) -mthumb -g -O3 -include sys/_stdint.h $(OPTIONS) -I $(ARM_CMSIS_PATH) -I $(STM32_DEVICE_PATH)
LDFLAGS = $(CPU_OPT) -mthumb -Wl,--gc-sections,-Map=$@.map,-cref,-u,Reset_Handler -T $(LD_SCRIPT)

BIN_FILES=main.bin

all: $(BIN_FILES)

SRC=main.c
OBJ=main.o

main.o: main.c
	$(CC) $(CFLAGS) -o $@ -c $<

main.elf: $@ $(OBJ)
	$(LD) $(LDFLAGS) -o $@ $(OBJ)

main.bin: main.elf
	$(OBJCP) $< $@

clean:
	-rm $(BIN_FILES) $(OBJ) main.elf
