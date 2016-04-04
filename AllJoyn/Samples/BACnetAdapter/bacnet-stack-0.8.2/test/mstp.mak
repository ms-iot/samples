#Makefile to build test case
CC      = gcc
SRC_DIR = ../src
INCLUDES = -I../include -I. -I../ports/linux
DEFINES = -DBIG_ENDIAN=0 -DTEST -DTEST_MSTP

CFLAGS  = -Wall $(INCLUDES) $(DEFINES) -g

SRCS = $(SRC_DIR)/mstp.c \
	$(SRC_DIR)/mstptext.c \
	$(SRC_DIR)/indtext.c \
	$(SRC_DIR)/crc.c \
	$(SRC_DIR)/ringbuf.c \
	ctest.c

TARGET = mstp

all: ${TARGET}

OBJS = ${SRCS:.c=.o}

${TARGET}: ${OBJS}
	${CC} -o $@ ${OBJS}

.c.o:
	${CC} -c ${CFLAGS} $*.c -o $@

depend:
	rm -f .depend
	${CC} -MM ${CFLAGS} *.c >> .depend

clean:
	rm -rf core ${TARGET} $(OBJS)

include: .depend
