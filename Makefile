CROSS = arm-none-eabi-
CC = $(CROSS)gcc
LD = $(CROSS)gcc

CFLAGS = -mcpu=cortex-m4 -mthumb -fno-builtin -nostdlib -g -O0 -I. 
LDFLAGS = -T linker.ld -Wl,--gc-sections

TARGET = rtos-day1
SRCS = startup.S main.c task.c mutex.c queue.c protocol.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET).elf

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.S
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET).elf: $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $@

run: $(TARGET).elf
	renode script.resc

clean:
	rm -f *.o *.elf

.PHONY: all run clean