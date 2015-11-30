#Makefile to build test case
CC      = gcc
SRC_DIR = ../../src
TEST_DIR = ../../test
HANDLER_DIR = ../handler
INCLUDES = -I../../include -I$(TEST_DIR) -I. -I$(HANDLER_DIR)
DEFINES = -DBIG_ENDIAN=0 -DBACDL_ALL -DTEST -DTEST_COMMAND

CFLAGS  = -Wall $(INCLUDES) $(DEFINES) -g

SRCS = command.c \
	$(SRC_DIR)/bacdcode.c \
	$(SRC_DIR)/bacint.c \
	$(SRC_DIR)/bacstr.c \
	$(SRC_DIR)/bacreal.c \
	$(SRC_DIR)/bacapp.c \
	$(SRC_DIR)/bactext.c \
	$(SRC_DIR)/indtext.c \
	$(SRC_DIR)/datetime.c \
	$(SRC_DIR)/lighting.c \
	$(SRC_DIR)/proplist.c \
	$(TEST_DIR)/ctest.c

TARGET = command

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
