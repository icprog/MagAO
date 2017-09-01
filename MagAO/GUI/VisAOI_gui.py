#!/usr/bin/env python
#
#+File: VisAOI_gui.py
#
# Shutter remote control
#
#-

import os, time
from qt import *
#import cfg_visao_gui
from AdOpt.WfsGUI import cfg_gui

from AdOpt import AOConstants, cfg

# Import the C library wrapper

from AdOpt.wrappers import msglib

#+Class: shutter_gui
#cfg
# Main window for the GUI tool
#-

class VisAOI_gui( QDialog):
    
   def __init__(self, AOwindow):
      '''
      Members to set before calling this constructor:
      self.prefix
      self.name
      self.unit
      '''

 
      QDialog.__init__(self)

      #cfg_visao_gui.init()
      cfg_gui.init()

      self.shutter_prefix = "shutterremote"
      self.name = "VisAO Remote Control"

      
      self.varname_shutter_status = cfg.varname( self.shutter_prefix, "STATUS")
      self.varname_shutter_curstate = cfg.varname( self.shutter_prefix, "STATE.CUR")
      self.varname_shutter_reqstate = cfg.varname( self.shutter_prefix, "STATE.REQ")
      self.varname_shutter_curctype = cfg.varname( self.shutter_prefix, "ConMode.CUR")
      self.varname_shutter_reqctype = cfg.varname( self.shutter_prefix, "ConMode.REQ")
      
      self.shutter_curctype = 0
      self.ButtonGroup = QButtonGroup()

     ## Draw the widgets

      self.setMinimumSize(QSize(460,560))
 
      self.layout = QGridLayout( self)#, 4)
      self.botspace = QSpacerItem(1,1)
      self.layout.addMultiCell(self.botspace, 23,23,0,9)
      
      self.titlelabel = QLabel("VisAO Remote Control", self)
          
      self.layout.addMultiCellWidget(self.titlelabel, 0,0,0,9, Qt.AlignHCenter)
                      
      sh_row = 2 #The starting row of the shutter panel
      self.shutter_title = QLabel("Shutter", self)
      qf = self.shutter_title.font()
      qf.setBold(True)
      self.shutter_title.setFont(qf)
      self.shutter_curstatus = QLabel("unknown", self)
      self.shutter_pix = QPushButton("", self)
      self.layout.addMultiCellWidget(self.shutter_title, sh_row,sh_row,0,2, Qt.AlignHCenter)
      self.layout.addMultiCellWidget(self.shutter_curstatus, sh_row+1,sh_row+1,1,2, Qt.AlignHCenter)
      self.layout.addMultiCellWidget(self.shutter_pix, sh_row+1,sh_row+1, 0,0,)
      
      self.shutter_controlLabel = QLabel("Control:", self)#, self.group1)#L)
      self.shutter_curCMode = QLabel("unknown", self)#, self.group1)#L)
      
      self.layout.addMultiCellWidget(self.shutter_controlLabel, sh_row+1,sh_row+1,4,5, Qt.AlignHCenter)
      self.layout.addMultiCellWidget(self.shutter_curCMode, sh_row,sh_row,4,8, Qt.AlignHCenter)
      
      self.shutter_remoteButton =   QPushButton("    Take Remote Control    ", self)#, self.group1R);
      self.layout.addMultiCellWidget(self.shutter_remoteButton, sh_row+1,sh_row+1,6,8)
      self.connect( self.shutter_remoteButton, SIGNAL("clicked()"), self.request_Shutter_remote)
      
      self.shutter_overrideCheck = QCheckBox("Override", self)#, self.group1R);
      self.layout.addMultiCellWidget(self.shutter_overrideCheck, sh_row+2,sh_row+2,6,8, Qt.AlignHCenter)
      self.connect( self.shutter_overrideCheck, SIGNAL("clicked()"), self.override_Shutter_red)
      
      self.shutter_curstateLabel = QLabel("Cur State:", self)
      self.shutter_curstate = QLabel("unknown", self)
      self.layout.addWidget(self.shutter_curstateLabel, sh_row+3,1)
      self.layout.addMultiCellWidget(self.shutter_curstate, sh_row+3, sh_row+3,2,3, Qt.AlignHCenter)
      
      self.openButton = QPushButton("Open", self);
      self.connect( self.openButton, SIGNAL("clicked()"), self.open_shutter)
      self.layout.addMultiCellWidget(self.openButton, sh_row+3,sh_row+3, 4,6)
      self.closeButton = QPushButton("Close", self)
      self.connect( self.closeButton, SIGNAL("clicked()"), self.close_shutter)
      self.layout.addMultiCellWidget(self.closeButton, sh_row+3,sh_row+3,7,8)
      
#Wollaston ==================================
      self.wollaston_prefix = "wollastonstatus"
      self.wollaston_curctype = 0

      self.varname_wollaston_status = cfg.varname( self.wollaston_prefix, "STATUS")
      self.varname_wollaston_curstate = cfg.varname( self.wollaston_prefix, "state.CUR")
      self.varname_wollaston_reqstate = cfg.varname( self.wollaston_prefix, "state.REQ")
      self.varname_wollaston_curctype = cfg.varname( self.wollaston_prefix, "ConMode.CUR")
      self.varname_wollaston_reqctype = cfg.varname( self.wollaston_prefix, "ConMode.REQ")


      wl_row = 17 #The starting row of the wollaston panel
      self.wollaston_title = QLabel("Wollaston", self)
      qf = self.wollaston_title.font()
      qf.setBold(True)
      self.wollaston_title.setFont(qf)
      self.wollaston_curstatus = QLabel("unknown", self)
      self.wollaston_pix = QPushButton("", self)
      self.layout.addMultiCellWidget(self.wollaston_title, wl_row,wl_row,0,2, Qt.AlignHCenter)
      self.layout.addMultiCellWidget(self.wollaston_curstatus, wl_row+1,wl_row+1,1,2, Qt.AlignHCenter)
      self.layout.addMultiCellWidget(self.wollaston_pix, wl_row+1,wl_row+1, 0,0,)


      self.wollaston_controlLabel = QLabel("Control:", self)#, self.group1)#L)
      self.wollaston_curCMode = QLabel("unknown", self)#, self.group1)#L)
      
      self.layout.addMultiCellWidget(self.wollaston_controlLabel, wl_row+1,wl_row+1,4,5, Qt.AlignHCenter)
      self.layout.addMultiCellWidget(self.wollaston_curCMode, wl_row,wl_row,4,8, Qt.AlignHCenter)
      
      self.wollaston_remoteButton =   QPushButton("    Take Remote Control    ", self)#, self.group1R);
      self.layout.addMultiCellWidget(self.wollaston_remoteButton, wl_row+1,wl_row+1,6,8)
      self.connect( self.wollaston_remoteButton, SIGNAL("clicked()"), self.request_wollaston_remote)
      
      self.wollaston_overrideCheck = QCheckBox("Override", self)#, self.group1R);
      self.layout.addMultiCellWidget(self.wollaston_overrideCheck, wl_row+2,wl_row+2,6,8, Qt.AlignHCenter)
      self.connect( self.wollaston_overrideCheck, SIGNAL("clicked()"), self.override_wollaston_red)
      
      self.wollaston_curstateLabel = QLabel("Cur State:", self)
      self.wollaston_curstate = QLabel("unknown", self)
      self.layout.addWidget(self.wollaston_curstateLabel, wl_row+3,1)
      self.layout.addMultiCellWidget(self.wollaston_curstate, wl_row+3, wl_row+3,2,3, Qt.AlignHCenter)
      
      self.upButton = QPushButton("Up", self);
      self.connect( self.upButton, SIGNAL("clicked()"), self.up_wollaston)
      self.layout.addMultiCellWidget(self.upButton, wl_row+3,wl_row+3, 4,6)
      self.downButton = QPushButton("Down", self)
      self.connect( self.downButton, SIGNAL("clicked()"), self.down_wollaston)
      self.layout.addMultiCellWidget(self.downButton, wl_row+3,wl_row+3,7,8)


#Focus ==================================
      self.focus_prefix = "focusmotor"
      self.focus_curctype = 0
      self.focus_status = 0
      
      self.varname_focus_status = cfg.varname( self.focus_prefix, "STATUS")
      self.varname_focus_curpos = cfg.varname( self.focus_prefix, "pos.CUR")
      self.varname_focus_reqpos = cfg.varname( self.focus_prefix, "pos.REQ")
      self.varname_focus_limsw = cfg.varname( self.focus_prefix, "limsw.CUR")
      self.varname_focus_reqabort = cfg.varname( self.focus_prefix, "abort.REQ")
      self.varname_focus_curctype = cfg.varname( self.focus_prefix, "ConMode.CUR")
      self.varname_focus_reqctype = cfg.varname( self.focus_prefix, "ConMode.REQ")
      self.varname_focus_preset = cfg.varname( self.focus_prefix, "preset.REQ")

      foc_row = 9 #The starting row of the focus panel
      self.focus_title = QLabel("Focus", self)
      qf = self.focus_title.font()
      qf.setBold(True)
      self.focus_title.setFont(qf)
      self.focus_curstatus = QLabel("unknown", self)
      self.focus_pix = QPushButton("", self)
      self.layout.addMultiCellWidget(self.focus_title, foc_row,foc_row,0,2, Qt.AlignHCenter)
      self.layout.addMultiCellWidget(self.focus_curstatus, foc_row+1,foc_row+1,1,2, Qt.AlignHCenter)
      self.layout.addMultiCellWidget(self.focus_pix, foc_row+1,foc_row+1, 0,0,)


      self.focus_controlLabel = QLabel("Control:", self)#, self.group1)#L)
      self.focus_curCMode = QLabel("unknown", self)#, self.group1)#L)
      
      self.layout.addMultiCellWidget(self.focus_controlLabel, foc_row+1,foc_row+1,4,5, Qt.AlignHCenter)
      self.layout.addMultiCellWidget(self.focus_curCMode, foc_row,foc_row,4,8, Qt.AlignHCenter)
      
      self.focus_remoteButton =   QPushButton("    Take Remote Control    ", self)#, self.group1R);
      self.layout.addMultiCellWidget(self.focus_remoteButton, foc_row+1,foc_row+1,6,8)
      self.connect( self.focus_remoteButton, SIGNAL("clicked()"), self.request_focus_remote)
      

      self.focus_overrideCheck = QCheckBox("Override", self)#, self.group1R);
      self.layout.addMultiCellWidget(self.focus_overrideCheck, foc_row+2,foc_row+2,6,8, Qt.AlignHCenter)
      self.connect( self.focus_overrideCheck, SIGNAL("clicked()"), self.override_focus_red)

      self.focus_limswStatus = QLabel("Front Limit", self)
      self.layout.addMultiCellWidget(self.focus_limswStatus, foc_row+2, foc_row+3,0,3, Qt.AlignHCenter)
      
      self.focus_presetGo = QPushButton("Go To Preset", self)
      self.layout.addMultiCellWidget(self.focus_presetGo, foc_row+3, foc_row+3, 2,5)
      self.connect( self.focus_presetGo, SIGNAL("clicked()"), self.presetGo)
      
      self.focus_posLabel = QLabel("Position:", self)
      self.focus_posValue = QLabel("-00000.0000 um", self)
      self.layout.addMultiCellWidget(self.focus_posLabel, foc_row+5, foc_row+5, 1,1)
      self.layout.addMultiCellWidget(self.focus_posValue, foc_row+5, foc_row+5, 2,3, Qt.AlignHCenter)

      self.focus_newPos = QLineEdit("", self)
      self.focus_newPosGo = QPushButton("Go", self)
      self.focus_Abort = QPushButton("Abort", self)
      self.layout.addMultiCellWidget(self.focus_newPos, foc_row+5, foc_row+5, 5,8)
      self.layout.addMultiCellWidget(self.focus_newPosGo, foc_row+6, foc_row+6, 5,6)
      self.layout.addMultiCellWidget(self.focus_Abort, foc_row+6, foc_row+6, 7,8)

      self.connect( self.focus_newPosGo, SIGNAL("clicked()"), self.focusGo)
      self.connect( self.focus_Abort, SIGNAL("clicked()"), self.focusAbort)
      
        # register slots

      AOwindow.Notify( self.varname_shutter_status, self.slotNotify, getNow = True);
      AOwindow.Notify( self.varname_shutter_curstate, self.slotNotify, getNow = True);
      AOwindow.Notify( self.varname_shutter_curctype, self.slotNotify, getNow = True);
        
      AOwindow.Notify( self.varname_wollaston_status, self.slotNotify, getNow = True);
      AOwindow.Notify( self.varname_wollaston_curctype, self.slotNotify, getNow = True);
      AOwindow.Notify( self.varname_wollaston_curstate, self.slotNotify, getNow = True);

      AOwindow.Notify( self.varname_focus_status, self.slotNotify, getNow = True);
      AOwindow.Notify( self.varname_focus_curctype, self.slotNotify, getNow = True);
      AOwindow.Notify( self.varname_focus_curpos, self.slotNotify, getNow = True);
      AOwindow.Notify( self.varname_focus_limsw, self.slotNotify, getNow = True);
      
    ## +Method: slotNotify
    ##

   def slotNotify(self, args):
      (message) = args
      varname = message['name']
      value = message['value']

        ## When one of the watched variables change, update our display

      if varname == self.varname_shutter_status:
            self.shutter_curstatus.setText( AOConstants.AOStatesType[ value])
            self.enableShutterButtons(value)
            self.set_shutterPix(value)

      elif varname == self.varname_shutter_curstate:
            self.set_shutterNewState( value)

      elif varname == self.varname_shutter_curctype:
            self.set_shutter_curctype( value)

      elif varname == self.varname_wollaston_status:
            self.wollaston_curstatus.setText( AOConstants.AOStatesType[ value])
            self.enableWollastonButtons(value)
            self.set_wollastonPix(value)
            
      elif varname == self.varname_wollaston_curstate:
            self.set_wollastonNewState( value)
            
      elif varname == self.varname_wollaston_curctype:
            self.set_wollaston_curctype( value)

      elif varname == self.varname_focus_status:
            self.focus_status = value
            self.focus_curstatus.setText( AOConstants.AOStatesType[ value])
            self.enablefocusButtons(value)
            self.set_focusPix(value)
            
      elif varname == self.varname_focus_curctype:
            self.set_focus_curctype( value)
         
      elif varname == self.varname_focus_curpos:
        if self.focus_status != AOConstants.AOStatesCode['STATE_NOCONNECTION']:
          self.focus_posValue.setText( "%0.4f um" % value )
        else:
          self.focus_posValue.setText("?")
          
      elif varname == self.varname_focus_limsw:
           self.set_focus_limsw(value)

           
   def set_shutterNewState(self, value):

      if value == 1:
         text = "OPEN"
         
      elif value == -1:
         text = "CLOSED"
        
      else:
         text = "UNKOWN"
         
               
      self.shutter_curstate.setText( text)
         
        

        
   def set_shutter_curctype(self, value):
      self.shutter_curctype = value
      
      if self.shutter_overrideCheck.isChecked() == 1:
           buttext = "***Take Remote Control***"
      else:
           buttext = "   Take Remote Control   "
      self.shutter_overrideCheck.setEnabled(1)
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
               
               
      self.shutter_curCMode.setText( text)
      self.shutter_remoteButton.setText(buttext)
      self.shutter_overrideCheck.setEnabled(checken)
        
   def open_shutter(self):
        result = msglib.SetIntVar( self.varname_shutter_reqstate, 1)

   def close_shutter(self):
        result = msglib.SetIntVar( self.varname_shutter_reqstate, -1)
         
   def request_Shutter_remote(self):
        if self.shutter_curctype != 1:
           if self.shutter_overrideCheck.isChecked() == 1:
              ct = 10
           else:
              ct = 1
           
           result = msglib.SetIntVar( self.varname_shutter_reqctype, ct)
           self.shutter_overrideCheck.setChecked(0)
        else:
           result = msglib.SetIntVar( self.varname_shutter_reqctype, 0)
        #self.shutter_remoteButton.setText("   Take Remote Control   ")
        
   def override_Shutter_red(self):
         
       if self.shutter_overrideCheck.isChecked() == 1:
           result = self.shutter_remoteButton.setText("***Take Remote Control***")
       else:
           result = self.shutter_remoteButton.setText("   Take Remote Control   ")
             
           
   def enableShutterButtons(self, status):
      if status == AOConstants.AOStatesCode['STATE_READY']:
         self.openButton.setEnabled(True)
         self.closeButton.setEnabled(True)
      else:
         self.openButton.setEnabled(False)
         self.closeButton.setEnabled(False)
            
   def set_shutterPix(self, status):
      if status == AOConstants.AOStatesCode['STATE_READY']:
         self.shutter_pix.setPixmap(cfg_gui.pixmaps.pix_ok)
      elif status == AOConstants.AOStatesCode['STATE_OPERATING']:
         self.shutter_pix.setPixmap(cfg_gui.pixmaps.pix_working)
      else:
         self.shutter_pix.setPixmap(cfg_gui.pixmaps.pix_ko)
         
#====================== Wollaston ==================================
   def up_wollaston(self):
        result = msglib.SetIntVar( self.varname_wollaston_reqstate, 1)

   def down_wollaston(self):
        result = msglib.SetIntVar( self.varname_wollaston_reqstate, -1)
         
   def request_wollaston_remote(self):
        if self.wollaston_curctype != 1:
           if self.wollaston_overrideCheck.isChecked() == 1:
              ct = 10
           else:
              ct = 1
           
           result = msglib.SetIntVar( self.varname_wollaston_reqctype, ct)
           self.wollaston_overrideCheck.setChecked(0)
        else:
           result = msglib.SetIntVar( self.varname_wollaston_reqctype, 0)
       
   def override_wollaston_red(self):
         
       if self.wollaston_overrideCheck.isChecked() == 1:
           result = self.wollaston_remoteButton.setText("***Take Remote Control***")
       else:
           result = self.wollaston_remoteButton.setText("   Take Remote Control   ")
           
   def set_wollaston_curctype(self, value):
     
      self.wollaston_curctype = value
      
      if self.wollaston_overrideCheck.isChecked() == 1:
           buttext = "***Take Remote Control***"
      else:
           buttext = "   Take Remote Control   "
      self.wollaston_overrideCheck.setEnabled(1)
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
               
      self.wollaston_curCMode.setText( text)
      self.wollaston_remoteButton.setText(buttext)
      self.wollaston_overrideCheck.setEnabled(checken)
      
   def set_wollastonNewState(self, value):
      if value == 1:
         text = "UP"
      elif value == -1:
         text = "DOWN"
      elif value == 0:
         text = "INTERMEDIATE"
      elif value == 11:
         text = "UP"
         if self.wollaston_curctype == 1 or self.wollaston_curctype == 0:
            answer = QMessageBox.question( self, "VisAO Wollaston", "You are in a non-SDI filter (F/W 3) but wollaston status is UP.", "DOWN", "Ignore","",1)
            if answer == 1:
               if self.wollaston_curctype == 0:
                 msglib.SetIntVar(self.varname_wollaston_reqctype, 1)
                 time.sleep(1)
               result = msglib.SetIntVar( self.varname_wollaston_reqstate, -2) #tells wollaston to ignore this
            if answer == 0:
               if self.wollaston_curctype == 0:
                 msglib.SetIntVar(self.varname_wollaston_reqctype, 1)
                 time.sleep(1)
               result = msglib.SetIntVar( self.varname_wollaston_reqstate, -1)

      elif value == 9:
         text = "DOWN"
         if self.wollaston_curctype == 1 or self.wollaston_curctype == 0:
            answer = QMessageBox.question( self, "VisAO Wollaston", "You are in an SDI filter (F/W 3) but wollaston status is DOWN.", "UP", "Ignore","",1)
            if answer == 1:
                if self.wollaston_curctype == 0:
                   msglib.SetIntVar(self.varname_wollaston_reqctype, 1)
                   time.sleep(1)
                result = msglib.SetIntVar( self.varname_wollaston_reqstate, -2) #tells wollaston to ignore this 
            if answer == 0:
                if self.wollaston_curctype == 0:
                   msglib.SetIntVar(self.varname_wollaston_reqctype, 1)
                   time.sleep(1)
                result = msglib.SetIntVar( self.varname_wollaston_reqstate, 1)
      elif value == 10:
         text = "INTERMEDIATE"
         if self.wollaston_curctype == 1 or self.wollaston_curctype == 0:
            answer = QMessageBox.question( self, "VisAO Wollaston", "Wollaston status is INTERMEDIATE.", "UP", "DOWN", "Ignore",2)
            if answer == 2:
                if self.wollaston_curctype == 0:
                   msglib.SetIntVar(self.varname_wollaston_reqctype, 1)
                   time.sleep(1)
                result = msglib.SetIntVar( self.varname_wollaston_reqstate, -2) #tells wollaston to ignore this 
            if answer == 0:
                if self.wollaston_curctype == 0:
                   msglib.SetIntVar(self.varname_wollaston_reqctype, 1)
                   time.sleep(1)
                result = msglib.SetIntVar( self.varname_wollaston_reqstate, 1)
            if answer == 1:
                if self.wollaston_curctype == 0:
                   msglib.SetIntVar(self.varname_wollaston_reqctype, 1)
                   time.sleep(1)
                result = msglib.SetIntVar( self.varname_wollaston_reqstate, -1)
      else:
         text = "UNKOWN"
                  
      self.wollaston_curstate.setText( text)

   def enableWollastonButtons(self, status):
      if status == AOConstants.AOStatesCode['STATE_READY']:
         self.upButton.setEnabled(True)
         self.downButton.setEnabled(True)
      else:
         self.upButton.setEnabled(False)
         self.downButton.setEnabled(False)
            
   def set_wollastonPix(self, status):
      if status == AOConstants.AOStatesCode['STATE_READY']:
         self.wollaston_pix.setPixmap(cfg_gui.pixmaps.pix_ok)
      else:
         self.wollaston_pix.setPixmap(cfg_gui.pixmaps.pix_ko)

#=================== Focus

   def request_focus_remote(self):
        if self.focus_curctype != 1:
           if self.focus_overrideCheck.isChecked() == 1:
              ct = 10
           else:
              ct = 1
           
           result = msglib.SetIntVar( self.varname_focus_reqctype, ct)
           self.focus_overrideCheck.setChecked(0)
        else:
           result = msglib.SetIntVar( self.varname_focus_reqctype, 0)
       
   def override_focus_red(self):
         
       if self.focus_overrideCheck.isChecked() == 1:
           result = self.focus_remoteButton.setText("***Take Remote Control***")
       else:
           result = self.focus_remoteButton.setText("   Take Remote Control   ")
           
   def set_focus_curctype(self, value):
     
      self.focus_curctype = value
      
      if self.focus_overrideCheck.isChecked() == 1:
           buttext = "***Take Remote Control***"
      else:
           buttext = "   Take Remote Control   "
      self.focus_overrideCheck.setEnabled(1)
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
               
      self.focus_curCMode.setText( text)
      self.focus_remoteButton.setText(buttext)
      self.focus_overrideCheck.setEnabled(checken)

   def enablefocusButtons(self, status):
      if status == AOConstants.AOStatesCode['STATE_READY']:
         #self.upButton.setEnabled(True)
         #self.downButton.setEnabled(True)
         ret = 1
      else:
         #self.upButton.setEnabled(False)
         #self.downButton.setEnabled(False)
         ret = 0
         
   def set_focusPix(self, status):
      if status == AOConstants.AOStatesCode['STATE_READY']:
         self.focus_pix.setPixmap(cfg_gui.pixmaps.pix_ok)
      elif status == AOConstants.AOStatesCode['STATE_OPERATING'] or status == AOConstants.AOStatesCode['STATE_HOMING']:
         self.focus_pix.setPixmap(cfg_gui.pixmaps.pix_working)
      else:
         self.focus_pix.setPixmap(cfg_gui.pixmaps.pix_ko)

   def set_focus_limsw(self, limsw):
      if limsw == 0:
        self.focus_limswStatus.setText("")
      elif limsw == -1:
        self.focus_limswStatus.setText("Negative Limit")
      elif limsw == 1:
        self.focus_limswStatus.setText("Positive Limit")
      elif limsw == -5:
        self.focus_limswStatus.setText("Positive Limit Disabled!")
      elif limsw == -10:
         if self.focus_status != AOConstants.AOStatesCode['STATE_OFF']:
            self.focus_limswStats.setText("Limit Switch Error")
            
   def focusGo(self):

        if(self.focus_newPos.text().contains(' ') or self.focus_newPos.text().contains('\t') or self.focus_newPos.text().length() == 0):
            QMessageBox.warning(self, 'Invalid position', 'The requested position is not valid.', QMessageBox.Ok, QMessageBox.NoButton)
            return
        tgtpos = float(self.focus_newPos.text().latin1())
 
        if not self.isTgtposValid(tgtpos):
            QMessageBox.warning(self, 'Invalid position', 'The requested position is not valid.', QMessageBox.Ok, QMessageBox.NoButton)
            return

        result = msglib.SetRealVar( self.varname_focus_reqpos, tgtpos)

   def presetGo(self):

        result = msglib.SetRealVar( self.varname_focus_preset, 1)
        
   def isTgtposValid(self, pos):
        valid = True
        if(pos == 0.0 and (self.focus_newPos.text().contains(' ') or self.focus_newPos.text().contains('\t'))):
          valid = False
        return valid
     
   def focusAbort(self):
      result = msglib.SetIntVar( self.varname_focus_reqabort, 1)