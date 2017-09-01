# Supervisor general makefile definitions
include $(ADOPT_SOURCE)/makefile.gen

TARGETS = WfsArbitrator

all: $(TARGETS) WfsFsmCore.png

# Dependencies
include $(ADOPT_SOURCE)/Arbitrator/framework/makefile.arbframework
include $(ADOPT_SOURCE)/lib/arblib/wfsArb/makefile.wfsarb
include $(ADOPT_SOURCE)/lib/arblib/aoArb/makefile.aoarb
include $(ADOPT_SOURCE)/lib/arblib/adSecArb/makefile.adsecarb

%o:%h

CPPFLAGS += -I$(ADOPT_SOURCE)/Arbitrator

OBJECTS = WfsArbitratorMain.o \
	WfsArbitrator.o \
	WfsFsm.o \
        WfsFsmCore.o \
	WfsCommandImpl.o \
	WfsSystem.o \
	AdSecSystem.o \
	MastDiagnSystem.o \
	OptLoopDiagSystem.o \
	WfsInterface.o \
	WfsCommandsImpl.o \
	adcLut.o

# NOTE: YOU NEED THE GOOD VERSION OF FSMC: 1.1.1 !!!
WfsFsmCore.cpp WfsFsmCore.h: WfsFsmCore.fsm
	fsmc WfsFsmCore.fsm -o WfsFsmCore.h
	fsmc -impl WfsFsmCore.h WfsFsmCore.fsm -o WfsFsmCore.cpp

WfsFsmCore.png: WfsFsmCore.fsm
	@echo "##################################################"
	@echo "#                                                #"
	@echo "# NOTE: WfsFsmCore.fsm has been updated.         #"
	@echo "#                                                #"
	@echo "# WfsFsmCore image must be recreated.            #"
	@echo "#                                                #"
	@echo "##################################################"

WfsFsm.cpp: WfsFsm.h WfsFsmCore.cpp WfsFsmCore.h


	
WfsArbitrator: $(ARBFRAMEWORK_COMPILE) $(WFSARBLIB_COMPILE) $(AOARBLIB_COMPILE) $(OBJECTS)
	$(LINKPP) -o WfsArbitrator $(OBJECTS) $(ARBFRAMEWORK_LNK) $(WFSARBLIB_LNK) $(AOARBLIB_LNK) $(PY_LIB_LNK) $(ADSECARBLIB_LNK)

adcTest.o: adcTest.cpp
	$(CPP) $(CPPFLAGS) -c adcTest.cpp

adcLut.o: adcLut.cpp
	$(CPP) $(CPPFLAGS) -c adcLut.cpp

adcTest: adcTest.o  adcLut.o
	$(LINKPP) -o adcTest adcTest.o adcLut.o $(ARBFRAMEWORK_LNK) $(WFSARBLIB_LNK) $(AOARBLIB_LNK) $(PY_LIB_LNK) $(ADSECARBLIB_LNK)


install: $(TARGETS)
	install -d $(ADOPT_ROOT)/bin
	install $(TARGETS) $(ADOPT_ROOT)/bin
	
clean:
	rm -f *.o *.d *.so core core.* *~ gmon.out
	rm -f $(TARGETS)
	
include $(ADOPT_SOURCE)/addeps.gen
