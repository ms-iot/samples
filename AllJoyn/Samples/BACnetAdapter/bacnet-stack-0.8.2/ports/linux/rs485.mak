#Makefile to build test case
#CC      = gcc
TARGET = rs485

# Directories
BACNET_SOURCE_DIR = ../../src
BACNET_INCLUDE = ../../include

# -g for debugging with gdb
DEFINES = -DBIG_ENDIAN=0 -DTEST_RS485 -DBACDL_TEST
INCLUDES = -I. -I../../ -I$(BACNET_INCLUDE)
CFLAGS  = -Wall $(INCLUDES) $(DEFINES) -g
LIBRARIES=-lc,-lgcc,-lrt,-lm
LFLAGS = -Wl,-Map=$(TARGET).map,$(LIBRARIES),--gc-sections

SRCS = rs485.c \
	${BACNET_SOURCE_DIR}/fifo.c

OBJS = ${SRCS:.c=.o}

all: ${TARGET}

${TARGET}: ${OBJS}
	${CC} ${OBJS} ${LFLAGS} -o $@

.c.o:
	${CC} -c ${CFLAGS} $*.c -o $@

depend:
	rm -f .depend
	${CC} -MM ${CFLAGS} *.c >> .depend

clean:
	rm -rf core ${TARGET} $(OBJS) *.bak *.1 *.ini

include: .depend
