# General Supervisor makefile
include $(ADOPT_SOURCE)/makefile.gen

all: bculib.a bcucommand.a
	@:

# Source objects
OBJECTS_BCULIB = CommandSender.o BcuMaps.o Commons.o
OBJECTS_BCUCOMMAND = bcucommand.o

OBJECTS = $(OBJECTS_BCULIB) $(OBJECTS_BCUCOMMAND)

### TARGETS ###


bculib.a: $(OBJECTS_BCULIB)
	$(AR) bculib.a $(OBJECTS_BCULIB)
	$(RANLIB) bculib.a
	
# This is an old library, still used
bcucommand.a: $(OBJECTS_BCUCOMMAND)
	$(AR) bcucommand.a $(OBJECTS_BCUCOMMAND)
	$(RANLIB) bcucommand.a
	
clean: 
	rm -f bculib.a bcucommand.a 
	rm -f *.o *.d

include $(ADOPT_SOURCE)/addeps.gen

