# TODO:
# * DEBUGGING
# * SIMULAVR
# * ASM Code
# * AVRDUDE
# * AVRSTUDIO (ASM Projects, ASM Includes, Upload)
# * ARDUINO
# * Statical Code analysis
# * Doxygen
# * ctags / cscope


# CPU Type
MCU = atmega32

# Project Name
#TARGET=sensor2
TARGET=main


# expliit List sources here
#CSRC= ds18x20lib.c uart.c 
#CXXSRC=$(TARGET).cpp
#ASRC=
CSRC= $(TARGET).c 
CXXSRC=
ASRC=

OPTIMIZE=-O1

# -------------- NO NEED TO TOUCH --------------

.DEFAULT_GOAL := all

# Dependency Files, if changed something, it needs a compile
GENDEPFLAGS = -MD -MP -MF .dep/$(@F).d
-include $(shell mkdir .dep 2>/dev/null) $(wildcard .dep/*)

GCCOPTS =  -g2 -Wall -Wextra ${OPTIMIZE}
GCCOPTS += -funsigned-char -funsigned-bitfields -ffunction-sections 
GCCOPTS += -fdata-sections -fpack-struct -fshort-enums
GCCOPTS += -mmcu=$(MCU) -I. $(GENDEPFLAGS) 

CC=avr-gcc
CFLAGS = ${GCCOPTS} -std=gnu11

CXX=avr-g++
CXXFLAGS = ${GCCOPTS} -std=gnu++11

ASFLAGS = -Wa,-adhlns=$(<:.S=.lst),-gstabs -mmcu=$(MCU) -I. -x assembler-with-cpp

OBJ = $(CSRC:.c=.o) $(ASRC:.S=.o) $(CXXSRC:.cpp=.o)

LDFLAGS = -Wl,-Map="$(TARGET).map" -Wl,--start-group -Wl,-lm  
LDFLAGS += -Wl,--end-group -Wl,--gc-section -mmcu=$(MCU)

all: $(addprefix $(TARGET)., elf hex eep sym lss) size

OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump
SIZE = avr-size
NM = avr-nm

# show size
size: $(TARGET).elf $(OBJ)
	@echo
	@$(SIZE) -C $(TARGET).elf --mcu=${MCU}
	@$(SIZE) *.o --mcu=${MCU}

# Link: create ELF output file from object files.
%.elf: $(OBJ)
	@echo Linking $@ ...
	@$(CC) -o $@  $(OBJ) $(LDFLAGS)

# Compile: create object files from C source files.
%.o : %.c
	@echo Compiling $< ...
	@$(CC) -c $(CFLAGS) $< -o $@ 

# Compile: create object files from C++ source files.
%.o : %.cpp
	@echo Compiling $< ...
	@$(CC) -c $(CXXFLAGS) $< -o $@ 

# Compile: create assembler files from C source files.
%.s : %.c
	@echo Create Assembler sources from $< ...
	@$(CC) -S $(CFLAGS) $< -o $@

%.E : %.c
	@echo Create Preprocessor output from $< ...
	@$(CC) -E $(CFLAGS) $< -o $@

# Create final output files (.hex, .eep) from ELF output file.
%.hex: %.elf
	@echo Build Flash $@ ...
	@$(OBJCOPY) -O ihex -R .eeprom -R .fuse -R .lock -R .signature -R .user_signatures $< $@

%.eep: %.elf
	@echo Build EEPROM $@ ...
	-@$(OBJCOPY) -j .eeprom --set-section-flags=.eeprom="alloc,load" \
	--change-section-lma .eeprom=0 --no-change-warnings -O ihex $< $@

# Create extended listing file from ELF output file.
%.lss: %.elf
	@echo Extended Listing $@ ...
	@$(OBJDUMP) -h -S $< > $@

# Create a symbol table from ELF output file.
%.sym: %.elf
	@echo Symbol Table $@ ...
	@$(NM) -n $< > $@

# Assemble: create object files from assembler source files.
%.o : %.S
	@echo Assembling $< ...
	@$(CC) -c $(ASFLAGS) $< -o $@


AVRDUDE_PROGRAMMER = stk500

# com1 = serial port. Use lpt1 to connect to parallel port.
AVRDUDE_PORT = com1    # programmer connected to serial device

AVRDUDE_WRITE_FLASH = -U flash:w:$(TARGET).hex
AVRDUDE_FLAGS = -p $(MCU) -P $(AVRDUDE_PORT) -c $(AVRDUDE_PROGRAMMER)
AVRDUDE_FLAGS += $(AVRDUDE_NO_VERIFY)
AVRDUDE_FLAGS += $(AVRDUDE_VERBOSE)
AVRDUDE_FLAGS += $(AVRDUDE_ERASE_COUNTER)


clean:
	@echo cleaning ...
	-@rm -rf $(TARGET).hex $(TARGET).obj $(TARGET).elf $(TARGET).eep \
		$(OBJ)  $(TARGET).lss $(TARGET).sym  $(TARGET).s $(TARGET).map *.E .dep 

help:
	@echo "Targets"
	@echo
	@echo " all         - make everything"
	@echo " clean       - clean out"
	@echo " program     - Download the hex file to the devie"
	@echo

.SECONDARY: # do not cleanup intermediate files
.PHONY: clean help all sizeafter
