#Makefile generale
include ../../makefile.gen

TARGETS = msglib.a thrdlib.a

OBJ_MSG = errlib.o msglib.o rtdblib.o \
          buflib.o msgtxt.o timelib.o

OBJ_THR = therrlib.o thmsglib.o thrdlib.o thrtdblib.o \
          thbuflib.o msgtxt.o timelib.o

OBJECTS = $(OBJ_MSG) $(OBJ_THR)

all: globals.h errordb.h vartypes.h msgcodes.h $(TARGETS) 

thrdlib.o: thrdlib.c
	$(CC) $(OPTIMIZE) $(CFLAGS) -DTHRDLIB -o thrdlib.o -c thrdlib.c

therrlib.o: errlib.c
	$(CC) $(OPTIMIZE) $(CFLAGS) -DTHRDLIB -o therrlib.o -c errlib.c

thrtdblib.o: rtdblib.c
	$(CC) $(OPTIMIZE) $(CFLAGS) -DTHRDLIB -o thrtdblib.o -c rtdblib.c

thmsglib.o: msglib.c
	$(CC) $(OPTIMIZE) $(CFLAGS) -DTHRDLIB -o thmsglib.o -c msglib.c

thbuflib.o: buflib.c
	$(CC) $(OPTIMIZE) $(CFLAGS) -DTHRDLIB -o thbuflib.o -c buflib.c

globals.h globals.c globals.pro AOGlobals.py: globals.txt
	../makerrordb.py globals.txt -pydef AOGlobals
	cp AOGlobals.py ../../PyModules/AdOpt

errordb.h errordb.c errordb.pro AOErrCode.py: errordb.txt
	../makerrordb.py errordb.txt -py AOErrCode
	cp AOErrCode.py ../../PyModules/AdOpt

msgcodes.h msgcodes.c msgcodes.pro AOMsgCode.py: msgcodes.txt
	../makerrordb.py msgcodes.txt -py AOMsgCode
	cp AOMsgCode.py ../../PyModules/AdOpt

vartypes.h vartypes.c vartypes.pro AOVarCode.py: vartypes.txt
	../makerrordb.py vartypes.txt -py AOVarCode
	mv -f AOVarCode.py ../../PyModules/AdOpt

msglib.a: $(OBJ_MSG)
	$(AR) msglib.a $?
	$(RANLIB) msglib.a

thrdlib.a: $(OBJ_THR)
	$(AR) thrdlib.a $?
	$(RANLIB) thrdlib.a

clean:
	rm -f *.o *.d core.* core *~ gmon.out *.pdf
	rm -f *.aux *.dvi *.log *.toc *.tmp
	rm -f $(TARGETS)

distclean:
	rm -f errordb.h errordb.c msgcodes.h msgcodes.c
	rm -f AOMsgCode.py AOErrCode.py AOVarCode.py
	rm -f vartypes.h vartypes.c 
	rm -f errordb.pro msgcodes.pro vartypes.pro
	
tags:
	ctags -t *.[ch]

docs: msglib.pdf rtdblib.pdf buflib.pdf

msglib.pdf: msglib.dpu msglib.c common.h msglib.h
	dpu -pdf msglib

rtdblib.pdf: rtdblib.dpu rtdblib.c common.h msglib.h rtdblib.h
	dpu -pdf rtdblib

buflib.pdf: buflib.dpu rtdblib.c common.h msglib.h buflib.h
	dpu -pdf buflib

include $(ADOPT_SOURCE)/addeps.gen

