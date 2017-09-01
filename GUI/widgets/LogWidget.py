#!/usr/bin/env python

import os
from qt import *

# +Class

# LogWidget     Widget for log viewer

# This class is a simple QT widget with a multi-line edit box to show log output
# The setFD() method must be called to set the file descriptor to read from, or the setFile() to specify
# which file must be followed with "tail -f"
# When an unexpected error occurs (for example, the file descriptor is closed by
# another process), the log action stop, and must be started again with another
# call to setFD()

class LogWidget( QTextEdit):

    def __init__(self, parent, title):
        QTextEdit.__init__(self, parent, title)
        self.resize( 200, 300)

        self.setReadOnly(True)
        self.setTextFormat(Qt.LogText)
        self.setMaxLogLines(500)

        # Colors are RGB
        self.colors = {}
        self.colors["TRA"] = '000000'
        self.colors["DEB"] = '000000'
        self.colors["INF"] = '000000'
        self.colors["WAR"] = '800000'
        self.colors["ERR"] = 'FF0000'
        self.colors["FAT"] = '808080'
        
        self.pid = -1
        self.proc = None


    def setFD(self, fd):
        self.notify = QSocketNotifier( fd, QSocketNotifier.Read, self)
        self.notify.setEnabled(1)
        self.connect( self.notify, SIGNAL("activated(int)"), self.slotNotify)

    def setFile( self, filename):

        if self.proc != None:
            self.proc.kill()

        self.filename = filename
        self.proc = QProcess()
        self.proc.addArgument('tail')
        self.proc.addArgument('-F')
        self.proc.addArgument(filename)
        self.proc.setCommunication(QProcess.Stdout | QProcess.Stderr)

        self.connect( self.proc, SIGNAL("readyReadStdout()"), self.slotDataAvail)
        self.proc.launch('')
        self.pid = self.proc.processIdentifier()
        
    def slotNotify(self, fd):
        try:
            logstring = os.read( fd, 1024).lstrip().rstrip()
            for s in logstring.split('\n'):
                if hasattr(s, 'latin1'):
                    self.addString(s.latin1())
                else:
                    self.addString(s)

        except OSError:
        #   self.notify.setEnabled(0)
            pass

    def slotDataAvail(self):
        while self.proc.canReadLineStdout():
            self.addString( self.proc.readLineStdout().latin1())
            

    def addString( self, s):
        pos1 = s.find('|')
        pos2 = s.find('|', pos1+1)
        if (pos1>=0) and (pos2>=0):
            key = s[pos1+1:pos2]
            if self.colors.has_key(key):
                s = '<font color="#%s">%s</font>' % (self.colors[key], s)
        self.append( s)



    def reset(self):
        while self.numLines()>1:
            self.removeLine(0)

    def kill(self):
        self.proc.kill()

