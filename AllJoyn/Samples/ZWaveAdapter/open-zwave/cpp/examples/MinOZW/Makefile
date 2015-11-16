#
# Makefile for OpenzWave Mac OS X applications
# Greg Satz

# GNU make only

# requires libudev-dev

.SUFFIXES:	.d .cpp .o .a
.PHONY:	default clean


DEBUG_CFLAGS    := -Wall -Wno-format -ggdb -DDEBUG
RELEASE_CFLAGS  := -Wall -Wno-unknown-pragmas -Wno-format -O3

DEBUG_LDFLAGS	:= -g

top_srcdir := $(abspath $(dir $(lastword $(MAKEFILE_LIST)))../../../)


INCLUDES	:= -I $(top_srcdir)/cpp/src -I $(top_srcdir)/cpp/tinyxml/ -I $(top_srcdir)/cpp/hidapi/hidapi/
LIBS =  $(wildcard $(LIBDIR)/*.so $(LIBDIR)/*.dylib $(top_builddir)/*.so $(top_builddir)/*.dylib $(top_builddir)/cpp/build/*.so $(top_builddir)/cpp/build/*.dylib )
LIBSDIR = $(abspath $(dir $(firstword $(LIBS))))
minozwsrc := $(notdir $(wildcard $(top_srcdir)/cpp/examples/MinOZW/*.cpp))
VPATH := $(top_srcdir)/cpp/examples/MinOZW

top_builddir ?= $(CURDIR)

default: $(top_builddir)/MinOZW

include $(top_srcdir)/cpp/build/support.mk

-include $(patsubst %.cpp,$(DEPDIR)/%.d,$(minozwsrc))

#if we are on a Mac, add these flags and libs to the compile and link phases 
ifeq ($(UNAME),Darwin)
CFLAGS += -DDARWIN -arch i386 -arch x86_64
LDFLAGS += -arch i386 -arch x86_64
endif

# Dup from main makefile, but that is not included when building here..
ifeq ($(UNAME),FreeBSD)
ifeq (,$(wildcard /usr/include/iconv.h))
CFLAGS += -I/usr/local/include
LDFLAGS+= -L/usr/local/lib -liconv
endif
LDFLAGS+= -lusb
endif

$(OBJDIR)/MinOZW:	$(patsubst %.cpp,$(OBJDIR)/%.o,$(minozwsrc))
	@echo "Linking $(OBJDIR)/MinOZW"
	$(LD) $(LDFLAGS) -o $@ $< $(LIBS) -pthread

$(top_builddir)/MinOZW: $(top_srcdir)/cpp/examples/MinOZW/MinOZW.in $(OBJDIR)/MinOZW
	@echo "Creating Temporary Shell Launch Script"
	@$(SED) \
		-e 's|[@]LDPATH@|$(LIBSDIR)|g' \
		< "$<" > "$@"
	@chmod +x $(top_builddir)/MinOZW

clean:
	@rm -rf $(DEPDIR) $(OBJDIR) $(top_builddir)/MinOZW

ifeq ($(XMLLINT),)
xmltest:	$(XMLLINT)
	$(error xmllint command not found.)
else
xmltest:	$(XMLLINT)
	@$(XMLLINT) --noout --schema ../../../config/zwcfg.xsd zwcfg_*.xml
	@$(XMLLINT) --noout --schema ../../../config/zwscene.xsd zwscene.xml
endif

install: $(OBJDIR)/MinOZW
	@echo "Installing into Prefix: $(PREFIX)"
	@install -d $(DESTDIR)/$(PREFIX)/bin/
	@cp $(OBJDIR)/MinOZW $(DESTDIR)/$(PREFIX)/bin/MinOZW
	@chmod 755 $(DESTDIR)/$(PREFIX)/bin/MinOZW
