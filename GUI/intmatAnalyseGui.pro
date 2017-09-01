
include ("cc.pro")

CONFIG += qt thread #debug

# Used for the "include" directive only
INCLUDEPATH += /usr/include/kde $(ADOPT_SOURCE)/lib/ 

HEADERS += intmatAnalyseGui.h AOAppGui.h QtDesigner/intmatAnalyse_gui.h
SOURCES += intmatAnalyseGui.cpp AOAppGui.cpp QtDesigner/intmatAnalyse_gui.cpp $(ADOPT_SOURCE)/lib/base/build_time.c

LIBS += -lkdecore -lkdeui -lkio \
		$(ADOPT_SOURCE)/lib/aoapp.a \
		$(ADOPT_SOURCE)/lib/configlib.a \
		$(ADOPT_SOURCE)/lib/logger.a \
		$(ADOPT_SOURCE)/lib/base/thrdlib.a \
		-lstdc++ -lpthread -lrt \ 
        -lboost_serialization -lboost_thread
