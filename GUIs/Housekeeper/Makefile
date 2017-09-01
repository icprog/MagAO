#############################################################################
# Makefile for building:   Housekeeper_gui
#############################################################################



include $(ADOPT_SOURCE)/makefile.gen

TARGET   = Housekeeper_gui
all: Makefile $(TARGET)

## Dependencies

include $(ADOPT_SOURCE)/DiagnApp/makefile.diagnapp
include $(ADOPT_SOURCE)/lib/makefile.aoapp


####### Compiler, tools and options
QWT_INCLUDE_DIR = /usr/local/qwt-5.1.2/include
QWT_LIB_DIR = /usr/local/qwt-5.1.2/lib
INCPATH  = -I$(QTDIR)/mkspecs/default -I. -I$(QTDIR)/include -I./ -I.ui -I./ -I/usr/include/kde -I$(QWT_INCLUDE_DIR) -I./
INCPATH2 = -I$(ADOPT_SOURCE)/lib/ -I$(ADOPT_SOURCE)/lib/base -I$(ADOPT_SOURCE)/DiagnApp

LIBS     = $(SUBLIBS) -L$(QTDIR)/lib -L$(QWT_LIB_DIR) -L/usr/lib/ -lkdeui -lqt-mt -lm -lqwt
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
		./Housekeeper_gui.o \
		./PlotForm.o
		#./PlotForm.cpp
SRCMOC   = ./moc_Housekeeper_gui.cpp \
		./moc_PlotForm.cpp
OBJMOC = ./moc_Housekeeper_gui.o \
		./moc_PlotForm.o


first: all
####### Implicit rules

CPPFLAGS += $(INCPATH)
CPPFLAGS += $(INCPATH2)

./moc_Housekeeper_gui.cpp: $(MOC) ./Housekeeper_gui.h
	$(MOC) ./Housekeeper_gui.h -o ./moc_Housekeeper_gui.cpp

./moc_PlotForm.cpp: $(MOC) ./PlotForm.h
	$(MOC) ./PlotForm.h -o ./moc_PlotForm.cpp


####### Build rules


$(TARGET):  $(UICDECLS) $(OBJECTS) $(OBJMOC) $(DIAGNAPP_COMPILE) $(AOAPP_COMPILE)
	$(LINKPP) $(LFLAGS) $(OPTIMIZE) -o $(TARGET) $(OBJECTS) $(OBJMOC) $(OBJCOMP) $(LIBS) $(LIBS3)

#mocables: $(SRCMOC)
#uicables: $(UICDECLS) $(UICIMPLS)

$(MOC): 
	( cd $(QTDIR)/src/moc && $(MAKE) )


#dist:
#	@mkdir -p ./lbt672_housekeeping && $(COPY_FILE) --parents $(SOURCES) $(HEADERS) $(FORMS) $(DIST) ./lbt672_housekeeping/ && ( cd `dirname ./lbt672_housekeeping` && $(TAR) lbt672_housekeeping.tar lbt672_housekeeping && $(GZIP) lbt672_housekeeping.tar ) && $(MOVE) `dirname ./lbt672_housekeeping`/lbt672_housekeeping.tar.gz . && $(DEL_FILE) -r ./lbt672_housekeeping

mocclean:
	-$(DEL_FILE) $(OBJMOC)
	-$(DEL_FILE) $(SRCMOC)

#uiclean:
#	-$(DEL_FILE) $(UICIMPLS) $(UICDECLS)

clean:
	-$(DEL_FILE) $(OBJECTS)
	-$(DEL_FILE) *~ core core.* *.core *.o *.d


####### Sub-libraries

distclean: clean
	-$(DEL_FILE) $(TARGET) $(TARGET)


FORCE:

####### Install

install: all
	install -d $(ADOPT_ROOT)/bin
	install Housekeeper_gui $(ADOPT_ROOT)/bin

uninstall:  

include $(ADOPT_SOURCE)/addeps.gen

