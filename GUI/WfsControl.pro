
include ("cc.pro")

CONFIG += qt thread #debug

# Used for the "include" directive only
INCLUDEPATH += /usr/include/kde $(ADOPT_SOURCE)/lib/ 

INTERFACES = QtDesigner/WfsControl_gui.ui

HEADERS += WfsControl.h AbstractArbControl.h AOAppGui.h
SOURCES += WfsControl.cpp AbstractArbControl.cpp AOAppGui.cpp $(ADOPT_SOURCE)/lib/base/build_time.c

LIBS += -lkdecore -lkdeui \
		$(ADOPT_SOURCE)/lib/arblib/base/basearb_intf.a \
		$(ADOPT_SOURCE)/lib/arblib/wfsArb/wfsarb_lib.a \
		$(ADOPT_SOURCE)/lib/arblib/base/basearb_lib.a \
		$(ADOPT_SOURCE)/lib/aoapp.a \
		$(ADOPT_SOURCE)/lib/configlib.a \
		$(ADOPT_SOURCE)/lib/logger.a \
		$(ADOPT_SOURCE)/lib/base/thrdlib.a \
		-lstdc++ -lpthread -lrt \
		-lboost_serialization -lboost_thread
