TEMPLATE = lib
CONFIG = qt staticlib
HEADERS += drawobjects.h
SOURCES += drawobjects.cpp 
TARGET = drawlib

QMAKE_CC = $(Q)echo "  [CC] $<"; gcc  $(ADOPT_INCLUDE)
QMAKE_CXX = $(Q)echo "  [CCP] $<"; g++ $(ADOPT_INCLUDE)
QMAKE_LINK = $(Q)echo "  [LD] $@"; gcc


