# Supervisor general makefile definitions
include $(ADOPT_SOURCE)/makefile.gen

all: wfsarb_lib.a
	@:

# Dependencies
include $(ADOPT_SOURCE)/lib/base/makefile.base

# Source objects
OBJECTS = WfsCommands.o 
			  
wfsarb_lib.a: $(OBJECTS)
	$(AR) wfsarb_lib.a $(OBJECTS)
	$(RANLIB) wfsarb_lib.a

clean: 
	rm -f *.d *.o
	rm -f wfsarb_lib.a

include $(ADOPT_SOURCE)/addeps.gen