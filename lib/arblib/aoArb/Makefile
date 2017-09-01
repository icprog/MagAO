# Supervisor general makefile definitions
include $(ADOPT_SOURCE)/makefile.gen

all: aoarb_lib.a 
	@:

# Source objects
OBJECTS = AOCommands.o

aoarb_lib.a: $(OBJECTS)
	$(AR) aoarb_lib.a $(OBJECTS)
	$(RANLIB) aoarb_lib.a	
	
clean: 
	rm -f *.d *.o
	rm -f aoarb_lib.a

include $(ADOPT_SOURCE)/addeps.gen
	