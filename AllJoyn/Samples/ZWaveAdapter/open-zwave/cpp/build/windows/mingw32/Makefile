#
# Makefile for OpenZWave MingW32 build
# Maarten Damen

.SUFFIXES:	.d .cpp .o .a
.PHONY:	default clean

CC     := $(CROSS_COMPILE)gcc
CXX    := $(CROSS_COMPILE)g++
LD     := $(CROSS_COMPILE)g++
AR     := $(CROSS_COMPILE)ar rc
RANLIB := $(CROSS_COMPILE)ranlib

DEBUG_CFLAGS    := -Wall -Wno-format -g -DDEBUG -DLOG_STDERR -DMINGW
RELEASE_CFLAGS  := -Wall -Wno-unknown-pragmas -Wno-format -O3 -DMINGW

DEBUG_LDFLAGS	:= -g

# Change for DEBUG or RELEASE
CFLAGS	:= -c $(DEBUG_CFLAGS)
LDFLAGS	:= $(DEBUG_LDFLAGS)

LIBDIR	:= ../../../lib/windows-mingw32

INCLUDES	:= -I ../../../src -I ../../../src/command_classes/ -I ../../../src/value_classes/ \
	-I ../../../src/platform/ -I ../../../src/platform/windows -I ../../../tinyxml/ -I ../../../hidapi/hidapi/
SOURCES		:= ../../../src ../../../src/command_classes ../../../tinyxml ../../../hidapi/windows \
	../../../src/value_classes ../../../src/platform ../../../src/platform/windows

VPATH = ../../../src:../../../src/command_classes:../../../tinyxml:../../../hidapi/windows:\
	../../../src/value_classes:../../../src/platform:../../../src/platform/windows

%.d : %.cpp
	@set -e; rm -f $@; \
	$(CXX) -MM $(INCLUDES) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

tinyxml := $(notdir $(wildcard ../../../tinyxml/*.cpp))
hidapi := $(notdir $(wildcard ../../../hidapi/linux/hid.c)) # we do not want the libusb version
cclasses := $(notdir $(wildcard ../../../src/command_classes/*.cpp))
vclasses := $(notdir $(wildcard ../../../src/value_classes/*.cpp))
pform := $(notdir $(wildcard ../../../src/platform/*.cpp)) \
	$(notdir $(wildcard ../../../src/platform/windows/*.cpp))
indep := $(notdir $(wildcard ../../../src/*.cpp))

%.o : %.cpp
	$(CXX) $(CFLAGS) $(INCLUDES) -o $@ $<

%.o : %.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $<

default:	$(LIBDIR)/openzwave.a

clean:
	rm -f *.d *.o

-include $(tinyxml:.cpp=.d)
-include $(hidapi:.c=.d)
-include $(cclasses:.cpp=.d)
-include $(vclasses:.cpp=.d)
-include $(pform:.cpp=.d)
-include $(indep:.cpp=.d)

vers.c:
	echo 'char ozw_vers[] = "OpenZWave version 1.0.'`svnversion ../..`'";' > vers.c

vers.o:	vers.c

$(LIBDIR)/openzwave.a:	$(patsubst %.cpp,%.o,$(tinyxml)) \
			$(patsubst %.c,%.o,$(hidapi)) \
			$(patsubst %.cpp,%.o,$(cclasses)) \
			$(patsubst %.cpp,%.o,$(vclasses)) \
			$(patsubst %.cpp,%.o,$(pform)) \
			$(patsubst %.cpp,%.o,$(indep)) vers.o
	$(AR) $@ $?
