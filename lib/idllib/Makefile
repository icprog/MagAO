# General Supervisor makefile
include $(ADOPT_SOURCE)/makefile.gen

# Source objects
OBJECTS = IdlCtrlInterface.o idllib.o


### TARGETS ###

all: idllib.a

idllib.a: $(OBJECTS)
	$(AR) idllib.a $(OBJECTS)
	$(RANLIB) idllib.a
	
clean: 
	rm -f idllib.a
	rm -f *.o *.d

include $(ADOPT_SOURCE)/addeps.gen

