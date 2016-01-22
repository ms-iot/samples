#Makefile to build BACnet Library with GCC

# tools - only if you need them.
# Most platforms have this already defined
# CC = gcc
# AR = ar
# MAKE = make
# SIZE = size
#
# Assumes rm and cp are available

BACNET_PORT_DIR = ../ports/${BACNET_PORT}
BACNET_OBJECT = ../demo/object
BACNET_HANDLER = ../demo/handler
BACNET_CORE = ../src
BACNET_INCLUDE = ../include
# compiler configuration
#STANDARDS = -std=c99
INCLUDE1 = -I$(BACNET_PORT_DIR) -I$(BACNET_OBJECT) -I$(BACNET_HANDLER)
INCLUDE2 = -I$(BACNET_INCLUDE)
INCLUDES = $(INCLUDE1) $(INCLUDE2)

# target
TARGET = bacnet
LIBRARY = lib$(TARGET).a

CORE_SRC = \
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

HANDLER_SRC = \
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
	$(BACNET_HANDLER)/h_rr_a.c \
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
	$(BACNET_HANDLER)/s_readrange.c  \
	$(BACNET_HANDLER)/s_rpm.c  \
	$(BACNET_HANDLER)/s_ts.c \
	$(BACNET_HANDLER)/s_cevent.c  \
	$(BACNET_HANDLER)/s_router.c  \
	$(BACNET_HANDLER)/s_uevent.c  \
	$(BACNET_HANDLER)/s_whohas.c \
	$(BACNET_HANDLER)/s_whois.c  \
	$(BACNET_HANDLER)/s_wpm.c  \
	$(BACNET_HANDLER)/s_upt.c \
	$(BACNET_HANDLER)/s_wp.c \
	$(BACNET_HANDLER)/s_getevent.c

PORT_ARCNET_SRC = \
	$(BACNET_PORT_DIR)/arcnet.c

PORT_MSTP_SRC = \
	$(BACNET_PORT_DIR)/rs485.c \
	$(BACNET_PORT_DIR)/dlmstp.c \
	$(BACNET_PORT_DIR)/timer.c \
	$(BACNET_CORE)/ringbuf.c \
	$(BACNET_CORE)/fifo.c \
	$(BACNET_CORE)/mstp.c \
	$(BACNET_CORE)/mstptext.c \
	$(BACNET_CORE)/crc.c \

PORT_ETHERNET_SRC = \
	$(BACNET_PORT_DIR)/ethernet.c

PORT_BIP_SRC = \
	$(BACNET_PORT_DIR)/bip-init.c \
	$(BACNET_CORE)/bvlc.c \
	$(BACNET_CORE)/bip.c

PORT_ALL_SRC = \
	$(BACNET_PORT_DIR)/arcnet.c \
	$(BACNET_PORT_DIR)/dlmstp.c \
	$(BACNET_PORT_DIR)/rs485.c \
	$(BACNET_PORT_DIR)/timer.c \
	$(BACNET_CORE)/ringbuf.c \
	$(BACNET_CORE)/fifo.c \
	$(BACNET_CORE)/mstp.c \
	$(BACNET_CORE)/crc.c \
	$(BACNET_PORT_DIR)/ethernet.c \
	$(BACNET_PORT_DIR)/bip-init.c \
	$(BACNET_CORE)/bvlc.c \
	$(BACNET_CORE)/bip.c

ifeq (${BACDL_DEFINE},-DBACDL_BIP=1)
PORT_SRC = ${PORT_BIP_SRC}
endif
ifeq (${BACDL_DEFINE},-DBACDL_MSTP=1)
PORT_SRC = ${PORT_MSTP_SRC}
endif
ifeq (${BACDL_DEFINE},-DBACDL_ARCNET=1)
PORT_SRC = ${PORT_ARCNET_SRC}
endif
ifeq (${BACDL_DEFINE},-DBACDL_ETHERNET=1)
PORT_SRC = ${PORT_ETHERNET_SRC}
endif
ifdef BACDL_ALL
PORT_SRC = ${PORT_ALL_SRC}
endif
ifneq (,$(findstring -DBAC_UCI,$(BACNET_DEFINES)))
UCI_SRC = $(BACNET_CORE)/ucix.c
endif

SRCS = ${CORE_SRC} ${PORT_SRC} ${HANDLER_SRC}

OBJS = ${SRCS:.c=.o}

# use local includes, but other values from calling Makefile
CFLAGS  = $(WARNINGS) $(DEBUGGING) $(OPTIMIZATION) $(STANDARDS) $(INCLUDES) $(DEFINES)

all: $(LIBRARY)

lib: $(LIBRARY)

$(LIBRARY): $(OBJS) Makefile
	$(AR) rcs $@ $(OBJS)

.c.o:
	${CC} -c ${CFLAGS} $*.c -o $@

depend:
	rm -f .depend
	${CC} -MM ${CFLAGS} *.c >> .depend

clean:
	rm -rf core $(OBJS) $(LIBRARY)

include: .depend
