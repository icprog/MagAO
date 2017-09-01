# Supervisor general makefile definitions
include $(ADOPT_SOURCE)/makefile.gen

# programs to be made
TARGETS = SlopeCompCtrl FrameGrabberCtrl SlopeCompTest

all: $(TARGETS)

# Dependencies for wraplib
include $(ADOPT_LIB)/makefile.aoapp
include $(ADOPT_LIB)/makefile.iolib
include $(ADOPT_LIB)/BcuLib/makefile.bculib
include $(ADOPT_LIB)/arblib/base/makefile.basearb


FITS_LIBS = -L$(ADOPT_SOURCE)/contrib/cfitsio -lcfitsio -lm
PATHS_LIBS = $(ADOPT_LIB)/configlib.a $(ADOPT_LIB)/Reorder.o

SlopeComp_memorymap.h: SlopeCalculator_5_02.map.xml
	./map.pl < SlopeCalculator_5_02.map.xml | grep _BSC > SlopeComp_memorymap.h
SlopeComp_defaults.h: SlopeComp_defaults.txt
	./config.pl < SlopeComp_defaults.txt > SlopeComp_defaults.h
	
# Shortcuts
LIBS_COMPILE =$(AOAPP_COMPILE) $(IOLIB_COMPILE) $(BCULIB_COMPILE) $(BCUCOMMAND_COMPILE)
ADOPT_LIBS = $(AOAPP_LNK) $(IOLIB_LNK) $(BCULIB_LNK) $(BCUCOMMAND_LNK) $(PATHS_LIBS)

SlopeComp.o: SlopeComp_memorymap.h SlopeComp_defaults.h


SlopeCompCtrl: SlopeComp.o SlopeCompCtrl.o BCUlbt.o BCUfile.o $(LIBS_COMPILE)
	$(LINKPP) -o SlopeCompCtrl SlopeCompCtrl.o BCUlbt.o BCUfile.o SlopeComp.o $(ADOPT_LIBS) $(FITS_LIBS) $(BASEARBINTF_LNK)

SlopeCompTest: SlopeComp.o SlopeCompTest.o BCUlbt.o BCUfile.o $(LIBS_COMPILE)
	$(LINKPP) -o SlopeCompTest SlopeCompTest.o BCUlbt.o BCUfile.o SlopeComp.o $(ADOPT_LIBS) $(FITS_LIBS) $(BASEARBINTF_LNK)

FrameGrabberCtrl: FrameGrabberCtrl.o BCUlbt.o BCUfile.o $(LIBS_COMPILE)
	$(LINKPP) -o FrameGrabberCtrl FrameGrabberCtrl.o BCUlbt.o BCUfile.o $(ADOPT_LIBS) $(FITS_LIBS)

BCUkicker: BCUkicker.o $(LIBS_COMPILE)
	$(LINKPP) -o BCUkicker BCUkicker.o $(ADOPT_LIBS)

BCUpixels: BCUpixels.o
	$(LINKPP) -o BCUpixels BCUpixels.o $(ADOPT_LIBS)

clean:
	rm -f *.o *.d core core.* *~ gmon.out $(TARGETS) SlopeComp_memorymap.h SlopeComp_defaults.h
tags:
	ctags -t *.[ch]

install: all
	install -d $(ADOPT_ROOT)/bin
	install SlopeCompCtrl $(ADOPT_ROOT)/bin
	install FrameGrabberCtrl $(ADOPT_ROOT)/bin

include $(ADOPT_SOURCE)/addeps.gen

