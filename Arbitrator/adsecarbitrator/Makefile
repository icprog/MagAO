# Supervisor general makefile definitions
include $(ADOPT_SOURCE)/makefile.gen


#TARGETS = AdSecArbitrator AdSecFsmCore.png
TARGETS = AdSecArbitrator


all: $(TARGETS) AdSecFsmCore.png

# Dependencies
include $(ADOPT_SOURCE)/Arbitrator/framework/makefile.arbframework
include $(ADOPT_SOURCE)/lib/arblib/adSecArb/makefile.adsecarb
include $(ADOPT_SOURCE)/lib/arblib/aoArb/makefile.aoarb
include $(ADOPT_SOURCE)/lib/idllib/makefile.idllib
include $(ADOPT_SOURCE)/lib/makefile.adamlib
include $(ADOPT_SOURCE)/lib/base/makefile.base
include $(ADOPT_SOURCE)/DiagnApp/makefile.diagnapp

%o:%h

CPPFLAGS += -I$(ADOPT_SOURCE)/Arbitrator

OBJECTS = AdSecArbitratorMain.o \
		  AdSecArbitrator.o \
	  	  AdSecFsm.o \
	  	  AdSecFsmCore.o \
	  	  AdSecCommandImpl.o \
	  	  HousekeeperSystem.o \
	  	  FastDiagnSystem.o \
	  	  MastDiagnSystem.o \
	  	  IdlSystem.o \
	  	  AdamSystem.o \
	  	  AdSecCommandsImpl.o 

LIBS = $(ADOPT_SOURCE)/DiagnApp/diagnapp.a

# NOTE: YOU NEED THE GOOD VERSION OF FSMC: 1.1.1 !!!
AdSecFsmCore.cpp AdSecFsmCore.h: AdSecFsmCore.fsm
	fsmc AdSecFsmCore.fsm -o AdSecFsmCore.h
	fsmc -impl AdSecFsmCore.h AdSecFsmCore.fsm -o AdSecFsmCore.cpp

AdSecFsmCore.png: AdSecFsmCore.fsm
	@echo "##################################################"
	@echo "#                                                #"
	@echo "# NOTE: AdSecFsmCore.fsm has been updated.       #"
	@echo "#                                                #"
	@echo "# AdSecFsmCore image must be recreated.          #"
	@echo "#                                                #"
	@echo "##################################################"

AdSecArbitrator: $(ARBFRAMEWORK_COMPILE) $(ADSECARBLIB_COMPILE) $(IDLLIB_COMPILE) $(AOARBLIB_COMPILE) $(OBJECTS) $(DIAGNAPP_COMPILE)
	$(LINKPP) -o AdSecArbitrator $(OBJECTS) $(LIBS) $(ADSECARBLIB_LNK) $(ARBFRAMEWORK_LNK) $(AOARBLIB_LNK) $(IDLLIB_LNK) $(ADAMLIB_LNK)

install: $(TARGETS)
	install -d $(ADOPT_ROOT)/bin
	install $(TARGETS) $(ADOPT_ROOT)/bin

AdSecFsm.cpp: AdSecFsm.h AdSecFsmCore.cpp AdSecFsmCore.h

AdSecArbitrator.o: AdSecFsmCore.h

clean:
	rm -f *.o *.d *.so core core.* *~ gmon.out
	rm -f $(TARGETS)

include $(ADOPT_SOURCE)/addeps.gen
