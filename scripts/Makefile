include $(ADOPT_SOURCE)/makefile.gen


# programs to be made

TARGETS = # eev39.o # provatables

# pixelsim.so - not needed

ADOPT_LIB = $(ADOPT_SOURCE)/lib
ADOPT_BASELIB = $(ADOPT_SOURCE)/lib/base


QTINCLUDE = $(QTDIR)/include
QTLIBS = $(QTDIR)/lib
PYTHONDIR = /usr/include/python
BCUDIR = $(ADOPT_SOURCE)/BCUCtrl

INCLUDE = -I$(ADOPT_BASELIB) -I$(ADOPT_LIB) -I$(PYTHONDIR) -I$(QTINCLUDE) -I$(BCUDIR)

LAO_LIBS = $(ADOPT_LIB)/commlib.o  \
           $(ADOPT_BASELIB)/errlib.o   \
           $(ADOPT_BASELIB)/msglib.o   \
           $(ADOPT_BASELIB)/rtdblib.o \
           $(ADOPT_LIB)/convert.o \
           $(ADOPT_LIB)/bculib.o 

LIBS = 

CC = gcc

#OPTIMIZE = -O 
#OPTIMIZE = -g -pg 
OPTIMIZE = -g

CFLAGS = -W -Wreturn-type -Wmissing-prototypes $(INCLUDE) -L$(QTLIBS)

LINK = $(CC) $(OPTIMIZE) 

.c.o:
	$(CC) $(OPTIMIZE) $(CFLAGS) -c $<

all: $(TARGETS)


provatables: provatables.c 
	$(CC) $(OPTIMIZE) $(CFLAGS) -o provatables provatables.c eev39.o $(LAO_LIBS)


clean:
	rm -f *.o *.d *.so core core.* *~ gmon.out
	rm -f $(TARGETS)

tags:
	ctags -t *.[ch]

install:
	install -d $(ADOPT_ROOT)/bin
	install thaoshell.py $(ADOPT_ROOT)/bin
	install runBoardSetup.py $(ADOPT_ROOT)/bin
	install pupilcheck.py $(ADOPT_ROOT)/bin
	install runCenterPupils.py $(ADOPT_ROOT)/bin
	install runAutoFocus47.py $(ADOPT_ROOT)/bin
	install runSourceAcq.py $(ADOPT_ROOT)/bin
	install equalize_quadrants2.py $(ADOPT_ROOT)/bin
	install change_binning.py $(ADOPT_ROOT)/bin
	install change_binning47.py $(ADOPT_ROOT)/bin
	install synch_binning47.py $(ADOPT_ROOT)/bin
	install rerot_proxy.py $(ADOPT_ROOT)/bin
	install thorlabs_proxy.py $(ADOPT_ROOT)/bin
	install irtc_proxy.py $(ADOPT_ROOT)/bin
	install muovileggirot.py $(ADOPT_ROOT)/py
	install muovileggirot_plot.pro $(ADOPT_ROOT)/idl/wfs_lib # This must be done after the installation of the idl directory
	install graph_ron.py $(ADOPT_ROOT)/py
	install test_dark_ron.py $(ADOPT_ROOT)/py
	install test_dark_ron_table.py $(ADOPT_ROOT)/py
	install test_dark_ron_single.py $(ADOPT_ROOT)/py
	install test_dark_ron_cont.py $(ADOPT_ROOT)/py
	install turnon.py $(ADOPT_ROOT)/py
	install turnoff.py $(ADOPT_ROOT)/py
	install tozzi.py $(ADOPT_ROOT)/py
	install log_archive.py $(ADOPT_ROOT)/py
	install acquire_batch.py $(ADOPT_ROOT)/bin
	install aops $(ADOPT_ROOT)/bin
	install enable_lab_mode.sh $(ADOPT_ROOT)/bin
	install disable_lab_mode.sh $(ADOPT_ROOT)/bin
	install force_stop.sh $(ADOPT_ROOT)/bin
	install test_elevation.sh $(ADOPT_ROOT)/bin
	install clean_ts8.py $(ADOPT_ROOT)/bin
	install make_m2c.py $(ADOPT_ROOT)/bin
	install stage_spiral.py $(ADOPT_ROOT)/bin

