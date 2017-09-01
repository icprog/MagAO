
all: idl_test.so

# ITT had the great idea of changing the name of
# the installation directory since idl6.3
# So if you have idl < 6.4 use
#IDLDIR = /usr/local/rsi/idl/external/include
# else use
IDLDIR = /usr/local/itt/idl/external/include

CPPFLAGS = -Wall -W -Wreturn-type -Wunused -D_GNU_SOURCE
MORE_INCLUDE = -I$(IDLDIR) -I/usr/include/python

%.o:%.h

.cpp.o:
	g++ $(CPPFLAGS) $(MORE_INCLUDE) -c $< -o $@
	
idl_test.so: idl_test.o $(IDLDIR)/idl_export.h
	g++ $(CPPFLAGS) -shared -o idl_test.so idl_test.o -lstdc++

clean:
	rm -f *.o *.so 
	
