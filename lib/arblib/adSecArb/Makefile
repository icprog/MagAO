# Supervisor general makefile definitions
include $(ADOPT_SOURCE)/makefile.gen

all: adsecarb_lib.a
	@:

# Dependencies
include $(ADOPT_SOURCE)/lib/base/makefile.base

# Source objects
OBJECTS = AdSecCommands.o

adsecarb_lib.a: $(OBJECTS)
	$(AR) adsecarb_lib.a $(OBJECTS)
	$(RANLIB) adsecarb_lib.a

clean: 
	rm -f *.d *.o
	rm -f adsecarb_lib.a

include $(ADOPT_SOURCE)/addeps.gen
