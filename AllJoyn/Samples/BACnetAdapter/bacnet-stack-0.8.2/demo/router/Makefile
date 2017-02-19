#Makefile to build BACnet Application for the Linux Port

# tools - only if you need them.
# Most platforms have this already defined
# CC = gcc

# Executable file name
TARGET = router

TARGET_BIN = ${TARGET}$(TARGET_EXT)

ifeq (${BACNET_PORT},linux)
#PFLAGS =
# -pthread
TARGET_EXT =
LIBS = -lpthread -lconfig
LFLAGS += $(LIBS)
endif

#DEFINES = $(BACNET_DEFINES) -DBACDL_MSTP -DBACDL_BIP
BACNET_SOURCE_DIR = ../../src

SRCS = main.c \
	${BACNET_PORT_DIR}/rs485.c \
	${BACNET_PORT_DIR}/timer.c \
	${BACNET_PORT_DIR}/bip-init.c \
	${BACNET_PORT_DIR}/dlmstp_linux.c \
	${BACNET_SOURCE_DIR}/bip.c \
	${BACNET_SOURCE_DIR}/bvlc.c \
	${BACNET_SOURCE_DIR}/fifo.c \
	${BACNET_SOURCE_DIR}/mstp.c \
	${BACNET_SOURCE_DIR}/mstptext.c \
	${BACNET_SOURCE_DIR}/debug.c \
	${BACNET_SOURCE_DIR}/indtext.c \
	${BACNET_SOURCE_DIR}/ringbuf.c \
	${BACNET_SOURCE_DIR}/crc.c \
	mstpmodule.c \
	ipmodule.c \
	portthread.c \
	msgqueue.c \
	network_layer.c
	

OBJS = ${SRCS:.c=.o}

all: Makefile ${TARGET_BIN}

${TARGET_BIN}: ${OBJS} Makefile
	${CC} ${PFLAGS} ${OBJS} ${LFLAGS} -o $@
	size $@
	cp $@ ../../bin

.c.o:
	${CC} -c ${CFLAGS} $*.c -o $@

depend:
	rm -f .depend
	${CC} -MM ${CFLAGS} *.c >> .depend

clean:
	rm -f core ${TARGET_BIN} ${OBJS} $(TARGET).map

include: .depend
