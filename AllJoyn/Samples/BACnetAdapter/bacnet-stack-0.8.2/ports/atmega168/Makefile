###############################################################################
# Makefile for BACnet
###############################################################################

## General Flags
MCU = atmega168
AVRDUDE_MCU = m168
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

# local files for this project
CSRC = main.c \
	timer.c \
	stack.c \
	rs485.c \
	dlmstp.c \
	apdu.c \
	h_rp.c \
	device.c \
	av.c \
	bv.c \
	h_whois.c \
	h_wp.c

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
	$(BACNET_CORE)/bacapp.c

#	$(BACNET_CORE)/version.c
#	$(BACNET_CORE)/bacprop.c \
#	$(BACNET_CORE)/bactext.c \
#	$(BACNET_CORE)/datetime.c \
#	$(BACNET_CORE)/indtext.c \
#	$(BACNET_CORE)/bigend.c \
#	$(BACNET_CORE)/arf.c \
#	$(BACNET_CORE)/awf.c \
#	$(BACNET_CORE)/cov.c \
#	$(BACNET_CORE)/dcc.c \
#	$(BACNET_CORE)/iam/iam_client.c \
#	$(BACNET_CORE)/ihave.c \
#	$(BACNET_CORE)/rd.c \
#	$(BACNET_CORE)/rpm.c \
#	$(BACNET_CORE)/timesync.c \
#	$(BACNET_CORE)/whohas.c \
#	$(BACNET_CORE)/filename.c \
#	$(BACNET_CORE)/tsm.c \
#	$(BACNET_CORE)/address.c \

## Include Directories
INCLUDES = -I. -I$(BACNET_INCLUDE)
INCLUDES += -I$(BACNET_OBJECT)
INCLUDES += -I$(BACNET_HANDLER)

# Source to Object conversion
COBJ = $(CSRC:.c=.o)
DEMOOBJ = $(DEMOSRC:.c=.o)
COREOBJ = $(CORESRC:.c=.o)

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
BFLAGS = -DBACDL_MSTP
BFLAGS += -DMAX_APDU=50
BFLAGS += -DBIG_ENDIAN=0
BFLAGS += -DMAX_TSM_TRANSACTIONS=0
#BFLAGS += -DCRC_USE_TABLE
BFLAGS += -DBACAPP_REAL
BFLAGS += -DBACAPP_OBJECT_ID
BFLAGS += -DBACAPP_UNSIGNED
BFLAGS += -DBACAPP_ENUMERATED
BFLAGS += -DBACAPP_CHARACTER_STRING
BFLAGS += -DWRITE_PROPERTY
BFLAGS += -DMAX_ANALOG_VALUES=10
BFLAGS += -DMAX_BINARY_VALUES=10
CFLAGS = $(COMMON)
# dead code removal
CFLAGS += -ffunction-sections -fdata-sections
CFLAGS += -Wall -gdwarf-2 $(BFLAGS) $(OPTIMIZATION) -fsigned-char
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

all: $(LIBRARY) $(TARGET_ELF) $(TARGET).hex $(TARGET).eep $(TARGET).lst \
	size Makefile

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

size: ${TARGET_ELF}
	@echo
	@${SIZE} ${TARGET_ELF}

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
	-rm -rf $(LIBRARY) $(COREOBJ) $(LIBRARY:.a=.lst)
	-rm -rf $(TARGET).hex $(TARGET).eep $(TARGET).lst $(TARGET).map

## Other dependencies
-include $(shell mkdir dep 2>/dev/null) $(wildcard dep/*)
