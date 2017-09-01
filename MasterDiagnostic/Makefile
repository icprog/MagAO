include $(ADOPT_SOURCE)/makefile.gen
             
TARGETS=MasterDiagnostic

all: $(TARGETS)

# Dependencies
include $(ADOPT_LIB)/makefile.aoapp
include $(ADOPT_LIB)/makefile.netlib
include $(ADOPT_LIB)/BcuLib/makefile.bculib
include $(ADOPT_LIB)/arblib/base/makefile.basearb

OBJECTS = FramesBouncer.o MasterDiagnostic.o StatusTable.o FramesFunnel.o \
		  AbstractFramesDownloader.o FramesDownloader.o  \
		  TechnicalViewer.o 

FASTDIAGN_LIBS = $(ADOPT_SOURCE)/lib/AdsecDiagnLib.o

OPTIMIZE = -g


MasterDiagnostic: $(AOAPP_COMPILE) $(NETLIB_COMPILE) $(BCULIB_COMPILE) $(BASEARBINTF_COMPILE) $(OBJECTS)
	$(LINKPP) -o MasterDiagnostic $(OBJECTS) $(AOAPP_LNK) $(NETLIB_LNK) $(BCULIB_LNK) $(FASTDIAGN_LIBS)  $(BASEARBINTF_LNK)

	
install: $(TARGETS)
	install -d $(ADOPT_ROOT)/bin
	install MasterDiagnostic $(ADOPT_ROOT)/bin
	chmod +s $(ADOPT_ROOT)/bin/MasterDiagnostic
	
clean:
	rm -f MasterDiagnostic *.o
	rm -f *.o *.d *.so core core.* *~ gmon.out

include $(ADOPT_SOURCE)/addeps.gen

