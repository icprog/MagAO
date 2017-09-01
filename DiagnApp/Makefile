include $(ADOPT_SOURCE)/makefile.gen

all: diagnapp.a

# Dependencies
include $(ADOPT_SOURCE)/lib/base/makefile.base

OBJECTS =  DiagnApp.o RunningMean.o DiagnVar.o DiagnValue.o DiagnSet.o DiagnTelemetry.o Funct.o diagnlib.o 

CPPFLAGS += -fno-strict-aliasing

diagnapp.a: $(BASE_COMPILE) $(OBJECTS) 
	$(AR) diagnapp.a $(OBJECTS)
	$(RANLIB) diagnapp.a

tests: 
	cd Test; make

install:

clean:
	rm -f $(OBJECTS)
	rm -f diagnapp.a
	rm -f *.o *.d *.so core core.* *~ gmon.out

tags:
	ctags *.[ch]
	cscope -Rbq

docs: DiagnApp.pdf

DiagnApp.pdf: DiagnApp.cpp DiagnApp.h DiagnApp.dpu
	dpu -pdf DiagnApp

include $(ADOPT_SOURCE)/addeps.gen

