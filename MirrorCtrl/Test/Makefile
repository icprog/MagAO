# General Supervisor makefile
include $(ADOPT_SOURCE)/makefile.gen

# Dependencies
include $(ADOPT_LIB)/makefile.aoapp
include $(ADOPT_LIB)/makefile.adamlib
include $(ADOPT_LIB)/BcuLib/makefile.bculib            
             
              
TESTED_OBJECTS= ../Commons/AbstractBcuRequestManager.o \
                ../Commons/TimeToDie.o \
                ../Commons/BcuPacket.o \
                ../Commons/BcuPacketQueue.o \
                ../Commons/BcuRequest.o \
                ../Commons/BcuRequestInfo.o \
                ../RequestManagers/BcuCommandHandler.o \
                ../RequestManagers/DiagnosticManager.o \
                ../BcuCommunicator/BcuCommunicator.o \
                ../BcuCommunicator/Scheduler.o \
                ../BcuCommunicator/RoundQueue.o


CPPFLAGS += -I$(ADOPT_SOURCE)/MirrorCtrl

# Because of modifications on tested classes, some of this classe don't compile
TARGETS = DestructorTest
		  # DiagnosticManagerTest \
		  # TestBufInfo \
		  # MasterDiagnostic \
		  # BcuCommunicatorTest_rq BcuSimulator MirrorCtrlTest
		  # BcuCommunicatorTest
		  # UdpConnectionTest BcuRequestInfoTest TaskPriorityTest RoundQueueTest

all: $(TARGETS)


### TEST TARGETS ###

DestructorTest: DestructorTest.o $(AOAPP_COMPILE) $(ADAMLIB_COMPILE) $(BCULIB_COMPILE)
	$(LINKPP) -o $@ DestructorTest.o $(AOAPP_LNK) $(ADAMLIB_LNK) $(BCULIB_LNK)

TestBufInfo: TestBufInfo.o $(AOAPP_COMPILE) $(ADAMLIB_COMPILE) $(BCULIB_COMPILE)
	$(LINKPP) -o $@ TestBufInfo.o $(AOAPP_LNK) $(ADAMLIB_LNK) $(BCULIB_LNK)

MasterDiagnostic: MasterDiagnostic.o $(AOAPP_COMPILE) $(ADAMLIB_COMPILE) $(BCULIB_COMPILE)
	$(LINKPP) -o $@ MasterDiagnostic.o $(AOAPP_LNK) $(ADAMLIB_LNK) $(BCULIB_LNK)

DiagnosticManagerTest: DiagnosticManagerTest.o $(AOAPP_COMPILE) $(ADAMLIB_COMPILE) $(BCULIB_COMPILE) $(TESTED_OBJECTS)
	$(LINKPP) -o $@ DiagnosticManagerTest.o $(TESTED_OBJECTS) $(AOAPP_LNK) $(ADAMLIB_LNK) $(BCULIB_LNK)

MirrorCtrlTest: MirrorCtrlTest_Main.o MirrorCtrlTest.o BcuCommandHandlerTest.o $(AOAPP_COMPILE) $(ADAMLIB_COMPILE) $(BCULIB_COMPILE) $(TESTED_OBJECTS)
	$(LINKPP) -o $@ MirrorCtrlTest_Main.o MirrorCtrlTest.o $(TESTED_OBJECTS) $(AOAPP_LNK) $(ADAMLIB_LNK) $(BCULIB_LNK)

BcuCommunicatorTest: BcuCommunicatorTest.o $(AOAPP_COMPILE) $(ADAMLIB_COMPILE) $(BCULIB_COMPILE) $(TESTED_OBJECTS) 
	$(LINKPP) -o $@ BcuCommunicatorTest.o $(TESTED_OBJECTS) $(AOAPP_LNK)  $(ADAMLIB_LNK) $(BCULIB_LNK)
	
BcuCommunicatorTest_rq: BcuCommunicatorTest_rq.o $(AOAPP_COMPILE) $(ADAMLIB_COMPILE) $(BCULIB_COMPILE) $(TESTED_OBJECTS)
	$(LINKPP) -o $@ BcuCommunicatorTest_rq.o $(TESTED_OBJECTS) $(AOAPP_LNK) $(ADAMLIB_LNK) $(BCULIB_LNK)
	
BcuRequestInfoTest: BcuRequestInfoTest.o $(AOAPP_COMPILE) $(ADAMLIB_COMPILE) $(BCULIB_COMPILE) $(TESTED_OBJECTS)
	$(LINKPP) -o $@ BcuRequestInfoTest.o $(TESTED_OBJECTS) $(AOAPP_LNK) $(ADAMLIB_LNK) $(BCULIB_LNK)
	
BcuSimulator: BcuSimulator.o $(AOAPP_COMPILE) $(ADAMLIB_COMPILE) $(BCULIB_COMPILE) $(TESTED_OBJECTS)
	$(LINKPP) -o $@ $(TESTED_OBJECTS) $(AOAPP_LNK) $(ADAMLIB_LNK) $(BCULIB_LNK)
	
RoundQueueTest: RoundQueueTest.o $(AOAPP_COMPILE) $(ADAMLIB_COMPILE) $(BCULIB_COMPILE) $(TESTED_OBJECTS)
	$(LINKPP) -o $@ BcuSimulator.o RoundQueueTest.o $(TESTED_OBJECTS) $(AOAPP_LNK) $(ADAMLIB_LNK) $(BCULIB_LNK)
	
TaskPriorityTest: TaskPriorityTest.o $(AOAPP_COMPILE) $(ADAMLIB_COMPILE) $(BCULIB_COMPILE)
	$(LINKPP) -o $@ TaskPriorityTest.o $(AOAPP_LNK) $(ADAMLIB_LNK) $(BCULIB_LNK)

UdpConnectionTest: UdpConnectionTest.o $(AOAPP_COMPILE) $(ADAMLIB_COMPILE) $(BCULIB_COMPILE) $(TESTED_OBJECTS)
	$(LINKPP) -o $@  UdpConnectionTest.o $(TESTED_OBJECTS) $(AOAPP_LNK) $(ADAMLIB_LNK) $(BCULIB_LNK)
	
clean:
	rm -f *.o *.d *.so core core.* *~ gmon.out
	rm -f $(TARGETS) *.o

include $(ADOPT_SOURCE)/addeps.gen

	
