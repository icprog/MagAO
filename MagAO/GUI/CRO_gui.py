#!/usr/bin/env python
#
#+File: CRO_gui.py
#
# CRO remote control
#
#-

import os
from qt import *
from AdOpt.WfsGUI import cfg_gui

from AdOpt import AOConstants, cfg

# Import the C library wrapper

from AdOpt.wrappers import msglib



class CRO_gui( QDialog):
    
   def __init__(self, AOwindow):
      '''
      Members to set before calling this constructor:
      self.prefix
      self.name
      self.unit
      '''
      self.prefix = "crocontroller"
      self.name = "CRO Remote Control"
      self.unit = "CRO"

      self.dstep = 500.
      
      QDialog.__init__(self)

      cfg_gui.init()

      msgdAddress = ''
      
      if os.environ['ADOPT_SUBSYSTEM'] == 'WFS':
         msgdAddress = '@M_ADSEC'
      
      self.varname_status = cfg.varname( self.prefix,"STATUS%s" % msgdAddress)
      
      self.A1_stat_cur = cfg.varname( self.prefix, "A1_stat.CUR%s" % msgdAddress)
      self.A1_pos_cur = cfg.varname( self.prefix, "A1_pos.CUR%s" % msgdAddress)
      self.A1_dpos_req = cfg.varname( self.prefix, "A1_dpos.REQ%s" % msgdAddress)
      
      self.B1_stat_cur = cfg.varname( self.prefix, "B1_stat.CUR%s" % msgdAddress)
      self.B1_pos_cur = cfg.varname( self.prefix, "B1_pos.CUR%s" % msgdAddress)
      self.B1_dpos_req = cfg.varname( self.prefix, "B1_dpos.REQ%s" % msgdAddress)

      self.C1_stat_cur = cfg.varname( self.prefix, "C1_stat.CUR%s" % msgdAddress)
      self.C1_pos_cur = cfg.varname( self.prefix, "C1_pos.CUR%s" % msgdAddress)
      self.C1_dpos_req = cfg.varname( self.prefix, "C1_dpos.REQ%s" % msgdAddress)

      self.A2_stat_cur = cfg.varname( self.prefix, "A2_stat.CUR%s" % msgdAddress)
      self.A2_pos_cur = cfg.varname( self.prefix, "A2_pos.CUR%s" % msgdAddress)
      self.A2_dpos_req = cfg.varname( self.prefix, "A2_dpos.REQ%s" % msgdAddress)

      self.B2_stat_cur = cfg.varname( self.prefix, "B2_stat.CUR%s" % msgdAddress)
      self.B2_pos_cur = cfg.varname( self.prefix, "B2_pos.CUR%s" % msgdAddress)
      self.B2_dpos_req = cfg.varname( self.prefix, "B2_dpos.REQ%s" % msgdAddress)

      self.X_stat_cur = cfg.varname( self.prefix, "X_stat.CUR%s" % msgdAddress)
      self.X_pos_cur = cfg.varname( self.prefix, "X_pos.CUR%s" % msgdAddress)
      self.X_dpos_req = cfg.varname( self.prefix, "X_dpos.REQ%s" % msgdAddress)

      self.Y_stat_cur = cfg.varname( self.prefix, "Y_stat.CUR%s" % msgdAddress)
      self.Y_pos_cur = cfg.varname( self.prefix, "Y_pos.CUR%s" % msgdAddress)
      self.Y_dpos_req = cfg.varname( self.prefix, "Y_dpos.REQ%s" % msgdAddress)

      self.Z_stat_cur = cfg.varname( self.prefix, "Z_stat.CUR%s" % msgdAddress)
      self.Z_pos_cur = cfg.varname( self.prefix, "Z_pos.CUR%s" % msgdAddress)
      self.Z_dpos_req = cfg.varname( self.prefix, "Z_dpos.REQ%s" % msgdAddress)

      self.abort_req = cfg.varname( self.prefix, "abort.REQ%s" % msgdAddress)
        
      self.pending_moves = cfg.varname(self.prefix, "pendingmoves.CUR%s" % msgdAddress)

      self.mode_cur = cfg.varname(self.prefix, "stepmode.CUR%s" % msgdAddress)
      self.mode_req = cfg.varname(self.prefix, "stepmode.REQ%s" % msgdAddress)
      
      self.ButtonGroup = QButtonGroup()

     ## Draw the widgets

      self.setMinimumSize(QSize(460,360))
 
      self.layout = QGridLayout( self)#, 4)
      self.botspace = QSpacerItem(1,1)
      self.layout.addMultiCell(self.botspace, 16,16,0,14)

      for i in range(20):
        self.layout.setColSpacing(i, 35)
        self.layout.setRowSpacing(i, 30)
        
      self.titlelabel = QLabel("CRO Remote Control", self)
          
      self.layout.addMultiCellWidget(self.titlelabel, 0,0,0,14, Qt.AlignHCenter)
                      
      self.curstatus = QLabel("unknown", self)
      self.pix = QLabel("", self)
      self.layout.addMultiCellWidget(self.curstatus, 1,1,0,14, Qt.AlignHCenter)
      #self.layout.addMultiCellWidget(self.pix, 1,1, 4,5, Qt.AlignHCenter)

      self.pmoves = QLabel("Pending Moves:", self)
      self.layout.addMultiCellWidget(self.pmoves, 6,6,0,3, Qt.AlignHCenter)
      self.npmoves = QLabel("0", self)
      self.layout.addMultiCellWidget(self.npmoves, 6,6,4,4, Qt.AlignLeft)

      self.ssizelab = QLabel("Step Size:", self)
      self.layout.addMultiCellWidget(self.ssizelab, 7,7,0,3, Qt.AlignHCenter)
      self.ssize = QLineEdit("500", self)
      self.layout.addMultiCellWidget(self.ssize, 7,7,4,5, Qt.AlignHCenter)
      self.connect( self.ssize, SIGNAL("returnPressed()"), self.ssize_changed)
      self.ssize_act = QLabel("500", self)
      self.layout.addMultiCellWidget(self.ssize_act, 7,7,6,7, Qt.AlignHCenter)
      self.ssize_act_nm = QLabel("0.015", self)
      self.layout.addMultiCellWidget(self.ssize_act_nm, 7,7,8,9, Qt.AlignLeft)
      
      self.speedl = QLabel("Step Speed:", self)
      self.layout.addMultiCellWidget(self.speedl, 8,8,0,3, Qt.AlignHCenter)
      self.speed = QComboBox(self)
      self.layout.addMultiCellWidget(self.speed, 8,8,4,5, Qt.AlignLeft)
      self.speed.insertItem("1", 0)
      self.speed.insertItem("2", 1)
      self.speed.insertItem("4", 2)
      self.speed.insertItem("8", 3)
      self.connect( self.speed, SIGNAL("activated(int)"), self.speed_changed)
      
      self.abortButton = QPushButton("Stop", self)
      self.layout.addMultiCellWidget(self.abortButton, 15,15,2,13)
      self.connect( self.abortButton, SIGNAL("clicked()"), self.send_abort)

      butw = 30
      self.XpButton = QPushButton("+x", self)
      self.XpButton.setMaximumWidth(butw)
      self.layout.addMultiCellWidget(self.XpButton, 4,4,3,3)
      self.connect( self.XpButton, SIGNAL("clicked()"), self.Xp_clicked)

      self.XnButton = QPushButton("-x", self)
      self.XnButton.setMaximumWidth(butw)
      self.layout.addMultiCellWidget(self.XnButton, 4,4,1,1)
      self.connect( self.XnButton, SIGNAL("clicked()"), self.Xn_clicked)
      
      self.YpButton = QPushButton("+y", self)
      self.YpButton.setMaximumWidth(butw)
      self.layout.addMultiCellWidget(self.YpButton, 3,3,2,2)
      self.connect( self.YpButton, SIGNAL("clicked()"), self.Yp_clicked)

      self.YnButton = QPushButton("-y", self)
      self.YnButton.setMaximumWidth(butw)
      self.layout.addMultiCellWidget(self.YnButton, 5,5,2,2)
      self.connect( self.YnButton, SIGNAL("clicked()"), self.Yn_clicked)
      
      self.ZpButton = QPushButton("+z", self)
      self.ZpButton.setMaximumWidth(butw)
      self.layout.addMultiCellWidget(self.ZpButton, 3,3,5,5)
      self.connect( self.ZpButton, SIGNAL("clicked()"), self.Zp_clicked)

      self.ZnButton = QPushButton("-z", self)
      self.ZnButton.setMaximumWidth(butw)
      self.layout.addMultiCellWidget(self.ZnButton, 5,5,5,5)
      self.connect( self.ZnButton, SIGNAL("clicked()"), self.Zn_clicked)

      self.Xposl = QLabel("X:", self)
      self.layout.addMultiCellWidget(self.Xposl, 3,3,6,6, Qt.AlignRight)
      self.Xpos = QLabel("10000", self)
      self.layout.addMultiCellWidget(self.Xpos, 3,3,7,8, Qt.AlignHCenter)
      self.Xpos_nm = QLabel("0.3", self)
      self.layout.addMultiCellWidget(self.Xpos_nm, 3,3,10,11, Qt.AlignLeft)
      ft = self.Xpos.font()
      ft.setPointSize(12)
      self.Xposl.setFont(ft)
      self.Xpos.setFont(ft)

      self.Yposl = QLabel("Y:", self)
      self.layout.addMultiCellWidget(self.Yposl, 4,4,6,6, Qt.AlignRight)
      self.Ypos = QLabel("10000", self)
      self.layout.addMultiCellWidget(self.Ypos, 4,4,7,8, Qt.AlignHCenter)
      self.Ypos_nm = QLabel("0.3", self)
      self.layout.addMultiCellWidget(self.Ypos_nm, 4,4,10,11, Qt.AlignLeft)
      self.Yposl.setFont(ft)
      self.Ypos.setFont(ft)

      self.Zposl = QLabel("Z:", self)
      self.layout.addMultiCellWidget(self.Zposl, 5,5,6,6, Qt.AlignRight)
      self.Zpos = QLabel("10000", self)
      self.layout.addMultiCellWidget(self.Zpos, 5,5,7,8, Qt.AlignHCenter)
      self.Zpos_nm = QLabel("0.3", self)
      self.layout.addMultiCellWidget(self.Zpos_nm, 5,5,10,11, Qt.AlignLeft)
      self.Zposl.setFont(ft)
      self.Zpos.setFont(ft)
      
      
      self.A1label = QLabel("A1 (x)", self)
      self.layout.addMultiCellWidget(self.A1label, 9,9, 0,2, Qt.AlignHCenter)
      self.A1pix = QLabel("", self)
      self.layout.addMultiCellWidget(self.A1pix, 10,10, 0,2, Qt.AlignHCenter)
      self.A1pix.setPixmap(cfg_gui.pixmaps.pix_working)
      self.A1pos = QLabel("10000.0", self)
      self.layout.addMultiCellWidget(self.A1pos, 11,11,0,2, Qt.AlignHCenter)
      self.A1pButton = QPushButton("+", self)
      self.A1pButton.setMaximumWidth(butw)
      self.layout.addMultiCellWidget(self.A1pButton, 12,12,1,1)
      self.connect( self.A1pButton, SIGNAL("clicked()"), self.A1p_clicked)
      self.A1nButton = QPushButton("-", self)
      self.A1nButton.setMaximumWidth(butw)
      self.layout.addMultiCellWidget(self.A1nButton, 13,13,1,1)
      self.connect( self.A1nButton, SIGNAL("clicked()"), self.A1n_clicked)
      
      self.B1label = QLabel("B1 (x)", self)
      self.layout.addMultiCellWidget(self.B1label, 9,9, 3,5, Qt.AlignHCenter)
      self.B1pix = QLabel("", self)
      self.layout.addMultiCellWidget(self.B1pix, 10,10, 3,5, Qt.AlignHCenter)
      self.B1pix.setPixmap(cfg_gui.pixmaps.pix_working)
      self.B1pos = QLabel("10000.0", self)
      self.layout.addMultiCellWidget(self.B1pos, 11,11,3,5, Qt.AlignHCenter)
      self.B1pButton = QPushButton("+", self)
      self.B1pButton.setMaximumWidth(butw)
      self.layout.addMultiCellWidget(self.B1pButton, 12,12,4,4)
      self.connect( self.B1pButton, SIGNAL("clicked()"), self.B1p_clicked)
      self.B1nButton = QPushButton("-", self)
      self.B1nButton.setMaximumWidth(butw)
      self.layout.addMultiCellWidget(self.B1nButton, 13,13,4,4)
      self.connect( self.B1nButton, SIGNAL("clicked()"), self.B1n_clicked)
      
      self.C1label = QLabel("C1 (y)", self)
      self.layout.addMultiCellWidget(self.C1label, 9,9, 6,8, Qt.AlignHCenter)
      self.C1pix = QLabel("", self)
      self.layout.addMultiCellWidget(self.C1pix, 10,10, 6,8, Qt.AlignHCenter)
      self.C1pix.setPixmap(cfg_gui.pixmaps.pix_working)
      self.C1pos = QLabel("10000.0", self)
      self.layout.addMultiCellWidget(self.C1pos, 11,11,6,8, Qt.AlignHCenter)
      self.C1pButton = QPushButton("+", self)
      self.C1pButton.setMaximumWidth(butw)
      self.layout.addMultiCellWidget(self.C1pButton, 12,12,7,7)
      self.connect( self.C1pButton, SIGNAL("clicked()"), self.C1p_clicked)
      self.C1nButton = QPushButton("-", self)
      self.C1nButton.setMaximumWidth(butw)
      self.layout.addMultiCellWidget(self.C1nButton, 13,13,7,7)
      self.connect( self.C1nButton, SIGNAL("clicked()"), self.C1n_clicked)
      
      self.A2label = QLabel("A2 (z)", self)
      self.layout.addMultiCellWidget(self.A2label, 9,9, 9,11, Qt.AlignHCenter)
      self.A2pix = QLabel("", self)
      self.layout.addMultiCellWidget(self.A2pix, 10,10, 9,11, Qt.AlignHCenter)
      self.A2pix.setPixmap(cfg_gui.pixmaps.pix_working)
      self.A2pos = QLabel("10000.0", self)
      self.layout.addMultiCellWidget(self.A2pos, 11,11,9,11, Qt.AlignHCenter)
      self.A2pButton = QPushButton("+", self)
      self.A2pButton.setMaximumWidth(butw)
      self.layout.addMultiCellWidget(self.A2pButton, 12,12,10,10)
      self.connect( self.A2pButton, SIGNAL("clicked()"), self.A2p_clicked)
      self.A2nButton = QPushButton("-", self)
      self.A2nButton.setMaximumWidth(butw)
      self.layout.addMultiCellWidget(self.A2nButton, 13,13,10,10)
      self.connect( self.A2nButton, SIGNAL("clicked()"), self.A2n_clicked)

      
      self.B2label = QLabel("B2 (z)", self)
      self.layout.addMultiCellWidget(self.B2label, 9,9, 12,14, Qt.AlignHCenter)
      self.B2pix = QLabel("", self)
      self.layout.addMultiCellWidget(self.B2pix, 10,10, 12,14, Qt.AlignHCenter)
      self.B2pix.setPixmap(cfg_gui.pixmaps.pix_working)
      self.B2pos = QLabel("10000.0", self)
      self.layout.addMultiCellWidget(self.B2pos, 11,11,12,14, Qt.AlignHCenter)
      self.B2pButton = QPushButton("+", self)
      self.B2pButton.setMaximumWidth(butw)
      self.layout.addMultiCellWidget(self.B2pButton, 12,12,13,13)
      self.connect( self.B2pButton, SIGNAL("clicked()"), self.B2p_clicked)
      self.B2nButton = QPushButton("-", self)
      self.B2nButton.setMaximumWidth(butw)
      self.layout.addMultiCellWidget(self.B2nButton, 13,13,13,13)
      self.connect( self.B2nButton, SIGNAL("clicked()"), self.B2n_clicked)
        # register slots

      AOwindow.Notify( self.varname_status, self.slotNotify, getNow = True);
      AOwindow.Notify( self.A1_stat_cur, self.slotNotify, getNow = True);
      AOwindow.Notify( self.B1_stat_cur, self.slotNotify, getNow = True);
      AOwindow.Notify( self.C1_stat_cur, self.slotNotify, getNow = True);
      AOwindow.Notify( self.A2_stat_cur, self.slotNotify, getNow = True);
      AOwindow.Notify( self.B2_stat_cur, self.slotNotify, getNow = True);

      AOwindow.Notify( self.A1_pos_cur, self.slotNotify, getNow = True);
      AOwindow.Notify( self.B1_pos_cur, self.slotNotify, getNow = True);
      AOwindow.Notify( self.C1_pos_cur, self.slotNotify, getNow = True);
      AOwindow.Notify( self.A2_pos_cur, self.slotNotify, getNow = True);
      AOwindow.Notify( self.B2_pos_cur, self.slotNotify, getNow = True);

      AOwindow.Notify( self.pending_moves, self.slotNotify, getNow = True);

      AOwindow.Notify( self.mode_cur, self.slotNotify, getNow = True);
      
      AOwindow.Notify( self.X_pos_cur, self.slotNotify, getNow = True);
      AOwindow.Notify( self.Y_pos_cur, self.slotNotify, getNow = True);
      AOwindow.Notify( self.Z_pos_cur, self.slotNotify, getNow = True);
      
   def slotNotify(self, args):
      (message) = args
      varname = message['name']
      
      value = message['value']

     ## When one of the watched variables change, update our display

      if varname == self.varname_status:
            self.curstatus.setText( AOConstants.AOStatesType[ value])
            self.enableButtons(value)
            
      elif varname == self.A1_stat_cur:# or vn2 == self.A1_stat_cur:
            if value == AOConstants.AOStatesCode['STATE_OPERATING']:
               self.A1pix.setPixmap(cfg_gui.pixmaps.pix_working)
            elif value == AOConstants.AOStatesCode['STATE_READY']:
               self.A1pix.setPixmap(cfg_gui.pixmaps.pix_ok)
            else:
               self.A1pix.setPixmap(cfg_gui.pixmaps.pix_ko)

      elif varname == self.B1_stat_cur:
            if value == AOConstants.AOStatesCode['STATE_OPERATING']:
               self.B1pix.setPixmap(cfg_gui.pixmaps.pix_working)
            elif value == AOConstants.AOStatesCode['STATE_READY']:
               self.B1pix.setPixmap(cfg_gui.pixmaps.pix_ok)
            else:
               self.B1pix.setPixmap(cfg_gui.pixmaps.pix_ko)
                  
      elif varname == self.C1_stat_cur:
            if value == AOConstants.AOStatesCode['STATE_OPERATING']:
               self.C1pix.setPixmap(cfg_gui.pixmaps.pix_working)
            elif value == AOConstants.AOStatesCode['STATE_READY']:
               self.C1pix.setPixmap(cfg_gui.pixmaps.pix_ok)
            else:
               self.C1pix.setPixmap(cfg_gui.pixmaps.pix_ko)
               
      elif varname == self.A2_stat_cur:
            if value == AOConstants.AOStatesCode['STATE_OPERATING']:
               self.A2pix.setPixmap(cfg_gui.pixmaps.pix_working)
            elif value == AOConstants.AOStatesCode['STATE_READY']:
               self.A2pix.setPixmap(cfg_gui.pixmaps.pix_ok)
            else:
               self.A2pix.setPixmap(cfg_gui.pixmaps.pix_ko)
               
      elif varname == self.B2_stat_cur:
            if value == AOConstants.AOStatesCode['STATE_OPERATING']:
               self.B2pix.setPixmap(cfg_gui.pixmaps.pix_working)
            elif value == AOConstants.AOStatesCode['STATE_READY']:
               self.B2pix.setPixmap(cfg_gui.pixmaps.pix_ok)
            else:
               self.B2pix.setPixmap(cfg_gui.pixmaps.pix_ko)
 
      elif varname == self.A1_pos_cur:
         self.A1pos.setText( "%0.1f" % -value ) # sign flipped
         
      elif varname == self.B1_pos_cur:
         self.B1pos.setText( "%0.1f" % -value ) # sign flipped
         
      elif varname == self.C1_pos_cur:
         self.C1pos.setText( "%0.1f" % -value ) # sign flipped
         
      elif varname == self.A2_pos_cur:
         self.A2pos.setText( "%0.1f" % value ) # sign not flipped
         
      elif varname == self.B2_pos_cur:
         self.B2pos.setText( "%0.1f" % value ) # sign not flipped

      elif varname == self.pending_moves:
         self.npmoves.setText( "%i" % value)

      elif varname == self.mode_cur:
         if value == 7:
           self.speed.setCurrentItem(0)
         if value == 6:
           self.speed.setCurrentItem(1)
         if value == 5:
           self.speed.setCurrentItem(2)
         if value == 4:
           self.speed.setCurrentItem(3)
           
      elif varname == self.X_pos_cur:
          self.Xpos.setText("%i" % value)
          self.Xpos_nm.setText("%f" % (value*.00003))
          
      elif varname == self.Y_pos_cur:
          self.Ypos.setText("%i" % value)
          self.Ypos_nm.setText("%f" % (value*.00003))
          
      elif varname == self.Z_pos_cur:
          self.Zpos.setText("%i" % value)
          self.Zpos_nm.setText("%f" % (value*.00003))
          
   def send_abort(self):
      result = msglib.SetIntVar( self.abort_req, 1)
  
   def Xp_clicked(self):
      result = msglib.SetIntVar( self.X_dpos_req, self.dstep)

   def Xn_clicked(self):
      result = msglib.SetIntVar( self.X_dpos_req, -self.dstep)

   def Yp_clicked(self):
      result = msglib.SetIntVar( self.Y_dpos_req, self.dstep)

   def Yn_clicked(self):
      result = msglib.SetIntVar( self.Y_dpos_req, -self.dstep)

   def Zp_clicked(self):
      result = msglib.SetIntVar( self.Z_dpos_req, self.dstep)

   def Zn_clicked(self):
      result = msglib.SetIntVar( self.Z_dpos_req, -self.dstep)

   def A1p_clicked(self):
      result = msglib.SetIntVar( self.A1_dpos_req, -self.dstep) # sign flipped

   def A1n_clicked(self):
      result = msglib.SetIntVar( self.A1_dpos_req, self.dstep) # sign flipped

   def B1p_clicked(self):
      result = msglib.SetIntVar( self.B1_dpos_req, -self.dstep) # sign flipped

   def B1n_clicked(self):
      result = msglib.SetIntVar( self.B1_dpos_req, self.dstep) # sign flipped

   def C1p_clicked(self):
      result = msglib.SetIntVar( self.C1_dpos_req, -self.dstep) # sign flipped

   def C1n_clicked(self):
      result = msglib.SetIntVar( self.C1_dpos_req, self.dstep) # sign flipped
         
   def A2p_clicked(self):
      result = msglib.SetIntVar( self.A2_dpos_req, self.dstep) # sign not flipped

   def A2n_clicked(self):
      result = msglib.SetIntVar( self.A2_dpos_req, -self.dstep) # sign not flipped
     
   def B2p_clicked(self):
      result = msglib.SetIntVar( self.B2_dpos_req, self.dstep)

   def B2n_clicked(self):
      result = msglib.SetIntVar( self.B2_dpos_req, -self.dstep)

   def ssize_changed(self):
     if(self.ssize.text().contains(' ') or self.ssize.text().contains('\t') or self.ssize.text().length() == 0):
            QMessageBox.warning(self, 'Invalid step size', 'The requested step size is not valid (no spaces, etc.).', QMessageBox.Ok, QMessageBox.NoButton)
            return
     self.dstep = int(float(self.ssize.text().latin1()))
     if(self.dstep < 0):
        self.dstep = -self.dstep

     self.ssize.setText("%i" % self.dstep)
     self.ssize_act.setText("%i" % self.dstep)
     self.ssize_act_nm.setText("%f" % (self.dstep*0.00003))
     
   def speed_changed(self, index):
      if index == 0:
        newspeed = 7
      if index == 1:
        newspeed = 6
      if index == 2:
        newspeed = 5
      if index == 3:
        newspeed = 4
      result = msglib.SetIntVar( self.mode_req, newspeed)

      
   def enableButtons(self, status):
      if status != AOConstants.AOStatesCode['STATE_OPERATING'] and status != AOConstants.AOStatesCode['STATE_READY']:
         self.abortButton.setEnabled(False)
         self.XpButton.setEnabled(False)
         self.XnButton.setEnabled(False)
         self.YpButton.setEnabled(False)
         self.YnButton.setEnabled(False)
         self.ZpButton.setEnabled(False)
         self.ZnButton.setEnabled(False)
         self.A1pButton.setEnabled(False)
         self.A1nButton.setEnabled(False)
         self.B1pButton.setEnabled(False)
         self.B1nButton.setEnabled(False)
         self.C1pButton.setEnabled(False)
         self.C1nButton.setEnabled(False)
         self.A2pButton.setEnabled(False)
         self.A2nButton.setEnabled(False)
         self.B2pButton.setEnabled(False)
         self.B2nButton.setEnabled(False)

         self.A1pix.setPixmap(cfg_gui.pixmaps.pix_ko)
         self.B1pix.setPixmap(cfg_gui.pixmaps.pix_ko)
         self.C1pix.setPixmap(cfg_gui.pixmaps.pix_ko)
         self.A2pix.setPixmap(cfg_gui.pixmaps.pix_ko)
         self.B2pix.setPixmap(cfg_gui.pixmaps.pix_ko)
      else:
         self.abortButton.setEnabled(True)
         self.XpButton.setEnabled(True)
         self.XnButton.setEnabled(True)
         self.YpButton.setEnabled(True)
         self.YnButton.setEnabled(True)
         self.ZpButton.setEnabled(True)
         self.ZnButton.setEnabled(True)
         self.A1pButton.setEnabled(True)
         self.A1nButton.setEnabled(True)
         self.B1pButton.setEnabled(True)
         self.B1nButton.setEnabled(True)
         self.C1pButton.setEnabled(True)
         self.C1nButton.setEnabled(True)
         self.A2pButton.setEnabled(True)
         self.A2nButton.setEnabled(True)
         self.B2pButton.setEnabled(True)
         self.B2nButton.setEnabled(True)

         self.A1pix.setPixmap(cfg_gui.pixmaps.pix_ok)
         self.B1pix.setPixmap(cfg_gui.pixmaps.pix_ok)
         self.C1pix.setPixmap(cfg_gui.pixmaps.pix_ok)
         self.A2pix.setPixmap(cfg_gui.pixmaps.pix_ok)
         self.B2pix.setPixmap(cfg_gui.pixmaps.pix_ok)