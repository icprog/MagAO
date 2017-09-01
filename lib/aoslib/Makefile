include $(ADOPT_SOURCE)/makefile.gen

# Libraries to be compiled

TARGETS = aoslib.a 

OBJECTS = aoslib.o

all: $(TARGETS)

aoslib.a: aoslib.o
	$(AR) aoslib.a aoslib.o
	$(RANLIB) aoslib.a

aoslib.o: aoscodes.c

aoscodes.h aoscodes.c: aoscodes.txt
	../makerrordb.py aoscodes.txt -py AOSCode
	mv -f AOSCode.py ../../PyModules/AdOpt

clean:
	rm -f *.a *.o *.d core.* core *~ gmon.out *.pdf
	rm -f *.aux *.dvi *.log *.toc *.tmp
	
	
tags:
	ctags -t *.[ch]

docs: aoslib.pdf 

aoslib.pdf: aoslib.dpu aoslib.c aoslib.h
	dpu -pdf msglib

include $(ADOPT_SOURCE)/addeps.gen

