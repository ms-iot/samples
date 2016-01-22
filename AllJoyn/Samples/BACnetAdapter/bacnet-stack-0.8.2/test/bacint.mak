#Makefile to build unit tests
CC      = gcc
SRC_DIR = ../src
INCLUDES = -I../include -I.
DEFINES = -DBIG_ENDIAN=0 -DTEST -DTEST_BACINT

CFLAGS  = -Wall $(INCLUDES) $(DEFINES) -g

TARGET = bacint

SRCS = $(SRC_DIR)/bacint.c \
	$(SRC_DIR)/bacstr.c \
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
