SOURCES += \
    $$PWD/cborencoder.c \
    $$PWD/cborencoder_close_container_checked.c \
    $$PWD/cborerrorstrings.c \
    $$PWD/cborparser.c \
    $$PWD/cborpretty.c \
    $$PWD/cbortojson.c \

QMAKE_CFLAGS *= $$QMAKE_CFLAGS_SPLIT_SECTIONS
QMAKE_LFLAGS *= $$QMAKE_LFLAGS_GCSECTIONS
INCLUDEPATH += $$PWD
