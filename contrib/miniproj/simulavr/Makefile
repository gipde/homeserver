SIMULAVR=$(HOME)/src/simulavr/src/simulavr

SRC=main

CPU=atmega128
#CPU=attiny2313

all:
	avr-gcc -Wall -Wextra -g -O2 -mmcu=$(CPU) -o $(SRC).elf $(SRC).c 
	$(SIMULAVR) --device $(CPU) --file $(SRC).elf --writetopipe 0x20,- --terminate exit \
		--cpufrequency=8000000 --irqstatistic --trace out.trace
