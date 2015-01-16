# TODO:
# * AVRSTUDIO (ASM Projects, ASM Includes, Upload)
# * ARDUINO
# * Macros for assertions / Testcases

# CPU Type
MCU = atmega32

# Project Name
NAME = homeserver

# expliit List objects here
SRC = $(NAME).o ds18x20lib.o debug.o

# linkage allows multiple definitions for functions in test doubles -> first wins
TESTOBJ = $(addprefix src/test/,TestBase.o mock.o sha1-asm.o)
TEST1_OBJ = $(addprefix ds18x20/,Ds18x20libTest.o ds18x20lib_hw.o)

ALLTESTS = TEST1 

# If you do Debugging its better to run with -O0
OPTIMIZE=-Os

AVRDUDE_CYCLE=4
AVRDUDE_PROGRAMMER = avrispmkII

SIMULAVR=contrib/simulavr/src/simulavr
SIMULAVR_OPTS=--writetopipe 0x20,- --writetoexit 0x21 --terminate exit --cpufrequency=8000000 --irqstatistic


# -------------- NO NEED TO TOUCH --------------

.DEFAULT_GOAL := all

ifdef SystemRoot
   MD = md 2>NUL
   FixPath = $(subst /,\,$1)
else
   ifeq ($(shell uname), Linux)
      MD = mkdir -p
      FixPath = $1
   endif
endif

BUILD=build
TARGET=$(BUILD)/$(NAME)
# Windows-Build-Elfs müssen nach dem erzeugen nach toplevel verschoben werden.

# Dependency Files, if changed something, it needs a compile
GENDEPFLAGS = -MD -MP -MF $(BUILD)/.dep/$(@F).d
include  $(wildcard $(BUILD)/.dep/*)

GCCOPTS = -g2 -Wall -Wextra ${OPTIMIZE}
GCCOPTS += -funsigned-char -funsigned-bitfields -ffunction-sections 
GCCOPTS += -fdata-sections -fpack-struct -fshort-enums
GCCOPTS += -mmcu=$(MCU) -I. $(GENDEPFLAGS) 

CC=avr-gcc
CFLAGS = ${GCCOPTS} -std=gnu11

CXX=avr-g++
CXXFLAGS = ${GCCOPTS} -std=gnu++11

ASFLAGS = -Wa,-adhlns=$(<:.S=.lst),-gstabs -mmcu=$(MCU) -I. -x assembler-with-cpp

OBJ = $(addprefix src/main/,$(SRC)) 

LDFLAGS = -Wl,-Map="$(TARGET).map" -Wl,--start-group -Wl,-lm -Wl,--allow-multiple-definition
LDFLAGS += -Wl,--end-group -Wl,--gc-section -mmcu=$(MCU) 

all: $(addprefix $(TARGET)., elf hex eep sym lss) size

OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump
SIZE = avr-size
NM = avr-nm

buildDir: 
	@echo Creating Build-Dir and gen tags ...
	-@$(MD) $(BUILD)/.dep
	-@ctags -R src
	-@cscope -R -ssrc -b

# show size
size: $(TARGET).elf $(OBJ)
	@echo .
	@$(SIZE) -C $(TARGET).elf --mcu=${MCU}
#	@$(SIZE) $(OBJ) --mcu=${MCU}

# Link: create ELF output file from object files.
%.elf: $(OBJ)
	@echo Linking $@ ...
	@$(CC) -o $@  $(OBJ) $(LDFLAGS)

# Compile: create object files from C source files.
%.o : %.c | buildDir
	@$(CC) $(TESTBUILD) -c $(CFLAGS) $< -o $@ 

# Compile: create object files from C++ source files.
%.o : %.cpp | buildDir
	@echo Compiling $@ ...
	@$(CXX) $(TESTBUILD) -c $(CXXFLAGS) $< -o $@ 
ifndef SystemRoot
	@if [ -f $(BUILD)/m4.clean ]; then cat $(BUILD)/m4.clean; cat $(BUILD)/m4.clean | xargs -i rm {}; rm $(BUILD)/m4.clean;  fi
endif


# Compile: create assembler files from C source files.
%.s : %.c
	@echo Create Assembler sources from $< ...
	@$(CC) -S $(CFLAGS) $< -o $@

%.E : %.c
	@echo Create Preprocessor output from $< ...
	@$(CC) $(TESTBUILD) -E $(CFLAGS) $< -o $@
%.E : %.cpp
	@echo Create Preprocessor output from $< ...
	@$(CXX) $(TESTBUILD) -E $(CXXFLAGS) $< -o $@


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



testenv:
	@echo Compiling in TESTMODE ...
	$(eval TESTBUILD=-D_TESTBUILD_)
	
define TEST_template 
CASEOBJ=$(addprefix src/test/,$($(1)_OBJ))
TESTCLEAN += $$(CASEOBJ)
$(BUILD)/$(1).elf: $$(CASEOBJ) $(TESTOBJ) $(OBJ)
	@echo Building for Test: $(1).elf ... 
	@$(CC) -o $(BUILD)/$(1).elf $$^ $(TESTOBJ) $(OBJ) $(LDFLAGS)

$(1): testenv $(BUILD)/$(1).elf $(BUILD)/$(1).lss 
	@echo .
	@$(SIZE) -C $(BUILD)/$(1).elf --mcu=${MCU}
	@echo .
	@echo Running Test $(1) ...
	@$(SIMULAVR) --file $(BUILD)/$(1).elf --device $(MCU) $(SIMULAVR_OPTS)

$(1)DEBUG: $(BUILD)/$(1).elf debug_help
	@echo .
	@echo Debugging Test $(1) ...
	@$(SIMULAVR) -g --file $(BUILD)/$(1).elf --device $(MCU) $(SIMULAVR_OPTS)

endef
$(foreach test,$(ALLTESTS),$(eval $(call TEST_template,$(test))))

%.cpp : %.case | buildDir 
	@echo Preprocessing $< ...
	@m4 -DFILE=$< src/test/testcases.m4 > $@
	$(eval M4TEMP = $<)
	@echo $@ > $(BUILD)/m4.clean

format:
	@echo Formatting...
	@astyle -v -A3 -H -p -f -k1 -W3 -c --max-code-length=72 -xL -r --suffix=none "src/*.cpp" "src/*.h" "src/*.c" "src/*.case"

AVRDUDE_WRITE_FLASH = -U flash:w:$(TARGET).hex
AVRDUDE_FLAGS = -p $(MCU) -B $(AVRDUDE_CYCLE) -c $(AVRDUDE_PROGRAMMER)
AVRDUDE_FLAGS += -v -v 
AVRDUDE_FLAGS += $(AVRDUDE_VERBOSE)
AVRDUDE_FLAGS += $(AVRDUDE_ERASE_COUNTER)

program: $(TARGET).hex
	@echo programming
	@avrdude $(AVRDUDE_FLAGS) $(AVRDUDE_WRITE_FLASH)

clean:
	@echo cleaning ...
	-@rm -rf $(BUILD) $(OBJ) $(TESTOBJ) $(TESTCLEAN) 
	-@find . -name "*.lst" -exec rm {} \;

check:
	@echo checking sources...
	cppcheck --enable=all src

test: $(ALLTESTS)

testprog: $(TARGET).elf 
	@echo starting all tests in Simulator
	@$(SIMULAVR) --file $(TARGET).elf --device $(MCU) $(SIMULAVR_OPTS) -c vcd:contrib/tracein.txt:${BUILD}/trace.vcd 


debug_help:
	@echo --------------------------------
	@echo starting Simulator to debug code ...
	@echo Now connect with a debugger
	@echo  avr-gdb --tui
	@echo  cgdb -d avr-gdb
	@echo  ddd --debugger avr-gdb
	@echo .
	@echo after that do this:
	@echo  file build/[name].elf
	@echo  target remote localhost:1212
	@echo  load
	@echo  b main
	@echo  c
	@echo .

debug: $(TARGET).elf debug_help 
	@$(SIMULAVR) -g --file $(TARGET).elf --device $(MCU) $(SIMULAVR_OPTS)

doc:
	doxygen doc/doxygen.conf

help:
	@echo Targets
	@echo .
	@echo  all             - make programm
	@echo  clean           - clean out
	@echo .
	@echo  testprog        - start program in simulator
	@echo  debug           - start program in debug mode
	@echo .
	@echo  test            - execute all tests in the simulator
	@echo  TESTNAME        - execute this test
	@echo  TESTNAME DEBUG - starts test in debug mode
	@echo .
	@echo  program         - download the hex file to the devic
	@echo .
	@echo  format          - format all C/C++ sources
	@echo  check           - static code analysis with cppcheck
	@echo .
	@echo  doc             - Generate Doc
	
.SECONDARY: # do not cleanup intermediate files
.PHONY: clean help all sizeafter format test debug_help check buildDir testenv doc
