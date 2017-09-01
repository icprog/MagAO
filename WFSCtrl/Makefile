# General Supervisor makefile
include $(ADOPT_SOURCE)/makefile.gen
# programs to be made
TARGETS = JoeCtrl thorlabsProxy

all: $(TARGETS)

# Dependencies
include $(ADOPT_LIB)/makefile.aoapp
include $(ADOPT_LIB)/hwlib/makefile.hwlib


#4 Nov 2010: VISAO_NO_MAIN flag added to turn off compilation of main()
#             if VISAO_NO_MAIN = -D__VISAO_NO_MAIN
#            by Jared R. Males, Steward Observatory
#.cpp.o:
#	$(CPP) $(VISAO_NO_MAIN) $(OPTIMIZE) $(CPPFLAGS) -c $<

CPPFLAGS += $(VISAO_NO_MAIN) 

SOURCES: thorlabsProxy.cpp JoeCtrl.cpp
OBJECTS: JoeCtrl.o thorlabsProxy.o

JoeCtrl: JoeCtrl.o $(AOAPP_COMPILE) $(HWLIB_COMPILE)
	$(LINKPP) -o JoeCtrl JoeCtrl.o $(AOAPP_LNK) $(HWLIB_LNK)

thorlabsProxy: thorlabsProxy.o $(AOAPP_COMPILE) $(HWLIB_COMPILE)
	$(LINKPP) -o thorlabsProxy thorlabsProxy.o $(AOAPP_LNK) $(HWLIB_LNK)

install: all
	install -d $(ADOPT_ROOT)/bin
	install JoeCtrl $(ADOPT_ROOT)/bin

clean:
	rm -f *.o *.d core* *~ gmon.out $(TARGETS)
	
tags:
	ctags -t *.[ch]

include $(ADOPT_SOURCE)/addeps.gen

