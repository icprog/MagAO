# General Supervisor makefile
include $(ADOPT_SOURCE)/makefile.gen

OBJECTS = AbstractBcuRequestManager.o TimeToDie.o \
		  BcuRequest.o BcuRequestInfo.o \
		  BcuPacket.o BcuPacketQueue.o
		  

CPPFLAGS += -I$(ADOPT_SOURCE)/MirrorCtrl

all: $(OBJECTS)

clean:
	rm -f *.o *.d core core.*

include $(ADOPT_SOURCE)/addeps.gen

