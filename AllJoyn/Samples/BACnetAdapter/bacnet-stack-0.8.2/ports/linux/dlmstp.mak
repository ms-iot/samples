#Makefile to build test case
CC      = gcc
BASEDIR = .
# -g for debugging with gdb
DEFINES = -DBIG_ENDIAN=0 -DBACDL_MSTP=1 -DTEST_DLMSTP
INCLUDES = -I. -I../../ 
CFLAGS  = -Wall $(INCLUDES) $(DEFINES) -g

SRCS = rs485.c \
	dlmstp.c \
	../../mstp.c \
	../../crc.c

OBJS = ${SRCS:.c=.o}

TARGET = dlmstp

all: ${TARGET}
 
${TARGET}: ${OBJS}
	${CC} -pthread -o $@ ${OBJS} 

.c.o:
	${CC} -c ${CFLAGS} $*.c -o $@
	
depend:
	rm -f .depend
	${CC} -MM ${CFLAGS} *.c >> .depend
	
clean:
	rm -rf core ${TARGET} $(OBJS) *.bak *.1 *.ini

include: .depend
