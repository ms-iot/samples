# Makefile for AT91SAM7S evaluation kit with RS-485 Transceiver on UART0
# Written by Steve Karg <skarg@users.sourceforge.net> 06-Aug-2007

TARGET=bacnet

# Tools
#PREFIX=arm-elf-
PREFIX ?= arm-none-eabi-
#
CC = $(PREFIX)gcc
OBJCOPY = $(PREFIX)objcopy
OBJDUMP = $(PREFIX)objdump
AR = $(PREFIX)ar
SIZE = $(PREFIX)size

LDSCRIPT = at91sam7s256.ld

BACNET_FLAGS = -DBACDL_MSTP
BACNET_FLAGS += -DMAX_TSM_TRANSACTIONS=0
BACNET_FLAGS += -DMAX_CHARACTER_STRING_BYTES=64
BACNET_FLAGS += -DMAX_OCTET_STRING_BYTES=64
BACNET_FLAGS += -DPRINT_ENABLED=0
BACNET_FLAGS += -DMAX_APDU=480
BACNET_FLAGS += -DCRC_USE_TABLE
#BACNET_FLAGS += -DDLMSTP_TEST

BACNET_CORE = ../../src
BACNET_DEMO = ../../demo
BACNET_INCLUDE = ../../include
BACNET_OBJECT = ../../demo/object
BACNET_HANDLER = ../../demo/handler
INCLUDES = -I.
INCLUDES += -I$(BACNET_INCLUDE)
INCLUDES += -I$(BACNET_OBJECT)
INCLUDES += -I$(BACNET_HANDLER)
#OPTIMIZATION = -O0
OPTIMIZATION = -Os
CFLAGS = -fno-common $(INCLUDES) $(BACNET_FLAGS) -Wall -g
CFLAGS += -mno-thumb-interwork
# dead code removal
CFLAGS += -fdata-sections -ffunction-sections
LIBRARY = lib$(TARGET).a
#  -Wa,<options> Pass comma-separated <options> on to the assembler
AFLAGS = -Wa,-ahls,-mapcs-32,-adhlns=$(<:.s=.lst)
#  -Wl,<options> Pass comma-separated <options> on to the linker
LIBRARIES=-lc,-lgcc,-lm,-L.,-l$(TARGET)
LDFLAGS = -nostartfiles
LDFLAGS += -Wl,-nostdlib,-Map=$(TARGET).map,$(LIBRARIES),-T$(LDSCRIPT)
# dead code removal
LDFLAGS += -Wl,--gc-sections,-static
CPFLAGS = --output-target=binary
ODFLAGS	= -x --syms

ASRC = crt.s

PORTSRC = main.c \
	timer.c \
	isr.c \
	init.c \
	blinker.c \
	rs485.c \
	dlmstp.c

DEMOSRC = ai.c \
	av.c \
	bi.c \
	bv.c \
	device.c \
	$(BACNET_DEMO)/handler/txbuf.c \
	$(BACNET_DEMO)/handler/noserv.c \
	$(BACNET_DEMO)/handler/h_npdu.c \
	$(BACNET_DEMO)/handler/h_whohas.c \
	$(BACNET_DEMO)/handler/h_whois.c \
	$(BACNET_DEMO)/handler/h_rd.c \
	$(BACNET_DEMO)/handler/h_rp.c \
	$(BACNET_DEMO)/handler/h_rpm.c \
	$(BACNET_DEMO)/handler/h_wp.c \
	$(BACNET_DEMO)/handler/h_dcc.c \
	$(BACNET_DEMO)/handler/s_iam.c \
	$(BACNET_DEMO)/handler/s_ihave.c

CORESRC = $(BACNET_CORE)/abort.c \
	$(BACNET_CORE)/apdu.c \
	$(BACNET_CORE)/bacaddr.c \
	$(BACNET_CORE)/bacapp.c \
	$(BACNET_CORE)/bacdcode.c \
	$(BACNET_CORE)/bacerror.c \
	$(BACNET_CORE)/bacint.c \
	$(BACNET_CORE)/bacreal.c \
	$(BACNET_CORE)/bacstr.c \
	$(BACNET_CORE)/crc.c \
	$(BACNET_CORE)/datetime.c \
	$(BACNET_CORE)/dcc.c \
	$(BACNET_CORE)/iam.c \
	$(BACNET_CORE)/ihave.c \
	$(BACNET_CORE)/lighting.c \
	$(BACNET_CORE)/memcopy.c \
	$(BACNET_CORE)/npdu.c \
	$(BACNET_CORE)/proplist.c \
	$(BACNET_CORE)/rd.c \
	$(BACNET_CORE)/reject.c \
	$(BACNET_CORE)/ringbuf.c \
	$(BACNET_CORE)/rp.c \
	$(BACNET_CORE)/rpm.c \
	$(BACNET_CORE)/version.c \
	$(BACNET_CORE)/whohas.c \
	$(BACNET_CORE)/whois.c \
	$(BACNET_CORE)/wp.c

CSRC = $(PORTSRC) $(DEMOSRC)
#CSRC = $(PORTSRC)

AOBJ = $(ASRC:.s=.o)
COBJ = $(CSRC:.c=.o)
COREOBJ = $(CORESRC:.c=.o)

all: $(TARGET).bin $(TARGET).elf
	$(OBJDUMP) $(ODFLAGS) $(TARGET).elf > $(TARGET).dmp
	$(SIZE) $(TARGET).elf

$(TARGET).bin:  $(TARGET).elf
	$(OBJCOPY) $(TARGET).elf $(CPFLAGS) $(TARGET).bin

$(TARGET).elf: $(COBJ) $(AOBJ) $(LIBRARY) Makefile
	$(CC) $(CFLAGS) $(AOBJ) $(COBJ) $(LDFLAGS) -o $@

lib: $(LIBRARY)

$(LIBRARY): $(COREOBJ) Makefile
	$(AR) rcs $@ $(COREOBJ)

# allow a single file to be unoptimized for debugging purposes
#dlmstp.o:
#	$(CC) -c $(CFLAGS) $*.c -o $@
#
#main.o:
#	$(CC) -c $(CFLAGS) $*.c -o $@
#
#$(BACNET_CORE)/npdu.o:
#	$(CC) -c $(CFLAGS) $*.c -o $@
#
#$(BACNET_CORE)/apdu.o:
#	$(CC) -c $(CFLAGS) $*.c -o $@

.c.o:
	$(CC) -c $(OPTIMIZATION) $(CFLAGS) $*.c -o $@

.s.o:
	$(CC) -c $(AFLAGS) $*.s -o $@

.PHONY: clean
clean:
	-rm -rf $(COBJ) $(AOBJ) $(COREOBJ)
	-rm -rf $(TARGET).elf $(TARGET).bin $(TARGET).dmp $(TARGET).map
	-rm -rf $(LIBRARY)
	-rm -rf *.lst

## Other dependencies
-include $(shell mkdir dep 2>/dev/null) $(wildcard dep/*)
