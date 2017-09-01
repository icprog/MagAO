TEMPLATE	= app
LANGUAGE	= C++

CONFIG	+= qt warn_on release

LIBS	+= $(ADOPT_LIB_LINK) -lqassistantclient -L/usr/local/qwt-5.1.2/lib/ -lqwt \
../MagAOIClient/MagAOIClient.o build_time.o

INCLUDEPATH	+= /usr/local/qwt-5.1.2/include ../MagAOIClient $(ADOPT_SOURCE)/lib /usr/include/qt3/ ../

HEADERS	+= myqlabel.h \
	myqstring.h \
	version.h \
	QMagAOIClient.h

SOURCES	+= main.cpp \
	myqlabel.cpp\
   QMagAOIClient.cpp  

FORMS	= aoigui.ui \
	aoigui_command_dialog.ui \
	aoigui_about.ui \
	aoigui_warning_dialog.ui

IMAGES	= images/filenew \
	images/fileopen \
	images/filesave \
	images/print \
	images/undo \
	images/redo \
	images/editcut \
	images/editcopy \
	images/editpaste \
	images/searchfind \
	images/OAALogo.png \
	images/sf6.png \
   images/University-of-Arizona-Billiard-Logo.png \
   images/Logo_3_transparent.png \
   images/Logo_3_transparent_128.png

MAKEFILE = makefile.aoigui


#EventCallbackAOS.cpp



unix {
  UI_DIR = .ui
  MOC_DIR = .moc
  OBJECTS_DIR = .obj
}



