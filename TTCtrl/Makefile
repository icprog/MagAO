# General Supervisor makefile
include $(ADOPT_SOURCE)/makefile.gen

# Note this is the FIRST target (force 'make' to do 'make all')
all: TTCtrl

# Dependencies
include $(ADOPT_LIB)/makefile.aoapp
include $(ADOPT_LIB)/BcuLib/makefile.bculib


OBJECTS = TTCtrl.o TipTilt.o Waves.o Converter.o CameraLens.o tt_matlab.o

### TARGETS ###

TTCtrl: $(AOAPP_COMPILE) $(BCULIB_COMPILE) $(OBJECTS)
	$(LINKPP) -o $@ $(OBJECTS)  /usr/local/MATLAB/R2011a/sys/os/glnx86/libstdc++.so.6 -L/usr/local/MATLAB/R2011a/bin/glnx86 $(AOAPP_LNK) $(BCULIB_LNK) /usr/local/MATLAB/R2011a/bin/glnx86/libeng.so

install: all
	install -d $(ADOPT_ROOT)/bin
	install TTCtrl $(ADOPT_ROOT)/bin
	
clean:
	rm -f *.o *.d *.so core core.* *~ gmon.out 
	rm -f TTCtrl $(OBJECTS) $(OBJECTS:.o=.o.d)

include $(ADOPT_SOURCE)/addeps.gen

