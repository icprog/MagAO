# Supervisor general makefile definitions
include $(ADOPT_SOURCE)/makefile.gen

all: arbframework.a

# Dependencies
include $(ADOPT_LIB)/makefile.aoapp

CPPFLAGS += -I$(ADOPT_SOURCE)/Arbitrator

	
OBJECTS = $(ADOPT_SOURCE)/Arbitrator/framework/AbstractArbitrator.o \
		  $(ADOPT_SOURCE)/Arbitrator/framework/CommandHandler.o \
		  $(ADOPT_SOURCE)/Arbitrator/framework/CommandImpl.o \
		  $(ADOPT_SOURCE)/Arbitrator/framework/AbstractFsm.o \
		  $(ADOPT_SOURCE)/Arbitrator/framework/AbstractSystem.o \
		  $(ADOPT_SOURCE)/Arbitrator/framework/CommandsImpl.o \
		  $(ADOPT_SOURCE)/Arbitrator/framework/SingleSystem.o \


# aoapp_compile creates AOStates.c and AOStates.h
arbframework.a: $(AOAPP_COMPILE) $(OBJECTS) 
	$(AR) arbframework.a $(OBJECTS)
	$(RANLIB) arbframework.a

clean: 
	rm -f *.o *.d *.so core core.* *~ gmon.out
	rm -f arbframework.a

include $(ADOPT_SOURCE)/addeps.gen
	