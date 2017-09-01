#
# General Supervisor makefile
include $(ADOPT_SOURCE)/makefile.gen

all: drawlib.a

# Source objects
OBJECTS = drawobjects.o
SOURCES = drawobjects.cpp


### TARGETS ###

drawlib.a: $(OBJECTS)
	$(Q)qmake -o QMakefile drawobjects.pro
	$(Q)$(MAKE) $(MAKEOPTS) -f QMakefile
	$(Q)rm -f QMakefile
	
clean: 
	rm -f *.o *.d *.a core *~ gmon.out *.pdf
	
tags:
	ctags -t *.[ch]

include $(ADOPT_SOURCE)/addeps.gen

