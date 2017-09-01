#!/usr/bin/env python
#
# tt_gui
#
# GUI tool for tip-tilt mirror control
#

import sys, string
from qt import *

from AdOpt import cfg, AOConstants, AOMsgCode
import cfg_gui

## Import the C library wrapper
from AdOpt.wrappers import msglib

        
#@+Class: TT_Main
#
# Main window for the GUI tool.
#@
class tt_gui(QDialog):
    
    def __init__(self, AOwindow):
        QDialog.__init__(self)
       
        cfg_gui.init()
        self.minVolt = cfg.cfg['ttctrl']['MIN_VOLT'].Value()
        self.maxVolt = cfg.cfg['ttctrl']['MAX_VOLT'].Value()
        self.rangeVolt = self.maxVolt - self.minVolt
        self.minSlider = -50
        self.maxSlider = 50
        self.rangeSlider = self.maxSlider - self.minSlider
 
        self.setFixedSize(550, 420);

        self.name = "Tip-tilt"
        self.ctrlname = 'ttctrl'
        self.descr = "Tip-Tilt controller"

        ## Draw the widgets
        self.gridLayout = QGridLayout(self, 6, 4, 20)
        self.gridLayout.setAutoAdd(1)

        self.label = QLabel("Status:", self)
        self.status = QLabel("unknown", self)
        self.dummy1 = QLabel("", self);
        self.pix = QPushButton("", self);

        self.labelSet = QLabel("Mirror set:", self)
        self.setStatus = QLabel("unknown", self)
        self.setButton = QPushButton("Set", self)
        self.restButton  = QPushButton("Rest", self);
        self.connect( self.setButton, SIGNAL("clicked()"), self.slotButtonSet)
        self.connect( self.restButton, SIGNAL("clicked()"), self.slotButtonRest)

        self.header1 = QLabel("<b>Parameter</b>", self)
        self.header2 = QLabel("<b>Current</b>", self)
        self.header3 = QLabel("<b>Requested</b>", self)
        self.dummy3  = QLabel("", self);

        self.axysRotLabel = QLabel("Axys rotation (degree)", self);
        self.axysRotValue = QLabel("unknown", self)
        self.axysRotEdit = QLineEdit("", self)
        self.axysRotEdit.setMaximumWidth(70)
        self.axysRotdummy  = QLabel("", self);

        self.label1 = QLabel("Amplitude (mRad, radius):", self)
        self.AmplitudeValue = QLabel("unknown", self)
        self.AmplitudeEdit = QLineEdit("", self)
        self.AmplitudeEdit.setMaximumWidth(70)
        self.dummy4  = QLabel("", self);

        self.label2 = QLabel("Frequency (Hertz):", self)
        self.FrequencyValue = QLabel("unknown", self)
        self.FrequencyEdit = QLineEdit("", self)
        self.FrequencyEdit.setMaximumWidth(70)
        self.FreqSyncCcdCheckBox = QCheckBox("Sync with CCD", self);
        #self.FreqSyncCcdCheckBox.setCheck(0)
        self.FreqSyncCcdCheckBox.setEnabled(False) 
        #self.connect(self.FreqSyncCcdCheckBox, SIGNAL("clicked()"), self.syncCcdClicked)

        self.label3 = QLabel("X Offset (mRad):", self)
        self.XOffsetValue = QLabel("unknown", self)
        self.XOffsetEdit = QLineEdit("", self)
        self.XOffsetEdit.setMaximumWidth(70)
        self.xslider  = QSlider(-50, 50, 1, 5, QSlider.Horizontal, self );
        self.xslider.setValue(0);
        self.xslider.setMaximumWidth(120)
        # Note that the signal is "sliderMoved" (not "valueChanged") because don't  
        # want to be notified when the slider is updated as consequence of a button
        # "set" pressed, but only when the slider is moved by user
        self.connect( self.xslider, SIGNAL("sliderMoved(int)"), self.slotXSlider)

        self.label4 = QLabel("Y Offset (mRad):", self)
        self.YOffsetValue = QLabel("unknown", self)
        self.YOffsetEdit = QLineEdit("", self)
        self.YOffsetEdit.setMaximumWidth(70)
        self.yslider  = QSlider(-50, 50, 1, 5, QSlider.Horizontal, self );
        self.yslider.setValue(0);
        self.yslider.setMaximumWidth(120)
        self.connect( self.yslider, SIGNAL("sliderMoved(int)"), self.slotYSlider)

        self.dummy2 = QLabel("", self)
        self.dummy3 = QLabel("", self)
        self.applyButton = QPushButton("Apply", self)
        self.connect( self.applyButton, SIGNAL("clicked()"), self.slotButtonApply)
        self.dummy6  = QLabel("", self);

        self.AOwindow = AOwindow

        QObject.connect( self.AOwindow, PYSIGNAL( "%d" % AOMsgCode['CLNREADY']), self.slotNotify)

        
        self.rtdbVars = [ cfg.varname('ttctrl', x) for x in \
                   ['AMP.CUR', 'FREQ.CUR', 'OFFX.CUR', 'OFFY.CUR', 'AXYSROT.CUR', 'ERRMSG', 'STATUS', 'MIRROR_SET.CUR']]

        ## We want a message when the TTCtrl is ready
        self.AOwindow.WaitCtlrReady(cfg.taskname(self.ctrlname), self.slotNotify)
        print "Waiting controller %s ready..." % self.ctrlname


    ## +Method: slotNotify
    ##
    ## This method is the network event handler for the whole application. It's called by QSocketNotifier when
    ## some data is waiting in the server-connected socked. Thus WaitMessage() is guaranteed to return some
    ## useful data. Different actions are taken based on the message contents.
    def slotNotify(self, args):
        (message) = args
        code = message['code']

        ## CLNREADY
        ##
        ## When the Stage control client is ready, start the variable management
        if code == AOMsgCode['CLNREADY']:
            for var in self.rtdbVars:
                 self.AOwindow.Notify(var, self.slotNotify, getNow = True);
            return

        ## VARREPLY
        ##
        ## When one of the watched variables change, update our display
        if code == AOMsgCode['VARREPLY'] or code == AOMsgCode['VARCHANGD']:
            
            varname = message['name']
            value = message['value']

            if varname == cfg.varname('ttctrl','STATUS'):
                self.status.setText( AOConstants.AOStatesType[ value])
                self.setPix(value)

            elif varname == cfg.varname('ttctrl',"ERRMSG"):
                self.status.setText(value)

            elif varname == cfg.varname('ttctrl', 'MIRROR_SET.CUR'):
                if value ==0:
                    self.setStatus.setText('<b>REST</b>ed')
                elif value == 1:
                    self.setStatus.setText('<b>SET</b>')
                else:
                    self.setStatus.setText('working...')

            elif varname == cfg.varname('ttctrl', "AMP.CUR"):
                self.AmplitudeValue.setText(self._format(value*1e3))
                if self.AmplitudeEdit.text().latin1() == "":
                    self.AmplitudeEdit.setText(self._format(value*1e3))

            elif varname == cfg.varname('ttctrl',"FREQ.CUR"):
                self.FrequencyValue.setText(self._format(value))
                if self.FrequencyEdit.text().latin1() == "":
                    self.FrequencyEdit.setText(self._format(value))
                if value == 0:
                    self.FreqSyncCcdCheckBox.setChecked(1)
                else:
                    self.FreqSyncCcdCheckBox.setChecked(0)
                self.enableButtons()

            elif varname == cfg.varname('ttctrl',"OFFX.CUR"):
                self.XOffsetValue.setText(self._format(value*1e3))
                if self.XOffsetEdit.text().latin1() == "":
                    self.XOffsetEdit.setText(self._format(value*1e3))
                # Update the slider with the CUR value: of course doesn't
                # make any sense (and it's impossible for values <-5 or >5) to
                # update the slider with the REQ value
                vv = (float(value)-self.minVolt) / self.rangeVolt
                self.xslider.setValue( self.minSlider + vv*self.rangeSlider)

            elif varname == cfg.varname('ttctrl',"OFFY.CUR"):
                self.YOffsetValue.setText(self._format(value*1e3))
                if self.YOffsetEdit.text().latin1() == "":
                    self.YOffsetEdit.setText(self._format(value*1e3))
                vv = (float(value)-self.minVolt) / self.rangeVolt
                self.yslider.setValue( self.minSlider + vv*self.rangeSlider)
            
            elif varname == cfg.varname('ttctrl',"AXYSROT.CUR"):
                self.axysRotValue.setText(self._format(value))
                if self.axysRotEdit.text().latin1() == "":
                    self.axysRotEdit.setText(self._format(value))

            return

        print "Got unexpected message: %d" % message['code']

    def _format(self,value):
        return "%.2f" % value            
            
    def strings_to_zero(self, value):
        if value == "" or value == "unknown":
            return "0"

        return value

    ## +Method: slotExitButton
    ##
    ## Handler for the Exit button. It terminates the application immediately
    def slotExitButton(self):
        self.close()
        
    def syncCcdClicked(self):
        self.enableButtons()
        self.slotButtonApply()

    def enableButtons(self):
        if self.FreqSyncCcdCheckBox.isChecked():
            self.FrequencyEdit.setEnabled(False)
            self.FrequencyValue.setEnabled(False)
        else:
            self.FrequencyEdit.setEnabled(True)
            self.FrequencyValue.setEnabled(True)
    
    ## +Method: slotXSlider
    ##
    ## Handler for the event "sliderMoved" for the sliderX
    ## Update the value of the XOffset field and ask for a "set"
    def slotXSlider(self, value):
        vv = float(value-self.minSlider) / self.rangeSlider
        xoffset = self.minVolt + vv*self.rangeVolt
        self.XOffsetEdit.setText(str(xoffset*1e3))
        self.slotButtonApply()
        
    ## +Method: slotYSlider
    ##
    ## Handler for the event "sliderMoved" for the sliderY
    ## Update the value of the YOffset field and ask for a "set"
    def slotYSlider(self, value):
        vv = float(value-self.minSlider) / self.rangeSlider
        yoffset = self.minVolt + vv*self.rangeVolt
        self.YOffsetEdit.setText(str(yoffset*1e3));
        self.slotButtonApply()

    def slotButtonSet(self):
         msglib.SetIntVar(cfg.varname('ttctrl',"MIRROR_SET.REQ"), 1)
        
    def slotButtonRest(self):
         msglib.SetIntVar(cfg.varname('ttctrl',"MIRROR_SET.REQ"), 0)
        

    ##+Method: slotButtonApply
    ##
    ## Handler for the Set button. Sends the parameters to the TT mirror
    def slotButtonApply(self):
        xoffset = self.XOffsetEdit.text().latin1()
        yoffset = self.YOffsetEdit.text().latin1()
        amp     = self.AmplitudeEdit.text().latin1()
        freq    = self.FrequencyEdit.text().latin1()
        rot     = self.axysRotEdit.text().latin1()

        if xoffset == "":
            xoffset = "0"
            self.XOffsetEdit.setText(xoffset)
        if yoffset == "":
            yoffset = "0"
            self.YOffsetEdit.setText(yoffset)
        if amp == "":
            amp = "0"
            self.AmplitudeEdit.setText(amp)
        if freq == "":
            freq = "0"
            self.FrequencyEdit.setText(freq)
        if rot == "":
            rot = "0"
            self.axysRotEdit.setText(rot);

        # Check ccd sync and override any frequency value in case
        if self.FreqSyncCcdCheckBox.isChecked():
            freq = "0"

        # PREVENT setting zero frequency without CCD sync!!
        if float(freq) == 0 and not self.FreqSyncCcdCheckBox.isChecked():
            print 'Forcing frequency to 10'
            freq = "10"
        
        self.setReq(xoffset, yoffset, amp, freq, rot)
        self.start()

    def setPix(self, status):
        if status == AOConstants.AOStatesCode['STATE_READY'] or \
           status == AOConstants.AOStatesCode['STATE_OPERATING']:
            self.pix.setPixmap(cfg_gui.pixmaps.pix_ok)
        else:
            self.pix.setPixmap(cfg_gui.pixmaps.pix_ko)

        

    ##
    ## PRIVATE METHODS interacting with RTDB

    def setReq(self, xoffset, yoffset, amp, freq, rot):
        print "Setting: x: %5.2f y: %5.2f amp: %5.2f f: %5.2f rot: %5.2f" % (float(xoffset), float(yoffset), float(amp), float(freq), float(rot))     
        msglib.SetRealVar(cfg.varname('ttctrl',"OFFX.REQ"), float(xoffset)/1e3)
        msglib.SetRealVar(cfg.varname('ttctrl',"OFFY.REQ"), float(yoffset)/1e3)
        msglib.SetRealVar(cfg.varname('ttctrl',"AMP.REQ"), (float(amp)/1e3))
        msglib.SetRealVar(cfg.varname('ttctrl',"FREQ.REQ"), float(freq)) 
        msglib.SetRealVar(cfg.varname('ttctrl',"AXYSROT.REQ"), float(rot)) 
        
    def start(self):
        msglib.SetIntVar(cfg.varname('ttctrl',"GO"), 1)


# Main
if __name__ == "__main__":
    from engineeringGui import EngineeringGui
    import sys
    app = QApplication(sys.argv)
    g = EngineeringGui( name='ttgui')
    myGui = tt_gui(g)
    app.setMainWidget(myGui)
    myGui.show()
    app.exec_loop()
