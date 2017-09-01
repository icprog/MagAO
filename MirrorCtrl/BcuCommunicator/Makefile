# General Supervisor makefile
include $(ADOPT_SOURCE)/makefile.gen

OBJECTS = BcuCommunicator.o Scheduler.o RoundQueue.o
		  
CPPFLAGS += -I$(ADOPT_SOURCE)/MirrorCtrl

all: $(OBJECTS)

clean:
	rm -f *.o *.d core core.*

include $(ADOPT_SOURCE)/addeps.gen

