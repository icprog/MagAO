#!/usr/bin/env python
#
#+File: shutter_gui.py
#
# Shutter remote control
#
#-

import os
from qt import *
from AdOpt import cfg_gui

from AdOpt import AOConstants, cfg

# Import the C library wrapper

from AdOpt.wrappers import msglib

#+Class: shutter_gui
#
# Main window for the GUI tool
#-

class VisAOPwrCtrl_gui( QDialog):
    
   def __init__(self, AOwindow):
      '''
      Members to set before calling this constructor:
      self.prefix
      self.name
      self.unit
      '''
      self.prefix = "visaopwrctrl"
      self.name = "VisAO Power Control"
      self.unit = "VisAO"
 
      QDialog.__init__(self)

      cfg_gui.init()

      
      self.varname_status = cfg.varname( self.prefix,"STATUS")
      
      self.varname_outlet1_cur = cfg.varname( self.prefix, "OUTLET1.CUR")
      self.varname_outlet1_req = cfg.varname( self.prefix, "OUTLET1.REQ")
      self.varname_outlet2_cur = cfg.varname( self.prefix, "OUTLET2.CUR")
      self.varname_outlet2_req = cfg.varname( self.prefix, "OUTLET2.REQ")
      self.varname_outlet3_cur = cfg.varname( self.prefix, "OUTLET3.CUR")
      self.varname_outlet3_req = cfg.varname( self.prefix, "OUTLET3.REQ")
      self.varname_outlet4_cur = cfg.varname( self.prefix, "OUTLET4.CUR")
      self.varname_outlet4_req = cfg.varname( self.prefix, "OUTLET4.REQ")
      self.varname_outlet5_cur = cfg.varname( self.prefix, "OUTLET5.CUR")
      self.varname_outlet5_req = cfg.varname( self.prefix, "OUTLET5.REQ")
      self.varname_outlet6_cur = cfg.varname( self.prefix, "OUTLET6.CUR")
      self.varname_outlet6_req = cfg.varname( self.prefix, "OUTLET6.REQ")
      self.varname_outlet7_cur = cfg.varname( self.prefix, "OUTLET7.CUR")
      self.varname_outlet7_req = cfg.varname( self.prefix, "OUTLET7.REQ")
      self.varname_outlet8_cur = cfg.varname( self.prefix, "OUTLET8.CUR")
      self.varname_outlet8_req = cfg.varname( self.prefix, "OUTLET8.REQ")
      
      self.varname_curctype = cfg.varname( self.prefix, "ConMode.CUR")
      self.varname_reqctype = cfg.varname( self.prefix, "ConMode.REQ")
        
      self.curctype = 0
      self.ButtonGroup = QButtonGroup()

      self.outletnames = {}
      
      try:
         for n in range(8):
            self.outletnames[n] = cfg.cfg[self.prefix]['outlet%dName' % (n+1)].Value() 
      except:
         # May happen if no custom pos. are defined
         print "VisAOPwrCtrl_gui: no outlet names processed.\n"
         pass
            
     ## Draw the widgets

      self.setMinimumSize(QSize(460,360))
 
      self.layout = QGridLayout( self)#, 4)
      self.botspace = QSpacerItem(1,1)
      self.layout.addMultiCell(self.botspace, 16,10,0,6)
      
      self.titlelabel = QLabel("VisAO Power Control", self)
          
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
      
      
      self.outlet1Check = QCheckBox("J1", self)
      self.connect( self.outlet1Check, SIGNAL("clicked()"), self.outlet1Clicked)
      self.layout.addWidget(self.outlet1Check, 6,1)
      if(len(self.outletnames) > 0):
         self.labelO1 = QLabel(self.outletnames[0], self)
         self.layout.addWidget(self.labelO1, 6, 2)
      
      self.outlet2Check = QCheckBox("J2", self)
      self.connect( self.outlet2Check, SIGNAL("clicked()"), self.outlet2Clicked)
      self.layout.addWidget(self.outlet2Check, 7,1)
      if(len(self.outletnames) > 1):
         self.labelO2 = QLabel(self.outletnames[1], self)
         self.layout.addWidget(self.labelO2, 7, 2)
         
      self.outlet3Check = QCheckBox("J3", self)
      self.connect( self.outlet3Check, SIGNAL("clicked()"), self.outlet3Clicked)
      self.layout.addWidget(self.outlet3Check, 8,1)
      if(len(self.outletnames) > 2):
         self.labelO3 = QLabel(self.outletnames[2], self)
         self.layout.addWidget(self.labelO3, 8, 2)
         
      self.outlet4Check = QCheckBox("J4", self)
      self.connect( self.outlet4Check, SIGNAL("clicked()"), self.outlet4Clicked)
      self.layout.addWidget(self.outlet4Check, 9,1)
      if(len(self.outletnames) > 3):
         self.labelO4 = QLabel(self.outletnames[3], self)
         self.layout.addWidget(self.labelO4, 9, 2)
         
      self.outlet5Check = QCheckBox("J5", self)
      self.connect( self.outlet5Check, SIGNAL("clicked()"), self.outlet5Clicked)
      self.layout.addWidget(self.outlet5Check, 10,1)
      if(len(self.outletnames) > 4):
         self.labelO5 = QLabel(self.outletnames[4], self)
         self.layout.addWidget(self.labelO5, 10, 2)
         
      self.outlet6Check = QCheckBox("J6", self)
      self.connect( self.outlet6Check, SIGNAL("clicked()"), self.outlet6Clicked)
      self.layout.addWidget(self.outlet6Check, 11,1)     
      if(len(self.outletnames) > 0):
         self.labelO6 = QLabel(self.outletnames[5], self)
         self.layout.addWidget(self.labelO6, 11, 2)
         
      self.outlet7Check = QCheckBox("J7", self)
      self.connect( self.outlet7Check, SIGNAL("clicked()"), self.outlet7Clicked)
      self.layout.addWidget(self.outlet7Check, 12,1)
      if(len(self.outletnames) > 0):
         self.labelO7 = QLabel(self.outletnames[6], self)
         self.layout.addWidget(self.labelO7, 12, 2)
         
      self.outlet8Check = QCheckBox("J8", self)
      self.connect( self.outlet8Check, SIGNAL("clicked()"), self.outlet8Clicked)
      self.layout.addWidget(self.outlet8Check, 13,1)
      if(len(self.outletnames) > 0):
         self.labelO8 = QLabel(self.outletnames[7], self)
         self.layout.addWidget(self.labelO8, 13, 2)
      
      self.labelBuff = QLabel(" ", self)
      self.layout.addWidget(self.labelBuff, 14, 1)
      
        # register slots

      AOwindow.Notify( self.varname_status, self.slotNotify, getNow = True);
      AOwindow.Notify( self.varname_outlet1_cur, self.slotNotify, getNow = True);
      AOwindow.Notify( self.varname_outlet2_cur, self.slotNotify, getNow = True);
      AOwindow.Notify( self.varname_outlet3_cur, self.slotNotify, getNow = True);
      AOwindow.Notify( self.varname_outlet4_cur, self.slotNotify, getNow = True);
      AOwindow.Notify( self.varname_outlet5_cur, self.slotNotify, getNow = True);
      AOwindow.Notify( self.varname_outlet6_cur, self.slotNotify, getNow = True);
      AOwindow.Notify( self.varname_outlet7_cur, self.slotNotify, getNow = True);
      AOwindow.Notify( self.varname_outlet8_cur, self.slotNotify, getNow = True);
        
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
            
      elif varname == self.varname_outlet1_cur:
            self.setNewState( 1, value)
            
      elif varname == self.varname_outlet2_cur:
            self.setNewState( 2, value)

      elif varname == self.varname_outlet3_cur:
            self.setNewState( 3, value)

      elif varname == self.varname_outlet4_cur:
            self.setNewState( 4, value)

      elif varname == self.varname_outlet5_cur:
            self.setNewState( 5, value)

      elif varname == self.varname_outlet6_cur:
            self.setNewState( 6, value)

      elif varname == self.varname_outlet7_cur:
            self.setNewState( 7, value)
            
      elif varname == self.varname_outlet8_cur:
            self.setNewState( 8, value)
            
      elif varname == self.varname_curctype:
            self.setcurctype( value)
            
   def setNewState(self, on, value):

      if on == 1:
         if value == 1:
            self.outlet1Check.setChecked(1)            
         else:
            self.outlet1Check.setChecked(0)
      if on == 2:
         if value == 1:
            self.outlet2Check.setChecked(1)            
         else:
            self.outlet2Check.setChecked(0)
      if on == 3:
         if value == 1:
            self.outlet3Check.setChecked(1)            
         else:
            self.outlet3Check.setChecked(0)
      if on == 4:
         if value == 1:
            self.outlet4Check.setChecked(1)            
         else:
            self.outlet4Check.setChecked(0)
      if on == 5:
         if value == 1:
            self.outlet5Check.setChecked(1)            
         else:
            self.outlet5Check.setChecked(0)
      if on == 6:
         if value == 1:
            self.outlet6Check.setChecked(1)            
         else:
            self.outlet6Check.setChecked(0)
      if on == 7:
         if value == 1:
            self.outlet7Check.setChecked(1)            
         else:
            self.outlet7Check.setChecked(0)
      if on == 8:
         if value == 1:
            self.outlet8Check.setChecked(1)            
         else:
            self.outlet8Check.setChecked(0)     
         
        
   
        
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
        
   def setOutletState(self, on, os):
      
      if on == 1:
         result = msglib.SetIntVar( self.varname_outlet1_req, os)
      if on == 2:
         result = msglib.SetIntVar( self.varname_outlet2_req, os)
      if on == 3:
         result = msglib.SetIntVar( self.varname_outlet3_req, os)
      if on == 4:
         result = msglib.SetIntVar( self.varname_outlet4_req, os)
      if on == 5:
         result = msglib.SetIntVar( self.varname_outlet5_req, os)
      if on == 6:
         result = msglib.SetIntVar( self.varname_outlet6_req, os)
      if on == 7:
         result = msglib.SetIntVar( self.varname_outlet7_req, os)
      if on == 8:
         result = msglib.SetIntVar( self.varname_outlet8_req, os) 
         
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
           
   def outlet1Clicked(self):
      st = self.outlet1Check.isChecked()
      self.setOutletState(1,st)
            
   def outlet2Clicked(self):
      st = self.outlet2Check.isChecked()
      self.setOutletState(2,st)
      
   def outlet3Clicked(self):
      st = self.outlet3Check.isChecked()
      self.setOutletState(3,st)
      
   def outlet4Clicked(self):
      st = self.outlet4Check.isChecked()
      self.setOutletState(4,st)
      
   def outlet5Clicked(self):
      st = self.outlet5Check.isChecked()
      self.setOutletState(5,st)
      
   def outlet6Clicked(self):
      st = self.outlet6Check.isChecked()
      self.setOutletState(6,st)
      
   def outlet7Clicked(self):
      st = self.outlet7Check.isChecked()
      self.setOutletState(7,st)
      
   def outlet8Clicked(self):
      st = self.outlet8Check.isChecked()
      self.setOutletState(8,st)   
           
   def enableButtons(self, status):
      if status == AOConstants.AOStatesCode['STATE_OPERATING']:
         self.outlet7Check.setEnabled(True)
         self.outlet8Check.setEnabled(True)
      else:
         self.outlet7Check.setEnabled(False)
         self.outlet8Check.setEnabled(False)
            
   
        