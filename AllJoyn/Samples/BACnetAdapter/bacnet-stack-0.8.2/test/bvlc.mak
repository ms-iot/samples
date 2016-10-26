#Makefile to build test case
CC      = gcc
SRC_DIR = ../src
INCLUDES = -I../include -I. -I../ports/linux
DEFINES = -DBACDL_BIP -DBIG_ENDIAN=0 -DTEST -DTEST_BVLC

CFLAGS  = -Wall $(INCLUDES) $(DEFINES) -g

SRCS = $(SRC_DIR)/bacdcode.c \
	$(SRC_DIR)/bacint.c \
	$(SRC_DIR)/bacstr.c \
	$(SRC_DIR)/bacreal.c \
	$(SRC_DIR)/bvlc.c \
	ctest.c

OBJS = ${SRCS:.c=.o}

TARGET = bvlc

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
