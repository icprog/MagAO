
include ("cc.pro")

CONFIG += qt thread #debug

# Used for the "include" directive only
INCLUDEPATH += /usr/include/kde $(ADOPT_SOURCE)/lib/ 

LIBPATH += $(ADOPT_SOURCE)/contrib/cfitsio

INTERFACES = QtDesigner/intmatAll_gui.ui

HEADERS += intmatAllGui.h
SOURCES += intmatAllGui.cpp $(ADOPT_SOURCE)/lib/base/build_time.c

LIBS += -lkdecore -lkdeui -lkio \
		$(ADOPT_SOURCE)/lib/intmatlib.o \
		$(ADOPT_SOURCE)/lib/bcu_diag.o \
		$(ADOPT_SOURCE)/lib/Reorder.o \
		$(ADOPT_SOURCE)/lib/diagBuffer.o \
        $(ADOPT_SOURCE)/lib/arblib/base/basearb_intf.a \
        $(ADOPT_SOURCE)/lib/arblib/adSecArb/adsecarb_lib.a \
        $(ADOPT_SOURCE)/lib/arblib/wfsArb/wfsarb_lib.a \
        $(ADOPT_SOURCE)/lib/arblib/base/basearb_lib.a \
		$(ADOPT_SOURCE)/lib/aoapp.a \
		$(ADOPT_SOURCE)/lib/configlib.a \
		$(ADOPT_SOURCE)/lib/logger.a \
		$(ADOPT_SOURCE)/lib/iolib.o \
		$(ADOPT_SOURCE)/lib/base/thrdlib.a \
		-lstdc++ -lpthread -lrt \ 
        -lboost_serialization -lboost_thread \
        -lcfitsio     # This one for last, otherwise iolib.o doesn't link properly
