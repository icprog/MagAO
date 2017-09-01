
# General Supervisor makefile
include $(ADOPT_SOURCE)/makefile.gen

# programs to be made
TARGETS = AnemometerMon

all: $(TARGETS)

# Dependencies
include $(ADOPT_LIB)/base/makefile.base
include $(ADOPT_LIB)/makefile.aoapp

OBJECTS = AnemometerMon.o

AnemometerMon: $(AOAPP_COMPILE) AnemometerMon.o
	$(LINKPP) -o AnemometerMon AnemometerMon.o $(AOAPP_LNK) $(ADOPT_LIB)/env/envlib.a

install: all
	install AnemometerMon $(ADOPT_ROOT)/bin

clean:
	rm -f $(TARGETS) *.o *.d *.mod *~

include $(ADOPT_SOURCE)/addeps.gen


