###############################################################################
# Makefile for BACnet
###############################################################################

## General Flags
MCU = atmega328p
AVRDUDE_MCU = m328
TARGET = bacnet
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

# Source locations
BACNET_CORE = ../../src
BACNET_INCLUDE = ../../include
BACNET_HANDLER = ../../demo/handler
BACNET_OBJECT = ../../demo/object
BACNET_DEMO = ../../demo
ARDUINO_CORE = external/Arduino/core
ARDUINO_ETHERNET = external/Arduino/Ethernet

# local files for this project
CSRC = main.c \
	uart.c \
	apdu.c \
	h_rp.c \
	device.c \
	av.c \
	bv.c \
	h_whois.c \
	h_wp.c \
	bip.c \
	bip-init.c \
	bvlc-arduino.c

# common demo files needed
DEMOSRC = $(BACNET_DEMO)/handler/txbuf.c \
	$(BACNET_DEMO)/handler/h_npdu.c \
	$(BACNET_DEMO)/handler/s_iam.c \
	$(BACNET_DEMO)/handler/noserv.c

# core BACnet stack files
CORESRC =  \
	$(BACNET_CORE)/crc.c \
	$(BACNET_CORE)/npdu.c \
	$(BACNET_CORE)/bacdcode.c \
	$(BACNET_CORE)/bacint.c \
	$(BACNET_CORE)/bacreal.c \
	$(BACNET_CORE)/bacstr.c \
	$(BACNET_CORE)/iam.c \
	$(BACNET_CORE)/rp.c \
	$(BACNET_CORE)/wp.c \
	$(BACNET_CORE)/whois.c \
	$(BACNET_CORE)/bacaddr.c \
	$(BACNET_CORE)/abort.c \
	$(BACNET_CORE)/reject.c \
	$(BACNET_CORE)/bacerror.c \
	$(BACNET_CORE)/bacapp.c \
#	$(BACNET_CORE)/ethernet.c


## Include Directories
INCLUDES = -I. -I$(BACNET_INCLUDE)
INCLUDES += -I$(BACNET_OBJECT)
INCLUDES += -I$(BACNET_HANDLER)
INCLUDES += -I$(ARDUINO_CORE)/include
INCLUDES += -I$(ARDUINO_ETHERNET)/include

# Source to Object conversion
COBJ = $(CSRC:.c=.o)
DEMOOBJ = $(DEMOSRC:.c=.o)
COREOBJ = $(CORESRC:.c=.o)


LIBRARY = lib$(TARGET).a

## Options common to compile, link and assembly rules
COMMON = -mmcu=$(MCU) -DF_CPU=16000000UL

#OPTIMIZE_FLAGS = -mcall-prologues
#OPTIMIZE_FLAGS += -finline-functions-called-once -ffunction-sections -fdata-sections
#OPTIMIZATION = -Os $(OPTIMIZE_FLAGS)
OPTIMIZATION = -Os

#OPTIMIZATION = -O3 $(OPTIMIZE_FLAGS)

## Compile options common for all C compilation units.
# BFLAGS = -DBACDL_MSTP
#BFLAGS = -DBACDL_ETHERNET=1
BFLAGS = -DBACDL_BIP=1
BFLAGS += -DMAX_APDU=100
BFLAGS += -DBIG_ENDIAN=0
BFLAGS += -DMAX_TSM_TRANSACTIONS=0
#BFLAGS += -DCRC_USE_TABLE
BFLAGS += -DBACAPP_REAL
BFLAGS += -DBACAPP_OBJECT_ID
BFLAGS += -DBACAPP_UNSIGNED
BFLAGS += -DBACAPP_ENUMERATED
BFLAGS += -DBACAPP_CHARACTER_STRING
BFLAGS += -DWRITE_PROPERTY
BFLAGS += -DMAX_ANALOG_VALUES=4
BFLAGS += -DMAX_BINARY_VALUES=4
#BFLAGS += -DDEBUG
CFLAGS = $(COMMON)
# dead code removal
CFLAGS += -ffunction-sections -fdata-sections
CFLAGS += -Wall -g2 -gstabs -std=gnu99 $(BFLAGS) $(OPTIMIZATION)
CFLAGS += -MMD -MP -MT $(*F).o -MF dep/$(@F).d
#CFLAGS += -Wall -gdwarf-2 $(BFLAGS) $(OPTIMIZATION) -fsigned-char
#CFLAGS += -MD -MP -MT $(*F).o -MF dep/$(@F).d

## Assembly specific flags
ASMFLAGS = $(COMMON)
ASMFLAGS += $(CFLAGS)
ASMFLAGS += -x assembler-with-cpp -Wa,-gdwarf2

## Linker flags
LDFLAGS = $(COMMON)
#dead code removal
LDFLAGS += -Wl,--gc-sections,-static
LDFLAGS += -Wl,-Map=$(TARGET).map,-L.,-l$(TARGET)
LDFLAGS += -Wl,-L$(ARDUINO_ETHERNET)/lib,-lArduinoEthernet
LDFLAGS += -Wl,-L$(ARDUINO_CORE)/lib,-lArduinoUnoCore,-lm

## Intel Hex file production flags
HEX_FLASH_FLAGS = -R .eeprom
HEX_EEPROM_FLAGS = -j .eeprom
HEX_EEPROM_FLAGS += --set-section-flags=.eeprom="alloc,load"
HEX_EEPROM_FLAGS += --change-section-lma .eeprom=0 --no-change-warnings

## Objects that must be built in order to link
OBJECTS = $(COBJ) $(DEMOOBJ)
#OBJECTS = $(COBJ)

## Build
TARGET_ELF=$(TARGET).elf

all: EthernetLib $(LIBRARY) $(TARGET_ELF) $(TARGET).hex $(TARGET).eep $(TARGET).lst \
	size Makefile

EthernetLib:
	$(MAKE) -s -C external/Arduino/Ethernet all

##Link
$(TARGET_ELF): $(OBJECTS) $(LIBRARY)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@

%.hex: $(TARGET_ELF)
	$(OBJCOPY) -O ihex $(HEX_FLASH_FLAGS) $< $@

%.eep: $(TARGET_ELF)
	-$(OBJCOPY) $(HEX_EEPROM_FLAGS) -O ihex $< $@ || exit 0

%.lst: $(TARGET_ELF)
	$(OBJDUMP) -h -S $< > $@

lib: $(LIBRARY)

$(LIBRARY): $(COREOBJ) Makefile
	$(AR) rcs $@ $(COREOBJ)
	$(OBJDUMP) --syms $@ > $(LIBRARY:.a=.lst)

.c.o:
	$(CC) -c $(INCLUDES) $(CFLAGS) $*.c -o $@

.cpp.o:
	$(CC) -c $(INCLUDES) $(CFLAGS) $*.cpp -o $@

size: ${TARGET_ELF}
	@echo
	@${SIZE} -C --mcu=${MCU} ${TARGET_ELF}

lint:
	$(LINT) $(BFLAGS) $(CSRC)

install: $(TARGET_ELF)
	$(AVRDUDE) -c $(AVRDUDE_PROGRAMMERID)   \
	 -p $(AVRDUDE_MCU) -P $(AVRDUDE_PORT) -e        \
	 -U flash:w:$(TARGET).hex

## Clean target
.PHONY: clean
clean:
	-rm -rf $(OBJECTS) $(TARGET_ELF) dep/*
	-rm -rf $(LIBRARY) $(COREOBJ)
#	-rm -rf $(LIBRARY) $(COREOBJ) $(LIBRARY:.a=.lst)
	-rm -rf $(TARGET).hex $(TARGET).eep $(TARGET).lst $(TARGET).map
	cd external/Arduino/Ethernet; make clean

## Other dependencies
-include $(shell mkdir dep 2>/dev/null) $(wildcard dep/*)
