include  $(ADOPT_SOURCE)/makefile.gen

# Dependencies      
include $(ADOPT_SOURCE)/lib/makefile.logger
include $(ADOPT_SOURCE)/lib/makefile.netlib
include $(ADOPT_SOURCE)/lib/makefile.configlib
include $(ADOPT_SOURCE)/lib/makefile.aoapp

### Note: if your O.S. has both the GNU readline and GNU history libraries, 
### then you can add line editing and history uncommenting the following two lines
HISTORY = -DHAS_HISTORY 
HISTORY_LIBS = -lreadline -lhistory -lcurses
#################################################

CPPFLAGS += $(HISTORY)

OBJECTS = hexapodtest.o Hexapod.o HexapodCtrl.o

### TARGETS ###

all: hexapodtest HexapodCtrl

install: all
	install -d $(ADOPT_ROOT)/bin
	install hexapodtest $(ADOPT_ROOT)/bin
	install HexapodCtrl $(ADOPT_ROOT)/bin

hexapodtest: $(LOGGER_COMPILE) $(NETLIB_COMPILE) $(CONFIGLIB_COMPILE) hexapodtest.o Hexapod.o 
	$(LINKPP) -o hexapodtest hexapodtest.o Hexapod.o  $(LOGGER_LNK) $(NETLIB_LNK) $(CONFIGLIB_LNK) $(LIBS) $(HISTORY_LIBS)

HexapodCtrl: $(AOAPP_COMPILE) $(NETLIB_COMPILE) Hexapod.o HexapodCtrl.o
	$(LINKPP) -o HexapodCtrl HexapodCtrl.o Hexapod.o $(AOAPP_LNK) $(NETLIB_LNK)

clean:
	rm -f *.o *.d core.* core *~ gmon.out HexapodCtrl hexapodtest

tags:
	ctags -t *.[ch]

include $(ADOPT_SOURCE)/addeps.gen

