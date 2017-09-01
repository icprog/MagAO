# General Supervisor makefile
include $(ADOPT_SOURCE)/makefile.gen

all: MirrorCtrl

# Dependencies
include $(ADOPT_LIB)/makefile.aoapp
include $(ADOPT_LIB)/makefile.adamlib
include $(ADOPT_LIB)/BcuLib/makefile.bculib

CPPFLAGS += -I$(ADOPT_SOURCE)/MirrorCtrl
 
OBJECTS = MirrorCtrl_Main.o  MirrorCtrl.o
             
SUB_OBJECTS = Commons/AbstractBcuRequestManager.o \
          Commons/TimeToDie.o \
          Commons/BcuPacket.o \
          Commons/BcuPacketQueue.o \
          Commons/BcuRequest.o \
          Commons/BcuRequestInfo.o \
          BcuCommunicator/BcuCommunicator.o \
          BcuCommunicator/Scheduler.o \
          BcuCommunicator/RoundQueue.o \
          RequestManagers/BcuCommandHandler.o \
          RequestManagers/DiagnosticManager.o \
          RequestManagers/DiagnosticManagerPool.o \
          RequestManagers/Watchdog.o
	  
SUBMODULES = Commons BcuCommunicator RequestManagers

.PHONY: $(SUBMODULES)

$(SUBMODULES):
	$(Q)$(MAKE) $(MAKEOPTS) -C $@ all

### TARGETS ###

MirrorCtrl: $(AOAPP_COMPILE) $(ADAMLIB_COMPILE) $(BCULIB_COMPILE) $(OBJECTS) $(SUBMODULES)
	$(LINKPP) -o MirrorCtrl $(OBJECTS) $(SUB_OBJECTS) $(AOAPP_LNK)  $(ADAMLIB_LNK) $(BCULIB_LNK)

install: MirrorCtrl
	install -d $(ADOPT_ROOT)/bin
	install MirrorCtrl $(ADOPT_ROOT)/bin

tests: 
	cd Test; ${MAKE} all

clean:
	rm -f *.o *.d *.so core core.* *~ gmon.out
	rm -f MirrorCtrl
	rm -f $(MAIN_OBJECTS)
	for dir in ${SUBMODULES}; do \
	 (cd $$dir; ${MAKE} clean); \
	done
	
cleanTests: 
	cd Test; ${MAKE} clean
	
include $(ADOPT_SOURCE)/addeps.gen
	
