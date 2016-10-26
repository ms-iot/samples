#Makefile to build test case

# tools - only if you need them.
# Most platforms have this already defined
# CC = gcc
# AR = ar
# MAKE = make
# SIZE = size
#
# Assumes rm and cp are available

SRC_DIR := ../src
INCLUDES := -I../include -I.
DEFINES := -DBIG_ENDIAN=0 -DTEST -DBACAPP_ALL -DTEST_TIMESYNC

CFLAGS  := $(INCLUDES) $(DEFINES) -g
CFLAGS += -Wall

TARGET := timesync

SRCS := $(SRC_DIR)/bacdcode.c \
	$(SRC_DIR)/bacint.c \
	$(SRC_DIR)/bacstr.c \
	$(SRC_DIR)/bacreal.c \
	$(SRC_DIR)/bacerror.c \
	$(SRC_DIR)/bacapp.c \
	$(SRC_DIR)/bactext.c \
	$(SRC_DIR)/indtext.c \
	$(SRC_DIR)/datetime.c \
	$(SRC_DIR)/lighting.c \
	$(SRC_DIR)/timesync.c \
	ctest.c

OBJS := ${SRCS:.c=.o}

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

run:
	./${TARGET}

include: .depend

.PHONY: all run clean
