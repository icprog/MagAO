#
# General Supervisor makefile
include $(ADOPT_SOURCE)/makefile.gen


# Source objects
OBJECTS = netseriallib.o joelib.o idrivelib.o copleylib.o

### TARGETS ###

all: hwlib.a

hwlib.a: $(OBJECTS)
	$(AR) hwlib.a $(OBJECTS)
	$(RANLIB) hwlib.a
	
clean: 
	rm -f *.o *.d core *~ gmon.out *.pdf
	rm -f *.aux *.dvi *.log *.toc *.tmp
	rm -f hwlib.a
	rm -f $(OBJECTS)
	
tags:
	ctags -t *.[ch]

docs: netseriallib.pdf idrivelib.pdf joelib.pdf

netseriallib.pdf: netseriallib.dpu netseriallib.c
	dpu -pdf netseriallib

idrivelib.pdf: idrivelib.dpu idrivelib.c
	dpu -pdf idrivelib

joelib.pdf: joelib.dpu joelib.c
	dpu -pdf joelib

# Dependencies
include $(ADOPT_SOURCE)/addeps.gen
