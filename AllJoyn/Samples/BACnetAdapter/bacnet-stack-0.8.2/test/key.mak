#Makefile to build unit tests
CC = gcc
SRC_DIR = ../src
INCLUDES = -I../include -I.
DEFINES = -DBIG_ENDIAN=0 -DTEST -DTEST_KEY

CFLAGS  = -Wall $(INCLUDES) $(DEFINES) -g

SRCS = $(SRC_DIR)/key.c \
	ctest.c

TARGET = key 

OBJS  = ${SRCS:.c=.o}

all: ${TARGET}

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
