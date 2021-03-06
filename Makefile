# Makefile for all the Adopt software

include makefile.gen

SUBSYSTEMS =          MsgD-RTDB StagesCtrl WFSCtrl TTCtrl BCUCtrl idl DiagnApp \
                      SimpleMotorCtrl GUI GUIs  ChUI Tester scripts MirrorDiagnostic \
                      PowerCtrl idl_control PyModules MasterDiagnostic MirrorCtrl Housekeeper AdamHousekeeper \
                      CameraLensCtrl Arbitrator HexapodCtrl SysCheck idl_wrapper \
                      OptLoopDiagnostic StartUp MagAO MagAO/GUI
					  #InterferometerCtrl 


all: $(SUBSYSTEMS)

#Dependencies between subdirs
$(SUBSYSTEMS): lib
MirrorDiagnostic: DiagnApp

lib $(SUBSYSTEMS):
	$(Q)$(MAKE) $(MAKEOPTS) -C $@

.PHONY:  install lib $(SUBSYSTEMS) contrib

contrib:
	$(Q)$(MAKE) $(MAKEOPTS) -C contrib all

install: install-paths
	for dir in ${SUBSYSTEMS}; do \
	 (cd $$dir; ${MAKE} install) || break; \
	done


install-paths:
	install -d $(ADOPT_ROOT)
	install -d $(ADOPT_ROOT)/bin
	install -d $(ADOPT_ROOT)/idl
	install -d $(ADOPT_ROOT)/idl/dlm
	install -d $(ADOPT_ROOT)/conf
	install -d $(ADOPT_ROOT)/conf/adsec
	install -d $(ADOPT_ROOT)/conf/wfs
	install -d $(ADOPT_ROOT)/calib
	install -d $(ADOPT_ROOT)/calib/adsec
	install -d $(ADOPT_ROOT)/calib/wfs
	install -d $(ADOPT_ROOT)/py

install-conf:
	@echo "!!!WARNING!!! Installing the conf directory will overwrite your current configuration. Continue? [yes/no]"
	@read answer && if [ "$$answer" = "yes" ] ; then install -d $(ADOPT_ROOT) && \
             rsync -avC --delete --exclude conf/adsec/585m/cksum_file --exclude conf/adsec/672b/cksum_file --exclude conf/adsec/672a/cksum_file --exclude 'adsec/current' --exclude 'wfs/current' conf $(ADOPT_ROOT) && echo "configuration installed"; else echo "Nothing done"; fi


install-calib:
	@echo "!!!WARNING!!! Installing the calib directory will overwrite your current calibration. Continue? [yes/no]"
	@read answer && if [ "$$answer" = "yes" ] ; then install -d $(ADOPT_ROOT)/calib && \
             install -d $(ADOPT_ROOT)/calib && \
             rsync -aC calib $(ADOPT_ROOT) && echo "calibration installed"; else echo "Nothing done"; fi

install-contrib:
	(cd contrib; $(MAKE) install)

sensor:
	(cd SensorCheck; $(MAKE))

clean: 
	rm -f *.pyc
	rm -f adopt_environment.sh
	rm -f install_environment.sh
	rm -f *.tgz
	for dir in lib ${SUBSYSTEMS}; do \
	 (cd $$dir; ${MAKE} clean) || break; \
	done

clean-contrib:
	$(Q)$(MAKE) $(MAKEOPTS) -C contrib clean

kit:
	./makekit.sh
