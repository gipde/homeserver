# TODO:
# * ASM Code -> avra
# * AVRSTUDIO (ASM Projects, ASM Includes, Upload)
# * ARDUINO
# * Doxygen
# * ctags / cscope

# * Unit Testing -> is schwierig, da selbst kleine Frameworks wie Unity zu gross sind, um auf der HW zu laufen
# * Macros for assertions / Testcases
# * Mocks

# CPU Type
MCU = atmega32

# Project Name
NAME = sensor2

# expliit List objects here
SRC = $(NAME).o ds18x20lib.o ds18x20lib_hw.o debug.o nop.o

# linkage allows multiple definitions for functions in test doubles -> first wins
TEST1_OBJ = cases/Ds18x20libTest.o doubles/ds18x20lib_hw.o
ALLTESTS = TEST1 

OPTIMIZE=-Os

AVRDUDE_CYCLE=4
AVRDUDE_PROGRAMMER = avrispmkII

SIMULAVR=$(HOME)contrib/simulavr/src/simulavr
BUILD=build

# -------------- NO NEED TO TOUCH --------------

.DEFAULT_GOAL := all

TARGET=$(BUILD)/$(NAME)

# Dependency Files, if changed something, it needs a compile
GENDEPFLAGS = -MD -MP -MF $(BUILD)/.dep/$(@F).d
-include $(shell mkdir -p $(BUILD)/.dep) $(wildcard $(BUILD)/.dep/*)

GCCOPTS = -g2 -Wall -Wextra ${OPTIMIZE}
GCCOPTS += -funsigned-char -funsigned-bitfields -ffunction-sections 
GCCOPTS += -fdata-sections -fpack-struct -fshort-enums
GCCOPTS += -mmcu=$(MCU) -I. $(GENDEPFLAGS) 

CC=avr-gcc
CFLAGS = ${GCCOPTS} -std=gnu99

CXX=avr-g++
CXXFLAGS = ${GCCOPTS} -std=gnu++98

ASFLAGS = -Wa,-adhlns=$(<:.S=.lst),-gstabs -mmcu=$(MCU) -I. -x assembler-with-cpp

OBJ = $(addprefix src/main/,$(SRC)) 

LDFLAGS = -Wl,-Map="$(TARGET).map" -Wl,--start-group -Wl,-lm -Wl,--allow-multiple-definition
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
	@-mkdir -p build 
	@$(CC) -o $@  $(OBJ) $(LDFLAGS)

# Compile: create object files from C source files.
%.o : %.c
	@echo Compiling $< ...
	@$(CC) -c $(CFLAGS) $< -o $@ 


# Compile: create object files from C++ source files.
%.o : %.cpp
	@echo Compiling $< ...
	@$(CC) -c $(CXXFLAGS) $< -o $@ 
	@if [ -f $(BUILD)/m4.clean ]; then cat $(BUILD)/m4.clean; cat $(BUILD)/m4.clean | xargs -i rm {}; rm $(BUILD)/m4.clean;  fi

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


TESTOBJ = src/test/cases/TestBase.o

define TEST_template 
CASEOBJ=$(addprefix src/test/,$($(1)_OBJ))
TESTCLEAN += $$(CASEOBJ)
$(1): $$(CASEOBJ) $(TESTOBJ) $(OBJ)
	@$(CC) -o $(BUILD)/$(1).elf $$^ $(TESTOBJ) $(OBJ) $(LDFLAGS)
	@$(SIMULAVR) --file $(BUILD)/$(1).elf --device $(MCU) --writetopipe 0x20,- --writetoexit 0x21 --terminate exit --cpufrequency=8000000 --irqstatistic
endef
$(foreach test,$(ALLTESTS),$(eval $(call TEST_template,$(test))))

%.cpp : %.case
	@echo Preprocessing $< ...
	@m4 -DFILE=$< src/test/cases/testcases.m4 > $@
	$(eval M4TEMP = $<)
	@echo $@ > $(BUILD)/m4.clean

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
	-@rm -rf $(BUILD) $(OBJ) $(TESTOBJ) $(TESTCLEAN) 

check:
	@echo checking sources...
	cppcheck --enable=all src

test: $(ALLTESTS)

testprog: $(TARGET).elf 
	@echo starting all tests in Simulator
	@$(SIMULAVR) --file $(TARGET).elf --device $(MCU) --writetopipe 0x20,- --terminate exit --cpufrequency=8000000 --irqstatistic

debug: $(TARGET).elf
	@echo --------------------------------
	@echo starting Simulator to debug code ...
	@echo Now connect with a debugger
	@echo  avr-gdb
	@echo  cgdb -d avr-gdb
	@echo  ddd --debugger avr-gdb
	@echo 
	@echo after that do this:
	@echo  file src/$(TARGET).elf
	@echo  target remote localhost:1212
	@echo  load
	@echo  b main
	@echo  c
	@echo 
	@$(SIMULAVR) --file $(TARGET).elf --device $(MCU) --writetopipe 0x20,- --terminate exit --cpufrequency=8000000 --irqstatistic -g

help:
	@echo "Targets"
	@echo
	@echo " all         - make everything"
	@echo " clean       - clean out"
	@echo " test        - execute all Tests in the simulator"
	@echo " testprob    - execute all Tests in the simulator"
	@echo
	@echo " program     - Download the hex file to the devie"
	@echo
	@echo " format      - Format all C/C++ sources"
	@echo
	@echo " doc         - Generate Doc"

.SECONDARY: # do not cleanup intermediate files
.PHONY: clean help all sizeafter format test
