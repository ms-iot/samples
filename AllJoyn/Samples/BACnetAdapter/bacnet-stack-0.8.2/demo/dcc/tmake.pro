TEMPLATE = app
CONFIG = warn_on debug console
CLEAN_FILES = core *~
TARGET =  bacdcc
DEFINES = BACDL_BIP=1 TSM_ENABLED=1 USE_INADDR=1 BIP_DEBUG
SOURCES = main.c \
       ../../filename.c \
       ../../bip.c  \
       ../../demo/handler/txbuf.c  \
       ../../demo/handler/noserv.c  \
       ../../demo/handler/h_whois.c  \
       ../../demo/handler/h_iam.c  \
       ../../demo/handler/h_rp.c  \
       ../../demo/handler/h_dcc.c  \
       ../../demo/handler/s_whois.c  \
       ../../demo/handler/s_dcc.c  \
       ../../bacdcode.c \
       ../../bacapp.c \
       ../../bacstr.c \
       ../../bactext.c \
       ../../indtext.c \
       ../../bigend.c \
       ../../whois.c \
       ../../iam.c \
       ../../rp.c \
       ../../wp.c \
       ../../arf.c \
       ../../awf.c \
       ../../dcc.c \
       ../../demo/object/bacfile.c \
       ../../demo/object/device.c \
       ../../demo/object/ai.c \
       ../../demo/object/ao.c \
       ../../demo/object/bi.c \
       ../../demo/object/bo.c \
       ../../demo/object/lsp.c \
       ../../datalink.c \
       ../../tsm.c \
       ../../address.c \
       ../../abort.c \
       ../../reject.c \
       ../../bacerror.c \
       ../../apdu.c \
       ../../npdu.c
unix:SOURCES +=  ../../ports/linux/bip-init.c
win32:SOURCES +=  ../../ports/win32/bip-init.c

INCLUDEPATH = . \
  ../../ \
  ../../demo/object \
  ../../demo/handler

unix:INCLUDEPATH += ../../ports/linux
win32:INCLUDEPATH += ../../ports/win32

#unix:HEADERS += ../../ports/linux/net.h
#win32:HEADERS += ../../ports/win32/stdint.h
#win32:HEADERS += ../../ports/win32/net.h
#win32:HEADERS += ../../ports/win32/stdbool.h
