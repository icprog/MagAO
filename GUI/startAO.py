#!/usr/bin/env python

#@File: startup.py
#
# Startup the system GUI, checking for:
#    - Chiller on (only alert)
#    - MsgD and MirrorCtrl started
#
#@

import os, sys
from qt import *

from AdOpt.QtDesigner.startup_dialog import AbstractSecurityDialog
from AdOpt.AOprocess import *
from AdOpt import processControl, cfg


class SecurityDialog(AbstractSecurityDialog):

    def __init__(self, app, parent=None):
        AbstractSecurityDialog.__init__(self, parent)
        
        self.app = app
        self.parent = parent
        
        self.processCheck = QTimer(self)
        self.connect( self.processCheck, SIGNAL("timeout()"), self.slotProcessCheck)
        self.processCheck.start(1000)

            
    def slotProcessCheck(self):
        if self.checkMainProcess('msgdrtdb'):
            self.msgdStatus.setPaletteForegroundColor(QColor(0,180,0))
            self.msgdStatus.setText("MsgD-RTDB is running")
        else:
            self.msgdStatus.setPaletteForegroundColor(QColor(255,0,0))
            self.msgdStatus.setText("MsgD-RTDB is not started")

        if self.checkMainProcess('mirrorctrl'):
            self.mirrorCtrlStatus.setPaletteForegroundColor(QColor(0,180,0))
            self.mirrorCtrlStatus.setText("Mirror Controller is running")
        else:
            self.mirrorCtrlStatus.setPaletteForegroundColor(QColor(255,0,0))
            self.mirrorCtrlStatus.setText("Mirror Controller is not started")
            
        if self.checkMainProcess('pinger'):
            self.pingerStatus.setPaletteForegroundColor(QColor(0,180,0))
            self.pingerStatus.setText("Pinger is running")
        else:
            self.pingerStatus.setPaletteForegroundColor(QColor(255,0,0))
            self.pingerStatus.setText("Pinger is not started")
            
            
    def continueButton_clicked(self):
        self.accept()
        print 'Starting AO Main Gui...'
        self.parent.show()

        if len(sys.argv)>1:
            sys.argv.pop(0)
            while len(sys.argv)>0:
                processControl.startProcessByName( sys.argv.pop(0))
        
        
    def abortButton_clicked(self):
        self.reject()
        print 'AO startup aborted'
        self.app.quit()

    def checkMainProcess(self, name):
        id = processControl.getProcessID(name)
        if id >0:
            return True
        return False
            
        return False
    

if __name__ == '__main__':

    subs = os.getenv('ADOPT_SUBSYSTEM')
    if subs==None:
        print 'No ADOPT_SUBSYSTEM environment variable set.'
        print 'Please set the ADOPT_SUBSYSTEM environment variable to either "WFS" or "ADSEC"'
        sys.exit(0)


    # Check that we are running on the right machine
    cmd = os.path.basename(sys.argv[0])

    if cmd=='wfseng' and subs!='WFS':
        print 'wfseng can only be run on the WFS computer.'
        sys.exit(0)

    if cmd=='adsceng' and subs!='ADSEC':
        print 'adsceng can only be run on the ADSEC computer.'
        sys.exit(0)


    # create the application
    app = QApplication(sys.argv)
    
    # Create the main GUI
    main = AOprocess( app, multi=1, rowSize=20, caption = "%s software" % cfg.subsystem, showStatus=False)
    
    main.addTitle("<b>%s INTERFACES</b>" % cfg.subsystem)
    main.addSeparator()
    
    for k in cfg.interfaces.split():
        main.addProcess( k)
    
    app.setMainWidget(main)

#   This security dialog is now useless
    
#    print 'Showing security dialog...'
#    win = SecurityDialog(app, main)
#    win.show()
   
    main.show() 
    app.exec_loop()


    
