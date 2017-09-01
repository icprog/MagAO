# General Supervisor makefile
include $(ADOPT_SOURCE)/makefile.gen

TARGETS = InterferometerCtrl

all: $(TARGETS)

# Dependencies
include $(ADOPT_SOURCE)/lib/makefile.aoapp

OBJECTS  = InterferometerCtrlMain.o InterferometerCtrl.o

CPPFLAGS += -I$(ADOPT_SOURCE)/InterferometerCtrl/AbstractBase -I$(ADOPT_SOURCE)/InterferometerCtrl/Impl4D

# Needed for parapin library!
OPTIMIZE = -ggdb3

########### ABSTRACT CONTROLLER ########## 
		   	  
InterferometerCtrl: $(OBJECTS) $(AOAPP_COMPILE)
	$(Q)$(MAKE) $(MAKEOPTS) -C AbstractBase all
	$(Q)$(MAKE) $(MAKEOPTS) -C Impl4D all
	$(LINKPP) -o InterferometerCtrl InterferometerCtrlMain.o InterferometerCtrl.o Impl4D/impl4d.a AbstractBase/abstractbase.a $(AOAPP_LNK) $(PY_LIB_LNK) -L../contrib/parapin-1.5.1-beta1 -lparapin
	
install: $(TARGETS)
	install -d $(ADOPT_ROOT)/bin
	install InterferometerCtrl $(ADOPT_ROOT)/bin
	
clean:
	cd AbstractBase; make clean; cd ..;
	cd Impl4D; make clean; cd ..;
	rm -f *.o *.d *.so core core.* *~ gmon.out
	rm -f InterferometerCtrl
		  
include $(ADOPT_SOURCE)/addeps.gen

	
