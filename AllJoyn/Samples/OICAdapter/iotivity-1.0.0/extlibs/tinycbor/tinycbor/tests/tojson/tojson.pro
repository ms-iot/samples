CONFIG += testcase parallel_test c++11
QT = core testlib

SOURCES += tst_tojson.cpp
INCLUDEPATH += ../../src
POST_TARGETDEPS += ../../lib/libtinycbor.a
LIBS += $$POST_TARGETDEPS
