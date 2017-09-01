
include ("cc.pro")

CONFIG += qt thread #debug

# Used for the "include" directive only
INCLUDEPATH += /usr/include/kde $(ADOPT_SOURCE)/lib/ 

HEADERS += intmatDisturbGui.h  AOAppGui.h QtDesigner/intmatDisturb_gui.h
SOURCES += intmatDisturbGui.cpp AOAppGui.cpp QtDesigner/intmatDisturb_gui.cpp $(ADOPT_SOURCE)/lib/base/build_time.c

LIBS += -lkdecore -lkdeui -lkio \
		$(ADOPT_SOURCE)/lib/aoapp.a \
		$(ADOPT_SOURCE)/lib/configlib.a \
		$(ADOPT_SOURCE)/lib/logger.a \
		$(ADOPT_SOURCE)/lib/base/thrdlib.a \
		-lstdc++ -lpthread -lrt \ 
        -lboost_serialization -lboost_thread
