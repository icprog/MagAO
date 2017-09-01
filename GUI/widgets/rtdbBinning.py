#!/usr/bin/python

from qt import *

import time, os.path
from AdOpt import calib, cfg
from AdOpt.wrappers import msglib
from AdOpt.WfsGUI import cfg_gui
from AdOpt.widgets.rtdbSingleValue import *


class rtdbBinning( rtdbSingleValue):

    def __init__( self, parent, name):

        rtdbSingleValue.__init__(self, parent, name)
        
    def setup( self, name, aoGui, ccd, binlist, watchdog = None):

        layout2 = QHBoxLayout(self,0,0,"layout2")

        self.nameLabel = QLabel('', self)
        self.nameLabel.setMinimumSize(QSize(110,20))
        self.nameLabel.setMaximumSize(QSize(110,20))
        layout2.addWidget( self.nameLabel)

        self.valueLabel = QLabel('', self)
        self.valueLabel.setMinimumSize(QSize(70,20))
        self.valueLabel.setMaximumSize(QSize(70,20))
        self.valueLabel.setBackgroundMode(QLabel.PaletteBackground)
        self.valueLabel.setFrameShape(QLabel.LineEditPanel)
        layout2.addWidget( self.valueLabel)

        self.valueEdit = QComboBox(0, self, "Combo")
        self.valueEdit.setMinimumSize(QSize(80,20))
        self.valueEdit.setMaximumSize(QSize(80,20))
        self.valueEdit.insertItem('--select--')
        for bin in binlist:
            self.valueEdit.insertItem('%dx%d' % (bin, bin))
        layout2.addWidget(self.valueEdit)

        self.spacer = QLabel('', self)
        self.spacer.setMinimumSize(QSize(30,20))
        self.spacer.setMaximumSize(QSize(30,20))
        layout2.addWidget(self.spacer)

        self.connect( self.valueEdit, SIGNAL("activated( const QString &)"), self.slotButtonPressed)

        self.nameLabel.setText(name)
        self.type = type
        aoGui.Notify( cfg.varname(ccd,'XBIN.CUR'), self.slotValueChanged, getNow = True);
        aoGui.Notify( cfg.varname(ccd,'YBIN.CUR'), self.slotValueChanged, getNow = True);

        self._curXbin = 0
        self._curYbin = 0

        self._lastUpdated = -1
        self._eraseColor = self.nameLabel.eraseColor()
        self._valid = False
        self._enabled = False

        if watchdog != None:
           self._timeout = 10
           QObject.connect( watchdog, SIGNAL("timeout()"), self.checkTimeout)

    def slotValueChanged(self, message):

        if 'XBIN' in message['name']:
            self._curXbin = message['value']
        if 'YBIN' in message['name']:
            self._curYbin = message['value']
        

        self.valueLabel.setText('%dx%d' % (self._curXbin, self._curYbin))
       

    def slotButtonPressed(self, s):

        s = s.latin1()
        binning = s[0:s.index('x')]

        command = os.environ['ADOPT_ROOT']+"/bin/change_binning.py %s %s last &" % (cfg.side, binning)
        #command = "xterm -T \"Change binning\" -e \""+command+"\""
        #print command

        os.system(command)



