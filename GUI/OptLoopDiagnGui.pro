
include ("cc.pro")

CONFIG += qt thread #debug

# Used for the "include" directive only
INCLUDEPATH += /usr/include/kde $(ADOPT_SOURCE)/lib/ 

INTERFACES = QtDesigner/optloopdiagn_gui.ui

HEADERS += OptLoopDiagnGui.h AOAppGui.h
SOURCES += OptLoopDiagnGui.cpp AOAppGui.cpp $(ADOPT_SOURCE)/lib/base/build_time.c

LIBS += -lkdecore -lkdeui \
        $(ADOPT_SOURCE)/lib/arblib/base/basearb_intf.a \
        $(ADOPT_SOURCE)/lib/arblib/wfsArb/wfsarb_lib.a \
        $(ADOPT_SOURCE)/lib/arblib/base/basearb_lib.a \
		$(ADOPT_SOURCE)/lib/aoapp.a \
		$(ADOPT_SOURCE)/lib/configlib.a \
		$(ADOPT_SOURCE)/lib/logger.a \
		$(ADOPT_SOURCE)/lib/base/thrdlib.a \
		$(ADOPT_SOURCE)/lib/aoslib/aoslib.a \
		-lstdc++ -lpthread -lrt \ 
        -lboost_serialization -lboost_thread
