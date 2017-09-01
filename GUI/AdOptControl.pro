
include ("cc.pro")

CONFIG += qt thread #debug

# Used for the "include" directive only
INCLUDEPATH += /usr/include/kde $(ADOPT_SOURCE)/lib/ 

HEADERS += AdOptControl.h AbstractArbControl.h AOAppGui.h QtDesigner/AdOptControl_gui.h
SOURCES += AdOptControl.cpp AbstractArbControl.cpp AOAppGui.cpp QtDesigner/AdOptControl_gui.cpp $(ADOPT_SOURCE)/lib/base/build_time.c

LIBS += -lkdecore -lkdeui \
		$(ADOPT_SOURCE)/lib/arblib/base/basearb_intf.a \
		$(ADOPT_SOURCE)/lib/arblib/aoArb/aoarb_lib.a \
		$(ADOPT_SOURCE)/lib/arblib/base/basearb_lib.a \
		$(ADOPT_SOURCE)/lib/aoapp.a \
		$(ADOPT_SOURCE)/lib/configlib.a \
		$(ADOPT_SOURCE)/lib/logger.a \
		$(ADOPT_SOURCE)/lib/base/thrdlib.a \
		-lstdc++ -lpthread -lrt \
		-lboost_serialization -lboost_thread
