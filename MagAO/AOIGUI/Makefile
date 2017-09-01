# General VisAO makefile
include $(ADOPT_SOURCE)/makefile.gen

ADOPT_LIB_LINK= $(ADOPT_SOURCE)/lib/logger.a $(ADOPT_SOURCE)/lib/aoapp.a $(ADOPT_SOURCE)/lib/base/thrdlib.a \
-pthread -lrt -lboost_serialization $(ADOPT_SOURCE)/lib/configlib.a -lqwt



all: btime

btime:
	$(MAKE_BUILD_TIME)
	uic aoigui.ui > aoigui.uic.h
	uic aoigui_about.ui > aoigui_about.uic.h
	uic aoigui_command_dialog.ui > aoigui_command_dialog.uic.h
	uic aoigui_error_dialog.ui > aoigui_error_dialog.uic.h
	uic aoigui_warning_dialog.ui > aoigui_warning_dialog.uic.h
	#/usr/share/qt3/bin/qmake
	/usr/lib/qt-3.3/bin/qmake
	$(MAKE) -f makefile.aoigui ADOPT_LIB_LINK='$(ADOPT_LIB_LINK)'

install:
	install -d $(ADOPT_ROOT)/bin
	install AOIGUI $(ADOPT_ROOT)/bin

clean:
	rm -f *.o *.d *~
	rm -f AOIGUI
	rm -f moc_* qrc_* ui_* *.uic.h
	rm -rf .ui .obj .moc
	rm -f makefile.aoigui
		
