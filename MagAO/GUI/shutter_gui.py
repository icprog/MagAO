#!/usr/bin/env python
#
#+File: shutter_gui.py
#
# Shutter remote control
#
#-

import os
from qt import *
import cfg_visao_gui

from AdOpt import AOConstants, cfg

# Import the C library wrapper

from AdOpt.wrappers import msglib

#+Class: shutter_gui
#
# Main window for the GUI tool
#-

class shutter_gui( QDialog):
    
   def __init__(self, AOwindow):
      '''
      Members to set before calling this constructor:
      self.prefix
      self.name
      self.unit
      '''
      self.prefix = "shutterremote"
      self.name = "Shutter Remote Control"
 
      QDialog.__init__(self)

      cfg_visao_gui.init()

      
      
      self.varname_status = cfg.varname( self.prefix, "STATUS")
      self.varname_swstatus = cfg.varname( self.prefix, "SW_STATUS")
      self.varname_curstate = cfg.varname( self.prefix, "STATE.CUR")
      self.varname_reqstate = cfg.varname( self.prefix, "STATE.REQ")
      self.varname_swstate = cfg.varname( self.prefix, "SW_STATE.CUR")
      self.varname_hwstate = cfg.varname( self.prefix,"HW_STATE.CUR")
      self.varname_curctype = cfg.varname( self.prefix, "ConMode.CUR")
      self.varname_reqctype = cfg.varname( self.prefix, "ConMode.REQ")
        
      self.curctype = 0
      self.ButtonGroup = QButtonGroup()

     ## Draw the widgets

      self.setMinimumSize(QSize(460,360))
 
      self.layout = QGridLayout( self)#, 4)
      self.botspace = QSpacerItem(1,1)
      self.layout.addMultiCell(self.botspace, 10,10,0,6)
      
      self.titlelabel = QLabel("VisAO Shutter Remote Control", self)
          
      self.layout.addMultiCellWidget(self.titlelabel, 0,0,0,6, Qt.AlignHCenter)
                      
      self.curstatus = QLabel("unknown", self)
      self.pix = QLabel("", self)
      self.layout.addMultiCellWidget(self.curstatus, 1,1,1,3, Qt.AlignRight)
      self.layout.addMultiCellWidget(self.pix, 1,1, 4,5, Qt.AlignHCenter)
      
      self.label5 = QLabel("Control:", self)#, self.group1)#L)
      self.curcont = QLabel("unknown", self)#, self.group1)#L)
      
      self.layout.addMultiCellWidget(self.label5, 3,3,0,1, Qt.AlignHCenter)
      self.layout.addMultiCellWidget(self.curcont, 4,4,0,1, Qt.AlignHCenter)
      
      self.remoteButton =   QPushButton("    Take Remote Control    ", self)#, self.group1R);
      self.layout.addMultiCellWidget(self.remoteButton, 3,3,2,5)
      self.connect( self.remoteButton, SIGNAL("clicked()"), self.request_remote)
      
      self.overrideCheck = QCheckBox("Override", self)#, self.group1R);
      self.layout.addMultiCellWidget(self.overrideCheck, 4,4,2,5, Qt.AlignHCenter)
      self.connect( self.overrideCheck, SIGNAL("clicked()"), self.override_red)
      
      self.label2 = QLabel("Cur State:", self)
      self.curstate = QLabel("unknown", self)
      self.layout.addWidget(self.label2, 6,1)
      self.layout.addWidget(self.curstate, 6,2)
      
      self.label3 = QLabel( "SW State:", self)
      self.swstate = QLabel("unknown", self)
      self.layout.addWidget(self.label3, 7,1)
      self.layout.addWidget(self.swstate, 7,2)
      
      self.label4 = QLabel( "HW State:", self)
      self.hwstate = QLabel("unknown", self)
      self.layout.addWidget(self.label4, 8,1)
      self.layout.addWidget(self.hwstate, 8,2)
            
      self.statepix = QLabel("", self)
      self.statepix.setPixmap(cfg_visao_gui.pixmaps.shutt_unk)
      self.layout.addMultiCellWidget(self.statepix, 6,8,3,5, Qt.AlignHCenter)
      
      
      self.openButton = QPushButton("Open", self);
      self.connect( self.openButton, SIGNAL("clicked()"), self.open_shutter)
      self.layout.addMultiCellWidget(self.openButton, 9,9, 1,2)
      self.closeButton = QPushButton("Close", self)
      self.connect( self.closeButton, SIGNAL("clicked()"), self.close_shutter)
      self.layout.addMultiCellWidget(self.closeButton, 9,9,4,5)
      

        # register slots

      AOwindow.Notify( self.varname_status, self.slotNotify, getNow = True);
      AOwindow.Notify( self.varname_curstate, self.slotNotify, getNow = True);
      AOwindow.Notify( self.varname_swstate, self.slotNotify, getNow = True);
      AOwindow.Notify( self.varname_hwstate, self.slotNotify, getNow = True);
      AOwindow.Notify( self.varname_curctype, self.slotNotify, getNow = True);
        
   
        
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

      elif varname == self.varname_curstate:
            self.setNewState( value)
            
      elif varname == self.varname_swstate:
            self.setNewSWState( value)
            
      elif varname == self.varname_hwstate:
            self.setNewHWState( value)
            
      elif varname == self.varname_curctype:
            self.setcurctype( value)
            
   def setNewState(self, value):

      if value == 1:
         text = "OPEN"
         self.statepix.setPixmap(cfg_visao_gui.pixmaps.shutt_open)
      elif value == -1:
         text = "CLOSED"
         self.statepix.setPixmap(cfg_visao_gui.pixmaps.shutt_shut)
      else:
         text = "UNKOWN"
         self.statepix.setPixmap(cfg_visao_gui.pixmaps.shutt_unk)
               
      self.curstate.setText( text)
         
        
   def setNewSWState(self, value):

      if value == 1:
         text = "OPEN"
      elif value == -1:
         text = "CLOSED"
      else:
         text = "UNKOWN"
               
      self.swstate.setText( text)
      
   def setNewHWState(self, value):

      if value == 1:
         text = "OPEN"
      elif value == -1:
         text = "CLOSED"
      else:
         text = "UNKOWN"
               
      self.hwstate.setText( text)
        
   def setcurctype(self, value):
      self.curctype = value
      
      if self.overrideCheck.isChecked() == 1:
           buttext = "***Take Remote Control***"
      else:
           buttext = "   Take Remote Control   "
      self.overrideCheck.setEnabled(1)
      checken = 1
      if value == 0:
         text = "NONE"
         checken = 0
      elif value == 1:
         text = "REMOTE"
         buttext = " Give up Remote Control "
         checken = 0
      elif value == 2:
         text = "LOCAL"
      elif value == 3:
         text = "SCRIPT"
      elif value == 4:
         text = "AUTO"
      else:
         text = "UNKOWN"
               
               
      self.curcont.setText( text)
      self.remoteButton.setText(buttext)
      self.overrideCheck.setEnabled(checken)
        
   def open_shutter(self):
        result = msglib.SetIntVar( self.varname_reqstate, 1)

   def close_shutter(self):
        result = msglib.SetIntVar( self.varname_reqstate, -1)    
         
   def request_remote(self):
        if self.curctype != 1:
           if self.overrideCheck.isChecked() == 1:
              ct = 10
           else:
              ct = 1
           
           result = msglib.SetIntVar( self.varname_reqctype, ct)    
           self.overrideCheck.setChecked(0)
        else:
           result = msglib.SetIntVar( self.varname_reqctype, 0)
        #self.remoteButton.setText("   Take Remote Control   ")
        
   def override_red(self):
         
       if self.overrideCheck.isChecked() == 1:
           result = self.remoteButton.setText("***Take Remote Control***")
       else:
           result = self.remoteButton.setText("   Take Remote Control   ")
             
           
   def enableButtons(self, status):
      if status == AOConstants.AOStatesCode['STATE_OPERATING']:
         self.openButton.setEnabled(True)
         self.closeButton.setEnabled(True)
      else:
         self.openButton.setEnabled(False)
         self.closeButton.setEnabled(False)
            
   def setPix(self, status):
      if status == AOConstants.AOStatesCode['STATE_OPERATING']: 
         self.pix.setPixmap(cfg_visao_gui.cfg_gui.pixmaps.pix_ok)
      else:
         self.pix.setPixmap(cfg_visao_gui.cfg_gui.pixmaps.pix_ko)
        