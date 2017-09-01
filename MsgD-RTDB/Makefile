
include $(ADOPT_SOURCE)/makefile.gen

# programs to be made

TARGETS = msgdrtdb 

all: $(TARGETS)

# Uncomment to debug memory allocation/deallocation in rtdb.c
# MEM_DEBUG = -DMEM_DEBUG    

# Dependencies

include $(ADOPT_LIB)/base/makefile.base

OBJECTS = msgdrtdb.o client.o logerr.o rtdb.o table.o shmlib.o configlib.o

CFLAGS += $(MEM_DEBUG)

msgdrtdb: $(OBJECTS) $(BASE_COMPILE)
	$(LINK) -o msgdrtdb $^ $(MSGLIB_A) -lpthread

clean:
	rm -f *.o *.d core core.* *~ gmon.out $(TARGETS)

tags:
	ctags -t *.[ch]

install: all
	install -d $(ADOPT_ROOT)/bin
	install  msgdrtdb $(ADOPT_ROOT)/bin

include $(ADOPT_SOURCE)/addeps.gen
