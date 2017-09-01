#!/usr/bin/env python
#
# sc_gui
#
# GUI tool for slope computer control
#

import sys, string
from qt import *

from AdOpt import cfg, AOConstants, AOMsgCode
import cfg_gui

## Import the C library wrapper
from AdOpt.wrappers import msglib

        
#@+Class: sc_gui
#
# Main window for the GUI tool.
#@
class sc_gui(QDialog):
    
    def __init__(self, AOwindow):
        QDialog.__init__(self)
       
        cfg_gui.init()
 
        self.setFixedSize(550, 420);

        self.name = 'Slope computer'
        self.ctrlname = 'slopecompctrl'
        self.descr = 'Slope computer configuration'

        ## Draw the widgets
        self.gridLayout = QGridLayout(self, 6, 4, 20)
        self.gridLayout.setAutoAdd(1)

        self.label = QLabel("Status:", self)
        self.status = QLabel("unknown", self)
        self.dummy1 = QLabel("", self);
        self.pix = QPushButton("", self);


        self.header1 = QLabel("<b>Parameter</b>", self)
        self.header2 = QLabel("<b>Current</b>", self)
        self.header3 = QLabel("<b>Requested</b>", self)
        self.dummy3  = QLabel("", self);

        self.slopeClipLabel = QLabel("Slope clipping value:", self);
        self.slopeClipValue = QLabel("unknown", self)
        self.slopeClipEdit = QLineEdit("", self)
        self.slopeClipEdit.setMaximumWidth(70)
        self.slopeClipDummy  = QLabel("", self);

        self.decimationLabel = QLabel("Diagnostic decimation:", self);
        self.decimationValue = QLabel("unknown", self)
        self.decimationEdit = QLineEdit("", self)
        self.decimationEdit.setMaximumWidth(70)
        self.decimationDummy  = QLabel("", self);

        self.dummy2 = QLabel("", self)
        self.dummy3 = QLabel("", self)
        self.applyButton = QPushButton("Apply", self)
        self.connect( self.applyButton, SIGNAL("clicked()"), self.slotButtonApply)
        self.dummy6  = QLabel("", self);

        self.AOwindow = AOwindow

        QObject.connect( self.AOwindow, PYSIGNAL( "%d" % AOMsgCode['CLNREADY']), self.slotNotify)

        self.rtdbVars = [ cfg.varname(self.ctrlname, x) for x in \
                   ['SLOPECLIP.CUR', 'MASTERD.DECIMATION.CUR', 'ERRMSG', 'STATUS']]

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

            if varname == cfg.varname( self.ctrlname,'STATUS'):
                self.status.setText( AOConstants.AOStatesType[ value])
                self.setPix(value)

            elif varname == cfg.varname( self.ctrlname,"ERRMSG"):
                self.status.setText(value)

            elif varname == cfg.varname( self.ctrlname, 'SLOPECLIP.CUR'):
                self.slopeClipValue.setText(self._formatFloat(value))
                if self.slopeClipEdit.text().latin1() == "":
                    self.slopeClipEdit.setText(self._formatFloat(value))

            elif varname == cfg.varname( self.ctrlname, 'MASTERD.DECIMATION.CUR'):
                self.decimationValue.setText(self._formatInt(value))
                if self.decimationEdit.text().latin1() == "":
                    self.decimationEdit.setText(self._formatInt(value))

            return

        print "Got unexpected message: %d" % message['code']

    def _formatFloat(self,value):
        return "%g" % value            
            
    def _formatInt(self,value):
        return "%d" % value            
            
    def strings_to_zero(self, value):
        if value == "" or value == "unknown":
            return "0"

        return value

    ## +Method: slotExitButton
    ##
    ## Handler for the Exit button. It terminates the application immediately
    def slotExitButton(self):
        self.close()
        
    ##+Method: slotButtonApply
    ##
    ## Handler for the Set button. Sends the parameters to the TT mirror
    def slotButtonApply(self):
        clip = float( self.slopeClipEdit.text().latin1())
        decimation = int( self.decimationEdit.text().latin1())

        print "Setting: clip: %g  decimation: %d" % (clip, decimation)
        msglib.SetRealVar(cfg.varname('slopecompctrl',"SLOPECLIP.REQ"), clip)
        msglib.SetIntVar(cfg.varname('slopecompctrl',"MASTERD.DECIMATION.REQ"), decimation)

    def setPix(self, status):
        if status == AOConstants.AOStatesCode['STATE_READY'] or \
           status == AOConstants.AOStatesCode['STATE_OPERATING']:
            self.pix.setPixmap(cfg_gui.pixmaps.pix_ok)
        else:
            self.pix.setPixmap(cfg_gui.pixmaps.pix_ko)

        

# Main
if __name__ == "__main__":
    from engineeringGui import EngineeringGui
    import sys
    app = QApplication(sys.argv)
    g = EngineeringGui( name='ttgui')
    myGui = sc_gui(g)
    app.setMainWidget(myGui)
    myGui.show()
    app.exec_loop()
