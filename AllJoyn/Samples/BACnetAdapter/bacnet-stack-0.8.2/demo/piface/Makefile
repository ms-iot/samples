###############################################################################
# Makefile for Project - PiFace
###############################################################################

## General Flags
TARGET = bacpiface

# Source locations
BACNET_CORE = ../../src
BACNET_INCLUDE = ../../include
BACNET_HANDLER = ../../demo/handler
BACNET_OBJECT = ../../demo/object
BACNET_DEMO = ../../demo
BACNET_PORT_DIR = ../../ports/linux
PIFACE_INCLUDE = libpifacedigital/src
PIFACE_LIB = libpifacedigital
MCP23S17_LIB = libmcp23s17

## BACnet options
BACDL_DEFINE ?= -DBACDL_BIP=1
# Declare your level of BBMD support
BBMD_DEFINE ?= -DBBMD_ENABLED=1

# local files for this project
CSRC = main.c \
	device.c

# common demo files needed
DEMOSRC = \
        $(BACNET_HANDLER)/dlenv.c \
        $(BACNET_HANDLER)/txbuf.c \
        $(BACNET_HANDLER)/noserv.c \
        $(BACNET_HANDLER)/h_npdu.c \
        $(BACNET_HANDLER)/h_whois.c \
        $(BACNET_HANDLER)/h_iam.c  \
        $(BACNET_HANDLER)/h_rp.c \
        $(BACNET_HANDLER)/h_rp_a.c \
        $(BACNET_HANDLER)/h_rpm.c \
        $(BACNET_HANDLER)/h_rpm_a.c \
        $(BACNET_HANDLER)/h_rr.c \
        $(BACNET_HANDLER)/h_wp.c  \
        $(BACNET_HANDLER)/h_wpm.c \
        $(BACNET_HANDLER)/h_alarm_ack.c  \
        $(BACNET_HANDLER)/h_arf.c  \
        $(BACNET_HANDLER)/h_arf_a.c  \
        $(BACNET_HANDLER)/h_awf.c  \
        $(BACNET_HANDLER)/h_rd.c  \
        $(BACNET_HANDLER)/h_dcc.c  \
        $(BACNET_HANDLER)/h_ts.c  \
        $(BACNET_HANDLER)/h_whohas.c  \
        $(BACNET_HANDLER)/h_ihave.c  \
        $(BACNET_HANDLER)/h_cov.c  \
        $(BACNET_HANDLER)/h_ccov.c  \
        $(BACNET_HANDLER)/h_ucov.c  \
        $(BACNET_HANDLER)/h_getevent.c  \
        $(BACNET_HANDLER)/h_gas_a.c  \
        $(BACNET_HANDLER)/h_get_alarm_sum.c  \
        $(BACNET_HANDLER)/h_getevent_a.c  \
        $(BACNET_HANDLER)/h_pt.c  \
        $(BACNET_HANDLER)/h_pt_a.c  \
        $(BACNET_HANDLER)/h_upt.c  \
        $(BACNET_HANDLER)/s_arfs.c \
        $(BACNET_HANDLER)/s_awfs.c \
        $(BACNET_HANDLER)/s_dcc.c \
        $(BACNET_HANDLER)/s_ihave.c \
        $(BACNET_HANDLER)/s_get_alarm_sum.c  \
        $(BACNET_HANDLER)/s_get_event.c  \
        $(BACNET_HANDLER)/s_iam.c  \
        $(BACNET_HANDLER)/s_cov.c  \
        $(BACNET_HANDLER)/s_ptransfer.c \
        $(BACNET_HANDLER)/s_rd.c \
        $(BACNET_HANDLER)/s_rp.c  \
        $(BACNET_HANDLER)/s_rpm.c  \
        $(BACNET_HANDLER)/s_ts.c \
        $(BACNET_HANDLER)/s_cevent.c  \
        $(BACNET_HANDLER)/s_router.c  \
        $(BACNET_HANDLER)/s_uevent.c  \
        $(BACNET_HANDLER)/s_whohas.c \
        $(BACNET_HANDLER)/s_whois.c  \
        $(BACNET_HANDLER)/s_wpm.c  \
        $(BACNET_HANDLER)/s_upt.c \
        $(BACNET_HANDLER)/s_wp.c

OBJSRC = \
	$(BACNET_OBJECT)/bi.c \
	$(BACNET_OBJECT)/bo.c

# core BACnet stack files
CORESRC =  \
        $(BACNET_CORE)/apdu.c \
        $(BACNET_CORE)/npdu.c \
        $(BACNET_CORE)/bacdcode.c \
        $(BACNET_CORE)/bacint.c \
        $(BACNET_CORE)/bacreal.c \
        $(BACNET_CORE)/bacstr.c \
        $(BACNET_CORE)/bacapp.c \
        $(BACNET_CORE)/bacprop.c \
        $(BACNET_CORE)/bactext.c \
        $(BACNET_CORE)/datetime.c \
        $(BACNET_CORE)/indtext.c \
        $(BACNET_CORE)/key.c \
        $(BACNET_CORE)/keylist.c \
        $(BACNET_CORE)/proplist.c \
        $(BACNET_CORE)/debug.c \
        $(BACNET_CORE)/bigend.c \
        $(BACNET_CORE)/arf.c \
        $(BACNET_CORE)/awf.c \
        $(BACNET_CORE)/cov.c \
        $(BACNET_CORE)/dcc.c \
        $(BACNET_CORE)/iam.c \
        $(BACNET_CORE)/ihave.c \
        $(BACNET_CORE)/rd.c \
        $(BACNET_CORE)/rp.c \
        $(BACNET_CORE)/rpm.c \
        $(BACNET_CORE)/timesync.c \
        $(BACNET_CORE)/whohas.c \
        $(BACNET_CORE)/whois.c \
        $(BACNET_CORE)/wp.c \
        $(BACNET_CORE)/wpm.c \
        $(BACNET_CORE)/abort.c \
        $(BACNET_CORE)/reject.c \
        $(BACNET_CORE)/bacerror.c \
        $(BACNET_CORE)/ptransfer.c \
        $(BACNET_CORE)/memcopy.c \
        $(BACNET_CORE)/filename.c \
        $(BACNET_CORE)/tsm.c \
        $(BACNET_CORE)/bacaddr.c \
        $(BACNET_CORE)/address.c \
        $(BACNET_CORE)/bacdevobjpropref.c \
        $(BACNET_CORE)/bacpropstates.c \
        $(BACNET_CORE)/alarm_ack.c \
        $(BACNET_CORE)/event.c \
        $(BACNET_CORE)/getevent.c \
        $(BACNET_CORE)/get_alarm_sum.c \
        $(BACNET_CORE)/readrange.c \
        $(BACNET_CORE)/timestamp.c \
        $(BACNET_CORE)/lighting.c \
        $(BACNET_CORE)/version.c

PORT_BIP_SRC = \
        $(BACNET_PORT_DIR)/bip-init.c \
        $(BACNET_CORE)/bvlc.c \
        $(BACNET_CORE)/bip.c

ifeq (${BACDL_DEFINE},-DBACDL_BIP=1)
PORT_SRC = ${PORT_BIP_SRC}
endif

## Include Directories
INCLUDES = -I. -I$(BACNET_INCLUDE) -I$(BACNET_HANDLER) -I$(BACNET_OBJECT)
INCLUDES += -I$(BACNET_PORT_DIR)
INCLUDES += -I$(PIFACE_INCLUDE)

# Source to Object conversion
COBJ = $(CSRC:%.c=%.o)
DEMOOBJ = $(DEMOSRC:.c=.o)
COREOBJ = $(CORESRC:.c=.o)
OBJOBJ = $(OBJSRC:.c=.o)
PORTOBJ = $(PORT_SRC:.c=.o)

# define something from the Makefile or batch file
DEFINES = $(BACDL_DEFINE) $(BBMD_DEFINE)
OPTIMIZATION = -Os  $(OPTIMIZE_FLAGS)
DEBUGGING = -g
ifeq (${BUILD},debug)
OPTIMIZATION = -O0
DEBUGGING = -g
endif
ifeq (${BUILD},release)
OPTIMIZATION = -Os $(OPTIMIZE_FLAGS)
DEBUGGING = -DNDEBUG
endif

BFLAGS += -DMAX_TSM_TRANSACTIONS=1
BFLAGS += -DWRITE_PROPERTY
BFLAGS += -DMAX_BINARY_INPUTS=4
BFLAGS += -DMAX_BINARY_OUTPUTS=8

## Compile options for C files
CFLAGS = $(DEFINES)
CFLAGS += $(DEBUGGING)
CFLAGS += $(INCLUDES)
# warnings
CFLAGS += -Wall
CFLAGS += -Wstrict-prototypes
CFLAGS += -Wmissing-prototypes
# put it all together
CFLAGS += $(BFLAGS) $(OPTIMIZATION)
CFLAGS += -MD -MP -MT $(*F).o -MF dep/$(@F).d

## Linker flags
LDPIFACE = -Wl,-L$(PIFACE_LIB),-lpifacedigital,-L$(MCP23S17_LIB),-lmcp23s17
LDFLAGS = -Wl,-lc,-lgcc,-lrt,-lm
LFLAGS := $(LDPIFACE) $(LDFLAGS)

## Objects not in library that must be built in order to link
OBJECTS = $(COBJ) $(DEMOOBJ) $(COREOBJ) $(OBJOBJ) $(PORTOBJ)

all: $(TARGET)

$(TARGET): $(OBJECTS) $(LIBRARY)
	$(CC) $(OBJECTS) $(LFLAGS) -o $@


.c.o:
	${CC} -c ${CFLAGS} $*.c -o $@

depend:
	rm -f .depend
	${CC} -MM ${CFLAGS} *.c >> .depend

clean:
	rm -f core ${TARGET} ${OBJECTS}

include: .depend


.PHONY: clean all include depend

## Other dependencies
-include $(shell mkdir dep 2>/dev/null) $(wildcard dep/*)

