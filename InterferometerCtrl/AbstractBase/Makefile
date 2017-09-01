# General Supervisor makefile
include $(ADOPT_SOURCE)/makefile.gen

all: abstractbase.a

# Dependencies
include $(ADOPT_SOURCE)/lib/base/makefile.base

OBJECTS = AbstractFactory.o AbstractMeasurement.o AbstractMeasurementBurst.o

CPPFLAGS += $(PY_INCLUDE)

abstractbase.a: $(BASE_COMPILE)  $(OBJECTS)
	$(AR) abstractbase.a $(OBJECTS)
	$(RANLIB) abstractbase.a

clean: 
	rm -f *.o *.d
	rm -f abstractbase.a

include $(ADOPT_SOURCE)/addeps.gen

