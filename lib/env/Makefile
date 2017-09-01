# General Supervisor makefile
include $(ADOPT_SOURCE)/makefile.gen

TARGETS = envlib.a
OBJECTS = Anemometer.o AnemometerData.o AnemometerRate.o chardev.o

all: $(TARGETS)

envlib.a: $(OBJECTS)
	$(AR) envlib.a ../TcpConnection.o ../UdpConnection.o $(OBJECTS)
	$(RANLIB) envlib.a

clean: 
	rm -f envlib.a
	rm -f *.o *.d *~

include $(ADOPT_SOURCE)/addeps.gen

