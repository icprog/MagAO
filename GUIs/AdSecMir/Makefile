#############################################################################
# Makefile for building:   AdSecMir_gui
#############################################################################



include $(ADOPT_SOURCE)/makefile.gen

TARGET   = AdSecMir_gui
all: Makefile $(TARGET)

## Dependencies

include $(ADOPT_SOURCE)/DiagnApp/makefile.diagnapp
include $(ADOPT_SOURCE)/lib/makefile.aoapp


####### Compiler, tools and options

INCPATH  = -I$(QTDIR)/mkspecs/default -I. -I$(QTDIR)/include -I./ -I.ui -I./ -I/usr/include/kde -I/usr/local/qwt-5.1.2/include -I./
INCPATH2 = -I$(ADOPT_SOURCE)/lib/ -I$(ADOPT_SOURCE)/lib/base -I$(ADOPT_SOURCE)/DiagnApp

LIBS     = $(SUBLIBS) -L$(QTDIR)/lib -L/usr/local/qwt-5.1.2/lib -L/usr/lib/ -lkdeui -lqt-mt -lXext -lX11 -lm -lqwt
LIBS2    = -L$(ADOPT_SOURCE)/contrib/cfitsio -lcfitsio -lpthread -lboost_serialization -lboost_thread -lgmp -lrt -lstdc++
LIBS3    = $(DIAGNAPP_LNK) $(AOAPP_LNK) 

MOC      = $(QTDIR)/bin/moc
UIC      = $(QTDIR)/bin/uic

DEL_FILE = rm -f
DEL_DIR  = rmdir
MOVE     = mv -f
CHK_DIR_EXISTS= test -d

####### Output directory

OBJECTS_DIR = ./

####### Files

HEADERS = 
SOURCES = main.cpp
OBJECTS = ./main.o \
		./Mirror.o \
		./PlotModes.o \
		./AdSecMir_gui.o 
SRCMOC   = ./moc_AdSecMir_gui.cpp  ./moc_Mirror.cpp  ./moc_PlotModes.cpp
OBJMOC = ./moc_AdSecMir_gui.o  ./moc_Mirror.o  ./moc_PlotModes.o


first: all
####### Implicit rules

CPPFLAGS += $(INCPATH)
CPPFLAGS += $(INCPATH2)

####### Build rules


$(TARGET):  $(UICDECLS) $(OBJECTS) $(OBJMOC) $(DIAGNAPP_COMPILE) $(AOAPP_COMPILE)
	$(LINKPP) $(LFLAGS) $(OPTIMIZE) -o $(TARGET) $(OBJECTS) $(OBJMOC) $(OBJCOMP) $(LIBS) $(LIBS2) $(LIBS3)

mocables: $(SRCMOC)
uicables: $(UICDECLS) $(UICIMPLS)

$(MOC): 
	( cd $(QTDIR)/src/moc && $(MAKE) )



mocclean:
	-$(DEL_FILE) $(OBJMOC)
	-$(DEL_FILE) $(SRCMOC)

#uiclean:
#	-$(DEL_FILE) $(UICIMPLS) $(UICDECLS)

clean:
	-$(DEL_FILE) $(OBJECTS)
	-$(DEL_FILE) *~ core *.core core.*


####### Sub-libraries

distclean: clean
	-$(DEL_FILE) $(TARGET) $(TARGET)

./moc_AdSecMir_gui.o: ./moc_AdSecMir_gui.cpp  ./AdSecMir_gui.h   ./Mirror.h  ./PlotModes.h
	$(CPP) -c $(CPPFLAGS) $(INCPATH) $(INCPATH2) $(OPTIMIZE) -o ./moc_AdSecMir_gui.o ./moc_AdSecMir_gui.cpp

./moc_AdSecMir_gui.cpp: $(MOC) ./AdSecMir_gui.h   ./Mirror.h   ./PlotModes.h
	$(MOC) ./AdSecMir_gui.h -o ./moc_AdSecMir_gui.cpp

./moc_PlotModes.o: ./moc_PlotModes.cpp   ./PlotModes.h
	$(CPP) -c $(CPPFLAGS) $(INCPATH) $(INCPATH2) $(OPTIMIZE) -o ./moc_PlotModes.o ./moc_PlotModes.cpp

./moc_PlotModes.cpp: $(MOC)  ./PlotModes.h
	$(MOC) ./PlotModes.h -o ./moc_PlotModes.cpp

./moc_Mirror.o: ./moc_Mirror.cpp   ./Mirror.h
	$(CPP) -c $(CPPFLAGS) $(INCPATH) $(INCPATH2) $(OPTIMIZE) -o ./moc_Mirror.o ./moc_Mirror.cpp

./moc_Mirror.cpp: $(MOC)  ./Mirror.h
	$(MOC) ./Mirror.h -o ./moc_Mirror.cpp




FORCE:

####### Install

install:  
	install -d $(ADOPT_ROOT)/bin
	install AdSecMir_gui $(ADOPT_ROOT)/bin

uninstall:  

include $(ADOPT_SOURCE)/addeps.gen

