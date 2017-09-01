#!/usr/bin/env python
#
#+File: adc.py
#
# Simple adc GUI
#
#-

import os
from qt import *
from AdOpt import AOConstants, cfg

# Import the C library wrapper

import msglib

#+Class: adc
#
# Main window for the GUI tool
#-

class adc( QDialog):
    
    def __init__(self, AOwindow):
        QDialog.__init__(self)

        self.homing_pos1 = cfg.cfg['adc1']['HomingPosition'].Value()
        self.homing_pos2 = cfg.cfg['adc2']['HomingPosition'].Value()
        self.abort_pos1  = cfg.cfg['adc1']['AbortPosition'].Value()
        self.abort_pos2  = cfg.cfg['adc2']['AbortPosition'].Value()

        self.varname_status1 = cfg.varname('adc1', 'STATUS')
        self.varname_curpos1 = cfg.varname('adc1', 'POS.CUR')
        self.varname_tgtpos1 = cfg.varname('adc1', 'POS.REQ')

        self.varname_status2 = cfg.varname('adc2', 'STATUS')
        self.varname_curpos2 = cfg.varname('adc2', 'POS.CUR')
        self.varname_tgtpos2 = cfg.varname('adc2', 'POS.REQ')


        self.name = "ADC"

        ## Draw the widgets

        self.gridLayout = QGridLayout( self, 7, 2, 5)
        self.gridLayout.setAutoAdd(1)

        self.label1 = QLabel("Wheel 1 status:", self)
        self.curstatus1 = QLabel("unknown", self)
        self.label2 = QLabel("Wheel 2 status:", self)
        self.curstatus2 = QLabel("unknown", self)

        self.label3 = QLabel("Wheel 1 position:", self)
        self.curposition1 = QLabel("unknown", self)
        self.label4 = QLabel("Wheel 2 position:", self)
        self.curposition2 = QLabel("unknown", self)

        self.abortButton = QPushButton("Abort movement", self);
        self.connect( self.abortButton, SIGNAL("clicked()"), self.slotAbort)

        self.homeButton = QPushButton("Start homing", self)
        self.connect( self.homeButton, SIGNAL("clicked()"), self.slotHome)

        self.label5 = QLabel("New zenithal angle:", self)
        self.angleValue = QLineEdit("0", self)

        self.label6 = QLabel("", self)
        self.applyButton = QPushButton("Apply", self)
        self.connect( self.applyButton, SIGNAL("clicked()"), self.slotApply)

        self.gridLayout.setResizeMode( QLayout.Minimum)

        # register slots

        AOwindow.Notify( self.varname_status1, self.slotNotify, getNow = 1)
        AOwindow.Notify( self.varname_curpos1, self.slotNotify, getNow = 1)
        AOwindow.Notify( self.varname_status2, self.slotNotify, getNow = 1)
        AOwindow.Notify( self.varname_curpos2, self.slotNotify, getNow = 1)

    ## +Method: slotNotify
    ##
    ## This method is the network event handler for the whole application. It's called by QSocketNotifier when
    ## some data is waiting in the server-connected socked. Thus WaitMessage() is guaranteed to return some
    ## useful data. Different actions are taken based on the message contents.

    def slotNotify(self, args):
        (message) = args
        varname = message['name']
        value = message['value']

        ## When one of the watched variables change, update our display

        if varname == self.varname_status1:
            self.curstatus1.setText( AOConstants.AOStatesType[ value])
        elif varname == self.varname_status2:
            self.curstatus2.setText( AOConstants.AOStatesType[ value])

        elif varname == self.varname_curpos1:
            self.curposition1.setText( "%5.1f" % self.adc_to_zenithal(value))
        elif varname == self.varname_curpos2:
            self.curposition2.setText( "%5.1f" % self.adc_to_zenithal(value))

    def slotHome(self):
        result = msglib.SetRealVar( self.varname_tgtpos1, self.homing_pos1)
        result = msglib.SetRealVar( self.varname_tgtpos2, self.homing_pos2)

    def slotAbort(self):
        result = SetRealVar( self.varname_tgtpos1, self.abort_pos1)
        result = SetRealVar( self.varname_tgtpos2, self.abort_pos2)

    def slotApply(self):
        tgtpos1 = self.zenithal_to_adc( float(self.angleValue.text().latin1()))
        tgtpos2 = - self.zenithal_to_adc( float(self.angleValue.text().latin1()))

        result = msglib.SetRealVar( self.varname_tgtpos1, tgtpos1)
        result = msglib.SetRealVar( self.varname_tgtpos2, tgtpos2)

	print self.varname_tgtpos1
	print self.varname_tgtpos2

    def zenithal_to_adc( self, angle):
	return float(angle)

    def adc_to_zenithal( self, angle):
	return float(angle)

        
