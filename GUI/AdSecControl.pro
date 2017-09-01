
include ("cc.pro")

CONFIG += qt thread

# Used for the "include" directive only
INCLUDEPATH += /usr/include/kde $(ADOPT_SOURCE)/lib/ 

HEADERS += AdSecControl.h AbstractArbControl.h AdSecCalibration.h QtDesigner/AdSecControl_gui.h QtDesigner/AdSecCalibration_gui.h RecPreview.h AOAppGui.h
SOURCES += AdSecControl.cpp AbstractArbControl.cpp AdSecCalibration.cpp QtDesigner/AdSecControl_gui.cpp QtDesigner/AdSecCalibration_gui.cpp RecPreview.cpp AOAppGui.cpp $(ADOPT_SOURCE)/lib/base/build_time.c

#CONFIG += qt debug

LIBS += -lkdecore -lkdeui -lkio \
		$(ADOPT_SOURCE)/lib/arblib/base/basearb_intf.a \
		$(ADOPT_SOURCE)/lib/arblib/adSecArb/adsecarb_lib.a \
		$(ADOPT_SOURCE)/lib/arblib/base/basearb_lib.a \
		$(ADOPT_SOURCE)/lib/aoapp.a \
		$(ADOPT_SOURCE)/lib/configlib.a \
		$(ADOPT_SOURCE)/lib/logger.a \
		$(ADOPT_SOURCE)/lib/adamlib.a \
		$(ADOPT_SOURCE)/lib/netlib.a \
		$(ADOPT_SOURCE)/lib/iolib.a \
		$(ADOPT_SOURCE)/lib/base/thrdlib.a \
		-lstdc++ -lpthread -lrt \
		-L$(ADOPT_SOURCE)/contrib/cfitsio -lcfitsio -lm\
		-lboost_serialization -lboost_thread
