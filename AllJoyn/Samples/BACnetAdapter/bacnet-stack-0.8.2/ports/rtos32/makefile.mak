#
# Simple makefile to build an RTB executable for RTOS-32
#
# This makefile assumes Borland bcc32 development environment
# on Windows NT/9x/2000/XP
#

!ifndef RTOS32_DIR
RTOS32_DIR_Not_Defined:
   @echo .
   @echo You must define environment variable RTOS32_DIR to compile.
!endif

!ifndef BORLAND_DIR
BORLAND_DIR_Not_Defined:
   @echo .
   @echo You must define environment variable BORLAND_DIR to compile.
!endif

PRODUCT = bacnet
PRODUCT_RTB = $(PRODUCT).rtb
PRODUCT_EXE = $(PRODUCT).exe

# Choose the Data Link Layer to Enable
#DEFINES = -DDOC;BIG_ENDIAN=0;TSM_ENABLED=1;PRINT_ENABLED=1;BACDL_BIP=1
#DEFINES = -DDOC;BIG_ENDIAN=0;TSM_ENABLED=1;PRINT_ENABLED=1;BACDL_ETHERNET=1
#DEFINES = -DDOC;BIG_ENDIAN=0;TSM_ENABLED=1;PRINT_ENABLED=1;BACDL_ARCNET=1
DEFINES = -DDOC;BIG_ENDIAN=0;TSM_ENABLED=1;PRINT_ENABLED=0;BACDL_MSTP=1

SRCS = main.c \
       ethernet.c \
       bip-init.c \
       dlmstp.c \
       rs485.c \
       init.c \
       ..\..\bip.c  \
       ..\..\mstp.c  \
       ..\..\crc.c  \
       ..\..\demo\handler\h_iam.c  \
       ..\..\demo\handler\h_npdu.c  \
       ..\..\demo\handler\h_whois.c  \
       ..\..\demo\handler\h_wp.c  \
       ..\..\demo\handler\h_rp.c  \
       ..\..\demo\handler\noserv.c  \
       ..\..\demo\handler\txbuf.c  \
       ..\..\demo\handler\s_iam.c  \
       ..\..\demo\handler\s_rp.c  \
       ..\..\demo\handler\s_whois.c  \
       ..\..\bacdcode.c \
       ..\..\bacstr.c \
       ..\..\bactext.c \
       ..\..\indtext.c \
       ..\..\bacapp.c \
       ..\..\bigend.c \
       ..\..\whois.c \
       ..\..\dcc.c \
       ..\..\iam.c \
       ..\..\rp.c \
       ..\..\wp.c \
       ..\..\arf.c \
       ..\..\awf.c \
       ..\..\demo\object\bacfile.c \
       ..\..\demo\object\device.c \
       ..\..\demo\object\ai.c \
       ..\..\demo\object\ao.c \
       ..\..\demo\object\av.c \
       ..\..\demo\object\bi.c \
       ..\..\demo\object\bo.c \
       ..\..\demo\object\bv.c \
       ..\..\demo\object\lsp.c \
       ..\..\demo\object\mso.c \
       ..\..\datalink.c \
       ..\..\tsm.c \
       ..\..\address.c \
       ..\..\abort.c \
       ..\..\reject.c \
       ..\..\bacerror.c \
       ..\..\apdu.c \
       ..\..\npdu.c

OBJS = $(SRCS:.c=.obj)

# Compiler definitions
#
CC = $(BORLAND_DIR)\bin\bcc32 +bcc32.cfg
LINK = $(BORLAND_DIR)\bin\tlink32
#LINK = $(BORLAND_DIR)\bin\ilink32
TLIB = $(BORLAND_DIR)\bin\tlib
LOCATE = $(RTOS32_DIR)\bin\rtloc

#
# Include directories
#
CC_DIR     = $(BORLAND_DIR)\BIN
INCL_DIRS = -I$(BORLAND_DIR)\include;$(RTOS32_DIR)\include;..\..\include;..\..\demo\handler\;..\..\demo\object\;.

CFLAGS = $(INCL_DIRS) $(CS_FLAGS) $(DEFINES)

# Libraries
#
RTOS32_LIB_DIR = $(RTOS32_DIR)\libbc
C_LIB_DIR = $(BORLAND_DIR)\lib

LIBDIR = $(RTOS32_LIB_DIR);$(C_LIB_DIR)

LIBS = $(RTOS32_LIB_DIR)\RTFILES.LIB \
$(RTOS32_LIB_DIR)\RTFSK32.LIB \
$(RTOS32_LIB_DIR)\DRVDOC.LIB \
$(RTOS32_LIB_DIR)\RTIP.LIB \
$(RTOS32_LIB_DIR)\RTK32.LIB \
$(RTOS32_LIB_DIR)\FLTEMUMT.LIB \
$(RTOS32_LIB_DIR)\DRVRT32.LIB \
$(RTOS32_LIB_DIR)\RTEMUMT.LIB \
$(RTOS32_LIB_DIR)\RTT32.LIB \
$(RTOS32_LIB_DIR)\RTTHEAP.LIB \
#$(C_LIB_DIR)\DPMI32.lib \
$(C_LIB_DIR)\IMPORT32.lib \
$(C_LIB_DIR)\CW32MT.lib

#
# Main target
#
# This should be the first one in the makefile

all : $(PRODUCT_RTB) monitor.rtb

monitor.rtb: monitor.cfg hardware.cfg
  $(LOCATE) monitor

# debug using COM3 (ISA Card) as the debug port
# boot from floppy
debugcom3: hardware.cfg software.cfg $(PRODUCT_RTB) monitor.rtb
  $(LOCATE) -DDEBUGCOM3 monitor
  $(LOCATE) -d- -DMONITOR -DDEBUGCOM3 $(PRODUCT) software.cfg

$(PRODUCT_RTB): bcc32.cfg hardware.cfg software.cfg $(PRODUCT_EXE)
		@echo Running Locate on $(PRODUCT)
	  $(LOCATE) $(PRODUCT) software.cfg

# Linker specific: the link below is for BCC linker/compiler. If you link
# with a different linker - please change accordingly.
#

# need a temp response file (@&&) because command line is too long
$(PRODUCT_EXE) : $(OBJS)
	@echo Running Linker for $(PRODUCT_EXE)
	$(LINK)	-L$(LINKER_LIB) -m -c -s -v @&&| # temp response file, starts with |
	  $(BORLAND_DIR)\lib\c0x32.obj $**  # $** lists each dependency
	$<
	$*.map
	$(LIBS)
| # end of temp response file

#
# Utilities

clean :
	@echo Deleting obj files, $(PRODUCT_EXE), $(PRODUCT_RTB) and map files.
	del *.obj
	del ..\..\*.obj
	del $(PRODUCT_EXE)
	del $(PRODUCT_RTB)
	del *.map
	del bcc32.cfg

install : $(PRODUCT)
	copy $(PRODUCT) ..\bin

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
bcc32.cfg :
   Copy &&|
$(CFLAGS)
-c
#-g2    #stop after gN warnings
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
