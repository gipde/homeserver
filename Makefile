# TODO:
# * DEBUGGING
# * SIMULAVR
# * ASM Code -> avra
# * AVRSTUDIO (ASM Projects, ASM Includes, Upload)
# * ARDUINO
# * Statical Code analysis
# * Doxygen
# * ctags / cscope

# CPU Type
MCU = atmega32

# Project Name
TARGET=src/sensor2

# expliit List sources here
CSRC= src/ds18x20lib.c src/uart.c src/debug.c
CXXSRC= ${TARGET}.cpp
ASRC=

OPTIMIZE=-Os

AVRDUDE_CYCLE=4
AVRDUDE_PROGRAMMER = avrispmkII

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
	@$(SIZE) $(OBJ) --mcu=${MCU}

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
%.E : %.cpp
	@echo Create Preprocessor output from $< ...
	@$(CC) -E $(CXXLAGS) $< -o $@


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


format:
	@echo Formatting...
	@astyle -v -A3 -H -p -f -k1 -W3 -c --max-code-length=72 -xL -r "src/*.cpp" "src/*.h" "src/*.c"

AVRDUDE_WRITE_FLASH = -U flash:w:$(TARGET).hex
AVRDUDE_FLAGS = -p $(MCU) -B $(AVRDUDE_CYCLE) -c $(AVRDUDE_PROGRAMMER)
AVRDUDE_FLAGS += -v -v 
AVRDUDE_FLAGS += $(AVRDUDE_VERBOSE)
AVRDUDE_FLAGS += $(AVRDUDE_ERASE_COUNTER)

program:
	@echo programming
	@echo avrdude $(AVRDUDE_FLAGS) $(AVRDUDE_WRITE_FLASH)


clean:
	@echo cleaning ...
	-@rm -rf $(TARGET).hex $(TARGET).obj $(TARGET).elf $(TARGET).eep \
		$(OBJ)  $(TARGET).lss $(TARGET).sym  $(TARGET).map $(TARGET).lst src/*.E src/.dep 

check:
	@echo checking sources...
	cppcheck --enable=all src

help:
	@echo "Targets"
	@echo
	@echo " all         - make everything"
	@echo " clean       - clean out"
	@echo
	@echo " program     - Download the hex file to the devie"
	@echo
	@echo " format      - Format all C/C++ sources"
	@echo
	@echo " doc         - Generate Doc"

.SECONDARY: # do not cleanup intermediate files
.PHONY: clean help all sizeafter format
