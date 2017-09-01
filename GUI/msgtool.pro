TEMPLATE	= app
LANGUAGE	= C++

CONFIG	+= qt thread

LIBS	+= $(ADOPT_SOURCE)/lib/aoapp.a $(ADOPT_SOURCE)/lib/configlib.a $(ADOPT_SOURCE)/lib/logger.a $(ADOPT_SOURCE)/lib/base/thrdlib.a $(ADOPT_SOURCE)/lib/aoslib/aoslib.a -lstdc++ -lpthread -lrt -lboost_serialization -lboost_thread

INCLUDEPATH	+= $(ADOPT_SOURCE)/lib/

HEADERS	+= msgtool.h

SOURCES	+= msgtool.cpp \
	$(ADOPT_SOURCE)/lib/base/build_time.c

FORMS	= QtDesigner/msgtool_gui.ui

IMAGES	=
