include  $(ADOPT_SOURCE)/makefile.gen

TARGETS = CameraLensCtrl

all: $(TARGETS)

# Dependencies
include $(ADOPT_SOURCE)/lib/makefile.aoapp
include $(ADOPT_SOURCE)/lib/BcuLib/makefile.bculib

OBJECTS = CameraLensCtrl.o             

### TARGETS ###

CameraLensCtrl: $(AOAPP_COMPILE) $(BCULIB_COMPILE) $(OBJECTS) 
	$(LINKPP) -o CameraLensCtrl $(OBJECTS) $(AOAPP_LNK) $(BCULIB_LNK) 

install:
	install -d $(ADOPT_ROOT)/bin
	install $(TARGETS) $(ADOPT_ROOT)/bin

clean:
	rm -f $(TARGETS) $(OBJECTS) $(OBJECTS:.o=.o.d)

include $(ADOPT_SOURCE)/addeps.gen
	
