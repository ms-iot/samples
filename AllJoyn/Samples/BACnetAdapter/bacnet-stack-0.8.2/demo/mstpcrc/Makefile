#Makefile to build BACnet Application for the Linux Port

# tools - only if you need them.
# Most platforms have this already defined
# CC = gcc

# Executable file name
TARGET = mstpcrc

TARGET_BIN = ${TARGET}$(TARGET_EXT)

# This demo seems to be a little unique
DEFINES = $(BACNET_DEFINES)
BACNET_SOURCE_DIR = ../../src

#libraries used
LIBRARIES=-lgcc,-lm

#build for release (default) or debug
DEBUGGING =
OPTIMIZATION = -Os
ifeq (${BUILD},debug)
OPTIMIZATION = -O0
DEBUGGING = -g
endif

# search order for included libraries 
INCLUDES = -I$(BACNET_INCLUDE_DIR)

SRCS = main.c \
	${BACNET_PORT_DIR}/timer.c \
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
