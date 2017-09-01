include $(ADOPT_SOURCE)/makefile.gen

# programs to be made
TARGETS = StagesCtrl CopleyCtrl CopleyCtrlDeadX

all: $(TARGETS)

# Dependencies
include $(ADOPT_LIB)/makefile.aoapp
include $(ADOPT_LIB)/hwlib/makefile.hwlib


OBJECTS = StagesCtrl.o CopleyCtrl.o CopleyCtrlDeadX.o

StagesCtrl: StagesCtrl.o $(AOAPP_COMPILE) $(HWLIB_COMPILE)
	$(LINKPP) -o StagesCtrl StagesCtrl.o $(AOAPP_LNK) $(HWLIB_LNK)

CopleyCtrl: CopleyCtrl.o $(AOAPP_COMPILE) $(HWLIB_COMPILE)
	$(LINKPP) -o CopleyCtrl CopleyCtrl.o $(AOAPP_LNK) $(HWLIB_LNK)

CopleyCtrlDeadX: CopleyCtrlDeadX.o $(AOAPP_COMPILE) $(HWLIB_COMPILE)
	$(LINKPP) -o CopleyCtrlDeadX CopleyCtrlDeadX.o $(AOAPP_LNK) $(HWLIB_LNK)

install: all
	install -d $(ADOPT_ROOT)/bin
	install CopleyCtrl $(ADOPT_ROOT)/bin
	install CopleyCtrlDeadX $(ADOPT_ROOT)/bin

clean:
	rm -f *.o *.d core core.* *~ gmon.out $(TARGETS)
tags:
	ctags -t *.[ch]

include $(ADOPT_SOURCE)/addeps.gen

