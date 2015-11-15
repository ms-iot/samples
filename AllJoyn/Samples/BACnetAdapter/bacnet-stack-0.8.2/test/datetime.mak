#Makefile to build test case
CC = gcc
SRC_DIR = ../src
INCLUDES = -I../include -I.
DEFINES = -DBIG_ENDIAN=0 -DTEST -DTEST_DATE_TIME

CFLAGS  = -Wall $(INCLUDES) $(DEFINES) -g

SRCS = $(SRC_DIR)/datetime.c \
	$(SRC_DIR)/bacdcode.c \
	$(SRC_DIR)/bacint.c \
	$(SRC_DIR)/bacstr.c \
	$(SRC_DIR)/bacreal.c \
	$(SRC_DIR)/bacapp.c \
	$(SRC_DIR)/lighting.c \
	$(SRC_DIR)/bactext.c \
	$(SRC_DIR)/indtext.c \
	ctest.c

OBJS = ${SRCS:.c=.o}

TARGET = datetime

all: ${TARGET}

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
