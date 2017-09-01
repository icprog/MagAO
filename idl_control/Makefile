# Supervisor general makefile
include $(ADOPT_SOURCE)/makefile.gen

TARGETS = idl_ctrl terminal rpcterminal

all: $(TARGETS)

# Dependencies
include $(ADOPT_SOURCE)/lib/base/makefile.base
include $(ADOPT_SOURCE)/lib/makefile.aoapp
include $(ADOPT_SOURCE)/lib/idllib/makefile.idllib
include $(ADOPT_SOURCE)/lib/makefile.logger
include $(ADOPT_LIB)/arblib/base/makefile.basearb

OBJECTS = idl_term.o idl_ctrl.o terminal.o rpcterminal.o

install: $(TARGETS)
	install -d $(ADOPT_ROOT)/bin
	install $(TARGETS) $(ADOPT_ROOT)/bin

idl_term: idl_term.o $(AOAPP_COMPILE) $(IDLLLIB_COMPILE)
	$(LINKPP) $(OPTIMIZE) $(CPPFLAGS) -o idl_term idl_term.o $(AOAPP_LNK) $(IDLLIB_LNK) -lreadline -lcurses

idl_ctrl: idl_ctrl.o $(AOAPP_COMPILE) $(IDLLLIB_COMPILE)
	$(LINKPP) $(OPTIMIZE) $(CPPFLAGS) -o idl_ctrl idl_ctrl.o $(AOAPP_LNK) $(IDLLIB_LNK) $(BASEARBINTF_LNK)

terminal: terminal.o $(IDLLIB_COMPILE) $(LOGGER_COMPILE)
	$(LINK) $(OPTIMIZE) $(CFLAGS) -o terminal terminal.o $(IDLLIB_LNK) -lreadline -lcurses  

rpcterminal: rpcterminal.o $(IDLLIB_COMPILE) $(LOGGER_COMPILE)
	$(LINK) $(OPTIMIZE) $(CFLAGS) -o rpcterminal rpcterminal.o $(IDLLIB_LNK) -lreadline -lcurses  

clean:
	rm -f *.o *.d *.so core core.* *~ gmon.out $(TARGETS)

tags:
	ctags -t *.[ch]

include $(ADOPT_SOURCE)/addeps.gen

