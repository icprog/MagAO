# 'supervisor' general makefile definitions
include $(ADOPT_SOURCE)/makefile.gen

all: testAOArbInterface testAdSecArbInterface  

# Dependencies
include $(ADOPT_SOURCE)/lib/arblib/base/makefile.basearb
include $(ADOPT_SOURCE)/lib/arblib/aoArb/makefile.aoarb
include $(ADOPT_SOURCE)/lib/arblib/adSecArb/makefile.adsecarb

OBJECTS = testAoArbIntf.o testAdSecArbIntf.o

testAOArbInterface: $(BASEARBINTF_COMPILE) $(AOARBLIB_COMPILE) testAoArbIntf.o 
	$(LINKPP) -o testAoArbIntf testAoArbIntf.o $(BASEARBINTF_LNK) $(AOARBLIB_LNK)
	
testAdSecArbInterface: $(BASEARBINTF_COMPILE) $(ADSECARBLIB_COMPILE) testAdSecArbIntf.o 
	$(LINKPP) -o testAdSecArbIntf testAdSecArbIntf.o $(BASEARBINTF_LNK) $(ADSECARBLIB_LNK)
	
clean: 
	rm -f *.d *.o
	rm -f testAoArbIntf testAdSecArbIntf

include $(ADOPT_SOURCE)/addeps.gen	
	
