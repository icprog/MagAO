## General Supervisor makefile
include $(ADOPT_SOURCE)/makefile.gen

TARGETS = acq acquire AdSecControl WfsControl OptLoopDiagnGui HexapodGui intmatDisturbGui intmatAcquireGui intmatAnalyseGui intmatAllGui AdOptControl SupervisorControl psfOptimizationGui acquire_batch_single BoardGui msgtool pupOptimizeGui AdSecConfGui# TelemetryGui AdSecCalibration

all: $(TARGETS) links designer python

# Dependencies
include $(ADOPT_SOURCE)/lib/arblib/base/makefile.basearb
include $(ADOPT_SOURCE)/lib/arblib/aoArb/makefile.aoarb
include $(ADOPT_SOURCE)/lib/arblib/adSecArb/makefile.adsecarb
include $(ADOPT_SOURCE)/lib/arblib/wfsArb/makefile.wfsarb
include $(ADOPT_SOURCE)/lib/aoslib/makefile.aoslib
include $(ADOPT_LIB)/makefile.aoapp


links:
	ln -sf ../scripts/BCUfunctions.py

designer:
	cd QtDesigner; make; cd ..;

designer_clean:
	cd QtDesigner; make clean; cd ..;


# TODO: Note that I compile ALL dependencies here!!!

AdOptControl: designer $(BASEARBINTF_COMPILE) $(AOARBLIB_COMPILE)
	$(Q)qmake -o QMakefile.AdOptControl AdOptControl.pro
	$(Q)$(MAKE) $(MAKEOPTS) -f QMakefile.AdOptControl
	$(Q)rm -f QMakefile.AdOptControl

#AdSecCalibration: designer $(BASEARBINTF_COMPILE) $(AOARBLIB_COMPILE)
#	$(Q)qmake -o QMakefile.AdSecCalibration AdSecCalibration.pro
#	$(Q)$(MAKE) $(MAKEOPTS) -f QMakefile.AdSecCalibration
#	$(Q)rm -f QMakefile.AdSecCalibration

SupervisorControl: designer $(BASEARBINTF_COMPILE) $(AOARBLIB_COMPILE) 
	$(Q)qmake -o QMakefile.SupervisorControl SupervisorControl.pro
	$(Q)$(MAKE) $(MAKEOPTS) -f QMakefile.SupervisorControl
	$(Q)rm -f QMakefile.SupervisorControl

AdSecControl: designer $(BASEARBINTF_COMPILE) $(ADSECARBLIB_COMPILE) 
	$(Q)qmake -o QMakefile.AdSecControl AdSecControl.pro
	$(Q)$(MAKE) $(MAKEOPTS) -f QMakefile.AdSecControl
	$(Q)rm -f QMakefile.AdSecControl

WfsControl: designer $(BASEARBINTF_COMPILE) $(WFSARBLIB_COMPILE) 
	$(Q)qmake -o QMakefile.WfsControl WfsControl.pro
	$(Q)$(MAKE) $(MAKEOPTS) -f QMakefile.WfsControl
	$(Q)rm -f QMakefile.WfsControl

HexapodGui: designer $(AOSLIB_COMPILE)
	$(Q)qmake -o QMakefile.HexapodGui HexapodGui.pro
	$(Q)$(MAKE) $(MAKEOPTS) -f QMakefile.HexapodGui
	$(Q)rm -f QMakefile.HexapodGui

BoardGui: designer $(AOSLIB_COMPILE)
	$(Q)qmake -o QMakefile.BoardGui BoardGui.pro
	$(Q)$(MAKE) $(MAKEOPTS) -f QMakefile.BoardGui
	$(Q)rm -f QMakefile.BoardGui

TelemetryGui: designer $(AOSLIB_COMPILE)
	$(Q)qmake -o QMakefile.TelemetryGui TelemetryGui.pro
	$(Q)$(MAKE) $(MAKEOPTS) -f QMakefile.TelemetryGui
	$(Q)rm -f QMakefile.TelemetryGui

OptLoopDiagnGui: designer
	$(Q)qmake -o QMakefile.OptLoopDiagnGui OptLoopDiagnGui.pro
	$(Q)$(MAKE) $(MAKEOPTS) -f QMakefile.OptLoopDiagnGui
	$(Q)rm -f QMakefile.OptLoopDiagnGui

intmatDisturbGui: designer
	$(Q)qmake -o QMakefile.intmatDisturbGui intmatDisturbGui.pro
	$(Q)$(MAKE) $(MAKEOPTS) -f QMakefile.intmatDisturbGui
	$(Q)rm -f QMakefile.intmatDisturbGui

intmatAcquireGui: designer
	$(Q)qmake -o QMakefile.intmatAcquireGui intmatAcquireGui.pro
	$(Q)$(MAKE) $(MAKEOPTS) -f QMakefile.intmatAcquireGui
	$(Q)rm -f QMakefile.intmatAcquireGui

psfOptimizationGui: designer
	$(Q)qmake -o QMakefile.psfOptimizationGui psfOptimizationGui.pro
	$(Q)$(MAKE) $(MAKEOPTS) -f QMakefile.psfOptimizationGui
	$(Q)rm -f QMakefile.psfOptimizationGui

acquire: designer
	$(Q)qmake -o QMakefile.acquire acquire.pro
	$(Q)$(MAKE) $(MAKEOPTS) -f QMakefile.acquire
	$(Q)rm -f QMakefile.acquire

acq: designer
	$(Q)qmake -o QMakefile.acq acq.pro
	$(Q)$(MAKE) $(MAKEOPTS) -f QMakefile.acq
	$(Q)rm -f QMakefile.acq

acquire_batch.o: acquire_batch.cpp acquire_batch.h
	$(CC) -g -c acquire_batch.cpp $(ADOPT_INCLUDE) 

acquire_batch_single: acquire_batch.o
	 $(LINKPP) -g -o acquire_batch_single acquire_batch.o  $(ADOPT_LIB)/intmatlib.o $(AOAPP_LNK) -L$(ADOPT_SOURCE)/contrib/cfitsio $(ADOPT_LIB)/arblib/base/basearb_intf.a $(ADOPT_LIB)/arblib/adSecArb/adsecarb_lib.a  $(ADOPT_LIB)/arblib/wfsArb/wfsarb_lib.a $(ADOPT_LIB)/arblib/base/basearb_lib.a $(ADOPT_LIB)/iolib.o $(ADOPT_LIB)/bcu_diag.o -lcfitsio $(ADOPT_LIB)/Reorder.o $(ADOPT_LIB)/diagBuffer.o

intmatAnalyseGui: designer
	$(Q)qmake -o QMakefile.intmatAnalyseGui intmatAnalyseGui.pro
	$(Q)$(MAKE) $(MAKEOPTS) -f QMakefile.intmatAnalyseGui
	$(Q)rm -f QMakefile.intmatAnalyseGui

intmatAllGui: designer
	$(Q)qmake -o QMakefile.intmatAllGui intmatAllGui.pro
	$(Q)$(MAKE) $(MAKEOPTS) -f QMakefile.intmatAllGui
	$(Q)rm -f QMakefile.intmatAllGui

msgtool: designer
	$(Q)qmake -o QMakefile.msgtool msgtool.pro
	$(Q)$(MAKE) $(MAKEOPTS) -f QMakefile.msgtool
	$(Q)rm -f QMakefile.msgtool

pupOptimizeGui: designer
	$(Q)qmake -o QMakefile.pupOptimizeGui pupOptimizeGui.pro
	$(Q)$(MAKE) $(MAKEOPTS) -f QMakefile.pupOptimizeGui
	$(Q)rm -f QMakefile.pupOptimizeGui

AdSecConfGui: designer
	$(Q)qmake -o QMakefile.AdSecConfGui AdSecConfGui.pro
	$(Q)$(MAKE) $(MAKEOPTS) -f QMakefile.AdSecConfGui
	$(Q)rm -f QMakefile.AdSecConfGui

clean: designer_clean
	rm -f moc_AdSecControl_gui.cpp moc_WfsControl_gui.cpp
	rm -f QMakefile.*
	rm -f *.o *.so *.d core* *~ gmon.out 
	rm -f $(TARGETS)
	rm -rf build dist

python:
	python setup.py build

install: $(TARGETS)
	install -d $(ADOPT_ROOT)/bin
	install vartool_AO.py $(ADOPT_ROOT)/bin
	install sys_processes.py $(ADOPT_ROOT)/bin
	install ccd_viewer.py $(ADOPT_ROOT)/bin
	install ccd_acq.py $(ADOPT_ROOT)/bin
	install startAO.py $(ADOPT_ROOT)/bin
	install wfshw.py $(ADOPT_ROOT)/bin
	install adsechw.py $(ADOPT_ROOT)/bin
#install wfshw2.py $(ADOPT_ROOT)/bin
	install engGui.py $(ADOPT_ROOT)/bin
	install -d $(ADOPT_ROOT)/images
	cp -a images/*.png $(ADOPT_ROOT)/images
	export PYTHONPATH=$(ADOPT_ROOT)/lib/python
	python setup.py install --home=$(ADOPT_ROOT)
	install AdOptControl $(ADOPT_ROOT)/bin
#install AdSecCalibration $(ADOPT_ROOT)/bin
	install SupervisorControl $(ADOPT_ROOT)/bin
	install AdSecControl $(ADOPT_ROOT)/bin
	install WfsControl $(ADOPT_ROOT)/bin
	install HexapodGui $(ADOPT_ROOT)/bin
	install acquire_batch_single $(ADOPT_ROOT)/bin
	install acq $(ADOPT_ROOT)/bin
	install OptLoopDiagnGui $(ADOPT_ROOT)/bin
	install intmatAcquireGui $(ADOPT_ROOT)/bin
	install intmatAnalyseGui $(ADOPT_ROOT)/bin
	install intmatDisturbGui $(ADOPT_ROOT)/bin
	install pupOptimizeGui $(ADOPT_ROOT)/bin
	install psfOptimizationGui $(ADOPT_ROOT)/bin
	install AutoFocusIrtcGui.py $(ADOPT_ROOT)/bin
	install AutoFocusPiscesGui.py $(ADOPT_ROOT)/bin
	install BoardGui $(ADOPT_ROOT)/bin
	install AdSecConfGui $(ADOPT_ROOT)/bin
	install PiscesGui.py $(ADOPT_ROOT)/bin
	install fastlinktest.py $(ADOPT_ROOT)/bin
	ln -sf $(ADOPT_ROOT)/bin/startAO.py $(ADOPT_ROOT)/bin/wfseng
	ln -sf $(ADOPT_ROOT)/bin/startAO.py $(ADOPT_ROOT)/bin/adsceng

tags:
	ctags -t *.[ch]

include $(ADOPT_SOURCE)/addeps.gen

