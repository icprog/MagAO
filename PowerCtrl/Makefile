# General Supervisor makefile
include $(ADOPT_SOURCE)/makefile.gen

# programs to be made
TARGETS = PowerCtrl RelayCtrl PicCtrl RelayCtrlW2 PicCtrlW2 MoxaCtrl

all: $(TARGETS)

# Dependencies
include $(ADOPT_LIB)/makefile.aoapp
include $(ADOPT_LIB)/BcuLib/makefile.bculib
include $(ADOPT_LIB)/hwlib/makefile.hwlib

OBJECTS = PowerCtrl.o AbstractPowerItem.o Bcu39Item.o RelayCtrl.o PowerBoardItem.o AbstractRelayCtrl.o RelayCtrlW2.o PicCtrl.o PicCtrlW2.o 

PowerCtrl: PowerCtrl.o AbstractPowerItem.o Bcu39Item.o $(AOAPP_COMPILE) $(BCULIB_COMPILE) $(BCUCOMMAND_COMPILE) $(HWLIB_COMPILE)
	$(LINK) -o PowerCtrl PowerCtrl.o AbstractPowerItem.o Bcu39Item.o $(AOAPP_LNK) $(BCULIB_LNK) $(BCUCOMMAND_LNK) $(HWLIB_LNK) $(ADOPT_LIB)/Paths.o

RelayCtrl: RelayCtrl.o AbstractPowerItem.o PowerBoardItem.o AbstractRelayCtrl.o $(AOAPP_COMPILE) $(BCULIB_COMPILE) $(BCUCOMMAND_COMPILE) $(HWLIB_COMPILE)
	$(LINK) -o RelayCtrl RelayCtrl.o AbstractPowerItem.o PowerBoardItem.o AbstractRelayCtrl.o $(AOAPP_LNK) $(BCULIB_LNK) $(BCUCOMMAND_LNK) $(HWLIB_LNK)

MoxaCtrl: MoxaCtrl.o AbstractPowerItem.o MoxaItem.o  $(AOAPP_COMPILE) $(HWLIB_COMPILE)
	$(LINK) -o MoxaCtrl MoxaCtrl.o AbstractPowerItem.o MoxaItem.o $(AOAPP_LNK) $(HWLIB_LNK) -lmxio

RelayCtrlW2: RelayCtrlW2.o AbstractPowerItem.o PowerBoardItem.o AbstractRelayCtrl.o $(AOAPP_COMPILE) $(BCULIB_COMPILE) $(BCUCOMMAND_COMPILE) $(HWLIB_COMPILE)
	$(LINK) -o RelayCtrlW2 RelayCtrlW2.o AbstractPowerItem.o PowerBoardItem.o AbstractRelayCtrl.o $(AOAPP_LNK) $(BCULIB_LNK) $(BCUCOMMAND_LNK) $(HWLIB_LNK)

PicCtrl: PicCtrl.o AbstractPowerItem.o PowerBoardItem.o AbstractRelayCtrl.o $(AOAPP_COMPILE) $(BCULIB_COMPILE) $(BCUCOMMAND_COMPILE) $(HWLIB_COMPILE)
	$(LINK) -o PicCtrl PicCtrl.o AbstractPowerItem.o PowerBoardItem.o AbstractRelayCtrl.o $(AOAPP_LNK) $(BCULIB_LNK) $(BCUCOMMAND_LNK) $(HWLIB_LNK)

PicCtrlW2: PicCtrlW2.o AbstractPowerItem.o PowerBoardItem.o AbstractRelayCtrl.o $(AOAPP_COMPILE) $(BCULIB_COMPILE) $(BCUCOMMAND_COMPILE) $(HWLIB_COMPILE)
	$(LINK) -o PicCtrlW2 PicCtrlW2.o AbstractPowerItem.o PowerBoardItem.o AbstractRelayCtrl.o $(AOAPP_LNK) $(BCULIB_LNK) $(BCUCOMMAND_LNK) $(HWLIB_LNK) -lm

clean:
	rm -f *.o *.d core* *~ gmon.out $(TARGETS)

install:
	install -d $(ADOPT_ROOT)/bin
	install PowerCtrl $(ADOPT_ROOT)/bin
	install RelayCtrl $(ADOPT_ROOT)/bin
	install RelayCtrlW2 $(ADOPT_ROOT)/bin
	install PicCtrl $(ADOPT_ROOT)/bin
	install PicCtrlW2 $(ADOPT_ROOT)/bin
	install MoxaCtrl $(ADOPT_ROOT)/bin

tags:
	ctags -t *.[ch]

include $(ADOPT_SOURCE)/addeps.gen

