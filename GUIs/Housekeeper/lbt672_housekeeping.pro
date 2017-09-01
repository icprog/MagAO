TEMPLATE	= app
LANGUAGE	= C++

CONFIG	+= qt warn_on release

SOURCES	+= main.cpp

FORMS	= ./.ui/lbt672_housekeeping.ui \
	./.ui/PlotForm.ui

unix {
  UI_DIR = .ui
  MOC_DIR = .moc
  OBJECTS_DIR = .obj
}


