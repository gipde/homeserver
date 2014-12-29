# TODO:
# * DEBUGGING
# * SIMULAVR
# * ASM Code
# * AVRDUDE
# * AVRSTUDIO

# CPU Type
MCU = atmega32

# Project Name
TARGET=sensor2

# Output format. (can be srec, ihex, binary)
FORMAT = ihex

# Debugging format
DEBUG = gdb

# List sources her
CSRC= ds18x20lib.c uart.c 
CXXSRC=$(TARGET).cpp
ASRC=

OPTIMIZE=-Os

# -------------- NO NEED TO TOUCH --------------

.DEFAULT_GOAL := help

CC=avr-gcc
CFLAGS = -g$(DEBUG) -Wall -Wextra -std=c99 ${OPTIMIZE}
CFLAGS += -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums

CXX=avr-g++
CXXFLAGS=-g$(DEBUG) -Wall -Wextra -Os -mmcu=$(MCU) ${OPTIMIZE}

ASFLAGS = -Wa,-adhlns=$(<:.S=.lst),-gstabs 

OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump
SIZE = avr-size
NM = avr-nm

# Define all object files.
OBJ = $(CSRC:.c=.o) $(ASRC:.S=.o) $(CXXSRC:.cpp=.o)

# Dependency Files, if changed something, it needs a compile
GENDEPFLAGS = -MD -MP -MF .dep/$(@F).d
-include $(shell mkdir .dep 2>/dev/null) $(wildcard .dep/*)

ALL_CFLAGS = -mmcu=$(MCU) -I. $(CFLAGS) $(GENDEPFLAGS)
ALL_CXXFLAGS = -mmcu=$(MCU) -I. $(CXXFLAGS) $(GENDEPFLAGS)
ALL_ASFLAGS = -mmcu=$(MCU) -I. -x assembler-with-cpp $(ASFLAGS)

#LDFLAGS=

elf: $(TARGET).elf
hex: $(TARGET).hex
eep: $(TARGET).eep
lss: $(TARGET).lss 
sym: $(TARGET).sym


AVRDUDE_PROGRAMMER = stk500

# com1 = serial port. Use lpt1 to connect to parallel port.
AVRDUDE_PORT = com1    # programmer connected to serial device

AVRDUDE_WRITE_FLASH = -U flash:w:$(TARGET).hex
AVRDUDE_FLAGS = -p $(MCU) -P $(AVRDUDE_PORT) -c $(AVRDUDE_PROGRAMMER)
AVRDUDE_FLAGS += $(AVRDUDE_NO_VERIFY)
AVRDUDE_FLAGS += $(AVRDUDE_VERBOSE)
AVRDUDE_FLAGS += $(AVRDUDE_ERASE_COUNTER)


all: elf hex eep sym lss size

# show size
size: elf $(OBJ)
	@$(SIZE) -C $(TARGET).elf --mcu=${MCU}
	@$(SIZE) *.o --mcu=${MCU}

# Link: create ELF output file from object files.
%.elf: $(OBJ)
	@echo Linking $@ ...
	@$(CC) $(ALLCFLAGS) $(OBJ) --output $@ $(LDFLAGS)

# Compile: create object files from C source files.
%.o : %.c
	@echo Compiling $< ...
	@$(CC) -c $(ALL_CFLAGS) $< -o $@ 

# Compile: create object files from C++ source files.
%.o : %.cpp
	@echo Compiling $< ...
	@$(CC) -c $(ALL_CXXFLAGS) $< -o $@ 

# Compile: create assembler files from C source files.
%.s : %.c
	@echo Create Assembler sources from $< ...
	@$(CC) -S $(ALL_CFLAGS) $< -o $@

%.E : %.c
	@echo create Preprocessor output from $< ...
	@$(CC) -E $(ALL_CFLAGS) $< -o $@

# Create final output files (.hex, .eep) from ELF output file.
%.hex: %.elf
	@echo
	@echo build flash... $@
	@$(OBJCOPY) -O $(FORMAT) -R .eeprom $< $@
%.eep: %.elf
	@echo
	@echo Build EEPROM  $@
	-@$(OBJCOPY) -j .eeprom --set-section-flags=.eeprom="alloc,load" \
	--change-section-lma .eeprom=0 -O $(FORMAT) $< $@

# Create extended listing file from ELF output file.
%.lss: %.elf
	@echo
	@echo Extended Listing... $@
	@$(OBJDUMP) -h -S $< > $@

# Create a symbol table from ELF output file.
%.sym: %.elf
	@echo
	@echo Symbol Table ... $@
	@$(NM) -n $< > $@

# Assemble: create object files from assembler source files.
%.o : %.S
	@echo
	@echo assembling... $<
	@$(CC) -c $(ALL_ASFLAGS) $< -o $@

clean:
	@echo cleaning...
	-@rm -rf $(TARGET).hex $(TARGET).obj $(TARGET).elf $(TARGET).eep \
		$(OBJ)  $(TARGET).lss $(TARGET).sym  $(TARGET).s *.E .dep



help:
	@echo "Targets"
	@echo
	@echo " all         - make everything"
	@echo " clean       - clean out"
	@echo " program     - Download the hex file to the devie"
	@echo

.SECONDARY: # do not cleanup intermediate files
.PHONY: clean help all sizeafter
