###############################################################################
# Makefile for BACnet
###############################################################################

## General Flags
MCU = atmega328p
AVRDUDE_MCU = m328
TARGET = ArduinoEthernet
## Tools
CC = avr-gcc
AR = avr-ar
OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump
SIZE = avr-size
AVRDUDE = avrdude
LINT = splint

# programmer id--check the avrdude for complete list
# # of available opts.  These should include stk500,
# # avr910, avrisp, bsd, pony and more.  Set this to
# # one of the valid "-c PROGRAMMER-ID" values
# # described in the avrdude info page.
# #
AVRDUDE_PROGRAMMERID = avrispmkII
#
# # port--serial or parallel port to which your
# # hardware programmer is attached
# #
AVRDUDE_PORT = /dev/ttyUSB0

# local files for this project
#CSRC = main.c

ARDUINOSRC = \
	src/socket.cpp \
	src/w5100.cpp \
	src/SPI.cpp \
	src/w5100Wrapper.cpp \
	src/socketWrapper.cpp

## Include Directories
INCLUDES = -Iinclude
INCLUDES += -I../core/include

LIBRARY = lib$(TARGET).a

## Options common to compile, link and assembly rules
COMMON = -mmcu=$(MCU)

OPTIMIZE_FLAGS = -mcall-prologues
#OPTIMIZE_FLAGS += -finline-functions
OPTIMIZE_FLAGS += -finline-functions-called-once
#OPTIMIZATION = -O0
#OPTIMIZATION = -Os
OPTIMIZATION = -Os $(OPTIMIZE_FLAGS)
#OPTIMIZATION = -O3 $(OPTIMIZE_FLAGS)

## Compile options common for all C compilation units.
CFLAGS = $(COMMON)
# dead code removal
CFLAGS += -ffunction-sections -fdata-sections
CFLAGS += -Wall -gdwarf-2 $(OPTIMIZATION) -fsigned-char
CFLAGS += -MD -MP -MT $(*F).o -MF dep/$(@F).d

## Assembly specific flags
ASMFLAGS = $(COMMON)
ASMFLAGS += $(CFLAGS)
ASMFLAGS += -x assembler-with-cpp -Wa,-gdwarf2

## Linker flags
LDFLAGS = $(COMMON)
#dead code removal
#LDFLAGS += -Wl,-nostartfiles,-nostdlib
LDFLAGS += -Wl,--gc-sections,-static
LDFLAGS += -Wl,-Map=$(TARGET).map,-L.,-l$(TARGET)
#LDFLAGS += -Wl,-Map=$(TARGET).map
LDFLAGS += -L../core/lib,-lArduinoUnoCore

## Intel Hex file production flags
HEX_FLASH_FLAGS = -R .eeprom
HEX_EEPROM_FLAGS = -j .eeprom
HEX_EEPROM_FLAGS += --set-section-flags=.eeprom="alloc,load"
HEX_EEPROM_FLAGS += --change-section-lma .eeprom=0 --no-change-warnings

## Objects that must be built in order to link
OBJS := ${SRCS:.cpp=.o}

all: $(LIBRARY)	size Makefile

default: all

lib: $(LIBRARY)

$(LIBRARY): $(OBJS) Makefile
	$(AR) rcs lib/$@ $(OBJS)
	$(OBJDUMP) --syms lib/$@ > lib/$(LIBRARY:.a=.lst)

.c.o:
	$(CC) -c $(INCLUDES) $(CFLAGS) $*.c -o $@

.cpp.o:
	$(CC) -c $(INCLUDES) $(CFLAGS) $*.cpp -o $@

size: ${TARGET_ELF}
	@echo
	@${SIZE} ${TARGET_ELF}

lint:
	$(LINT) $(BFLAGS) $(CSRC)

## Clean target
.PHONY: clean
clean:
	-rm -rf $(OBJS) dep/*
	-rm -rf $(LIBRARY) $(OBJS)

## Other dependencies
-include $(shell mkdir dep 2>/dev/null) $(wildcard dep/*)
