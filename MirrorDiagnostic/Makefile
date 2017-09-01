include $(ADOPT_SOURCE)/makefile.gen

# Here to be sure is the first target (force 'make' to do 'make all')
all: fastdiagn producer_fast

# Dependencies
include $(ADOPT_SOURCE)/DiagnApp/makefile.diagnapp
include $(ADOPT_LIB)/BcuLib/makefile.bculib
include $(ADOPT_LIB)/makefile.adamlib
include $(ADOPT_LIB)/makefile.adsecdiagnlib
include $(ADOPT_LIB)/BcuLib/makefile.bculib
include $(ADOPT_LIB)/aoslib/makefile.aoslib

FASTDIAGN_OBJECTS = fastdiagn.o FastDiagnostic.o FastDiagnostic_convert.o
FASTPROD_OBJECTS = producer_fast.o

OBJECTS = $(FASTDIAGN_OBJECTS) $(FASTPROD_OBJECTS)

CPPFLAGS += -I$(ADOPT_SOURCE)/DiagnApp

OPTIMIZE= -ggdb3 -O3 -march=pentium4

### TARGETS ###

tests: 
	cd Test; ${MAKE} all


fastdiagn: $(DIAGNAPP_COMPILE) $(ADAMLIB_COMPILE) $(ADSECDIAGNLIB_COMPILE) $(MASTDIAGNLIB_COMPILE) $(BCULIB_COMPILE) $(AOSLIB_COMPILE) $(FASTDIAGN_OBJECTS)
	$(LINKPP) -o fastdiagn $(FASTDIAGN_OBJECTS) $(DIAGNAPP_LNK) $(ADAMLIB_LNK) $(MASTDIAGNLIB_LNK) $(BCULIB_LNK) $(AOSLIB_LNK) $(ADSECDIAGNLIB_LNK)

producer_fast: $(CONFIGLIB_COMPILE) $(LOGGER_COMPILE) $(FASTPROD_OBJECTS)
	$(LINKPP) -o producer_fast $(FASTPROD_OBJECTS)  $(CONFIGLIB_LNK) $(LOGGER_LNK) $(IOLIB_LNK) \
				 -L$(ADOPT_SOURCE)/contrib/cfitsio -lcfitsio -lpthread -lrt -lm -lstdc++

install: all
	install -d $(ADOPT_ROOT)/bin
	install fastdiagn $(ADOPT_ROOT)/bin
	install producer_fast $(ADOPT_ROOT)/bin

# CLEAN
clean:
	rm -f fastdiagn producer_fast
	rm -f *.o *.d *.so core core.* *~ gmon.out

#DOCS
docs: FastDiagnostic.pdf

FastDiagnostic.pdf: FastDiagnostic.cpp FastDiagnostic.h FastDiagnostic.dpu
	dpu -pdf FastDiagnostic

include $(ADOPT_SOURCE)/addeps.gen

