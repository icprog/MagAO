include $(ADOPT_SOURCE)/makefile.gen

TARGETS = housekeeper housekeeperWFS

all: $(TARGETS)

# Dependencies
include $(ADOPT_SOURCE)/DiagnApp/makefile.diagnapp
include $(ADOPT_LIB)/BcuLib/makefile.bculib
include $(ADOPT_LIB)/makefile.adamlib
include $(ADOPT_LIB)/makefile.iolib
include $(ADOPT_LIB)/aoslib/makefile.aoslib

OBJECTS_ADSEC = housekeeper.o HouseKeeper.o HouseKeeper_convert.o 
OBJECTS_WFS = housekeeperWFS.o HouseKeeperWFS.o HouseKeeper_convert.o 

OBJECTS = $(OBJECTS_ADSEC) $(OBJECTS_WFS)
		  
CPPFLAGS += -I$(ADOPT_SOURCE)/DiagnApp

LIBS = -lpthread -lrt -lm -lstdc++
FITS_LIBS   = $(ADOPT_LIB)/iolib.o  -L$(ADOPT_SOURCE)/contrib/cfitsio -lcfitsio 
SERIALIZE_LIBS = -lboost_serialization -lboost_thread


### TARGETS ###

housekeeper: $(DIAGNAPP_COMPILE) $(ADAMLIB_COMPILE) $(MASTDIAGNLIB_COMPILE) $(BCULIB_COMPILE) $(AOSLIB_COMPILE) $(OBJECTS_ADSEC)
	$(LINKPP) -o housekeeper $(OBJECTS_ADSEC) $(DIAGNAPP_LNK) $(ADAMLIB_LNK) $(MASTDIAGNLIB_LNK) $(BCULIB_LNK) $(AOSLIB_LNK) \
				 -L$(ADOPT_SOURCE)/contrib/cfitsio -lcfitsio -lpthread -lrt -lm -lstdc++

housekeeperWFS: $(DIAGNAPP_COMPILE) $(ADAMLIB_COMPILE) $(MASTDIAGNLIB_COMPILE) $(BCULIB_COMPILE) $(AOSLIB_COMPILE) $(OBJECTS_WFS)
	$(LINKPP) -o housekeeperWFS $(OBJECTS_WFS) $(DIAGNAPP_LNK) $(ADAMLIB_LNK) $(MASTDIAGNLIB_LNK) $(BCULIB_LNK) $(AOSLIB_LNK) \
				 -L$(ADOPT_SOURCE)/contrib/cfitsio -lcfitsio -lpthread -lrt -lm -lstdc++

install: $(TARGETS)
	install -d $(ADOPT_ROOT)/bin
	install housekeeper $(ADOPT_ROOT)/bin
	install housekeeperWFS $(ADOPT_ROOT)/bin

clean:
	rm -f housekeeper housekeeperWFS
	rm -f *.o *.d *.so core core.* *~ gmon.out


#DOCS
docs: HouseKeeper.pdf HouseKeeperWFS.pdf

HouseKeeper.pdf: HouseKeeper.cpp HouseKeeper.h HouseKeeper.dpu
	dpu -pdf Housekeeper

HouseKeeperWFS.pdf: HouseKeeperWFS.cpp HouseKeeperWFS.h HouseKeeper.dpu
	dpu -pdf HousekeeperWFS

include $(ADOPT_SOURCE)/addeps.gen

