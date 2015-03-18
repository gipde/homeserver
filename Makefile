# TODO:
# * AVRSTUDIO (ASM Projects, ASM Includes, Upload)
# * ARDUINO
# * Macros for assertions / Testcases
# * Bauen des Bootloaders
# * Bootloader damit ich nur noch Seriell brauche
# * Upload Application with Uart (wie macht der Arduino Bootloader das, das das Programm läuft, und immer ein Upload möglich ist)


# Serial speed in Baud
BAUD = 38400

# CPU Type
MCU = atmega32
FREQ = 16000000
FUSES = -U lfuse:w:0xcf:m -U hfuse:w:0xc0:m

# Project Name
NAME = homeserver

# explizit List objects and bootobjects here
SRC = $(NAME).o ds18x20lib.o debug.o enc28j60.o hello-world.o uip/uip.o


# Achtung, die Bootstart Adresse ist häufig in Words angegeben
BOOTSRC = boot/bootloader.o sha1-asm.o
BOOTSTART = 0x7000

TRANSFER = atool

# linkage allows multiple definitions for functions in test s -> first wins
TESTOBJ = $(addprefix src/test/,TestBase.o mock.o) src/main/sha1-asm.o
TEST1_OBJ = $(addprefix ds18x20/,Ds18x20libTest.o ds18x20lib_hw.o)

ALLTESTS = TEST1 

# If you do Debugging its better to run with -O0
OPTIMIZE=-Os

AVRDUDE_CYCLE=4
AVRDUDE_PROGRAMMER = avrispmkII

BUILD=build

SIMULAVR=contrib/simulavr/src/simulavr
SIMULAVR_OPTS = --writetopipe 0x20,- --writetoexit 0x21 --terminate exit --cpufrequency=$(FREQ) --irqstatistic 
SIMULAVR_OPTS+= -c vcd:contrib/tracein.txt:${BUILD}/trace.vcd 


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

TARGET=$(BUILD)/$(NAME)
# Windows-Build-Elfs müssen nach dem erzeugen nach toplevel verschoben werden.

# Dependency Files, if changed something, it needs a compile
GENDEPFLAGS = -MD -MP -MF $(BUILD)/.dep/$(@F).d
include $(wildcard $(BUILD)/.dep/*)

GCCOPTS = -g2 -Wall -Wextra ${OPTIMIZE} -DF_CPU=$(FREQ) -DBAUD=$(BAUD)
GCCOPTS += -funsigned-char -funsigned-bitfields -ffunction-sections 
GCCOPTS += -fdata-sections -fpack-struct -fshort-enums -mrelax -nodefaultlibs -nostartfiles -fno-inline-small-functions
GCCOPTS += -mmcu=$(MCU) -I. $(GENDEPFLAGS) 

CC=avr-gcc
HOSTCC=gcc
CFLAGS = ${GCCOPTS} -std=gnu11

CXX=avr-g++
CXXFLAGS = ${GCCOPTS} -std=gnu++11

ASFLAGS = -Wa,-adhlns=$(<:.S=.lst),-gstabs -mmcu=$(MCU) -I. -x assembler-with-cpp

OBJ = $(addprefix src/main/,$(SRC)) 
BOOTOBJ = $(addprefix src/main/,$(BOOTSRC))

LDFLAGS = -Wl,-Map="$(TARGET).map" -Wl,--start-group -Wl,-lm -Wl,--allow-multiple-definition
LDFLAGS += -Wl,--end-group -Wl,--gc-section -mmcu=$(MCU) 

all: $(addprefix $(TARGET)., elf hex eep sym lss) $(addprefix $(BUILD)/bootloader., elf hex eep sym lss) size $(BUILD)/$(TRANSFER)

OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump
SIZE = avr-size
NM = avr-nm

buildDir: 
	@echo Creating Build-Dir and gen tags ...
	-@$(MD) $(BUILD)/.dep
	-@ctags -R src
	-@cscope -R -ssrc -b

$(BUILD)/transfer: src/main/$(TRANSFER)
	@echo Building Transfer Utility $< ...
	@$(HOSTCC) -c $(CFLAGS) $< -o $@

# show size
size: $(TARGET).elf $(OBJ) $(BOOTOBJ)
	@echo .
	@$(SIZE) -C $(TARGET).elf --mcu=${MCU}
	@$(SIZE) $(OBJ) --mcu=${MCU}

# Link: create ELF output file from object files.
%.elf: $(OBJ)
	@echo Linking Application $@ ...
	@$(CC) -o $@  $(OBJ) $(LDFLAGS)

$(BUILD)/bootloader.elf: $(BOOTOBJ)
	@echo Linking Bootloader $@ ...
	@$(CC) -o $@ $(BOOTOBJ) $(LDFLAGS) -Wl,-Ttext=$(BOOTSTART)
	@$(SIZE) -C $(BUILD)/bootloader.elf --mcu=$(MCU)

# Compile: create object files from C source files.
%.o : %.c | buildDir
	@echo Compiling $@ ...
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

$(BUILD)/$(TRANSFER) : src/main/$(TRANSFER).c | buildDir
	gcc -std=gnu11 -O2 -g -o $(BUILD)/$(TRANSFER) src/main/$(TRANSFER).c -lcrypto 

testenv:
	@echo Compiling in TESTMODE ...
	$(eval TESTBUILD=-D_TESTBUILD_)
	
define TEST_template 
CASEOBJ=$(addprefix src/test/,$($(1)_OBJ))
TESTCLEAN += $$(CASEOBJ)
$(BUILD)/$(1).elf: $$(CASEOBJ) $(TESTOBJ) $(OBJ)
	@echo Building for Test: $(1).elf ... 
	$(CC) -o $(BUILD)/$(1).elf $$^ $(TESTOBJ) $(OBJ) $(LDFLAGS)

$(1): testenv $(BUILD)/$(1).elf $(BUILD)/$(1).lss 
	@echo .
	@$(SIZE) -C $(BUILD)/$(1).elf --mcu=${MCU}
	@echo .
	@echo DO NOT FORGET TO CLEAN!
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

AVRDUDE_FLAGS = -p $(MCU) -B $(AVRDUDE_CYCLE) -c $(AVRDUDE_PROGRAMMER)
AVRDUDE_FLAGS += -v -v 
AVRDUDE_FLAGS += $(AVRDUDE_VERBOSE)
AVRDUDE_FLAGS += $(AVRDUDE_ERASE_COUNTER)

program_fuses:
	@avrdude $(AVRDUDE_FLAGS) $(FUSES)

program: $(TARGET).hex $(BUILD)/bootloader.hex
	@echo programming device ...
#	@avrdude $(AVRDUDE_FLAGS) -U flash:w:$(TARGET).hex 
	avrdude $(AVRDUDE_FLAGS) -U flash:w:$(BUILD)/bootloader.hex 
#	avrdude $(AVRDUDE_FLAGS) -U flash:r:$(BUILD)/flash.hex.read:i

upload: $(TARGET).hex
	@echo uploading $< ...

clean:
	@echo cleaning ...
	-@rm -rf $(BUILD) $(OBJ) $(TESTOBJ) $(BOOTOBJ) $(TESTCLEAN) 
	-@find . -name "*.lst" -exec rm {} \;

check:
	@echo checking sources...
	cppcheck --enable=all src

test: $(ALLTESTS)

testprog: $(TARGET).elf 
	@echo starting all tests in Simulator
	@$(SIMULAVR) --file $(TARGET).elf --device $(MCU) $(SIMULAVR_OPTS) 

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
	@echo  "all (dflt)      - make application"
	@echo  clean           - clean out
	@echo .
	@echo  testprog        - start program in simulator
	@echo  debug           - start program in debug mode
	@echo .
	@echo  test            - execute all tests in the simulator
	@echo  TESTNAME        - execute this test
	@echo  TESTNAME DEBUG - starts test in debug mode
	@echo .
	@echo  program         - download the hex file to the device
	@echo  upload          - upload programm via bootloader
	@echo .
	@echo  format          - format all C/C++ sources
	@echo  check           - static code analysis with cppcheck
	@echo .
	@echo  doc             - Generate Doc
	
.SECONDARY: # do not cleanup intermediate files
.PHONY: clean help all sizeafter format test debug_help check buildDir testenv doc
