include $(ADOPT_SOURCE)/makefile.gen


#
# Note: most of the programs built from this Makefile are for testing purposes,
# so they are not installed as binary executables.
#
# Only exceptions are: thrdtest, killer


# programs to be made

TARGETS = pinger echoer killer vars00 vars01 stresstest consumer producer \
          sizeof simplevar logger gentest thrdtest thconsum \
          cfgparsertest vartest aostest errtest info parselog \
          iptest mVar logtest logtestNone bculibtest adamtest adamcmd dfits nettest anemometertest \
          modbustest splitdata

all: $(TARGETS) 

include $(ADOPT_SOURCE)/addeps.gen

include $(ADOPT_LIB)/makefile.logger
include $(ADOPT_LIB)/makefile.netlib
include $(ADOPT_LIB)/makefile.adamlib

### Note: if your O.S. has both the GNU readline and GNU history libraries, 
### then you can add line editing and history uncommenting the following two lines
HISTORY = -DHAS_HISTORY 
HISTORY_LIBS = -lreadline -lhistory -lcurses
#################################################

CFLAGS += $(HISTORY)
CPPFLAGS += $(HISTORY)

MORE_LIBS = $(HISTORY_LIBS)

INCLUDE := -I$(ADOPT_INCLUDE)


STDCONFIG_LIBS = $(ADOPT_LIB)/stdconfig.o

LOG_LIBS = $(ADOPT_LIB)/logger.a

CPP_LIBS = $(ADOPT_LIB)/aoapp.a $(ADOPT_LIB)/configlib.a $(ADOPT_LIB)/logger.a -lboost_serialization -lboost_thread -lstdc++  

LIBS = -lpthread -lrt -lm 

MSGLIB_F_DEPS =  $(F_MSGCODES) $(F_ERRORDB) $(F_MSGLIB)

THRDLIB_F_DEPS =  $(F_MSGCODES) $(F_ERRORDB) $(F_THRDLIB) $(F_AOSLIB)

AOSLIB = $(ADOPT_AOSLIB)/aoslib.o

pinger.o: $(MSGLIB_F_DEPS) pinger.c 

consumer.o: $(MSGLIB_F_DEPS) consumer.c 

thconsum.o: $(THRDLIB_F_DEPS) thconsum.c 

producer.o: $(MSGLIB_F_DEPS) producer.c 

echoer.o: $(MSGLIB_F_DEPS) echoer.c 

vars00.o: $(MSGLIB_F_DEPS) vars00.c 
vars01.o: $(MSGLIB_F_DEPS) vars01.c 
simplevar.o: $(MSGLIB_F_DEPS) simplevar.c 

killer.o: $(MSGLIB_F_DEPS) killer.c 

gentest.o: $(MSGLIB_F_DEPS) gentest.c 

thrdtest.o: $(THRDLIB_F_DEPS) thrdtest.c 

testlog.o: $(THRLIB_F_DEPS) testlog.cpp

splitdata.o: $(THRLIB_F_DEPS) splitdata.cpp

errtest.o:  errtest.c 

iptest.o: $(THRDLIB_F_DEPS) iptest.c 

info.o:  $(MSGLIB_F_DEPS) info.c 

aostest.o: $(THRDLIB_F_DEPS) $(F_AOSLIB) aostest.c 

stresstest.o: $(THRDLIB_F_DEPS) stresstest.c stresstest.h 

testaoapp.o: $(THRDLIB_F_DEPS) testaoapp.cpp

logger.o: $(MSGLIB_F_DEPS) logger.c 

cfgparsertest.o: cfgparsertest.cpp 

mVar.o: mVar.cpp mVar.h
logtest.o: logtest.cpp logtest.h
nettest.o: nettest.cpp nettest.h

logtestNone.o: $(THRDLIB_F_DEPS) logtestNone.cpp

bculibtest.o: bculibtest.cpp

adamtest.o: adamtest.cpp
adamcmd.o: adamcmd.cpp

modbustest.o: modbustest.cpp

anemometertest.o: anemometertest.cpp


pinger: pinger.o
	$(LINK) -o pinger $^ $(MSGLIB_A)  -lm

mVar: mVar.o
	$(LINK) -o mVar $^ $(CPP_LIBS) $(LIBS) $(THRDLIB_A)

logtest: logtest.o
	$(LINK) -o logtest $^ $(CPP_LIBS) $(LIBS) $(THRDLIB_A)

nettest: nettest.o
	$(LINK) -o nettest $^ $(CPP_LIBS) $(LIBS) $(THRDLIB_A)

sizeof: sizeof.o
	$(LINK)  -o sizeof $^

killer: killer.o
	$(LINK)  -o killer $^ $(MSGLIB_A)

echoer: echoer.o 
	$(LINK) -o echoer $^ $(MSGLIB_A)

stresstest: stresstest.o  
	$(LINK) -o stresstest $^ $(THRDLIB_A) $(LIBS)  

logger: logger.o 
	$(LINK) -o logger $^ $(MSGLIB_A) $(LIBS) 

vars00: vars00.o 
	$(LINK) -o vars00 $^ $(MSGLIB_A) $(LIBS) 
vars01: vars01.o 
	$(LINK) -o vars01 $^ $(MSGLIB_A)  $(LIBS) 
simplevar: simplevar.o 
	$(LINK) -o simplevar $^ $(MSGLIB_A)  $(LIBS) 

consumer: consumer.o 
	$(LINK)  -o consumer $^ $(MSGLIB_A) $(LIBS) 

thconsum: thconsum.o
	$(LINK)  -o thconsum $^ $(THRDLIB_A) $(LIBS) 

producer: producer.o 
	$(LINK)  -o producer $^ $(MSGLIB_A) $(LIBS) 

gentest: gentest.o 
	$(LINK) -o gentest $^ $(MSGLIB_A) $(LIBS) 

testaoapp: testaoapp.o  
	$(LINK) -o testaoapp $^ $(CPP_LIBS) $(LIBS) $(THRDLIB_A)

thrdtest: thrdtest.o  
	$(LINK) -o thrdtest $^ $(AOSLIB) $(THRDLIB_A) $(LIBS) $(HISTORY_LIBS)

testlog: testlog.o
	$(LINK) -o testlog $^ $(THRDLIB_A) $(LIBS) ../lib/Logger.o -lstdc++ ../lib/base/errlib.o

splitdata: splitdata.o
	$(LINK) -o splitdata $^ $(THRDLIB_A) $(LIBS)  -lstdc++ ../lib/base/errlib.o

logtestNone: logtestNone.o  
	$(LINK) -o logtestNone $^ $(CPP_LIBS) $(LIBS) $(THRDLIB_A)

errtest: errtest.o  
	$(LINK) -o errtest $^ $(MSGLIB_A) $(LIBS) 

iptest: iptest.o  
	$(LINK) -o iptest $^ $(THRDLIB_A) $(LIBS) 

info: info.o  
	$(LINK) -o info info.o

aostest: aostest.o 
	$(LINK) -o aostest aostest.o $(AOSLIB) $(THRDLIB_A) $(LIBS) $(MORE_LIBS) $(HISTORY_LIBS)

cfgparsertest: cfgparsertest.o $(STDCONFIG_LIBS) $(LOG_LIBS)
	$(LINKPP) -o  cfgparsertest $^ $(THRDLIB_A) $(LIBS)


vartest: vartest.o $(LOG_LIBS) $(THRDLIB_A)
	$(LINKPP) -o vartest $^ $(CPP_LIBS) $(THRDLIB_A) $(LIBS)

adamtest: adamtest.o
	$(LINKPP) -o adamtest adamtest.o $(ADAMLIB_LNK)

adamcmd: adamcmd.o
	$(LINKPP) -o adamcmd adamcmd.o $(HISTORY_LIBS) $(ADOPT_LIB)/adamtestlib.a $(LOGGER_LNK) $(NETLIB_LNK) $(CONFIGLIB_LNK) -lstdc++ -lboost_thread

modbustest: modbustest.o
	$(LINKPP) -o modbustest modbustest.o $(ADAMLIB_LNK) -lcurses

bculibtest: bculibtest.o $(STDCONFIG_LIBS) $(LOG_LIBS)
	$(LINK) -o bculibtest $^ $(THRDLIB_A) $(LIBS) ../lib/configlib.a ../lib/BcuLib/bculib.a -lstdc++ 

dfits: dfits.c

anemometertest: anemometertest.o
	$(LINKPP) -o anemometertest $^ $(ADAMLIB_LNK) ../lib/env/envlib.a

#####################################################

clean:
	rm -f *.o *.d core *~ gmon.out $(TARGETS)

tags:
	ctags -t *.[ch]

install: thrdtest killer mVar dfits
	install -d $(ADOPT_ROOT)/bin
	install thrdtest $(ADOPT_ROOT)/bin
	install killer $(ADOPT_ROOT)/bin
	install mVar $(ADOPT_ROOT)/bin
	install dfits $(ADOPT_ROOT)/bin

