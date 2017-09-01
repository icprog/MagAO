# General Supervisor makefile
include $(ADOPT_SOURCE)/makefile.gen

all: i4d.a

OBJECTS = Factory4D.o Measurement4D.o MeasurementBurst4D.o

## Ristrutturazione direttive include
## Da cancellare dopo la modifica di makefile.gen
## CPP = g++ $(ADOPT_INCLUDE)
########

CPPFLAGS += -I$(ADOPT_SOURCE)/InterferometerCtrl/AbstractBase $(PY_INCLUDE)

i4d.a: $(OBJECTS)
	$(AR) impl4d.a $(OBJECTS)
	$(RANLIB) impl4d.a

clean: 
	rm -f *.o *.d *.so core core.* *~ gmon.out
	rm -f impl4d.a

include $(ADOPT_SOURCE)/addeps.gen

