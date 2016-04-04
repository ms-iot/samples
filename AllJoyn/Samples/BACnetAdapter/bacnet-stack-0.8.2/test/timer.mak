#Makefile to build test case
CC      = gcc
SRC_DIR = ../ports/bdk-atxx4-mstp
INCLUDES = -I../include -I${SRC_DIR} -I.
DEFINES = -DBIG_ENDIAN=0 -DTEST -DTEST_TIMER

CFLAGS  = -Wall $(INCLUDES) $(DEFINES) -g

SRCS = $(SRC_DIR)/timer.c \
	ctest.c

TARGET = timer

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
	rm -rf core ${TARGET} $(OBJS) *.bak *.1 *.ini

include: .depend

