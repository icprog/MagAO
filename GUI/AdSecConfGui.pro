
include ("cc.pro")

CONFIG += qt thread #debug

LIBPATH += $(ADOPT_SOURCE)/contrib/cfitsio

# Used for the "include" directive only
INCLUDEPATH += /usr/include/kde $(ADOPT_SOURCE)/lib/ 

INTERFACES = QtDesigner/AdSecConf_gui.ui

HEADERS += AdSecConfGui.h 
SOURCES += AdSecConfGui.cpp $(ADOPT_SOURCE)/lib/base/build_time.c

LIBS += -lkdecore -lkdeui \
        $(ADOPT_SOURCE)/lib/arblib/base/basearb_intf.a \
        $(ADOPT_SOURCE)/lib/arblib/wfsArb/wfsarb_lib.a \
        $(ADOPT_SOURCE)/lib/arblib/base/basearb_lib.a \
		$(ADOPT_SOURCE)/lib/iolib.a \
		$(ADOPT_SOURCE)/lib/aoapp.a \
		$(ADOPT_SOURCE)/lib/configlib.a \
		$(ADOPT_SOURCE)/lib/logger.a \
		$(ADOPT_SOURCE)/lib/base/thrdlib.a \
		$(ADOPT_SOURCE)/lib/aoslib/aoslib.a \
		-lstdc++ -lpthread -lrt \ 
        -lboost_serialization -lboost_thread \
	-lcfitsio     # This one for last, otherwise iolib.o doesn't link properly

