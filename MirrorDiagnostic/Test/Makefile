include ../../makefile.gen

TEST_TARGETS = Test_DiagnLib Test_RunningMean Test_DiagnRange Test_DiagnVar Test_DiagnSet Test_DiagnParam Test_DiagnApp Test_Thread

ADOPT_DIAGN = $(ADOPT_SOURCE)/DiagnApp

ADOPT_LIBS  = $(ADOPT_BASELIB)/rtdblib.o \
              $(ADOPT_LIB)/AOApp.o \
              $(ADOPT_LIB)/stdconfig.o \
              $(ADOPT_LIB)/RTDBvar.o \
              $(ADOPT_LIB)/Utils.o \
              $(ADOPT_LIB)/Logger.o \
              $(BASELIB_A)

DIAGN_LIBS = $(ADOPT_DIAGN)/diagnlib.o

CPPFLAGS += -I.. -I$(ADOPT_DIAGN)

LIBS = -lpthread -lrt -lm
SERIAL_LIB  = -lboost_serialization
FITS_LIBS   = $(ADOPT_LIB)/iolib.o  -L$(ADOPT_SOURCE)/contrib/cfitsio -lcfitsio -lm


all: $(TEST_TARGETS)

#Test_DiagnSet.o: Test_DiagnSet.cpp $(ADOPT_DIAGN)/DiagnSet.h 

DIAGN_MODULES = $(ADOPT_DIAGN)/DiagnApp.o \
				$(ADOPT_DIAGN)/DiagnVar.o \
				$(ADOPT_DIAGN)/RunningMean.o \
				$(ADOPT_DIAGN)/DiagnValue.o

FAST_MODULES = $(DIAGN_MODULES) $(ADOPT_DIAGN)/FastDiagnostic.o $(ADOPT_DIAGN)/FastDiagnostic_convert.o 

Test_DiagnLib: Test_DiagnLib.o $(DIAGN_LIBS)
	$(LINKPP) -o Test_DiagnLib Test_DiagnLib.o $(LIBS) $(ADOPT_LIBS) $(DIAGN_MODULES) $(DIAGN_LIBS) $(SERIAL_LIB)

Test_RunningMean: Test_RunningMean.o $(ADOPT_DIAGN)/RunningMean.o 
	$(LINKPP) -o Test_RunningMean Test_RunningMean.o $(ADOPT_DIAGN)/RunningMean.o $(ADOPT_LIBS) $(LIBS) $(SERIAL_LIB) 

Test_DiagnRange: Test_DiagnRange.o $(ADOPT_DIAGN)/DiagnRange.h 
	$(LINKPP) -o Test_DiagnRange Test_DiagnRange.o $(ADOPT_LIBS)  $(LIBS)

Test_DiagnVar: Test_DiagnVar.o $(DIAGN_MODULES)
	$(LINKPP) -o Test_DiagnVar Test_DiagnVar.o $(DIAGN_MODULES) $(ADOPT_LIBS) $(LIBS) $(SERIAL_LIB)

Test_DiagnSet: Test_DiagnSet.o $(DIAGN_MODULES) 
	$(LINKPP) -o Test_DiagnSet Test_DiagnSet.o $(DIAGN_MODULES) $(ADOPT_LIBS) $(LIBS) $(SERIAL_LIB)

Test_DiagnParam: Test_DiagnParam.o $(ADOPT_DIAGN)/DiagnParam.h 
	$(LINKPP) -o Test_DiagnParam Test_DiagnParam.o $(LIBS) $(ADOPT_LIBS) $(SERIAL_LIB)

Test_DiagnApp: Test_DiagnApp.o $(DIAGN_MODULES)
	$(LINKPP) -o Test_DiagnApp Test_DiagnApp.o $(DIAGN_MODULES) $(LIBS) $(ADOPT_LIBS) $(SERIAL_LIB) 

Test_Thread: Test_Thread.o 
	$(LINKPP) -o Test_Thread Test_Thread.o  $(LIBS) $(ADOPT_LIBS)

#DeSerializeFastDiagnostic: DeSerializeFastDiagnostic.o $(FAST_MODULES) 
#	$(LINK) -o DeSerializeFastDiagnostic DeSerializeFastDiagnostic.o  $(FAST_MODULES) $(ADOPT_LIBS) $(LIBS) $(FITS_LIBS) -lgmp $(SERIAL_LIB)

DeSerialize: DeSerialize.o $(ADOPT_DIAGN)/RunningMean.o 
	$(LINKPP) -o DeSerialize DeSerialize.cpp $(ADOPT_DIAGN)/RunningMean.o $(ADOPT_LIBS) $(LIBS) $(SERIAL_LIB)

DeSerializeDiagnVar: DeSerializeDiagnVar.o $(ADOPT_DIAGN)/DiagnVar.o $(ADOPT_DIAGN)/RunningMean.o 
	$(LINKPP) -o DeSerializeDiagnVar DeSerializeDiagnVar.cpp  $(DIAGN_MODULES) $(ADOPT_LIBS) $(LIBS) $(SERIAL_LIB)

DeSerializeDiagnParam: DeSerializeDiagnParam.o  
	$(LINKPP) -o DeSerializeDiagnParam DeSerializeDiagnParam.cpp $(SERIAL_LIB)

clean:
	rm -f *.o *.d core core.* *~ gmon.out $(TEST_TARGETS)

tags:
	ctags *.[ch]
	cscope -Rbq

include $(ADOPT_SOURCE)/addeps.gen

