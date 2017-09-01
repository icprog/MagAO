# General Supervisor makefile
include $(ADOPT_SOURCE)/makefile.gen

# Note this is the FIRST target (force 'make' to do 'make all')
all: BCUread BCUread BCUwrite 

# Dependencies
include $(ADOPT_LIB)/makefile.configlib
include $(ADOPT_LIB)/BcuLib/makefile.bculib

### TARGETS ###

getvar: $(CONFIGLIB_COMPILE) $(BCULIB_COMPILE) getvar.o 
	$(LINK) -o getvar getvar.o $(BCULIB_LNK) $(CONFIGLIB_LNK)

setvar: $(CONFIGLIB_COMPILE) $(BCULIB_COMPILE) setvar.o 
	$(LINK) -o setvar setvar.o $(BCULIB_LNK) $(CONFIGLIB_LNK) 

StagesCmd: $(CONFIGLIB_COMPILE) $(BCULIB_COMPILE) StagesCmd.o 
	$(LINK) -o StagesCmd StagesCmd.o $(BCULIB_LNK) $(CONFIGLIB_LNK)  -lpthread

#BCUread: $(CONFIGLIB_COMPILE) $(BCULIB_COMPILE) bcucommand_compile BCUread.o 
BCUread: $(CONFIGLIB_COMPILE) $(BCULIB_COMPILE) BCUread.o 
	$(LINKPP) -o BCUread BCUread.o $(BCULIB_LNK) $(BCUCOMMAND_LNK) $(CONFIGLIB_LNK) -lpthread

BCUwrite: $(CONFIGLIB_COMPILE) $(BCULIB_COMPILE) BCUwrite.o 
	$(LINK) -o BCUwrite BCUwrite.o $(BCULIB_LNK) $(CONFIGLIB_LNK) -lpthread

clean:
	rm -f *.o *.d *.so core core.* *~ gmon.out

tags:
	ctags -t *.[ch]

install: all
	#install BCUread $(ADOPT_ROOT)/bin
