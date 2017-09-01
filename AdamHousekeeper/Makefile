include $(ADOPT_SOURCE)/makefile.gen

TARGETS = adamHousekeeper

all: $(TARGETS)

# Dependencies
include $(ADOPT_SOURCE)/DiagnApp/makefile.diagnapp
include $(ADOPT_LIB)/BcuLib/makefile.bculib
include $(ADOPT_LIB)/makefile.adamlib
include $(ADOPT_LIB)/aoslib/makefile.aoslib

OBJECTS = adamHousekeeper.o AdamHousekeeper.o AdamHousekeeper_convert.o 
	
CPPFLAGS += -I$(ADOPT_SOURCE)/DiagnApp	  

LIBS = -lpthread -lrt -lm -lstdc++
FITS_LIBS   = $(ADOPT_LIB)/iolib.o  -L$(ADOPT_SOURCE)/contrib/cfitsio -lcfitsio 
SERIALIZE_LIBS = -lboost_serialization -lboost_thread

### TARGETS ###

adamHousekeeper: $(DIAGNAPP_COMPILE) $(ADAMLIB_COMPILE) $(MASTDIAGNLIB_COMPILE) $(BCULIB_COMPILE) $(AOSLIB_COMPILE) $(OBJECTS)
	$(LINKPP) -o adamHousekeeper $(OBJECTS) $(DIAGNAPP_LNK) $(ADAMLIB_LNK) $(MASTDIAGNLIB_LNK) $(BCULIB_LNK) $(AOSLIB_LNK) \
				 -L$(ADOPT_SOURCE)/contrib/cfitsio -lcfitsio -lpthread -lrt -lm -lstdc++

install:  $(TARGETS)
	install -d $(ADOPT_ROOT)/bin
	install adamHousekeeper $(ADOPT_ROOT)/bin

clean:
	rm -f adamHousekeeper  
	rm -f *.o *.d core* *~ gmon.out



#DOCS
docs: adamHouseKeeper.pdf 

HouseKeeper.pdf: AdamHousekeeper.cpp AdamHousekeeper.h AdamHousekeeper.dpu
	dpu -pdf Housekeeper

include $(ADOPT_SOURCE)/addeps.gen


