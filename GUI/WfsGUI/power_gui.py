#!/usr/bin/env python
#
#+File: power_gui
#
# GUI tool for power control
#+

import string, time, types
from qt import *
from qtnetwork import *
from AdOpt import AOConstants, AOMsgCode, cfg, processControl
import cfg_gui

# Import the C library wrapper

from AdOpt.wrappers import msglib

# +Class: power_gui
#
# Main window for the GUI tool.

class power_gui( QDialog):
    
    def __init__(self, AOwindow):
        QDialog.__init__(self)

        cfg_gui.init()

        self.name = "Power controller"

        gridLayout = QGridLayout( self, 1, 4, 5)

        gridLayout.setAutoAdd(1)


        self.turnOnLabel = QLabel("BCU/TT/CL power ON", self)
        self.turnOnLabel2 = QLabel("--->", self)
        self.turnOnButton = QPushButton("Turn on", self)
        self.turnOnLabel3 = QLabel("", self)

        self.spacerLabel1 = QLabel("", self)
        self.spacerLabel2 = QLabel("", self)
        self.spacerLabel3 = QLabel("", self)
        self.spacerLabel4 = QLabel("", self)

        self.turnOffLabel = QLabel("BCU/TT/CL power OFF", self)
        self.turnOffLabel2 = QLabel("--->", self)
        self.turnOffButton = QPushButton("Turn off", self)
        self.turnOffLabel3 = QLabel("", self)

        self.connect( self.turnOnButton, SIGNAL("clicked()"), self.slotTurnOnButton)
        self.connect( self.turnOffButton, SIGNAL("clicked()"), self.slotTurnOffButton)

        self.lowlevelCheck = QCheckBox("Enable Low-level\npower control", self)
        self.spacerLabel6 = QLabel("", self)
        self.spacerLabel7 = QLabel("", self)
        self.spacerLabel8 = QLabel("", self)

        self.connect( self.lowlevelCheck, SIGNAL("clicked()"), self.slotLowLevelCheckbox)

        self.OnButtonGroup = QButtonGroup(self)
        self.OffButtonGroup = QButtonGroup(self)

        self.OnButtonGroup.hide()
        self.OffButtonGroup.hide()

        dummy1 = QLabel("", self)
        dummy2 = QLabel("", self)

	# Load the list of devices - but only the ones present in the cfg file
        devices = ['moxadioccd','moxadiodrv','picctrl', 'visaopwrctrl']

        self.devices = []	
	for device in devices:	
            if cfg.cfg.has_key(device):
		self.devices.append(device)	

	# These will be ANDed with the configuration file to display only the devices actually present.
        ordering = ['bcu', 'ccd39', 'ccd47', 'adc1', 'adc2', 'fw1', 'rirot', 'xstage', 'ystage', 'zstage', 'pic','lamp','visaogimbal', 'visaomotors', 'visaoshutter', 'visaofstop', 'visaofans', 'visaots8', 'visaocomp', 'visaoswitch']

        self.items = {}

        self.onText = " On "
        self.offText = " Off "
        self.unreachableText = "offline"


        self.statusVars = {}
        self.devicelists = {}

        for device in self.devices:
            self.devicelists[device] = []
            num_items = cfg.cfg[device]['num_items'].Value()
            for i in range(num_items):
               	code = cfg.cfg[device]['item%d.code' % i].Value()
                if not code in ordering:
                    continue
               	self.items[code] = cfg.varname(device, code)
               	self.devicelists[device].append(code)

            self.statusVars[device] = cfg.varname( device, 'STATUS')
            AOwindow.Notify( self.statusVars[device], self.slotStatusChange, getNow = True)
 

        self.labels = {}
        self.states = {}
        self.on_buttons = {}
        self.off_buttons = {}
        self.id_buttonOn = {}

	# Remove unused devices from the ordering array
	self.ordering = []
	for device in ordering:
		if self.items.has_key(device):
			self.ordering.append(device)

        for code in self.ordering:
            var_prefix = self.items[code]
            msglib.GetVar( var_prefix + '.NAME')
            self.labels[code] = QLabel("          ", self)
            self.states[code] = QLabel("unknown", self)
            id_on = self.OnButtonGroup.insert( QPushButton("Turn on", self))
            id_off = self.OffButtonGroup.insert( QPushButton("Turn off", self))
            self.on_buttons[ id_on] = code
            self.off_buttons[ id_off] = code 
            self.id_buttonOn[code] = id_on

        self.connect( self.OnButtonGroup, SIGNAL("clicked(int)"), self.slotOnButton)
        self.connect( self.OffButtonGroup, SIGNAL("clicked(int)"), self.slotOffButton)

        # Register slots and variable notifications

        QObject.connect( AOwindow, PYSIGNAL( "%d" % AOMsgCode['CLNREADY'] ), self.slotNotify)
        
        for var_prefix in self.items.values():
            AOwindow.Notify( var_prefix+".CUR", self.slotNotify, getNow = True)
            AOwindow.Notify( var_prefix+".NAME", self.slotNotify, getNow = True)

        self.ttAmp=-1
        AOwindow.Notify( "ttctrl.L.AMP_RADIUS.CUR", self.slotTTAmpChange, getNow = True)

        self.enableBcuPowerButtons(False)

    def slotLowLevelCheckbox(self):
        enable = self.lowlevelCheck.isChecked()
        self.enableBcuPowerButtons(enable)


    def enableBcuPowerButtons(self, enable):
        self.getOnButton('bcu').setEnabled(enable)
        self.getOnButton('ccd39').setEnabled(enable)
        self.getOffButton('bcu').setEnabled(enable)
        self.getOffButton('ccd39').setEnabled(enable)


    def slotTurnOnButton(self):
        answer = QMessageBox.question( self, "System power on", "You are about to turn ON the BCU Mini-crate, tip-tilt mirror and camera lens.\n\
                                                                 Are you sure?", QMessageBox.Yes, QMessageBox.No)
        if answer != QMessageBox.Yes:
            return
        processControl.startProcessByName('turnon', multi=0)

    def slotTurnOffButton(self):
        answer = QMessageBox.question( self, "System power off", "You are about to turn OFF the BCU Mini-crate, tip-tilt mirror and camera lens.\n\
                                                                 Are you sure?", QMessageBox.Yes, QMessageBox.No)
        if answer != QMessageBox.Yes:
            return
        processControl.startProcessByName('turnoff', multi=0)

    def slotTTAmpChange(self, args):
        (message) = args
        varname = message['name']
        value = message['value']

        self.ttAmp = value
        print 'Power GUI: current TT amplitude is ',self.ttAmp

    def slotStatusChange(self, args):
        (message) = args
        varname = message['name']
        value = message['value']

        for device in self.devices:
            if self.statusVars[device] == varname:

                enable = False
                if value == AOConstants.AOStatesCode['STATE_READY'] or value == AOConstants.AOStatesCode['STATE_OPERATING']:
                    enable = True

                for code in self.devicelists[device]:

                    myEnable = enable
                    if (code=='bcu') or (code=='ccd39'):
                       myEnable = enable and self.lowlevelCheck.isChecked()

                    if code == 'visaoswitch':
                      self.getOnButton(code).setEnabled(False) #can only reboot the visaoswitch
                    else:
                      self.getOnButton(code).setEnabled(myEnable)
                      
                    self.getOffButton(code).setEnabled(myEnable)


    ## +Method: slotNotify
    ##
    ## This method is the network event handler for the whole application. It's called by QSocketNotifier when
    ## some data is waiting in the server-connected socked. Thus WaitMessage() is guaranteed to return some
    ## useful data. Different actions are taken based on the message contents.

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

            for code in self.items.keys():
                if varname == self.items[code]+".CUR":
                    if value == 1:
                        self.states[code].setText( self.onText)
                        self.states[code].setEraseColor( cfg_gui.colors.green)
                        myEnable=True
                        if (code=='bcu') or (code=='ccd39'):
                            myEnable = myEnable and self.lowlevelCheck.isChecked()
                        self.getOnButton(code).setEnabled(myEnable)
                        self.getOffButton(code).setEnabled(myEnable)

                    elif value == 0:
                        self.states[code].setText( self.offText)
                        self.states[code].setEraseColor( cfg_gui.colors.red)
                        myEnable=True
                        if (code=='bcu') or (code=='ccd39'):
                            myEnable = myEnable and self.lowlevelCheck.isChecked()
                        self.getOnButton(code).setEnabled(myEnable)
                        self.getOffButton(code).setEnabled(myEnable)

                    elif value == 2:
                        self.states[code].setText( self.unreachableText)
                        self.states[code].setEraseColor( cfg_gui.colors.yellow)
                        self.getOnButton(code).setEnabled(False)
                        self.getOffButton(code).setEnabled(False)



                if varname == self.items[code]+".NAME":
                    self.labels[code].setText( value)
            
    ## +Method: slotOnButton
    ##
    ## Handler for all "turn on" buttons

    def slotOnButton( self, id):
        
        # Do not turn on ccd39 if the tip/tilt mirror is modulating
        if 'ccd39' in self.items[ self.on_buttons[id]]:
            if self.ttAmp != 0:
                QMessageBox.warning( self, "Tip/tilt is modulating", "Cannot turn on ccd39: tip/tilt modulation is not zero\n(Reported value: %g mRad)" % self.ttAmp, QMessageBox.Ok)
                return
    
        varname = self.items[ self.on_buttons[id]]+".REQ"
        msglib.SetIntVar( varname, 1)

        # Force little joe fans ON when turning on the CCDs
        #if ('ccd39' in self.items[ self.on_buttons[id]]) or ('ccd47' in self.items[ self.on_buttons[id]]):
        #    self.slotOnButton( self.id_buttonOn['fans_LJ'])
            

    #@Method: slotOffButton
    #
    # Handler for all "turn off" buttons

    def slotOffButton( self, id):
       # Warning before turning off the bcu39
        if ('bcu' in self.items[ self.on_buttons[id]]):
            answer = QMessageBox.question( self, "Rest tip/tilt", "You are about to turn off BCU Mini-crate.  Is the tip/tilt mirror resting?", QMessageBox.Yes, QMessageBox.No)
            if answer != QMessageBox.Yes:
                return

      # Warning before turning off the VisAO Computer
        if ('visaofans' in self.items[ self.on_buttons[id]]):
            answer = QMessageBox.question( self, "VisAO Fans", "You are about to turn off a VisAO Cooling Fan.  This may cause the ViAO box to overheat.  Continue?", QMessageBox.Yes, QMessageBox.No)
            if answer != QMessageBox.Yes:
                   return
      # Warning before turning off the VisAO Computer
        if ('visaocomp' in self.items[ self.on_buttons[id]]):
            answer = QMessageBox.question( self, "VisAO Computer", "You are about to turn off the ViSAO Computer - without software shutdown.  This will require restarting the VisAO system.  Continue?", QMessageBox.Yes, QMessageBox.No)
            if answer != QMessageBox.Yes:
                return
            answer = QMessageBox.question( self, "Really?", "Are you serious about this? You are about to turn off the ViSAO Computer!", QMessageBox.Yes, QMessageBox.No)
            if answer != QMessageBox.Yes:
                   return
        # Warning before turning off the VisAO wfs network switch
        if ('visaoswitch' in self.items[ self.on_buttons[id]]):
            answer = QMessageBox.question( self, "VisAO Switch", "You are about to reboot the ViSAO wfs network switch. This will cause loss of communications with the entire WFS for ~5 seconds.  Continue?", QMessageBox.Yes, QMessageBox.No)
            if answer != QMessageBox.Yes:
                return

        # Turn off bayside stages control before removing power
        delay=False
        if ('xstage' in self.items[ self.on_buttons[id]]):
                msglib.SetIntVar( cfg.varname('baysidex', 'ENABLE.REQ'), AOConstants.AOStatesCode['ENABLE_OFF'])
		delay=True
        if ('ystage' in self.items[ self.on_buttons[id]]):
                msglib.SetIntVar( cfg.varname('baysidey', 'ENABLE.REQ'), AOConstants.AOStatesCode['ENABLE_OFF'])
		delay=True
        if ('zstage' in self.items[ self.on_buttons[id]]):
                msglib.SetIntVar( cfg.varname('baysidez', 'ENABLE.REQ'), AOConstants.AOStatesCode['ENABLE_OFF'])
		delay=True
	if delay:
                time.sleep(4)  # Brake delay

        varname = self.items[ self.on_buttons[id]]+".REQ"
        msglib.SetIntVar( varname, 0)

    #@Method: getOnButton
    #
    # Get the "on" button id from the item code

    def getOnButton( self, code):
        for id in self.on_buttons.keys():
            if self.on_buttons[id] == code:
                return self.OnButtonGroup.find(id)

        return None

    #@Method: getOffButton
    #
    # Get the "off" button id from the item code

    def getOffButton( self, code):
        for id in self.off_buttons.keys():
            if self.off_buttons[id] == code:
                return self.OffButtonGroup.find(id)

        return None



