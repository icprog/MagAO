# 'supervisor' general makefile definitions
include $(ADOPT_SOURCE)/makefile.gen

TARGETS = AOArbitrator 
DOCS    = AOArbFsmCore.png

all: $(TARGETS) $(DOCS)


# Dependencies
include $(ADOPT_SOURCE)/lib/arblib/aoArb/makefile.aoarb
include $(ADOPT_SOURCE)/lib/arblib/adSecArb/makefile.adsecarb
include $(ADOPT_SOURCE)/lib/arblib/wfsArb/makefile.wfsarb
include $(ADOPT_SOURCE)/lib/makefile.configlib
include $(ADOPT_SOURCE)/Arbitrator/framework/makefile.arbframework
include $(ADOPT_LIB)/makefile.adsecdiagnlib
include $(ADOPT_LIB)/makefile.iolib



CPPFLAGS += -I$(ADOPT_SOURCE)/Arbitrator

OBJECTS = AOArbitratorMain.o AOArbitrator.o AOArbFsm.o AOArbFsmCore.o AOCommandImpl.o \
	      WfsArbSystem.o AdsecArbSystem.o AOCommandsImpl.o  

	
# NOTE: YOU NEED THE GOOD VERSION OF FSMC Version 1.1.0 (by L.Fini)

AOArbitrator.cpp: AOArbitrator.h AOCommandImplFactory.h AOArbFsm.h AdsecArbSystem.h WfsArbSystem.h \
                  AOArbFsmCore.h 
AOArbFsm.cpp: AOArbFsmCore.cpp  AOArbFsm.h

AOArbFsmCore.png: AOArbFsmCore.fsm
	@echo
	@echo "*************************************************"
	@echo "* AOArbFsmCore.fsm has been updated.            *"
	@echo "* The FSM diagram file AOArbFsmCore.png must be *"
	@echo "* recreated (unfortunately by hand)             *"
	@echo "*************************************************"
	@echo


AOArbFsmCore.h AOArbFsmCore.cpp: AOArbFsmCore.fsm
	fsmc AOArbFsmCore.fsm -o AOArbFsmCore.h
	fsmc -impl AOArbFsmCore.h AOArbFsmCore.fsm -o AOArbFsmCore.cpp
	
AOArbitrator: $(ARBFRAMEWORK_COMPILE) $(AOARBLIB_COMPILE) $(WFSARBLIB_COMPILE) $(ADSECARBLIB_COMPILE) $(OBJECTS)
	$(LINKPP) -o AOArbitrator $(OBJECTS) $(ARBFRAMEWORK_LNK) $(AOARBLIB_LNK) $(WFSARBLIB_LNK) $(ADSECARBLIB_LNK) $(CONFIGLIB_LNK) $(ADSECDIAGNLIB_LNK) $(IOLIB_LNK) -L$(ADOPT_SOURCE)/contrib/cfitsio -lcfitsio

	
install: $(TARGETS)
	install -d $(ADOPT_ROOT)/bin
	install $(TARGETS) $(ADOPT_ROOT)/bin
	
clean:
	rm -f *.o *.d *.so core core.* *~ gmon.out
	rm -f $(TARGETS)

include $(ADOPT_SOURCE)/addeps.gen


	
	
