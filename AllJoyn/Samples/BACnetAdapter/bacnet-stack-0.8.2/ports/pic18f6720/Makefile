###############################################################################
# Makefile for BACnet
###############################################################################

## General Flags
MCU = pic18
PORT = 18f6720
TARGET = bacnet
## Tools
CC = sdcc
PACK = packihx

# Source locations
BACNET_CORE = ../../src
BACNET_INCLUDE = ../../include
BACNET_DEMO = ../../demo

CSRC = main.c \
	isr.c \
	rs485.c \
	dlmstp.c \
	mstp.c \
	$(BACNET_CORE)/crc.c

DEMOSRC = h_rp.c \
	device.c \
	ai.c \
	av.c \
	bi.c \
	bv.c \
	$(BACNET_DEMO)/handler/txbuf.c \
	$(BACNET_DEMO)/handler/noserv.c \
	$(BACNET_DEMO)/handler/h_npdu.c \
	$(BACNET_DEMO)/handler/h_whois.c \
	h_wp.c

CORESRC =  \
	$(BACNET_CORE)/apdu.c \
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
	$(BACNET_CORE)/version.c

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

# Source to Object conversion
COBJ = $(CSRC:.c=.o)
DEMOOBJ = $(DEMOSRC:.c=.o)
COREOBJ = $(CORESRC:.c=.o)

LIBRARY = lib$(TARGET).a

## Options common to compile, link and assembly rules
COMMON = -m$(MCU) -p$(PORT)
OPTIMIZATION = --opt-code-size

## Compile options common for all C compilation units.
BFLAGS = -DBACDL_MSTP
BFLAGS += -DMAX_APDU=128
BFLAGS += -DBIG_ENDIAN=0
BFLAGS += -DMAX_TSM_TRANSACTIONS=0
#BFLAGS += -DCRC_USE_TABLE
BFLAGS += -DBACAPP_REAL
CFLAGS = $(COMMON)
# dead code removal
CFLAGS += -ffunction-sections -fdata-sections
CFLAGS += -Wall $(BFLAGS) $(OPTIMIZATION)
CFLAGS += -MD -MP -MT $(*F).o -MF dep/$(@F).d

## Assembly specific flags
ASMFLAGS = $(COMMON)
ASMFLAGS += $(CFLAGS)
ASMFLAGS += -x assembler-with-cpp -Wa,-gdwarf2

## Linker flags
LDFLAGS = $(COMMON)
#dead code removal
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
	@${SIZE} -C --mcu=${MCU} ${TARGET_ELF}

## Clean target
.PHONY: clean
clean:
	touch Makefile
	-rm -rf $(OBJECTS) $(TARGET_ELF) dep/*
	-rm -rf $(LIBRARY) $(COREOBJ) $(LIBRARY:.a=.lst)
	-rm -rf $(TARGET).hex $(TARGET).eep $(TARGET).lst $(TARGET).map

## Other dependencies
-include $(shell mkdir dep 2>/dev/null) $(wildcard dep/*)
