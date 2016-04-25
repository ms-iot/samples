SOURCES += tst_parser.cpp ../../src/cborparser.c

CONFIG += testcase parallel_test c++11
QT = core testlib
DEFINES += CBOR_PARSER_MAX_RECURSIONS=16

INCLUDEPATH += ../../src
POST_TARGETDEPS += ../../lib/libtinycbor.a
LIBS += $$POST_TARGETDEPS
