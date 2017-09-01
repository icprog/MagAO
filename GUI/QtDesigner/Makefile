# General Supervisor makefile
include $(ADOPT_SOURCE)/makefile.gen

PYTHONS = point_and_source.py ho_arm.py tv_arm.py ccd_acq_template.py engineeringGui_template.py ccd_gui.py startup_dialog.py restoreSetup_template.py AutoFocusIrtc_gui.py AutoFocusPisces_gui.py Pisces_gui.py pupilshift.py fastlinktest.py

all: $(PYTHONS) \
	 AdOptControl_gui.h AdSecCalibration_gui.h SupervisorControl_gui.h AdSecControl_gui.h WfsControl_gui.h \
     AdOptControl_gui.cpp AdSecCalibration_gui.cpp SupervisorControl_gui.cpp  AdSecControl_gui.cpp WfsControl_gui.cpp \
     hexapod_gui.cpp hexapod_gui.h intmatDisturb_gui.cpp intmatDisturb_gui.h intmatAcquire_gui.cpp intmatAcquire_gui.h \
     intmatAnalyse_gui.cpp intmatAnalyse_gui.h optloopdiagn_gui.cpp optloopdiagn_gui.h acquire_gui.cpp acquire_gui.h \
     intmatAll_gui.cpp intmatAll_gui.h psfOptimization_gui.cpp psfOptimization_gui.h board_gui.cpp board_gui.h \
     telemetry.cpp telemetry.h pupOptimize_gui.cpp pupOptimize_gui.h AdSecConf_gui.h AdSecConf_gui.cpp

# QT .ui compilation rules
%.h: %.ui
	uic $< -o $@
	
%.cpp: %.ui
	uic -impl $*.h $< -o $@
	
%.py: %.ui
	pyuic $< > $@

	
clean:
	rm -f  *.o *.d *.h *.cpp $(PYTHONS) *.pyc

