#!/usr/bin/env python

import os
from qt import *

from AdOpt.widgets import LogWidget

# +Class

# LogViewer     Window for log viewer

# This class is a simple QT window with a multi-line edit box to show log output
# The setFD() method must be called to set the file descriptor to read from, or the setFile() to specify
# which file must be followed with "tail -f"
# When an unexpected error occurs (for example, the file descriptor is closed by
# another process), the log action stop, and must be started again with another
# call to setFD()

class LogViewer( QDialog):

    def __init__(self, parent, title):
        QDialog.__init__(self, parent, title)
        self.resize( 200, 300)
        self.hide()

        self.gridlayout = QGridLayout( self, 1, 1)
        self.gridlayout.setAutoAdd(1)

        self.logWidget = LogWidget.LogWidget( self, title)

    def setFD(self, fd):
        self.logWidget.setFD(fd)

    def setFile( self, filename):
        self.logWidget.setFile( filename)
        
    def addString( self, stringa):
        self.logWidget.addString( stringa)
    
    def reset(self):
        self.logWidget.reset()

    def close(self, delete=1):
        self.hide()
        return False

    def kill(self):
        self.logWidget.kill()
