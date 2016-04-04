#Makefile to build test case
CC      = gcc
SRC_DIR = ../../src
TEST_DIR = ../../test
PORTS_DIR = ../../ports/linux
INCLUDES = -I../../include -I$(TEST_DIR) -I$(PORTS_DIR) -I.
DEFINES = -DBIG_ENDIAN=0
DEFINES += -DTEST -DBACDL_TEST
DEFINES += -DBACAPP_ALL
DEFINES += -DMAX_TSM_TRANSACTIONS=0
DEFINES += -DTEST_DEVICE

CFLAGS  = -Wall $(INCLUDES) $(DEFINES) -g

SRCS = device.c \
	$(SRC_DIR)/bacdcode.c \
	$(SRC_DIR)/bacint.c \
	$(SRC_DIR)/bacstr.c \
	$(SRC_DIR)/bacreal.c \
	$(SRC_DIR)/datetime.c \
	$(SRC_DIR)/bacapp.c \
	$(SRC_DIR)/bactext.c \
	$(SRC_DIR)/indtext.c \
	$(SRC_DIR)/proplist.c \
	$(SRC_DIR)/lighting.c \
	$(SRC_DIR)/apdu.c \
	$(SRC_DIR)/address.c \
	$(SRC_DIR)/bacaddr.c \
	$(SRC_DIR)/dcc.c \
	$(SRC_DIR)/version.c \
	$(TEST_DIR)/ctest.c

TARGET = device

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
