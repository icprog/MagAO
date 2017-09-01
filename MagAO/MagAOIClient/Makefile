# Supervisor general makefile
include $(ADOPT_SOURCE)/makefile.gen

all: MagAOIClient

# Dependencies
include $(ADOPT_LIB)/makefile.aoapp
include $(ADOPT_LIB)/hwlib/makefile.hwlib

# Dependencies for wraplib (for FrameGrabberCtrl)
include $(ADOPT_LIB)/makefile.iolib
include $(ADOPT_LIB)/BcuLib/makefile.bculib
include $(ADOPT_LIB)/arblib/base/makefile.basearb
include $(ADOPT_LIB)/arblib/aoArb/makefile.aoarb
#include $(ADOPT_SOURCE)/Arbitrator/framework/makefile.arbframework

OPTIMIZE = -g
     
FITS_LIBS = -L$(ADOPT_SOURCE)/contrib/cfitsio -lcfitsio -lm
PATHS_LIBS = $(ADOPT_LIB)/configlib.a $(ADOPT_LIB)/Reorder.o

	
#.cpp.o:
#	$(CPP) $(OPTIMIZE) $(CPPFLAGS) -I../BCUCtrl -I../MasterDiagnostic -c $<

#%.o:%.h



MagAOIClient: MagAOIClient.o MagAOIClient_main.o
	$(LINKPP) -o MagAOIClient_test MagAOIClient.o MagAOIClient_main.o $(AOAPP_LNK) $(HWLIB_LNK)

	
clean:
	rm -f MagAOIClient_test
	rm -f *.d *.o *.so core core.* *~ gmon.out
