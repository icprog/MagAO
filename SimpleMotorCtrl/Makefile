# Supervisor general makefile
include $(ADOPT_SOURCE)/makefile.gen

all: SimpleMotorCtrl SimpleMotorCtrl_NoMain

# Dependencies
include $(ADOPT_LIB)/makefile.aoapp
include $(ADOPT_LIB)/hwlib/makefile.hwlib


#10 Dec 2010: VISAO_NO_MAIN flag added to turn off compilation of main()
#             if VISAO_NO_MAIN = -D__VISAO_NO_MAIN
#            by Jared R. Males, Steward Observatory
#.cpp.o:
#	$(CPP) $(VISAO_NO_MAIN) $(OPTIMIZE) $(CPPFLAGS) -c $<

CPPFLAGS += $(VISAO_NO_MAIN)


OBJECTS = SimpleMotorCtrl.o SimpleMotor.o ADCWheel.o Rerotator.o FilterWheel.o MCBL2805.o Mercury.o 


SimpleMotorCtrl: $(OBJECTS) $(AOAPP_COMPILE) $(HWLIB_COMPILE)
	$(LINKPP) -o SimpleMotorCtrl $(OBJECTS) $(AOAPP_LNK) $(HWLIB_LNK)

SimpleMotorCtrl_NoMain: $(OBJECTS) $(AOAPP_COMPILE) $(HWLIB_COMPILE)
	$(CPP) $(OPTIMIZE) $(CPPFLAGS) -D__VISAO_NO_MAIN -o SimpleMotorCtrl_NoMain.o -c SimpleMotorCtrl.cpp
	
install: all
	install -d $(ADOPT_ROOT)/bin
	install SimpleMotorCtrl $(ADOPT_ROOT)/bin

clean:
	rm -f SimpleMotorCtrl
	rm -f *.o *.d *.so core core.* *~ gmon.out
tags:
	ctags -t *.[ch]

include $(ADOPT_SOURCE)/addeps.gen

