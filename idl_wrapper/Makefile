# Supervisor general makefile definitions
include $(ADOPT_SOURCE)/makefile.gen

# programs to be made
TARGETS = idl_wraplib.so

all: $(TARGETS)

# Dependencies for wraplib
include $(ADOPT_SOURCE)/DiagnApp/makefile.diagnapp
include $(ADOPT_SOURCE)/lib/makefile.adamlib
include $(ADOPT_SOURCE)/lib/BcuLib/makefile.bculib
include $(ADOPT_SOURCE)/lib/aoslib/makefile.aoslib
include $(ADOPT_SOURCE)/lib/IntfLib/makefile.intflib
include $(ADOPT_SOURCE)/lib/idllib/makefile.idllib

BCUDIR = $(ADOPT_SOURCE)/BCUCtrl
DIAGNDIR = $(ADOPT_SOURCE)/DiagnApp

CPPFLAGS += -I$(DIAGNDIR) 

OBJECTS = idl_wraplib.o idl_diagnlib.o idl_hexlib.o idl_4dlib.o idl_getpid.o

idl_wraplib.so: $(OBJECTS) $(WRAPLIB_LIBS) $(AOSLIB_COMPILE) $(CONFIGLIB_COMPILE) $(ADAMLIB_COMPILE) $(BCULIB_COMPILE) $(DIAGNAPP_COMPILE)
	$(LINKPP) $(OPTIMIZE) $(CPPFLAGS) -shared -o idl_wraplib.so idl_wraplib.o idl_diagnlib.o idl_hexlib.o idl_4dlib.o $(WRAPLIB_LIBS) $(ADAMLIB_LNK) $(CONFIGLIB_LNK) $(BCULIB_LNK) $(DIAGNAPP_LNK) $(AOAPPLIB_LNK) $(AOSLIB_LNK) $(INTFLIB_LNK) $(IDLLIB_LNK) -lstdc++ -L../contrib/parapin-1.5.1-beta1
	$(LINKPP) $(OPTIMIZE) $(CPPFLAGS) -shared -o idl_getpid.so idl_getpid.o -lstdc++ 
	
install: $(TARGETS)
	install -d $(ADOPT_ROOT)/idl/dlm/
	install idl_wraplib.so  $(ADOPT_ROOT)/idl/dlm/	# TODO oput in the correct subfolder
	install idl_wraplib.dlm $(ADOPT_ROOT)/idl/dlm/	# TODO oput in the correct subfolder
	install idl_getpid.so   $(ADOPT_ROOT)/idl/dlm/	# TODO oput in the correct subfolder
	install idl_getpid.dlm  $(ADOPT_ROOT)/idl/dlm/	# TODO oput in the correct subfolder

clean:
	rm -f *.o *.d *.so core *~ gmon.out $(TARGETS)

tags:
	ctags -t *.[ch]

	
include $(ADOPT_SOURCE)/addeps.gen
	

