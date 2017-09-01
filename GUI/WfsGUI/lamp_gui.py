#!/usr/bin/env python
#
#+File: lamp_gui.py
#
# Simple lamp_gui
#
#-

import os
from qt import *
from AdOpt import cfg

# Import the C library wrapper

from AdOpt.wrappers import msglib

#+Class: rerot_gui
#
# Main window for the GUI tool
#-

class lamp_gui( QDialog):
    
    def __init__(self, AOwindow):
        QDialog.__init__(self)

        self.name = "Source lamp"

        ## Draw the widgets

        self.layout = QVBoxLayout(self, 10)
        self.layout.setAutoAdd(1)

        self.setMinimumSize(QSize(400,100))

        self.label1 = QLabel("Lamp intensity:", self)

        self.var_cur = cfg.varname('picctrl', 'LampIntensity.CUR')
        self.var_req = cfg.varname('picctrl', 'LampIntensity.REQ')

        # Crea lo slider di ingrandimento
        self.slider = QSlider( 0, 255, 2, 1, QSlider.Horizontal, self, "")
        self.slider.setTickInterval(5)
        self.slider.setTickmarks( QSlider.Above)
        self.connect( self.slider, SIGNAL("sliderMoved(int)"), self.slotSlider)
        self.connect( self.slider, SIGNAL("sliderReleased()"), self.slotSlider)

        self.progress = QProgressBar( self, "Current intensity")
        self.progress.setTotalSteps(255)

        # register slots

        AOwindow.Notify( self.var_cur, self.slotNotify, getNow = True)

    ## +Method: slotNotify
    ##

    def slotNotify(self, args):
        (message) = args
        varname = message['name']
        value = message['value']

        ## When one of the watched variables change, update our display

        if varname == self.var_cur:
            self.progress.setProgress( value)

    def slotSlider(self):
        msglib.SetIntVar( self.var_req, self.slider.value())



        
