# Supervisor general makefile
include $(ADOPT_SOURCE)/makefile.gen

all: VisAOSimpleMotorCtrl VisAOPwrCtrl MagAOI ActiveOpt mmti MagFrameGrabberCtrl CROController windmon gainSetter

# Dependencies
include $(ADOPT_LIB)/makefile.aoapp
include $(ADOPT_LIB)/hwlib/makefile.hwlib
include $(ADOPT_LIB)/makefile.iolib
# Dependencies for wraplib (for FrameGrabberCtrl)
include $(ADOPT_LIB)/makefile.iolib
include $(ADOPT_LIB)/BcuLib/makefile.bculib
include $(ADOPT_LIB)/arblib/base/makefile.basearb
include $(ADOPT_LIB)/arblib/aoArb/makefile.aoarb
#include $(ADOPT_SOURCE)/Arbitrator/framework/makefile.arbframework

OPTIMIZE = -g
     
FITS_LIBS = -L$(ADOPT_SOURCE)/contrib/cfitsio -lcfitsio -lm
PATHS_LIBS = $(ADOPT_LIB)/configlib.a $(ADOPT_LIB)/Reorder.o

SimpleMotorOBJs = ../SimpleMotorCtrl/SimpleMotorCtrl_NoMain.o ../SimpleMotorCtrl/ADCWheel.o ../SimpleMotorCtrl/FilterWheel.o ../SimpleMotorCtrl/MCBL2805.o ../SimpleMotorCtrl/Mercury.o ../SimpleMotorCtrl/Rerotator.o ../SimpleMotorCtrl/SimpleMotor.o
#.cpp.o:
#	$(CPP) $(OPTIMIZE) $(CPPFLAGS) -I../BCUCtrl -I../MasterDiagnostic -c $<

#%.o:%.h

VisAOSimpleMotorCtrl: VisAOFilterWheel.o VisAOSimpleMotorCtrl.o VisAOSimpleMotorCtrl_main.o
	$(LINKPP) -o VisAOSimpleMotorCtrl VisAOFilterWheel.o VisAOSimpleMotorCtrl.o VisAOSimpleMotorCtrl_main.o $(SimpleMotorOBJs) $(AOAPP_LNK) $(HWLIB_LNK)

VisAOPwrCtrl: VisAOPwrCtrl.o VisAOPwrCtrl_main.o 
	$(LINKPP) -o VisAOPwrCtrl VisAOPwrCtrl.o VisAOPwrCtrl_main.o $(AOAPP_LNK) $(HWLIB_LNK)

MagAOI: MagAOI.o MagAOI_main.o
	$(LINKPP) -o MagAOI MagAOI.o MagAOI_main.o $(AOAPP_LNK) $(HWLIB_LNK) $(BASEARBINTF_LNK) $(AOARBLIB_LNK)

ActiveOpt: ActiveOpt.o ActiveOpt_main.o
	$(LINKPP) -o ActiveOpt ActiveOpt.o ActiveOpt_main.o $(AOAPP_LNK) $(HWLIB_LNK) $(BASEARBINTF_LNK) $(AOARBLIB_LNK)

MagAOIClient: MagAOIClient/MagAOIClient.o MagAOIClient/MagAOIClient_main.o
	$(LINKPP) -o MagAOIClient_test MagAOIClient.o MagAOIClient_main.o $(AOAPP_LNK) $(HWLIB_LNK)

mmti: MagAOIClient mmti.o mmti_main.o
	$(LINKPP) -o mmti mmti.o mmti_main.o MagAOIClient/MagAOIClient.o $(AOAPP_LNK) $(HWLIB_LNK)

#AOIGUI: AOIGUI/aoigui.ui.h AOIGUI/aoigui_command_dialog.ui.h 
#	cd AOIGUI; ${MAKE}

windmon: windmon.o windmon_main.o
	$(LINKPP) -o windmon windmon.o windmon_main.o $(AOAPP_LNK) $(HWLIB_LNK)

windmon2: windmon2.o windmon2_main.o
	$(LINKPP) -o windmon2 windmon2.o windmon2_main.o $(AOAPP_LNK) $(HWLIB_LNK)

#$(BASEARBINTF_LNK) $(AOARBLIB_LNK)

MagFrameGrabberCtrl: MagFrameGrabberCtrl.o MagFrameGrabberCtrl_main.o $(LIBS_COMPILE)
	$(LINKPP) -o MagFrameGrabberCtrl MagFrameGrabberCtrl.o MagFrameGrabberCtrl_main.o $(AOAPP_LNK) $(IOLIB_LNK) $(FITS_LIBS) 

CROController: CROController.o CROController_main.o CROlib/dcn.o
	$(LINKPP) -o CROController CROController.o CROController_main.o dcn.o $(AOAPP_LNK) $(HWLIB_LNK)

TestFramesDownloader: TestFramesDownloader.o
	$(LINKPP) -o TestFramesDownloader TestFramesDownloader.o $(AOAPP_LNK) $(IOLIB_LNK) $(FITS_LIBS) ../MasterDiagnostic/AbstractFramesDownloader.o ../MasterDiagnostic/TechnicalViewer.o ../lib/logger.a ../lib/netlib.a ../lib/base/thrdlib.a

gainSetter: gainSetter.o gainSetter_main.o $(BASEARBINTF_COMPILE) $(ADSECARBLIB_COMPILE)
	$(LINKPP) -o gainSetter gainSetter.o gainSetter_main.o $(AOAPP_LNK) $(ADOPT_SOURCE)/lib/iolib.a $(ADOPT_SOURCE)/contrib/cfitsio/libcfitsio.a $(ADOPT_SOURCE)/lib/arblib/base/basearb_intf.a $(ADOPT_SOURCE)/lib/arblib/base/basearb_lib.a $(ADOPT_SOURCE)/lib/arblib/aoArb/aoarb_lib.a $(ADOPT_SOURCE)/lib/arblib/adSecArb/adsecarb_lib.a -lboost_thread

WFRecon: recmat.o WFRecon.o WFRecon_main.o 
	$(LINKPP) -o WFRecon recmat.o WFRecon.o WFRecon_main.o $(AOAPP_LNK) -lcfitsio -lgsl -L/usr/local/atlas/lib -latlas -lcblas -latlas -lgslcblas -lfftw3

install: all
	install -d $(ADOPT_ROOT)/bin
	install VisAOSimpleMotorCtrl $(ADOPT_ROOT)/bin
	install VisAOPwrCtrl $(ADOPT_ROOT)/bin
	install MagFrameGrabberCtrl $(ADOPT_ROOT)/bin
	install MagAOI $(ADOPT_ROOT)/bin
	install windmon $(ADOPT_ROOT)/bin
	install ActiveOpt $(ADOPT_ROOT)/bin
	install mmti $(ADOPT_ROOT)/bin
	install start_visao_process.py $(ADOPT_ROOT)/bin
	install CROController $(ADOPT_ROOT)/bin
	install gainSetter $(ADOPT_ROOT)/bin
	install seedump.sh ~/bin
	#cd AOIGUI; ${MAKE} install

	#ln -sf $(ADOPT_ROOT)/bin/start_visao_process.py $(ADOPT_ROOT)/bin/VisAOSimpleMotorCtrl
	ln -sf $(ADOPT_ROOT)/bin/start_visao_process.py $(ADOPT_ROOT)/bin/VisAOJoeCtrl
	ln -sf $(ADOPT_ROOT)/bin/start_visao_process.py $(ADOPT_ROOT)/bin/FilterWheelLocal
	ln -sf $(ADOPT_ROOT)/bin/start_visao_process.py $(ADOPT_ROOT)/bin/FilterWheelLocal
	ln -sf $(ADOPT_ROOT)/bin/start_visao_process.py $(ADOPT_ROOT)/bin/ShutterRemoteControl
	ln -sf $(ADOPT_ROOT)/bin/start_visao_process.py $(ADOPT_ROOT)/bin/VisAOIClient
	ln -sf $(ADOPT_ROOT)/bin/start_visao_process.py $(ADOPT_ROOT)/bin/PwrMon
	
#	install MagFrameGrabberCtrl $(ADOPT_ROOT)/bin
	
clean:
	rm -f VisAOPwrCtrl
	rm -f MagAOI
	rm -f MagFrameGrabberCtrl
	rm -f *.d *.o *.so core core.* *~ gmon.out
