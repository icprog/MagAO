#!/usr/bin/python

from qt import *
import sys

from AdOpt import processControl, AOMsgCode, cfg, calib
from AdOpt.AOConstants import *
from AdOpt.QtDesigner.ccd_gui import *
from AdOpt.wrappers import msglib

class ccd_gui_impl(ccd_gui):
    
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
        ccd_gui.__init__(self)
        #self.setFixedSize(QSize(460,450))

        
        self.curBlacks = []
        self.reqBlacks = []
        for i in range(self.numBlacks):
            self.curBlacks.append(vars(self)['black' + str(i+1) + 'CurValue'])
            self.reqBlacks.append(vars(self)['black' + str(i+1) + 'ReqValue'])
            vars(self)['black' + str(i+1) + 'ReqValue'].setEnabled(False)
            vars(self)['black' + str(i+1) + 'CurValue'].setEnabled(False)

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
        self.varname_curYbin = cfg.varname(ccd, 'YBIN.CUR')
        self.varname_curHzFreq = cfg.varname(ccd, 'FRMRT.CUR')
        self.varname_reqHzFreq = cfg.varname(ccd, 'FRMRT.REQ')
        self.varname_curRepFreq = cfg.varname(ccd, 'REP.CUR')
        self.varname_reqRepFreq = cfg.varname(ccd, 'REP.REQ')
        self.varname_curClamp = cfg.varname(ccd, 'CLAMP.CUR')
        self.varname_reqClamp = cfg.varname(ccd, 'CLAMP.REQ')
        self.varname_curSample = cfg.varname(ccd, 'SAMPLE.CUR')
        self.varname_reqSample = cfg.varname(ccd, 'SAMPLE.REQ')

        self.varname_reqGain = cfg.varname(ccd, 'GAIN.REQ')
        self.varname_curGain = cfg.varname(ccd, 'GAIN.CUR')

        self.varname_ttamp_error = cfg.varname(ccd, "TTAMP_ERROR");

        self.varname_curTTAmp = cfg.varname("ttctrl", 'AMP.CUR')
        self.curTTAmp = -1
        
        self.varname_darkReq = cfg.ccd_darkvariable[self.prefix]+'.REQ'
        self.varname_darkCur = cfg.ccd_darkvariable[self.prefix]+'.CUR'

        self.curxbin=0
        self.curybin=0

        self.connect(aoGui, PYSIGNAL("%d" % AOMsgCode['CLNREADY']), self.slotNotify)
        msglib.SendMessage("", AOMsgCode['WAITCLRDY'], self.taskname)
        print "Waiting controller %s ready..." % self.taskname
        
    
    def enableCustomButtons(self, enable):
        pass
    
    ################################## EVENT LISTENERS ########################################
    ###########################################################################################
    
    def freqHzSelect_clicked(self):
        if self.freqHzSelect.isChecked():
            # Disable freqRep
            self.freqRepSelect.setChecked(False)
            self.freqRepReqValue.setEnabled(False)
            # Enable freqHz
            self.freqHzReqValue.setEnabled(True)
        else:
            self.freqHzSelect.setChecked(True)
        

    def freqRepSelect_clicked(self):
        if self.freqRepSelect.isChecked():
            # Disable freqHz
            self.freqHzSelect.setChecked(False)
            self.freqHzReqValue.setEnabled(False)
            # Enable freqRep
            self.freqRepReqValue.setEnabled(True)
        else:
            self.freqRepSelect.setChecked(True)
            
            
    def startStopButton_clicked(self):
        if self.startStopButton.text() == "Start":
            msglib.SetIntVar(self.varname_reqEnable, 1)
        elif self.startStopButton.text() == "Stop":
            msglib.SetIntVar(self.varname_reqEnable, 0)
            
    
    def applyButton_clicked(self):
 
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

        # Apply black levels AFTER the readout program, because this contains
        # its own black levels
        b_values = []
        for n in range(len(self.reqBlacks)):
            value = self.reqBlacks[n].text().latin1()
            b_values.append(int(value))
        msglib.SetVar(self.varname_reqBlacks, msglib.INT_VARIABLE, len(b_values), b_values)

        # Apply clamp and sample levels
        clamp  = int(self.clampReqValue.text().latin1())
        sample = int(self.sampleReqValue.text().latin1())
        # Disabled until further testing
        #msglib.SetIntVar(self.varname_reqClamp, clamp)
        #msglib.SetIntVar(self.varname_reqSample, sample)
        
            
        print "Settings applied !"
        
        
    def equalizeButton_clicked(self):
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
        ## When the CCD control process is ready, start the variable management
        if code == AOMsgCode['CLNREADY']:

            for varname in [ self.varname_status, self.varname_curEnable, self.varname_speeds, self.varname_curReadout, self.varname_curTemps, \
                  self.varname_curBlacks, self.varname_curXbin, self.varname_curYbin, self.varname_curHzFreq, self.varname_curRepFreq, self.varname_darkCur, self.varname_curGain, self.varname_curTTAmp, self.varname_curSample, self.varname_curClamp ]:

              self.aoGui.Notify( varname, self.slotNotify, getNow = True)

            for varname in [ self.varname_ttamp_error ]:
              self.aoGui.Notify( varname, self.slotNotify, getNow = False)
            return



        ## VARREPLY
        ##
        ## When one of the watched variables change, update our display
        varname = message['name']
        value = message['value']

        # Status
        if varname == self.varname_status:
            self.statusCurValue.setText(AOStatesType[value])
            if AOStatesCode["STATE_NOCONNECTION"] == value:
                self.newBinning.setEnabled(False)
            if AOStatesCode["STATE_READY"] == value or AOStatesCode["STATE_OPERATING"] == value:
                self.newBinning.setEnabled(True)
            else:
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
            while self.readoutReqValue.count() > 0:
                self.readoutReqValue.removeItem(0)
            for speed in value:
                if speed > 0:
                    self.readoutReqValue.insertItem("%d" % speed)
                    
        # Readout speed
        elif varname == self.varname_curReadout:
            self.readoutCurValue.setText(str(value));
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
            
        ## Gain
        elif varname == self.varname_curGain:
            self.curybin = value
            if value == 1:
              text = 'Med. Hi'
              
            elif value == 2:
              text = 'Med. Low'
             
            elif value == 3:
              text = 'Low'
              
            else:
              text = 'High'
           
            self.ccdCurGain.setText(text)
             
        # Temps
        elif varname == self.varname_curTemps:
            text = 'Case: %dC Head: %dC %dC' % (value[0], value[1], value[2])
            self.ccdCurTemp.setText(text)
            
        # Freq in repetitions
        elif varname == self.varname_curRepFreq:
            self.freqRepCurValue.setText(str(value))
        
        # Freq in Hz
        elif varname == self.varname_curHzFreq:
            self.freqHzCurValue.setText(str("%.2f" %value))

        # Clamp level 
        elif varname == self.varname_curClamp:
            self.clampCurValue.setText(str("%d" %value))

        # Sample level 
        elif varname == self.varname_curSample:
            self.sampleCurValue.setText(str("%d" %value))

        # TT amplitude
        elif varname == self.varname_curTTAmp:
            self.curTTAmp = value

        elif varname == self.varname_darkCur:
            f = value
            if f == '':
                f = 'None'
            self.labelCurDarkFrame.setText( 'Current dark frame: %s' % f)

        elif varname == self.varname_ttamp_error:
            QMessageBox.information( self, "SETTINGS NOT APPLIED", "SETTINGS NOT APPLIED\nCannot change frequency and/or repetition value when the tip/tilt mirror modulation amplitude is not zero", QMessageBox.Ok)



    def BrowseDarkFrameButton_clicked(self):

        dir = calib.BackgDir( self.prefix, self.curxbin)

        filename = QFileDialog.getOpenFileName( dir, "", self, "", "Choose a file")
        if type(filename) != "NoneType":
            f = filename.latin1()
            f = f[f.rindex('/')+1:]
            self.DarkFrameEdit.setText(f)


    def SendDarkFrameButton_clicked(self):

        filename = self.DarkFrameEdit.text().latin1()
        msglib.SetVar( self.varname_darkReq, msglib.CHAR_VARIABLE, len(filename), filename)




