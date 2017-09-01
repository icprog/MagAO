#!/usr/bin/env python
#
#+File: Stages_gui
#
# GUI tool for Bayside stages control 

import time
from qt import *
from AdOpt import AOConstants, AOMsgCode, cfg

## Import the C library wrapper

from AdOpt.wrappers import msglib

# +Class: singleStageGui
#
# Widget group for a stage control

class singleStageGui( QWidget):

    def __init__( self, parent, prefix, name):
        QWidget.__init__(self, parent)

        self.prefix    = prefix

        self.layout = QVBoxLayout(self, 10)
        self.row1   = QHBoxLayout( self.layout, 10)
        self.row2   = QHBoxLayout( self.layout, 10)
        self.row3   = QHBoxLayout( self.layout, 10)

        self.label_name    = QLabel( name, self)
        self.label_status  = QLabel( "      ", self)

        self.enable = QLabel("Enable:", self)
        self.enable_stage = QCheckBox( "Stage", self)
        self.software_limit = QCheckBox( "Software limit", self)
        self.switch_limit   = QCheckBox( "Limit switches", self)
        self.always_active   = QCheckBox( "Control always active", self)

        self.row1.addWidget( self.label_name)
        self.row1.addWidget( self.label_status)
        self.row1.addWidget( self.enable)
        self.row1.addWidget( self.enable_stage)
        self.row1.addWidget( self.software_limit)
        self.row1.addWidget( self.switch_limit)
        self.row1.addWidget( self.always_active)

        self.label_curpos  = QLabel( "      ", self)
        self.label_current = QLabel( "      ", self)
        self.edit_newpos   = QLineEdit( "      ", self)
        self.button_move   = QPushButton( "Move", self)
        self.spacer        = QLabel( "            ", self)

        self.row2.addWidget( self.label_curpos)
        self.row2.addWidget( self.label_current)
        self.row2.addWidget( self.button_move)
        self.row2.addWidget( self.edit_newpos)
        self.row2.addWidget( self.spacer)

        self.error_status  = QLabel("No error", self)
        self.button_recover = QPushButton("Recover", self)
        self.button_homing = QPushButton("Start homing", self)


        self.row3.addWidget( self.error_status)
        self.row3.addWidget( self.button_recover)
        self.row3.addWidget( self.button_homing)


        # Make a few variables with the same name as MsgD-RTDB variables

        self.v = {}
        self.v[ 'POS.CUR'] = 0
        self.v[ 'STATUS'] = 0
        self.v[ 'ERRMSG'] = ""
        self.v[ 'CURRENT'] = 0
        self.v[ 'ENABLE.CUR'] = 0
        self.v[ 'HIEND'] = 0
        self.v[ 'LOEND'] = 0
        self.v[ 'NAME'] = ""
        self.v[ 'LIMIT.CUR'] = 0
        self.v[ 'ERRCODE'] = 0

        for name in self.v.keys():
            parent.AOwindow.Notify( cfg.varname( prefix, name), parent.slotNotify, getNow = True)
        
        self.connect( self.button_move, SIGNAL("clicked()"), self.slotButtonMove)
        self.connect( self.enable_stage, SIGNAL("clicked()"), self.slotButtonEnable)
        self.connect( self.always_active, SIGNAL("clicked()"), self.slotButtonEnable)
        self.connect( self.software_limit, SIGNAL("clicked()"), self.slotButtonSoftwareLimit)
        self.connect( self.switch_limit, SIGNAL("clicked()"), self.slotButtonSwitchLimit)
        self.connect( self.button_recover, SIGNAL("clicked()"), self.slotButtonRecoverLimit)
        self.connect( self.button_homing, SIGNAL("clicked()"), self.slotButtonHoming)

    def slotButtonMove(self):
        try:
            newpos = float(self.edit_newpos.text().latin1())
        except:
            QMessageBox.information ( self, "", "Error in position command", QMessageBox.Ok)
            return
            
        msglib.SetRealVar( cfg.varname( self.prefix, 'POS.REQ'), newpos)

    def computeEnable(self, request):
        '''
        Compute the right ENABLE.REQ value given a 1/0 request
        and correcting for the always_active value
        '''
        if not request:
            return AOConstants.AOStatesCode['ENABLE_OFF']
        if self.always_active.isChecked():
            return AOConstants.AOStatesCode['ENABLE_ALWAYS']
        return AOConstants.AOStatesCode['ENABLE_OFFAFTERMOVE']

    def slotButtonEnable(self):
        a = self.enable_stage.isChecked()
        msglib.SetIntVar( cfg.varname( self.prefix, 'ENABLE.REQ'), self.computeEnable(a))

    def slotButtonSoftwareLimit(self):
        a = self.v[ 'LIMIT.CUR']

        if self.software_limit.isChecked():
            a = 0x0C + (a & 0x03)
        else:
            a = 0x00 + (a & 0x03)

        msglib.SetIntVar( cfg.varname( self.prefix, 'LIMIT.REQ'), a)

    def slotButtonSwitchLimit(self):
        a = self.v[ 'LIMIT.CUR']

        if self.switch_limit.isChecked():
            a = 0x03 + (a & 0x0C)
        else:
            a = 0x00 + (a & 0x0C)

        msglib.SetIntVar( cfg.varname( self.prefix, 'LIMIT.REQ'), a)

    def slotButtonRecoverLimit(self):
        if self.v[ 'ERRCODE'] == 21571:
            # Negative limit switch
            self.recover_limit(+3)

        if self.v[ 'ERRCODE'] == 21570:
            # Positive limit switch
            self.recover_limit(-3)


    def recover_limit(self, amount):
        # Disable limit switches (and save previous status)
        switches = self.v[ 'LIMIT.CUR']
        msglib.SetIntVar( cfg.varname( self.prefix, 'LIMIT_REQ'), 0)
        time.sleep(0.5)

        # Force quit error
        msglib.SetIntVar( cfg.varname( self.prefix, 'ENABLE.REQ'), AOConstants.AOStatesCode['ENABLE_OFFAFTERMOVE'])
        msglib.SetIntVar( cfg.varname( self.prefix, 'ENABLE.REQ'), AOConstants.AOStatesCode['ENABLE_OFF'])
        time.sleep(0.5)
       
        # Move away
        msglib.SetIntVar( cfg.varname( self.prefix, 'ENABLE.REQ'), AOConstants.AOStatesCode['ENABLE_OFFAFTERMOVE'])
        msglib.SetRealVar( cfg.varname( self.prefix, 'POS.REQ'), float(self.v[ 'POS.CUR'] + amount))
        time.sleep(4)

        msglib.SetIntVar( cfg.varname( self.prefix, 'ENABLE.REQ'), AOConstants.AOStatesCode['ENABLE_OFF'])
        time.sleep(0.5)
        msglib.SetIntVar( cfg.varname( self.prefix, 'LIMIT.REQ'), switches)
        time.sleep(0.5)
        msglib.SetIntVar( cfg.varname( self.prefix, 'ENABLE.REQ'), self.computeEnable(1))
        time.sleep(0.5)


    def slotButtonHoming(self):
        msglib.SetIntVar( cfg.varname( self.prefix, 'SPECIAL'), 1)

    def redisplay(self):
        name = self.v['NAME']
        self.label_name.setText(name)

        self.label_status.setText( AOConstants.AOStatesType[ self.v['STATUS']])

       	self.enable_stage.setChecked(self.v['ENABLE.CUR'] != AOConstants.AOStatesCode['ENABLE_OFF'] )
        if (self.v['ENABLE.CUR'] != AOConstants.AOStatesCode['ENABLE_OFF']):
       		self.always_active.setChecked(self.v['ENABLE.CUR'] == AOConstants.AOStatesCode['ENABLE_ALWAYS'] )
        self.software_limit.setChecked( self.v['LIMIT.CUR'] & 0xC)
        self.switch_limit.setChecked( self.v['LIMIT.CUR'] & 0x3)

        self.label_curpos.setText( "Position: %5.3f" % float(self.v['POS.CUR']))
        self.label_current.setText( "Current: %5.2f A" % float(self.v['CURRENT']))

        s = self.v['ERRMSG']
        c = QColor(0,255,0)
        e = False

        self.error_status.setText(s)
        self.error_status.setEraseColor(c)
        self.button_recover.setEnabled(e)



    ## +Method: slotTimeout
    ##
    ## Handler for the timer object. (if some periodic operation is needed)

    def slotTimeout(self):
        pass
    

class stages_gui(QDialog):

    def __init__(self, AOwindow):
        QDialog.__init__(self)

        self.AOwindow = AOwindow
        self.name = "Bayside stages"

        self.layout = QVBoxLayout( self)
        self.stages = []

        self.stages.append( singleStageGui( self, 'baysidex', "X"))
        self.stages.append( singleStageGui( self, 'baysidey', "Y"))
        self.stages.append( singleStageGui( self, 'baysidez', "Z"))

        for s in self.stages:
            self.layout.addWidget(s)

        #QObject.connect( AOwindow, PYSIGNAL( "%d" % AOMsgCode['VARREPLY']), self.slotNotify)
        #QObject.connect( AOwindow, PYSIGNAL( "%d" % AOMsgCode['VARCHANGD']), self.slotNotify)

    ## +Method: slotNotify
    ##

    def slotNotify(self, args):
        (message) = args
        code = message['code']

        ## VARREPLY
        ##
        ## When one of the watched variables change, update our display

        if code == AOMsgCode['VARREPLY'] or code == AOMsgCode['VARCHANGD']:
            varname = message['name']
            type = message['type']
            owner = message['owner'] or 'global'
            value = message['value']

            name = cfg.split_varname(varname).name

            for s in self.stages:
                if s.prefix == varname[:len(s.prefix)]:
                   s.v[name] = value
                   s.redisplay()



# Main
if __name__ == "__main__":
    import sys
    from engineeringGui import EngineeringGui
    app = QApplication(sys.argv)
    g = EngineeringGui( name='stagesgui')
    myGui = stages_gui(g)
    app.setMainWidget(myGui)
    myGui.show()
    app.exec_loop()

 
