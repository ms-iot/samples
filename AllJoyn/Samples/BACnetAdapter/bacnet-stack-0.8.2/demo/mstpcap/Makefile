#Makefile to build BACnet Application for the Linux Port

# tools - only if you need them.
# Most platforms have this already defined
# CC = gcc

# Executable file name
TARGET = mstpcap

TARGET_BIN = ${TARGET}$(TARGET_EXT)

# This demo seems to be a little unique
DEFINES = $(BACNET_DEFINES) -DBACDL_MSTP
BACNET_SOURCE_DIR = ../../src

SRCS = main.c \
	${BACNET_PORT_DIR}/rs485.c \
	${BACNET_PORT_DIR}/timer.c \
	${BACNET_SOURCE_DIR}/fifo.c \
	${BACNET_SOURCE_DIR}/mstp.c \
	${BACNET_SOURCE_DIR}/mstptext.c \
	${BACNET_SOURCE_DIR}/debug.c \
	${BACNET_SOURCE_DIR}/indtext.c \
	${BACNET_SOURCE_DIR}/ringbuf.c \
	${BACNET_SOURCE_DIR}/bacdcode.c \
	${BACNET_SOURCE_DIR}/iam.c \
	${BACNET_SOURCE_DIR}/crc.c

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
