#Makefile to build test case
CC      = gcc
SRC_DIR = ../src
INCLUDES = -I../include -I.
DEFINES = -DBIG_ENDIAN=0 -DTEST -DTEST_OBJECT_LIST

CFLAGS  = -Wall $(INCLUDES) $(DEFINES) -g

SRCS = $(SRC_DIR)/objects.c \
	$(SRC_DIR)/keylist.c \
	$(SRC_DIR)/key.c \
	ctest.c

TARGET = rp

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

