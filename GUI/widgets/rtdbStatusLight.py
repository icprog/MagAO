#!/usr/bin/python

from qt import *

from AdOpt.WfsGUI import cfg_gui

class rtdbStatusLight( QWidget):

    def __init__( self, parent, name, xsize=16):
        
        QWidget.__init__(self, parent)

        self.setMinimumSize(QSize(xsize,20))
        self.setMaximumSize(QSize(xsize,40))

        self._size = xsize

    def setup( self, name, aoGui, varname, ontext="", offtext="", oncolor= None, offcolor=None):

        layout2 = QHBoxLayout(self,0,0,"layout2")

        self.label = QLabel(name, self)
        self.label.setMinimumSize(QSize(self._size,20))
        self.label.setMaximumSize(QSize(self._size,40))
        layout2.addWidget( self.label)

        self.varname = varname
        if varname != None:
            if hasattr(aoGui, "Notify"):
                aoGui.Notify( varname+'.CUR', self.slotValueChanged, getNow = True);
            else:
                aoGui.VarNotif(  varname, self.slotValueChanged)

        self._ontext = ontext
        self._offtext = offtext
        if oncolor:
            self._oncolor = oncolor
        else:
            self._oncolor= cfg_gui.colors.red

        if offcolor:
            self._offcolor = offcolor
        else:
            self._offcolor = cfg_gui.colors.green

        self.update(0)

    def slotValueChanged(self, message):
        self.update(message['value'])

    def update(self, value):
        if value == 0:
            self.label.setEraseColor( self._offcolor)
            self.label.setText( self._offtext)
        else:
            self.label.setEraseColor( self._oncolor)
            self.label.setText( self._ontext)


    def setValid( self, valid):
        self.label.setEnabled(valid)

        

