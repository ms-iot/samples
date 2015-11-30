#
# Simple makefile to build an executable for Win32 console
#
# This makefile assumes Borland development environment
# on Windows NT/9x/2000/XP
# Tools: bcc32, make, ilink

!ifndef BORLAND_DIR
BORLAND_DIR_Not_Defined:
   @echo .
   @echo You must define environment variable BORLAND_DIR to compile.
!endif

PRODUCT = rs485
PRODUCT_EXE = $(PRODUCT).exe

# Choose the Data Link Layer to Enable
DEFINES = -DBACDL_MSTP=1;TEST_RS485;TEST_RS485_TRANSMIT

SRCS = rs485.c
OBJS = $(SRCS:.c=.obj)

# Compiler definitions
#
BCC_CFG = bcc32.cfg
CC = $(BORLAND_DIR)\bin\bcc32 +$(BCC_CFG)
#LINK = $(BORLAND_DIR)\bin\tlink32
LINK = $(BORLAND_DIR)\bin\ilink32
TLIB = $(BORLAND_DIR)\bin\tlib

#
# Include directories
#
CC_DIR     = $(BORLAND_DIR)\BIN
BACNET_INCL = ..\..\include;.
INCL_DIRS = -I$(BORLAND_DIR)\include;$(BACNET_INCL)

CFLAGS = $(INCL_DIRS) $(CS_FLAGS) $(DEFINES)

# Libraries
#
C_LIB_DIR = $(BORLAND_DIR)\lib

LIBS = $(C_LIB_DIR)\IMPORT32.lib \
$(C_LIB_DIR)\CW32MT.lib

#
# Main target
#
# This should be the first one in the makefile

all : $(BCC_CFG) $(PRODUCT_EXE)

# Linker specific: the link below is for BCC linker/compiler. If you link
# with a different linker - please change accordingly.
#

# need a temp response file (@&&) because command line is too long
$(PRODUCT_EXE) : $(OBJS)
	@echo Running Linker for $(PRODUCT_EXE)
	$(LINK)	-L$(C_LIB_DIR) -m -c -s -v @&&| # temp response file, starts with |
	  $(BORLAND_DIR)\lib\c0x32.obj $**  # $** lists each dependency
	$<
	$*.map
	$(LIBS)
| # end of temp response file

#
# Utilities

clean :
	@echo Deleting obj files, $(PRODUCT_EXE) and map files.
	del *.obj
	del ..\..\*.obj
	del ..\..\demo\handler\*.obj
	del ..\..\demo\object\*.obj
	del $(PRODUCT_EXE)
	del *.map
	del $(BCC_CFG)

#
# Generic rules
#
.SUFFIXES: .cpp .c .sbr .obj

#
# cc generic rule
#
.c.obj:
	$(CC) -o$@ $<

# Compiler configuration file
$(BCC_CFG) :
   Copy &&|
$(CFLAGS) 
-c 
-y     #include line numbers in OBJ's
-v     #include debug info
-w+    #turn on all warnings
-Od    #disable all optimizations
#-a4    #32 bit data alignment
#-M     # generate link map
#-ls    # linker options
#-WM-   #not multithread
-WM    #multithread
-w-aus # ignore warning assigned a value that is never used
-w-sig # ignore warning conversion may lose sig digits
| $@

# EOF: makefile