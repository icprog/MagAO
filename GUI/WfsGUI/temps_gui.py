#!/usr/bin/env python
#
#+File: temps_gui.py
#
# GUI to show system temperatures
#
#-

import os, time
from qt import *
from AdOpt import cfg
import cfg_gui

# Import the C library wrapper

from AdOpt.wrappers import msglib



class temperature_item:
    def __init__(self, parent, name = "", varname = "", index = None, alarm = None, unit = "C", varlabel = "", varmtime = None):
        '''
        Builds a temperature item.
        name    : item name
        varname : name of the controlling variable
        index   : index in the controlling variable (or None if not applicable) 
        alarm   : alarm threshold (or None if no threshold set)
        unit    : unit of measure string
        varlabel : name of the variable holding the label (or "" if the item name was passed as a parameter)
        varmtime : if set, last modification time of the variables. Otherwise, the current time will be used
        '''

        cfg_gui.init()

        self.parent = parent
        self.name = name

        if self.name != 'NC':
            self.name_label = QLabel( name, self.parent)
            self.value_label = QLabel( "N/A", self.parent)
            self.button = QPushButton("", self.parent)

        if varmtime != None:
	    self.lasttime = varmtime
        else:
            self.lasttime = time.time()
        self.varname = varname
        self.index = index
        self.alarm  = alarm
        self.unit   = unit
        self.varlabel = varlabel


    def check( self, varname, varvalue, mtime, timeout):

        if self.name == 'NC':
            return

        if self.varlabel == varname:
            self.name_label.setText(varvalue)

        if self.varname == varname:
	    #self.lasttime = mtime      #	- use this when the MsgD bug is fixed
            self.lasttime = time.time()

            if self.index == None:
                self.value = varvalue
            else:
                self.value = varvalue[self.index]

            self.value_label.setText( "%5.2f %s" % (self.value, self.unit))

            if self.alarm != None:
                if self.value >= self.alarm:
                    self.button.setPixmap( self.parent.pix_alarm)
                    return
            self.button.setPixmap( self.parent.pix_ok)

    def checkTimeout( self, now, timeout):
        '''
        Checks whether the timeout has expired and self-updates accordingly.
        Return True if the timeout was expired, False otherwise
        '''

        if self.name == 'NC':
            return False

	#print 'check timeout %-20s: time %d  timeout %d' % (self.name, now-self.lasttime, timeout)
        if now - self.lasttime > timeout:
            self.displayTimeout()
            return True

        return False

    def displayTimeout( self):
        self.value_label.setText("N/A")
        self.button.setPixmap( self.parent.pix_timeout)


#+Class: temps_gui
#
# Main window for the GUI tool
#-

class temps_gui( QDialog):
    
    def __init__(self, AOwindow):
        QDialog.__init__(self)

        self.powerboard_t_num = cfg.cfg['powerboard']['t_num'].Value()
        self.flowerpot_t_num  = cfg.cfg['picctrl']['temps.num_items'].Value()
        self.flowerpot_h_num  = cfg.cfg['picctrl']['humidity.num_items'].Value()
        self.flowerpot_d_num  = cfg.cfg['picctrl']['dews.num_items'].Value()

        self.name = "Temperatures"

        self.watchdog_timer = 30.0      # Watchdog timeout in seconds

        ## Draw the widgets

        height = self.powerboard_t_num + self.flowerpot_t_num + self.flowerpot_h_num + self.flowerpot_d_num + 2

        self.gridLayout = QGridLayout( self, height, 3, 1)
        self.gridLayout.setAutoAdd(1)

        self.dummy = QLabel("Sensor", self)
        self.dummy = QLabel("Temperature", self)
        self.dummy = QLabel("", self)

        self.items = []

        cfg_gui.init()

        self.pix_ok = cfg_gui.pixmaps.smallpix_ok
        self.pix_alarm = cfg_gui.pixmaps.smallpix_ko
        self.pix_timeout = cfg_gui.pixmaps.smallpix_ko

        # Add all powerboard and flowerpot temperatures/humidity/dew points

        # Elaborate parsing of cfg file follows... any way to simplify?
        powerboard_dict  = {'task': 'powerboard', 'num':self.powerboard_t_num, 'var':'TEMPS',\
                            'nameTemplate':'temp%d_name', 'alarmTemplate':'temp%d_alarm', 'unitTemplate':'temp%d_unit'}
        flowerpot_t_dict = {'task': 'picctrl', 'num':self.flowerpot_t_num, 'var':'TEMPS', \
                            'nameTemplate':'temps.item%d_name', 'alarmTemplate':'temps.item%d_alarm', 'unitTemplate':'temps.item%d_unit'}
        flowerpot_h_dict = {'task': 'picctrl', 'num':self.flowerpot_h_num, 'var':'HUM', \
                            'nameTemplate':'humidity.item%d_name', 'alarmTemplate':'humidity.item%d_alarm', 'unitTemplate':'humidity.item%d_unit'}
        flowerpot_d_dict = {'task': 'picctrl', 'num':self.flowerpot_d_num, 'var':'DEW', \
                            'nameTemplate':'dews.item%d_name', 'alarmTemplate':'dews.item%d_alarm', 'unitTemplate':'dews.item%d_unit'}

        for d in [powerboard_dict, flowerpot_t_dict, flowerpot_h_dict, flowerpot_d_dict]:
            c = cfg.cfg[d['task']]
            varname = cfg.varname(d['task'], d['var'])
            for i in range(d['num']):
                name = c[d['nameTemplate'] % (i+1)].Value()
                self.items.append( temperature_item( parent = self, name = name, \
                                   varname = varname, \
                                   index = i, \
                                   alarm = c[d['alarmTemplate'] % (i+1)].Value(),
                                   unit  = c[d['unitTemplate'] % (i+1)].Value()))
   
            AOwindow.Notify( varname, self.slotNotify, getNow = True) 

        # Now the internal CCD temperatures
        varname = cfg.varname('ccd39', "TEMPS")
        self.items.append( temperature_item( parent = self, name = 'LittleJoe 39 - internal', \
                           varname = varname, index = 0, alarm = 1e6))
        AOwindow.Notify( varname, self.slotNotify, getNow = True)

        varname = cfg.varname('ccd47', "TEMPS")
        self.items.append( temperature_item( parent = self, name = 'LittleJoe 47 - internal', \
                           varname = varname, index = 0, alarm = 1e6))
        AOwindow.Notify( varname, self.slotNotify, getNow = True)

        # Now the housekeeper BCU-internal temperatures

        for n in range(3):
            varvalue = cfg.varname('housekeeperWFS', "TEMP%d.VALUE" % (n+1))
            varlabel = cfg.varname('housekeeperWFS', "TEMP%d.NAME" % (n+1))
            self.items.append( temperature_item( parent = self, name = "", \
                               varname = varvalue, varlabel = varlabel, index=None, alarm = 80))
            AOwindow.Notify( varvalue, self.slotNotify, getNow = True)
            AOwindow.Notify( varlabel, self.slotNotify, getNow = True)

        # Separator:

        sep1 = QLabel("-------", self)
        sep2 = QLabel("", self)
        sep3 = QLabel("", self)

        # Temp. alarm thresholds
        label1 = QLabel( 'Alarm threshold (electronics):', self)
        self._th1Cur = QLabel('unknown',self)
        self._th1Spin = QSpinBox(0, 100, 1, self)

        label2 = QLabel( 'Alarm threshold (water/ccd39):', self)
        self._th2Cur = QLabel('unknown',self)
        self._th2Spin = QSpinBox(0, 100, 1, self)

        QObject.connect( self._th1Spin, SIGNAL("valueChanged(int)"), self.slotTh1spin)
        QObject.connect( self._th2Spin, SIGNAL("valueChanged(int)"), self.slotTh2spin)

        label3 = QLabel( 'Control loop:', self)
        self._loopOn = QPushButton('On', self)
        self._loopOff = QPushButton('Off', self)

        label4 = QLabel('Alarm status:', self)
        label5 = QLabel('', self)
        self._loopAlarmLabel = QLabel('', self)

        QObject.connect( self._loopOn, SIGNAL("clicked()"), self.slotLoopOn)
        QObject.connect( self._loopOff, SIGNAL("clicked()"), self.slotLoopOff)

        self.varname_th1Cur = cfg.varname('powerboard', 'Overtemp.th1.CUR')
        self.varname_th2Cur = cfg.varname('powerboard', 'Overtemp.th2.CUR')
        self.varname_th1Req = cfg.varname('powerboard', 'Overtemp.th1.REQ')
        self.varname_th2Req = cfg.varname('powerboard', 'Overtemp.th2.REQ')
        self.varname_loopActive = cfg.varname('powerboard', 'Overtemp.active.REQ')
        self.varname_loopAlarm = cfg.varname('powerboard', 'Overtemp.alarm')

        AOwindow.Notify( self.varname_th1Cur, self.slotNotify, getNow=True)
        AOwindow.Notify( self.varname_th2Cur, self.slotNotify, getNow=True)
        AOwindow.Notify( self.varname_loopAlarm, self.slotNotify, getNow=True)


        # Watchdog timer
        self.timer = QTimer(self)
        self.timer.start( self.watchdog_timer * 1000)
        QObject.connect( self.timer, SIGNAL("timeout()"), self.slotTimeout)
        self.slotTimeout()

    def slotTh1spin(self, value):
        msglib.SetVar( self.varname_th1Req, msglib.REAL_VARIABLE, 1, value)

    def slotTh2spin(self, value):
        msglib.SetVar( self.varname_th2Req, msglib.REAL_VARIABLE, 1, value)

    def slotLoopOn(self):
        msglib.SetVar( self.varname_loopActive, msglib.INT_VARIABLE, 1, 1)

    def slotLoopOff(self):
        msglib.SetVar( self.varname_loopActive, msglib.INT_VARIABLE, 1, 0)


    ## +Method: slotNotify
    ##

    def slotNotify(self, args):
        (message) = args
        varname = message['name']
        value = message['value']
        mtime = message['mtime']

        if varname == self.varname_th1Cur:
            self._th1Cur.setText( '%5.1f' % value)

        elif varname == self.varname_th2Cur:
            self._th2Cur.setText( '%5.1f' % value)

        elif varname == self.varname_loopAlarm:
            if value:
                self._loopAlarmLabel.setText('ALARM')
                self._loopAlarmLabel.setPaletteForegroundColor( cfg_gui.colors.red)
            else:
                self._loopAlarmLabel.setText('Good')
                self._loopAlarmLabel.setPaletteForegroundColor( cfg_gui.colors.green)

        ## When one of the watched variables change, update our display
        else:
            for item in self.items:
                item.check( varname, value, mtime, self.watchdog_timer)

    def slotTimeout(self):

        now = time.time()
        for item in self.items:
            item.checkTimeout( now, self.watchdog_timer)
                 
