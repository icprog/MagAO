#!/usr/bin/env python
#
# tt_lowlevel_gui
#
# GUI tool for tip-tilt mirror control
#
 
from qt import *
from AdOpt import cfg, AOMsgCode

## Import the C library wrapper

from AdOpt.wrappers import msglib


#@+Class: TT_Main
#
# Main window for the GUI tool.
#@

class tt_lowlevel_gui(QDialog):

    def __init__(self, AOwindow):
        QDialog.__init__(self)

        self.name = "Tip-tilt low level"
        self.taskname = cfg.taskname('ttctrl')

        ## Draw the widgets

        self.gridLayout = QGridLayout( self, 11, 3, 10)
        self.gridLayout.setAutoAdd(1)

        self.label = QLabel("Status:", self);
        self.status = QLabel("unknown", self)
        self.dummy1 = QLabel("", self);
        

        # Wave 1

        self.header1 = QLabel("<b>WAVE 1</b>", self)
        self.header2 = QLabel("<b>Cur value</b>", self)
        self.header3 = QLabel("<b>New value</b>", self)

        self.label1 = QLabel("Amplitude (volts, p2v):", self)
        self.AmplitudeValue1 = QLabel("unknown", self)
        self.AmplitudeEdit1 = QLineEdit("", self)

        self.label2 = QLabel("Frequency (Hertz):", self)
        self.FrequencyValue1 = QLabel("unknown", self)
        self.FrequencyEdit1 = QLineEdit("", self)

        self.label3 = QLabel("Offset (volts):", self)
        self.OffsetValue1 = QLabel("unknown", self)
        self.OffsetEdit1 = QLineEdit("", self)

        self.label4 = QLabel("Phase (degrees):", self)
        self.PhaseValue1 = QLabel("unknown", self)
        self.PhaseEdit1 = QLineEdit("", self)

        # Wave 2

        self.header4 = QLabel("<b>WAVE 2</b>", self)
        self.header5 = QLabel("<b>Cur value</b>", self)
        self.header6 = QLabel("<b>New value</b>", self)

        self.label5 = QLabel("Amplitude (volts, p2v):", self)
        self.AmplitudeValue2 = QLabel("unknown", self)
        self.AmplitudeEdit2 = QLineEdit("", self)

        self.label6 = QLabel("Frequency (Hertz):", self)
        self.FrequencyValue2 = QLabel("unknown", self)
        self.FrequencyEdit2 = QLineEdit("", self)
        self.FrequencyEdit2.setEnabled(False)

        self.label7 = QLabel("Offset (volts):", self)
        self.OffsetValue2 = QLabel("unknown", self)
        self.OffsetEdit2 = QLineEdit("", self)

        self.label8 = QLabel("Phase (degrees):", self)
        self.PhaseValue2 = QLabel("unknown", self)
        self.PhaseEdit2 = QLineEdit("", self)

        # Wave 3

        self.header7 = QLabel("<b>WAVE 3</b>", self)
        self.header8 = QLabel("<b>Cur value</b>", self)
        self.header9 = QLabel("<b>New value</b>", self)

        self.label9 = QLabel("Amplitude (volts, p2v):", self)
        self.AmplitudeValue3 = QLabel("unknown", self)
        self.AmplitudeEdit3 = QLineEdit("", self)

        self.label10 = QLabel("Frequency (Hertz):", self)
        self.FrequencyValue3 = QLabel("unknown", self)
        self.FrequencyEdit3 = QLineEdit("", self)
        self.FrequencyEdit3.setEnabled(False)

        self.label11 = QLabel("Offset (volts):", self)
        self.OffsetValue3 = QLabel("unknown", self)
        self.OffsetEdit3 = QLineEdit("", self)

        self.label12 = QLabel("Phase (degrees):", self)
        self.PhaseValue3 = QLabel("unknown", self)
        self.PhaseEdit3 = QLineEdit("", self)


        self.dummy2 = QLabel("", self)
        self.dummy3 = QLabel("", self)
        self.button_set = QPushButton("Set", self)
        self.connect( self.button_set, SIGNAL("clicked()"), self.slotButtonSet)

        self.AOwindow = AOwindow
        QObject.connect( AOwindow, PYSIGNAL( "%d" % AOMsgCode['CLNREADY']), self.slotNotify)

        self.rtdbVars = [ cfg.varname('ttctrl', x) for x in \
                   ['LOWAMP.CUR', 'LOWFREQ.CUR', 'LOWOFF.CUR', 'LOWPHASE.CUR', 'GO', 'ERRMSG']]

        ## We want a message when the TTCtrl is ready
        
        msglib.SendMessage( "", AOMsgCode['WAITCLRDY'], self.taskname)


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
                self.AOwindow.Notify(var, self.slotNotify, getNow = True)
            return

        ## VARREPLY
        ##
        ## When one of the watched variables change, update our display

        if code == AOMsgCode['VARREPLY'] or code == AOMsgCode['VARCHANGD']:

            varname = message['name']
            type = message['type']
            owner = message['owner'] or 'global'
            value = message['value']

            if varname == cfg.varname('ttctrl', "GO"):
                self.status.setText( ('Not running', 'Running')[value])
                return

            if varname == cfg.varname('ttctrl', "ERRMSG"):
                self.status.setText(  value)
                return

            if varname == cfg.varname('ttctrl', "LOWAMP.CUR"):
                self.AmplitudeValue1.setText(self._format(value[0]*2))
                self.AmplitudeValue2.setText(self._format(value[1]*2))
                self.AmplitudeValue3.setText(self._format(value[2]*2))
                if self.AmplitudeEdit1.text().latin1() == "":
                    self.AmplitudeEdit1.setText(self._format(value[0]*2))
                if self.AmplitudeEdit2.text().latin1() == "":
                    self.AmplitudeEdit2.setText(self._format(value[1]*2))
                if self.AmplitudeEdit3.text().latin1() == "":
                    self.AmplitudeEdit3.setText(self._format(value[2]*2))
                return

            if varname == cfg.varname('ttctrl', "LOWFREQ.CUR"):
                self.FrequencyValue1.setText(self._format(value[0]))
                self.FrequencyValue2.setText(self._format(value[1]))
                self.FrequencyValue3.setText(self._format(value[2]))
                if self.FrequencyEdit1.text().latin1() == "":
                    self.FrequencyEdit1.setText(self._format(value[0]))
                if self.FrequencyEdit2.text().latin1() == "":
                    self.FrequencyEdit2.setText(self._format(value[1]))
                if self.FrequencyEdit3.text().latin1() == "":
                    self.FrequencyEdit3.setText(self._format(value[2]))
                return

            if varname == cfg.varname('ttctrl', "LOWOFF.CUR"):
                self.OffsetValue1.setText(self._format(value[0]))
                self.OffsetValue2.setText(self._format(value[1]))
                self.OffsetValue3.setText(self._format(value[2]))
                if self.OffsetEdit1.text().latin1() == "":
                    self.OffsetEdit1.setText(self._format(value[0]))
                if self.OffsetEdit2.text().latin1() == "":
                    self.OffsetEdit2.setText(self._format(value[1]))
                if self.OffsetEdit2.text().latin1() == "":
                    self.OffsetEdit2.setText(self._format(value[2]))
                return

            if varname == cfg.varname('ttctrl', "LOWPHASE.CUR"):
                self.PhaseValue1.setText(self._format(value[0]))
                self.PhaseValue2.setText(self._format(value[1]))
                self.PhaseValue3.setText(self._format(value[2]))
                if self.PhaseEdit1.text().latin1() == "":
                    self.PhaseEdit1.setText(self._format(value[0]))
                if self.PhaseEdit2.text().latin1() == "":
                    self.PhaseEdit2.setText(self._format(value[1]))
                if self.PhaseEdit3.text().latin1() == "":
                    self.PhaseEdit3.setText(self._format(value[2]))
                return


    ##+Method: slotButtonSet
    ##
    ## Handler for the Set button. Sends the parameters to the TT mirror

    def slotButtonSet(self):
        offset1 = self.OffsetEdit1.text().latin1()
        amp1 = self.AmplitudeEdit1.text().latin1()
        freq1 = self.FrequencyEdit1.text().latin1()
        phase1 = self.PhaseEdit1.text().latin1()

        offset2 = self.OffsetEdit2.text().latin1()
        amp2 = self.AmplitudeEdit2.text().latin1()
        freq2 = self.FrequencyEdit1.text().latin1()	# Frequecy locked to freq1
        phase2 = self.PhaseEdit2.text().latin1()

        offset3 = self.OffsetEdit3.text().latin1()
        amp3 = self.AmplitudeEdit3.text().latin1()
        freq3 = self.FrequencyEdit1.text().latin1()     # Frequency locked to freq1
        phase3 = self.PhaseEdit3.text().latin1()

        if offset1 == "":
            offset1 = 0
        if amp1 == "":
            amp1 = 0
        if freq1 == "":
            freq1 =0
        if phase1 == "":
            phase1 = 0

        if offset2 == "":
            offset2 = 0
        if amp2 == "":
            amp2 =0
        if freq2 == "":
            freq2 = 0
        if phase2 == "":
            phase2 = 0

        if offset3 == "":
            offset3 = 0
        if amp3 == "":
            amp3 = 0
        if freq3 == "":
            freq3 = 0
        if phase3 == "":
            phase3 = 0

        freq = ( float(freq1), float(freq2), float(freq3))
        amp = ( float(amp1)/2.0, float(amp2)/2.0, float(amp3)/2.0)
        offset = ( float(offset1), float(offset2), float(offset3))
        phase = ( float(phase1), float(phase2), float(phase3))

        msglib.SetVar( cfg.varname('ttctrl', "LOWFREQ.REQ"), msglib.REAL_VARIABLE, 3, freq)
        msglib.SetVar( cfg.varname('ttctrl', "LOWAMP.REQ"), msglib.REAL_VARIABLE, 3, amp)
        msglib.SetVar( cfg.varname('ttctrl', "LOWOFF.REQ"), msglib.REAL_VARIABLE, 3, offset)
        msglib.SetVar( cfg.varname('ttctrl', "LOWPHASE.REQ"), msglib.REAL_VARIABLE, 3, phase)

        # New TTCtrl will perform a start or a restart
        msglib.SetIntVar(cfg.varname('ttctrl', "GO"), 1)

    def _format(self,value):
        return "%.2f" % value 

# Testing main
if __name__ == "__main__":
    from engineeringGui import EngineeringGui
    import sys
    app = QApplication(sys.argv)
    g = EngineeringGui( name='ttllgui')
    myGui = tt_lowlevel_gui(g)
    app.setMainWidget(myGui)
    myGui.show()
    app.exec_loop()

