#Makefile to build test case
CC      = gcc
SRC_DIR = ../src
INCLUDES = -I../include -I. -I../demo/object
DEFINES = -DBIG_ENDIAN=0 -DTEST -DBACAPP_ALL -DTEST_EVENT

CFLAGS  = -Wall $(INCLUDES) $(DEFINES) -g

SRCS = $(SRC_DIR)/bacdcode.c \
	$(SRC_DIR)/bacint.c \
	$(SRC_DIR)/bacstr.c \
	$(SRC_DIR)/bacreal.c \
	$(SRC_DIR)/bacerror.c \
	$(SRC_DIR)/bacapp.c \
	$(SRC_DIR)/bactext.c \
	$(SRC_DIR)/indtext.c \
	$(SRC_DIR)/datetime.c \
	$(SRC_DIR)/lighting.c \
	$(SRC_DIR)/memcopy.c \
	$(SRC_DIR)/timestamp.c \
	$(SRC_DIR)/bacpropstates.c \
	$(SRC_DIR)/bacdevobjpropref.c \
	$(SRC_DIR)/event.c \
	ctest.c

TARGET = event

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
