# General Supervisor makefile
include $(ADOPT_SOURCE)/makefile.gen

all: interferometerlib.a
	@:

# Dependencies
include $(ADOPT_SOURCE)/lib/base/makefile.base

# Needed for parapin library
OPTIMIZE = -O

OBJECTS = InterferometerInterface.o

interferometerlib.a: $(OBJECTS)
	$(AR) interferometerlib.a $(OBJECTS) 
	$(RANLIB) interferometerlib.a

clean: 
	rm -f *.d *.o
	rm -f interferometerlib.a

include $(ADOPT_SOURCE)/addeps.gen



	