#Makefile to build unit tests
CC      = gcc
SRC_DIR = ../src
INCLUDES = -I../include -I.
DEFINES = -DBIG_ENDIAN=0 -DTEST -DTEST_DECODE -DMAX_APDU=50

CFLAGS  = -Wall $(INCLUDES) $(DEFINES) -g

TARGET = bacdcode

SRCS = $(SRC_DIR)/bacdcode.c \
	$(SRC_DIR)/bacint.c \
	$(SRC_DIR)/bacstr.c \
	$(SRC_DIR)/bacreal.c \
	ctest.c

OBJS = ${SRCS:.c=.o}

all: ${TARGET}

${TARGET}: ${OBJS}
	${CC} -o $@ ${OBJS}

.c.o:
	${CC} -c ${CFLAGS} $*.c -o $@

depend:
	rm -f .depend
	${CC} -MM ${CFLAGS} *.c >> .depend

clean:
	rm -rf ${OBJS} ${TARGET}

include: .depend
