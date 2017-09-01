# Supervisor general makefile definitions
include $(ADOPT_SOURCE)/makefile.gen

all: basearb_lib.a basearb_intf.a
	@:

# Source objects
LIB_OBJECTS = Serializator.o SerializableMessage.o Alerts.o Command.o \
          CommandDecorator.o Commands.o

INTF_OBJECTS = ArbitratorInterface.o

OBJECTS = $(LIB_OBJECTS) $(INTF_OBJECTS)

basearb_lib.a: $(LIB_OBJECTS)
	$(AR) basearb_lib.a $(LIB_OBJECTS)
	$(RANLIB) basearb_lib.a
	
basearb_intf.a: $(INTF_OBJECTS)
	$(AR) basearb_intf.a $(INTF_OBJECTS)
	$(RANLIB) basearb_intf.a

clean: 
	rm -f *.d *.o
	rm -f basearb_lib.a basearb_intf.a

include $(ADOPT_SOURCE)/addeps.gen
