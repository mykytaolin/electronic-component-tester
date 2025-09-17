MCU = atmega328p
F_CPU = 16000000
PORT = /dev/ttyUSB0

CC = avr-gcc
CFLAGS = -mmcu=$(MCU) -DF_CPU=$(F_CPU)UL -Os -Wall -Wextra

SRCS = src/main.c src/capacitance.c src/uart.c src/setup.c
OBJS = $(SRCS:.c=.o)

all: main.hex

main.elf: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

main.hex: main.elf
	avr-objcopy -O ihex $^ $@

flash: main.hex
	avrdude -p m328p -c usbasp -P usb -U flash:w:$<

clean:
	rm -f $(OBJS) main.elf main.hex