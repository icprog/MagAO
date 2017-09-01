#!/usr/bin/env python
#
#+File: quick_selection.py
#
# GUI tool for quick parameter selection (REC, background, etc.)
#
# This window will search for available REC and background files and
# display them. Selecting one will put them on the BCU.
#
#-

import os, re
from qt import *
from AdOpt import cfg, fits_lib, calib

# Import the C library wrapper

from AdOpt.wrappers import msglib

#+Class: quick_selection.py
#
# Main window for the GUI tool.
#-

class quick_selection( QDialog):
    
    def __init__(self, AOwindow):
        QDialog.__init__(self)

        self.name = "Quick selection"

        ## Draw the widgets

        self.gridLayout = QGridLayout( self, 9, 4, 20)

        self.label3 = QLabel("Current Dark file:", self)
        self.gridLayout.addWidget( self.label3, 2, 0)

        self.cur_backg = QLabel("None", self)
        self.gridLayout.addWidget( self.cur_backg, 2, 1)

        #self.label4 = QLabel("Select a new Dark file:", self)
        self.label4 = QLabel("", self)
        self.gridLayout.addWidget( self.label4, 3, 0)

        self.backg_list = QListBox( self)
        self.gridLayout.addWidget( self.backg_list, 3, 1)

        self.backg_send = QPushButton("Send Dark", self)
        self.gridLayout.addWidget( self.backg_send, 3, 2)
        self.connect( self.backg_send, SIGNAL("clicked()"), self.slotBackgSend)

        self.label5 = QLabel("Current slopes null:",self)
        self.gridLayout.addWidget( self.label5, 4, 0)

        self.cur_slopes = QLabel("None", self)
        self.gridLayout.addWidget( self.cur_slopes, 4, 1)
    
        #self.label6 = QLabel("Select a new Slopes null:", self)
        self.label6 = QLabel("", self)
        self.gridLayout.addWidget( self.label6, 5, 0)

        self.slopenull_list = QListBox(self)
        self.gridLayout.addWidget( self.slopenull_list, 5, 1)

        self.slopenull_send = QPushButton("Send NULL", self)
        self.gridLayout.addWidget( self.slopenull_send, 5, 2)
        self.connect( self.slopenull_send, SIGNAL("clicked()"), self.slotSlopenullSend)

        self.refresh = QPushButton("Refresh lists", self)
        self.gridLayout.addWidget( self.refresh, 8, 2)
        self.connect( self.refresh, SIGNAL("clicked()"), self.slotRefresh)
        

        # Ask for variable notification

        self.var_dark_cur = cfg.varname('slopecompctrl','DARK.CUR')
        self.var_dark_req = cfg.varname('slopecompctrl','DARK.REQ')
        self.var_slopenull_cur = cfg.varname('slopecompctrl','SLOPENULL.CUR')
        self.var_slopenull_req = cfg.varname('slopecompctrl','SLOPENULL.REQ')
        self.var_binning_cur = cfg.varname('ccd39','XBIN.CUR')

        msglib.VarNotif( self.var_dark_cur, 1)
        msglib.VarNotif( self.var_slopenull_cur, 1)
        msglib.VarNotif( self.var_binning_cur, 1) 

        # Register to listen for VARREPLY messages (to update our display)

        QObject.connect( AOwindow, PYSIGNAL( "%s" % self.var_dark_cur), self.slotNotify)
        QObject.connect( AOwindow, PYSIGNAL( "%s" % self.var_slopenull_cur), self.slotNotify)
        QObject.connect( AOwindow, PYSIGNAL( "%s" % self.var_binning_cur), self.slotNotify)

        self._curBin=1
        self.slotRefresh()


    # Method: slotNotify
    #
    # Updates our display when one of the parameters is changed by another program

    def slotNotify( self, args):
        (message) = args
        varname = message['name']
        value = message['value']

        if varname == self.var_dark_cur:
                self.cur_backg.setText(value)

        elif varname == self.var_slopenull_cur:
                self.cur_slopes.setText(value)

        elif varname == self.var_binning_cur:
            self._curBin = value
            self.slotRefresh()


    #++Method: fillBackgList
    #
    # Refresh the background list
    #-

    def fillBackgList(self):

        self.backg_list.clear()

        backgs = os.listdir( calib.BackgDir('ccd39', self._curBin))
        for b in backgs:
            self.backg_list.insertItem( b, -1)

        self.backg_list.sort()

    #++Method: fillSlopenullList
    #
    # Refresh the Slope NULL list
    #-

    def fillSlopenullList(self):

        self.slopenull_list.clear()

        nulls = os.listdir( calib.slopenullDir(self._curBin))
        for l in nulls:
            self.slopenull_list.insertItem( l, -1)

        self.slopenull_list.sort()

    #++Method: slotBackgSend
    #
    # Handler for the "Send Background" button: sends a new background to the BCU
    #-

    def slotBackgSend(self):
        backg = self.backg_list.currentText().latin1()
        msglib.SetVar(self.var_dark_req, msglib.CHAR_VARIABLE, len(backg), backg) 

    #++Method: slotSlopenullSend
    #
    # Handler for the "Send Slope NULL" button: sends a new slope null reference to the BCU
    #-

    def slotSlopenullSend(self):
        null = self.slopenull_list.currentText().latin1()
        msglib.SetVar(self.var_slopenull_req, msglib.CHAR_VARIABLE, len(null), null)

    def slotRefresh(self):
#        self.fillRECList()
        self.fillBackgList()
        self.fillSlopenullList()
#        self.fillDisturbList()
