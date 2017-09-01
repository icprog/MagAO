include $(ADOPT_SOURCE)/makefile.gen

# Here to be sure is the first target (force 'make' to do 'make all')
all: OptLoopDiagnostic 

# Dependencies
include $(ADOPT_LIB)/BcuLib/makefile.bculib
include $(ADOPT_LIB)/makefile.logger
include $(ADOPT_LIB)/makefile.iolib
include $(ADOPT_LIB)/makefile.configlib
include $(ADOPT_LIB)/makefile.aoapp
include $(ADOPT_LIB)/arblib/base/makefile.basearb
include $(ADOPT_LIB)/arblib/wfsArb/makefile.wfsarb
include $(ADOPT_LIB)/arblib/adSecArb/makefile.adsecarb


OPTLOOPDIAGN_OBJECTS = OptLoopDiagnostic.o
OPTLOOPDIAGN_NUVOLA_OBJECTS = OptLoopDiagnostic_nuvola.o 

OBJECTS = $(OPTLOOPDIAGN_OBJECTS) $(OPTLOOPDIAGN_NUVOLA_OBJECTS)

OPTLOOPDIAGN_LIBS = $(ADOPT_LIB)/AdsecDiagnLib.o $(ADOPT_LIB)/iolib.o $(ADOPT_LIB)/bcu_diag.o $(ADOPT_LIB)/diagBuffer.o $(ADOPT_LIB)/Reorder.o

FITS_LIBS = -L$(ADOPT_SOURCE)/contrib/cfitsio -lcfitsio -lm


OPTIMIZE= -ggdb3 -O3 -march=pentium4


### TARGETS ###


OptLoopDiagnostic: $(BCULIB_COMPILE) $(OPTLOOPDIAGN_OBJECTS)
	$(LINKPP) -o OptLoopDiagnostic $(OPTLOOPDIAGN_OBJECTS) $(BCULIB_LNK) $(AOAPP_LNK) $(OPTLOOPDIAGN_LIBS)  $(FITS_LIBS) $(BASEARBINTF_LNK) $(WFSARBLIB_LNK) $(ADSECARBLIB_LNK)
	

OptLoopDiagnostic_nuvola: $(BCULIB_COMPILE) $(OPTLOOPDIAGN_NUVOLA_OBJECTS)
	$(LINKPP) -o OptLoopDiagnostic_nuvola $(OPTLOOPDIAGN_NUVOLA_OBJECTS) $(BCULIB_LNK) $(AOAPP_LNK) $(OPTLOOPDIAGN_LIBS)  $(FITS_LIBS) $(BASEARBINTF_LNK) $(WFSARBLIB_LNK) $(ADSECARBLIB_LNK)
	

install: all
	install -d $(ADOPT_ROOT)/bin
	install OptLoopDiagnostic $(ADOPT_ROOT)/bin

# CLEAN
clean:
	rm -f OptLoopDiagnostic 
	rm -f *.o *.d *.so core core.* *~ gmon.out

#DOCS
docs: OptLoopDiagnostic.pdf

OptLoopDiagnostic.pdf: OptLoopDiagnostic.cpp OptLoopDiagnostic.h OptLoopDiagnostic.dpu
	dpu -pdf OptLoopDiagnostic

include $(ADOPT_SOURCE)/addeps.gen

