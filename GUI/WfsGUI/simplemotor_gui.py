#!/usr/bin/env python
#
#+File: rerot_gui.py
#
# Simple rerot_gui GUI
#
#-

import os
from qt import *
import cfg_gui

from AdOpt import AOConstants, cfg

# Import the C library wrapper

from AdOpt.wrappers import msglib

#+Class: rerot_gui
#
# Main window for the GUI tool
#-

class simplemotor_gui( QDialog):
    
    def __init__(self, AOwindow):
	'''
	Members to set before calling this constructor:
	self.prefix
	self.name
	self.unit
	'''

        QDialog.__init__(self)

        cfg_gui.init()

        self.homing_pos = cfg.cfg[self.prefix]['HomingPosition'].Value()
        self.abort_pos  = cfg.cfg[self.prefix]['AbortPosition'].Value()
        self.goodwindow = cfg.cfg[self.prefix]['GoodWindow'].Value()

        self.varname_status   = cfg.varname( self.prefix, "STATUS")
        self.varname_swstatus = cfg.varname( self.prefix, "SW_STATUS")
        self.varname_curpos   = cfg.varname( self.prefix, "POS.CUR")
        self.varname_tgtpos   = cfg.varname( self.prefix, "POS.REQ")

        self.custompos = {}
        try:
            num = cfg.cfg[self.prefix]['customPositionNum'].Value()
            for n in range(num):
                name = cfg.cfg[self.prefix]['pos%d_name' % n].Value() 
                pos = cfg.cfg[self.prefix]['pos%d_pos' % n].Value() 
                self.custompos[n] = (name, pos)
        except:
            # May happen if no custom pos. are defined
            pass

        self.ButtonGroup = QButtonGroup()

        ## Draw the widgets

        self.setMinimumSize(QSize(460,360))
        #self.setMaximumSize(QSize(550,360))


        self.layout = QVBoxLayout( self, 5)
        self.layout.setAutoAdd(1)

        self.group0 = QGroupBox( 2, Qt.Horizontal, "", self)
        self.group1 = QGroupBox( 3, Qt.Horizontal, "", self)

        self.curstatus = QLabel("unknown", self.group0)
        self.pix = QPushButton("", self.group0)

        self.label2 = QLabel("Cur position:", self.group1)
        self.curposition = QLabel("unknown", self.group1)
        self.group1.addSpace(0)

        self.label3 = QLabel( "Absolute movement ("+self.unit+"):", self.group1)
        self.angleValue = QLineEdit("0", self.group1)
        self.applyButton = QPushButton("Move", self.group1)
        self.connect( self.applyButton, SIGNAL("clicked()"), self.slotApply)

        self.label7 = QLabel("Relative movement ("+self.unit+"):", self.group1)
        self.relValue = QLineEdit("0", self.group1)
        self.moveRelButton = QPushButton("Move", self.group1)
        self.connect( self.moveRelButton, SIGNAL("clicked()"), self.slotMoveRel)

        self.swcheck = QCheckBox("Limit switch active", self.group1)
        self.swcheck.setChecked(0)

        if len(self.custompos)>0:
            self.labels = []
            self.buttons = []

            self.group2 = QGroupBox( 3, Qt.Horizontal, "", self)

            for n in range(len(self.custompos)):
                name, value = self.custompos[n]

                if n == 0:
                    self.labels.append( QLabel("Custom positions", self.group2))
                else:
                    self.group2.addSpace(0)
                self.buttons.append( QPushButton( name, self.group2))
                self.ButtonGroup.insert( self.buttons[-1])

                self.group2.addSpace(0)

            self.connect( self.ButtonGroup, SIGNAL("clicked(int)"), self.slotButtonGroup)

        self.group3 = QGroupBox( 2, Qt.Horizontal, "", self)

        self.abortButton = QPushButton("Abort movement", self.group3);
        self.connect( self.abortButton, SIGNAL("clicked()"), self.slotAbort)

        self.homeButton = QPushButton("Start homing", self.group3)
        self.connect( self.homeButton, SIGNAL("clicked()"), self.slotHome)
        self.dummy = QLabel("", self)

        self.curpos =0


        # register slots

        AOwindow.Notify( self.varname_status, self.slotNotify, getNow = True);
        AOwindow.Notify( self.varname_curpos, self.slotNotify, getNow = True);
        AOwindow.Notify( self.varname_swstatus, self.slotNotify, getNow = True);

    def slotButtonGroup( self, number):
        name, tgtpos = self.custompos[number]
        result = msglib.SetRealVar( self.varname_tgtpos, tgtpos)

        
    ## +Method: slotNotify
    ##

    def slotNotify(self, args):
        (message) = args
        varname = message['name']
        value = message['value']

        ## When one of the watched variables change, update our display

        if varname == self.varname_status:
            self.curstatus.setText( AOConstants.AOStatesType[ value])
            self.enableButtons(value)
            self.setPix(value)

        elif varname == self.varname_curpos:
            self.setNewPos( value)

        elif varname == self.varname_swstatus:
            self.swcheck.setChecked(value)

    def setNewPos(self, value):

        text = "%5.3f %s" % (value, self.unit)
        for pos in self.custompos.values():
            name, posvalue = pos
            if abs(value - posvalue) < self.goodwindow:
                text += " (%s)" % name
                
        self.curposition.setText( text)
        self.curpos = value


    def slotHome(self):
        result = msglib.SetRealVar( self.varname_tgtpos, self.homing_pos)

    def slotAbort(self):
        result = msglib.SetRealVar( self.varname_tgtpos, self.abort_pos)

    def slotApply(self):
        tgtpos = float(self.angleValue.text().latin1())
        if not self.isTgtposValid(tgtpos):
            QMessageBox.warning(self, 'Invalid position', 'The requested position is not valid for the motor', QMessageBox.Ok, QMessageBox.NoButton)
            return

        result = msglib.SetRealVar( self.varname_tgtpos, tgtpos)

    def slotMoveRel(self):
        tgtpos = float(self.relValue.text().latin1())
        tgtpos += self.curpos
        if not self.isTgtposValid(tgtpos):
            QMessageBox.warning(self, 'Invalid position', 'The requested position is not valid for the motor', QMessageBox.Ok, QMessageBox.NoButton)
            return

        result = msglib.SetRealVar( self.varname_tgtpos, tgtpos)

    def isTgtposValid(self, pos):
        valid = True
        if pos == self.homing_pos or pos == self.abort_pos:
            valid = False
        return valid


    def enableButtons(self, status):
        if status == AOConstants.AOStatesCode['STATE_READY']:
            self.homeButton.setEnabled(True)
            self.applyButton.setEnabled(True)
            self.moveRelButton.setEnabled(True)
        else:
            self.homeButton.setEnabled(False)
            self.applyButton.setEnabled(False)
            self.moveRelButton.setEnabled(False)

	if status == AOConstants.AOStatesCode['STATE_READY'] or \
	   status == AOConstants.AOStatesCode['STATE_OPERATING']:
		self.curposition.setEnabled(True)
	else:
		self.curposition.setEnabled(False)

    def setPix(self, status):
	if status == AOConstants.AOStatesCode['STATE_READY'] or \
	   status == AOConstants.AOStatesCode['STATE_OPERATING']:
		self.pix.setPixmap(cfg_gui.pixmaps.pix_ok)
	elif status == AOConstants.AOStatesCode['STATE_HOMING']:
		self.pix.setPixmap(cfg_gui.pixmaps.pix_working)
        else:
		self.pix.setPixmap(cfg_gui.pixmaps.pix_ko)
	

        
