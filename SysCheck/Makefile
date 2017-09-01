# General Supervisor makefile
include $(ADOPT_SOURCE)/makefile.gen

# programs to be made
TARGETS = Pinger VarsMonitor #sensorcheckd

all: $(TARGETS)

# Dependencies
include $(ADOPT_LIB)/base/makefile.base
include $(ADOPT_LIB)/makefile.aoapp
include $(ADOPT_LIB)/hwlib/makefile.hwlib
include $(ADOPT_LIB)/BcuLib/makefile.bculib

OBJECTS = Pinger.o args.o chips.o lib.o rrd.o sense.o sensorcheck.o VarsMonitor.o

Pinger: Pinger.o $(AOAPP_COMPILE) $(HWLIB_COMPILE) $(BCULIB_COMPILE) 
	$(LINKPP) -o Pinger Pinger.o $(AOAPP_LNK) $(HWLIB_LNK) $(BCULIB_LNK)

sensorcheckd: args.o chips.o lib.o rrd.o sense.o sensorcheck.o
	$(LINKPP) -o $@ $+ $(MSGLIB_LNK) 

VarsMonitor: $(AOAPP_COMPILE) VarsMonitor.o
	$(LINKPP) -o VarsMonitor VarsMonitor.o $(AOAPP_LNK)

install: all
	install -d $(ADOPT_ROOT)/bin
	install Pinger $(ADOPT_ROOT)/bin
	install VarsMonitor $(ADOPT_ROOT)/bin

clean:
	rm -f $(TARGETS) *.o *.d *.mod

include $(ADOPT_SOURCE)/addeps.gen


