# General Supervisor makefile
include $(ADOPT_SOURCE)/makefile.gen

TARGET = VectorFromConfig testMastDiagnIntf #FramesHistoryTest AdamTest InterfInterfaceTest

all:  $(TARGET)

include $(ADOPT_LIB)/makefile.logger
include $(ADOPT_LIB)/makefile.mastdiagnlib
include $(ADOPT_LIB)/makefile.adamlib
include $(ADOPT_LIB)/IntfLib/makefile.intflib

CPPFLAGS = -Wall -W -Wreturn-type -Wunused $(INCLUDE)
OPTIMIZE = -g
#OPTIMIZE = -O2
CC = gcc

MORE_INCLUDE = -I/usr/include/qt3 -I/usr/include/kde

.cpp.o:
	$(CPP) $(OPTIMIZE) $(MORE_INCLUDE) $(CPPFLAGS) -c $<

%.o:%.h

VectorFromConfig: VectorFromConfig.o
	$(LINKPP) -o VectorFromConfig $+ $(CONFIGLIB_LNK) 

FramesHistoryTest: FramesHistoryTest.o
	$(LINKPP) -o FramesHistoryTest $+ $(INTFLIB_LNK) 

InterfInterfaceTest: InterfInterfaceTest.o
	$(LINKPP) -o InterfInterfaceTest $+ $(INTFLIB_LNK) 

InterferometerParams: InterferometerParams.o
	$(LINKPP) -o InterferometerParams $+ 

testMastDiagnIntf: testMastDiagnIntf.o
	$(LINKPP) -o testMastDiagnIntf $+ $(MASTDIAGNLIB_LNK) 

AdamTest: AdamTest.o
	$(LINKPP) -o AdamTest $+ $(ADAMLIB_LNK) 

P672: P672.o
	qmake -o QMakefile.P672 P672.pro
	make -f QMakefile.P672

sdl: sdl.c
	gcc -o sdl sdl.c -lSDL -lpthread

RTDBVarTest:  RTDBVarTest.o $(ADOPT_LIB)/RTDBvar.o
	$(LINKPP) -o RTDBVarTest $+ $(LOGGER_LNK) 

clean:
	rm -f $(TARGET) *.d core core.* *.o

include ../../addeps.gen

