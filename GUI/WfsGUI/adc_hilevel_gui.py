#!/usr/bin/env python
#
# adc_hilevel_gui
#
# GUI tool for ADC high_level control
#
 
from qt import *

from AdOpt import cfg, AOMsgCode


## Import the C library wrapper

from AdOpt.wrappers import msglib


#@+Class: adc_hilevel_gui
#
# Main window for the GUI tool.
#@

class adc_hilevel_gui(QDialog):

    def __init__(self, AOwindow):
        QDialog.__init__(self)

        self.name = "ADC hi-level"

        ## Draw the widgets

        self.gridLayout = QGridLayout( self, 11, 4, 10)
        self.gridLayout.setAutoAdd(1)

        self.header1 = QLabel("", self)
        self.header2 = QLabel("<b>Cur value</b>", self)
        self.header3 = QLabel("<b>New value</b>", self)
        self.dummy3 = QLabel("", self)

        self.label1 = QLabel("Delta Dispersion angle (degrees):", self)
        self.axisValue = QLabel("unknown", self)
        self.axisEdit = QLineEdit("", self)
        self.axisButton = QPushButton("Set", self)

        self.label2 = QLabel("ADC1 Offset (degrees):", self)
        self.angle1Value = QLabel("unknown", self)
        self.angle1Edit = QLineEdit("", self)
        self.angle1Button = QPushButton("Set", self)

        self.label3 = QLabel("ADC2 Offset (degrees):", self)
        self.angle2Value = QLabel("unknown", self)
        self.angle2Edit = QLineEdit("", self)
        self.angle2Button = QPushButton("Set", self)
        
        self.connect( self.axisButton, SIGNAL("clicked()"), self.slotButtonDeltaSet)
        self.connect( self.angle1Button, SIGNAL("clicked()"), self.slotButton1Set)
        self.connect( self.angle2Button, SIGNAL("clicked()"), self.slotButton2Set)

        # register slots
        self.var_off1_cur = cfg.varname('wfsarb', 'ADC1.OFFSET.CUR')
        self.var_off1_req = cfg.varname('wfsarb', 'ADC1.OFFSET.REQ')
        self.var_off2_cur = cfg.varname('wfsarb', 'ADC2.OFFSET.CUR')
        self.var_off2_req = cfg.varname('wfsarb', 'ADC2.OFFSET.REQ')
        
        self._off1 = 0
   	self._off2 = 0

        AOwindow.Notify( self.var_off1_cur, self.slotNotify, getNow = True);
        AOwindow.Notify( self.var_off2_cur, self.slotNotify, getNow = True);


    def redisplay(self):
        axis = (self._off1 + self._off2)/2.0
        angle = abs(self._off1 - axis)
        
        self.axisValue.setText('%5.2f' % angle)
        self.angle1Value.setText('%5.2f' % self._off1)
        self.angle2Value.setText('%5.2f' % self._off2) 


    ## +Method: slotNotify
    ##
    ## Called by AOGUI when something we are interested in is arriving.

    def slotNotify(self, args):
        (message) = args
        code = message['code']

        ## VARREPLY
        ##
        ## When one of the watched variables change, update our display

        if code == AOMsgCode['VARREPLY'] or code == AOMsgCode['VARCHANGD']:
            varname = message['name']
            value = message['value']

        if varname == self.var_off1_cur:
            self._off1 = value
            self.redisplay()

        if varname == self.var_off2_cur:
            self._off2 = value
            self.redisplay()

    ##+Method: slotButtonSet
    ##
    ## Handler for the Set button. Sends the parameters to the TT mirror

    def slotButtonDeltaSet(self):

        off1 = -float(self.axisEdit.text().latin1())
        off2 = -off1

        msglib.SetVar( self.var_off1_req, msglib.REAL_VARIABLE, 1, off1)
        msglib.SetVar( self.var_off2_req, msglib.REAL_VARIABLE, 1, off2)

    def slotButton1Set(self):

        off1 = float(self.angle1Edit.text().latin1())
        
        msglib.SetVar( self.var_off1_req, msglib.REAL_VARIABLE, 1, off1)
        
    def slotButton2Set(self):

        off2 = float(self.angle2Edit.text().latin1())
        
        msglib.SetVar( self.var_off2_req, msglib.REAL_VARIABLE, 1, off2)
        
# Testing main
if __name__ == "__main__":
    from engineeringGui import EngineeringGui
    import sys
    app = QApplication(sys.argv)
    g = EngineeringGui( name='ttllgui')
    myGui = adc_hilevel_gui(g)
    app.setMainWidget(myGui)
    myGui.show()
    app.exec_loop()

