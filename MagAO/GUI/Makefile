# Supervisor general makefile
include $(ADOPT_SOURCE)/makefile.gen

all: VisAOccd_gui

# Dependencies
include $(ADOPT_LIB)/makefile.aoapp


VisAOccd_gui: VisAOccd_gui.ui
	pyuic VisAOccd_gui.ui > VisAOccd_gui.py

install: all
	install -d $(ADOPT_ROOT)/lib/python/AdOpt/VisAOGUI/
	install *.py $(ADOPT_ROOT)/lib/python/AdOpt/VisAOGUI/

clean:
	rm -f VisAOccd_gui.py
	rm -f *~ *.pyc
	
