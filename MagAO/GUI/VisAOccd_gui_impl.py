#!/usr/bin/python

from qt import *
import sys

from AdOpt import processControl, cfg, calib
from AdOpt.AOConstants import *
from VisAOccd_gui import *
from AdOpt.wrappers import msglib

class VisAOccd_gui_impl(VisAOccd_gui):
    
    ## aoGui is an EngineeringGui, the main gui connected to MsgD, wich wait and 
    ## dispatch rtdb variable changes.
    def __init__(self, ccd, aoGui):
        
        self.aoGui = aoGui
        self.prefix = ccd
        
        self.liveView_process = None;
        self.saveToFile_process = None;
        self.changeBinning_process = None;
        self.equalizeQuadrants_process = None;
        
        
        # --- READ BASIC CONFIG ---#
        self.taskname  = cfg.taskname(self.prefix)
        self.name      = cfg.cfg[self.prefix]['ccdName'].Value()
        self.numBlacks = cfg.cfg[self.prefix]['ccdBlacksNum'].Value()
        
        
        # --- INITITALIZE GRAPHICS --- #
        VisAOccd_gui.__init__(self)
        #self.setFixedSize(QSize(460,450))

        
        self.curBlacks = []
        self.reqBlacks = []
        for i in range(self.numBlacks):
            self.curBlacks.append(vars(self)['black' + str(i+1) + 'CurValue'])
            self.reqBlacks.append(vars(self)['black' + str(i+1) + 'ReqValue'])
            #vars(self)['black' + str(i+1) + 'ReqValue'].setEnabled(False)
            #vars(self)['black' + str(i+1) + 'CurValue'].setEnabled(False)

        self.gainReq.insertItem("High")
        self.gainReq.insertItem("Med High")
        self.gainReq.insertItem("Med Low")
        self.gainReq.insertItem("Low")
        
#        self.tempBox.setStretchFactor(4)
#        self.binningBox.setStretchFactor(1)
        
        # --- INITIALIZE SYSTEM STATUS --- 
        
        # Save var names(for code simplicity)
        self.varname_status = cfg.varname(ccd, 'STATUS')
        self.varname_curEnable = cfg.varname( ccd, 'ENABLE.CUR')
        self.varname_reqEnable = cfg.varname( ccd, 'ENABLE.REQ')
        self.varname_speeds = cfg.varname(ccd, 'SPEEDS')        # Available readout speeds
        self.varname_curReadout = cfg.varname(ccd, 'SPEED.CUR')
        self.varname_reqReadout = cfg.varname(ccd, 'SPEED.REQ')
        self.varname_curTemps = cfg.varname(ccd, 'TEMPS')
        self.varname_curBlacks = cfg.varname(ccd, 'BLACK.CUR')
        self.varname_reqBlacks = cfg.varname(ccd, 'BLACK.REQ')
        self.varname_curXbin = cfg.varname(ccd, 'XBIN.CUR')
        self.varname_reqXbin = cfg.varname(ccd, 'XBIN.REQ')
        self.varname_curYbin = cfg.varname(ccd, 'YBIN.CUR')
        self.varname_reqYbin = cfg.varname(ccd, 'YBIN.REQ')
        self.varname_curHzFreq = cfg.varname(ccd, 'FRMRT.CUR')
        self.varname_reqHzFreq = cfg.varname(ccd, 'FRMRT.REQ')
        self.varname_curRepFreq = cfg.varname(ccd, 'REP.CUR')
        self.varname_reqRepFreq = cfg.varname(ccd, 'REP.REQ')
        self.varname_curGain = cfg.varname(ccd, 'GAIN.CUR')
        self.varname_reqGain = cfg.varname(ccd, 'GAIN.REQ')
        self.varname_curWinX = cfg.varname(ccd, 'DX.CUR')
        self.varname_reqWinX = cfg.varname(ccd, 'DX.REQ')
        self.varname_curWinY = cfg.varname(ccd, 'DY.CUR')
        self.varname_reqWinY = cfg.varname(ccd, 'DY.REQ')
        self.varname_windowxs = cfg.varname(ccd, 'WINDOWXS')
        self.varname_windowys = cfg.varname(ccd, 'WINDOWYS')
        self.varname_curctype = cfg.varname(ccd, "ConMode.CUR")
        self.varname_reqctype = cfg.varname(ccd, "ConMode.REQ")
        
        self.varname_darkReq = cfg.ccd_darkvariable[self.prefix]+'.REQ'
        self.varname_darkCur = cfg.ccd_darkvariable[self.prefix]+'.CUR'


        self.curxbin=0
        self.curybin=0
        self.curxwin=0
        self.curywin=0
        self.winsx_set = 0
        self.winsy_set = 0
        self.curSpeed = 0;
        
        # Ask to be notified on client raedy
        #self.connect(aoGui, PYSIGNAL(self.ctrlname), self.slotNotify)

        self.connect(aoGui, PYSIGNAL("%d" % AOMsgCode['CLNREADY']), self.slotNotify)
        msglib.SendMessage("", AOMsgCode['WAITCLRDY'], self.taskname)
        
  
        print "Waiting controller %s ready..." % self.taskname
        
      
    
    def enableCustomButtons(self, enable):
        pass
    
    ################################## EVENT LISTENERS ########################################
    ###########################################################################################
    
    def remoteButton_clicked(self):
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
        
    def overrideCheck_checked(self):
         
       if self.overrideCheck.isChecked() == 1:
           result = self.remoteButton.setText("***Take Remote Control***")
       else:
           result = self.remoteButton.setText("   Take Remote Control   ")
           
    def freqHzSelect_clicked(self):
        print "freqHzSelect_clicked"
        if self.freqHzSelect.isChecked():
            # Disable freqRep
            self.freqRepSelect.setChecked(False)
            self.freqRepReqValue.setEnabled(False)
            # Enable freqHz
            self.freqHzReqValue.setEnabled(True)
        else:
            self.freqHzSelect.setChecked(True)
        

    def freqRepSelect_clicked(self):
        print "freqRepSelect_clicked"
        if self.freqRepSelect.isChecked():
            # Disable freqHz
            self.freqHzSelect.setChecked(False)
            self.freqHzReqValue.setEnabled(False)
            # Enable freqRep
            self.freqRepReqValue.setEnabled(True)
        else:
            self.freqRepSelect.setChecked(True)
            
            
    def startStopButton_clicked(self):
        if self.curctype != 1:
          QMessageBox.information( self, "ccd47 Control","You must first take Remote Control to change settings on ccd47 from here.")
          return
        if self.startStopButton.text() == "Start":
            msglib.SetIntVar(self.varname_reqEnable, 1)
        elif self.startStopButton.text() == "Stop":
            msglib.SetIntVar(self.varname_reqEnable, 0)
            
    
    def applyButton_clicked(self):

        if self.curctype != 1:
          QMessageBox.information( self, "ccd47 Control","You must first take Remote Control to change settings on ccd47 from here.")
          return
        
        try:
            if self.freqHzSelect.isChecked():
                if float(self.freqHzReqValue.text().latin1()) > 600:
                    QMessageBox.information( self, "Frequency out of range", "SETTINGS NOT APPLIED\n\nIt is not possible to apply a frequency greater than 600 Hz from this interface.\nUse the \"Loop parameters\" setting in the Wfs Arbitrator interface instead.")
                    return
        except ValueError:
            # Happens when the editbox is empty
            pass

        
 
        # Apply readout program
        try:
            msglib.SetIntVar(self.varname_reqReadout, int(self.readoutReqValue.currentText().latin1()))
        except TypeError:
            print "Invalid readout speed not applied !"
            
        # Apply frequency in repetitions or in Hz
        if self.freqRepSelect.isChecked():
            try:
                msglib.SetVar(self.varname_reqRepFreq, msglib.INT_VARIABLE, 1, int(self.freqRepReqValue.text().latin1()))
            except ValueError:
                print "Invalid frequency repetitions not applied !"
        else:
            try:
                msglib.SetVar(self.varname_reqHzFreq, msglib.REAL_VARIABLE, 1, float(self.freqHzReqValue.text().latin1()))
            except ValueError:
                print "Invalid frequency Hz not applied !"

        #Apply gain
        val = self.gainReq.currentItem()
        print val
        msglib.SetVar(self.varname_reqGain, msglib.INT_VARIABLE, 1, val)
        
        # Apply black levels AFTER the readout program, because this contains
        # its own black levels
        b_values = []
        for n in range(len(self.reqBlacks)):
            value = self.reqBlacks[n].text().latin1()
            b_values.append(int(value))
        msglib.SetVar(self.varname_reqBlacks, msglib.INT_VARIABLE, len(b_values), b_values)
        
            
        print "Settings applied !"
        
        
    def equalizeButton_clicked(self):
        if self.curctype != 1:
          QMessageBox.information( self, "ccd47 Control","You must first take Remote Control to change settings on ccd47 from here.")
          return
        print "Starting quadrants equalization..."
        processControl.startProcessByName(self.equalizeQuadrants_process, multi=1)  
        
        
    def liveViewButton_clicked(self):
        processControl.startProcessByName(self.liveView_process, multi=1)
        
        
    def saveToFileButton_clicked(self):
        processControl.startProcessByName(self.saveToFile_process, multi=1)
        
            
    ## Handle notifies from AOGui for rtdb variables change
    def slotNotify(self, args):
        (message) = args
        code = message['code']

        ## CLNREADY
        ##
        ## When the Stage control client is ready, start the variable management
        if code == AOMsgCode['CLNREADY']:
            print "Controller is ready !"

            for varname in [ self.varname_status, self.varname_curEnable, self.varname_speeds, self.varname_curReadout, self.varname_curTemps, \
                  self.varname_curBlacks, self.varname_curXbin, self.varname_curYbin, self.varname_curHzFreq, self.varname_curRepFreq, \
                  self.varname_curGain,\
                  self.varname_curWinX, self.varname_curWinY, self.varname_windowxs, self.varname_windowys, self.varname_darkCur, self.varname_curctype ]:

              self.aoGui.Notify( varname, self.slotNotify, getNow = True)
            return



        ## VARREPLY
        ##
        ## When one of the watched variables change, update our display
        varname = message['name']
        value = message['value']

        # Status
        if varname == self.varname_status:
            self.statusCurValue.setText(AOStatesType[value])
            #if AOStatesCode["STATE_NOCONNECTION"] == value:
                #self.newBinning.setEnabled(False)
            if AOStatesCode["STATE_READY"] == value or AOStatesCode["STATE_OPERATING"] == value:
                self.newBinning.setEnabled(True)
                self.startStopButton.setEnabled(True)
                self.applyButton.setEnabled(True)
                self.liveViewButton.setEnabled(True)
                self.BrowseDarkFrameButton.setEnabled(True)
                self.SendDarkFrameButton.setEnabled(True)
                self.enableCustomButtons(True)
                for i in range(self.numBlacks):
                    vars(self)['black' + str(i+1) + 'ReqValue'].setEnabled(True)
                    vars(self)['black' + str(i+1) + 'CurValue'].setEnabled(True)
            else:
                self.newBinning.setEnabled(False)
                self.startStopButton.setEnabled(False)
                self.applyButton.setEnabled(False)
                self.liveViewButton.setEnabled(False)
                self.BrowseDarkFrameButton.setEnabled(False)
                self.SendDarkFrameButton.setEnabled(False)
                self.enableCustomButtons(False)
                for i in range(self.numBlacks):
                    vars(self)['black' + str(i+1) + 'ReqValue'].setEnabled(False)
                    vars(self)['black' + str(i+1) + 'CurValue'].setEnabled(False)
            
        # Ccd started or stopped
        if varname ==self.varname_curEnable:
            if value == 1:
                self.startStopButton.setText("Stop")
            elif value == 0:
                self.startStopButton.setText("Start")
            
        # Available readout speed
        elif varname == self.varname_speeds:
            # empty readout combobox
            self.curSpeeds = value
            curidx = 0
            while self.readoutReqValue.count() > 0:
                self.readoutReqValue.removeItem(0)
            i = 0
            for speed in value:
                if speed > 0:
                    self.readoutReqValue.insertItem("%d" % speed)
                    if speed == self.curSpeed:
                       curidx = i
                i = i+1
            
            self.readoutReqValue.setCurrentItem(curidx)
                   
        # Available windows
        elif varname == self.varname_windowxs:
            self.winsx_set = 1
            self.curwinxs = value
            if self.winsy_set:
              self.winsx_set = 0
              self.winsy_set = 0
              while self.newWindow.count() > 0:
                self.newWindow.removeItem(0)
              i = 0
              for win in self.curwinxs:
                if self.curwinxs[i] >0 and self.curwinys[i] > 0:
                    self.newWindow.insertItem("%dX%d" % (self.curwinxs[i], self.curwinys[i]))
                i = i+1

        elif varname == self.varname_windowys:
            self.winsy_set = 1
            self.curwinys = value
            if self.winsx_set:
              self.winsx_set = 0
              self.winsy_set = 0
              while self.newWindow.count() > 0:
                self.newWindow.removeItem(0)
              i = 0
              for win in self.curwinys:
                if self.curwinxs[i] >0 and self.curwinys[i] > 0:
                    self.newWindow.insertItem("%dX%d" % (self.curwinxs[i], self.curwinys[i]))
                i = i+1

                    
        # Readout speed
        elif varname == self.varname_curReadout:
            self.readoutCurValue.setText(str(value));
            self.curSpeed = value
            curidx = 0
            i = 0
            for speed in self.curSpeeds:
                if speed > 0:
                    if speed == self.curSpeed:
                       curidx = i
                i = i+1
            self.readoutReqValue.setCurrentItem(curidx)
               
               
            if value != 0:
                self.startStopButton.setEnabled(True)
                self.applyButton.setEnabled(True)
                self.liveViewButton.setEnabled(True)
                self.BrowseDarkFrameButton.setEnabled(True)
                self.SendDarkFrameButton.setEnabled(True)
                self.enableCustomButtons(True)
                for i in range(self.numBlacks):
                    vars(self)['black' + str(i+1) + 'ReqValue'].setEnabled(True)
                    vars(self)['black' + str(i+1) + 'CurValue'].setEnabled(True)
            
        # Black levels
        elif varname == self.varname_curBlacks:
            for i in range(len(value)):
                self.curBlacks[i].setText(str(value[i]))
                self.reqBlacks[i].setValue(value[i]) # Needed when readout program changes
                
        # X binning
        elif varname == self.varname_curXbin:
            
            self.curxbin = value
            try:
                text = '%dX%d' % (self.curxbin, self.curybin)
            except:  # Happens when not both binnings are set
                pass
            self.ccdCurBinning.setText(text)

        # Y binning
        elif varname == self.varname_curYbin:
            self.curybin = value
            text = '%dX%d' % (self.curxbin, self.curybin)
            self.ccdCurBinning.setText(text)

        # X Window
        elif varname == self.varname_curWinX:
          
            self.curxwin = value
            text = '%dX%d' % (self.curxwin, self.curywin)
            
            self.ccdCurWindow.setText(text)

        # Y Window
        elif varname == self.varname_curWinY:
            self.curywin = value
            text = '%dX%d' % (self.curxwin, self.curywin)
            
            self.ccdCurWindow.setText(text)
            
        
        # Temps
        elif varname == self.varname_curTemps:
            text = 'Case: %dC Head: %dC %dC' % (value[0], value[1], value[2])
            self.ccdCurTemp.setText(text)
            
        # Freq in repetitions
        elif varname == self.varname_curRepFreq:
            self.freqRepCurValue.setText(str(value))
            self.freqRepReqValue.setText(str(value))
            
        # Freq in Hz
        elif varname == self.varname_curHzFreq:
            self.freqHzCurValue.setText(str("%.2f" %value))
            self.freqHzReqValue.setText(str("%.2f" %value))


            # Gain
        elif varname == self.varname_curGain:
            if value == 0:
              self.gainCur.setText("High")
            if value == 1:
              self.gainCur.setText("Med High")
            if value == 2:
              self.gainCur.setText("Med Low")
            if value == 3:
              self.gainCur.setText("Low")
            self.gainReq.setCurrentItem(value)
              
        elif varname == self.varname_darkCur:
            f = value
            if f == '':
                f = 'None'
            self.labelCurDarkFrame.setText( 'Current dark frame: %s' % f)

        elif varname == self.varname_curctype:
            self.setcurctype( value)





    def BrowseDarkFrameButton_clicked(self):

        if self.prefix == 'ccd39':
            ccd = 'ccd39'
        if self.prefix == 'ccd47':
            ccd = 'ccd47'
        dir = calib.BackgDir( ccd, self.curxbin)

        filename = QFileDialog.getOpenFileName( dir, "", self, "", "Choose a file")
        if type(filename) != "NoneType":
            f = filename.latin1()
            f = f[f.rindex('/')+1:]
            self.DarkFrameEdit.setText(f)


    def SendDarkFrameButton_clicked(self):

        filename = self.DarkFrameEdit.text().latin1()
        msglib.SetVar( self.varname_darkReq, msglib.CHAR_VARIABLE, len(filename), filename)

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


