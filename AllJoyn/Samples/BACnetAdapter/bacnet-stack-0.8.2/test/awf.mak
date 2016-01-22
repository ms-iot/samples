#Makefile to build test case
CC      = gcc

SRC_DIR = ../src
INCLUDES = -I../include -I. -I../demo/object
DEFINES = -DBACFILE=1 -DBIG_ENDIAN=0 -DTEST -DTEST_ATOMIC_WRITE_FILE
CFLAGS  = -Wall $(INCLUDES) $(DEFINES) -g

SRCS = $(SRC_DIR)/bacdcode.c \
	$(SRC_DIR)/bacint.c \
	$(SRC_DIR)/bacstr.c \
	$(SRC_DIR)/bacreal.c \
	$(SRC_DIR)/awf.c \
	ctest.c

OBJS = ${SRCS:.c=.o}

TARGET = awf

all: ${TARGET}
 
${TARGET}: ${OBJS}
	${CC} -o $@ ${OBJS} 

.c.o:
	${CC} -c ${CFLAGS} $*.c -o $@
	
depend:
	rm -f .depend
	${CC} -MM ${CFLAGS} *.c >> .depend
	
clean:
	rm -rf ${TARGET} $(OBJS)

include: .depend
