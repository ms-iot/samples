#Makefile to build test case
CC      = gcc
SRCDIR = ../../src
INCDIR = ../../include
# -g for debugging with gdb
DEFINES = -DBIG_ENDIAN=0 -DBACDL_MSTP=1
INCLUDES = -I. -I$(INCDIR)
CFLAGS  = -Wall $(INCLUDES) $(DEFINES) -g

SRCS = rs485.c \
	rx_fsm.c \
	$(SRCDIR)/mstp.c \
	$(SRCDIR)/mstptext.c \
	$(SRCDIR)/indtext.c \
	$(SRCDIR)/crc.c

OBJS = ${SRCS:.c=.o}

TARGET = rx_fsm

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
