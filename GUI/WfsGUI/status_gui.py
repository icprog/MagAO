#!/usr/bin/env python
#
#+File: status_gui.py
#
# GUI to show system status
#
#-

import os, time
from qt import *
from AdOpt import cfg
from AdOpt.AOConstants import *
import cfg_gui

state_ko    = 0
state_busy  = 1
state_ok    = 2
state_error = 3

class device:

    def __init__(self, deviceName):
        self._deviceName = deviceName
        self._varName = ''

    def getState(self, value):
        return state_ko        

class pinged_device(device):

    def __init__( self, deviceName):
        device.__init__(self, deviceName)
        self._varName = cfg.varname('pinger', deviceName)

    def getState(self, value):
        if value>0:
            return state_ok
        else:
            return state_ko

class controlled_device(device):

    def __init__( self, deviceName):
        device.__init__(self, deviceName)
        self._varName = cfg.varname( deviceName, 'STATUS')

    def getState(self, value):
        if value == AOStatesCode['STATE_INVALID'] or \
           value == AOStatesCode['STATE_UNDEFINED'] or \
           value == AOStatesCode['STATE_NOCONNECTION'] or \
           value == AOStatesCode['STATE_CONNECTED']:
            return state_ko

        if value == AOStatesCode['STATE_CONFIGURING'] or \
           value == AOStatesCode['STATE_CONFIGURED'] or \
           value == AOStatesCode['STATE_HOMING'] or \
           value == AOStatesCode['STATE_POLEFINDING'] or \
           value == AOStatesCode['STATE_LMRECOVER']:
            return state_busy

        if value == AOStatesCode['STATE_OFF'] or \
           value == AOStatesCode['STATE_READY'] or \
           value == AOStatesCode['STATE_OPERATING'] or \
           value == AOStatesCode['STATE_BUSY']:
            return state_ok

        if value == AOStatesCode['STATE_ERROR']:
            return state_error


class device_item:
    def __init__(self, device, button, lasttime):
        self.device = device
        self.button = button
        self.lasttime = lasttime

#+Class: status_gui
#
# Main window for the GUI tool
#-

class status_gui( QDialog):
    
    def __init__(self, AOwindow):
        QDialog.__init__(self)

        self.device_array = [ pinged_device(device[7:]) for device in filter( lambda x: x[0:7] == 'device_', cfg.cfg['pinger'].keys()) if not device[7:] == 'mirror']

        devices = ['filterwheel1', 'filterwheel2','filterwheel3', 'adc1', 'adc2', \
                   'rerotator', 'cubestage', 'mercury', \
                    'baysidex', 'baysidey', 'baysidez', 'ttctrl', \
                    'ccd39', 'ccd47', \
                    'powerboard', 'picctrl', 'VisAO_shutter', 'VisAO_pwrctrl'] 

        devices.sort()

        self.device_array += [ controlled_device(device) for device in devices]

        self.name = "Status check"

        self.watchdog_timer = 5.0      # Watchdog timeout

        ## Draw the widgets

        self.gridLayout = QGridLayout( self, len(self.device_array)+1, 2, 5)
        self.gridLayout.setAutoAdd(1)

        self.labels = []
        self.label = QLabel("Item", self)
        self.dummy = QLabel("Status", self)

        #for p in self.device_array:


        ## Dictionary of lists: [ device, button, last_check_time], indexed by variable name
        self.device_items = {}
        for device in self.device_array:
            self.labels.append( QLabel( device._deviceName, self))
            self.device_items[device._varName] = device_item( device, QPushButton("checking...", self), 0)

        cfg_gui.init()
        self.pix = {}
        self.pix[state_ok]    = cfg_gui.pixmaps.pix_ok
        self.pix[state_busy]  = cfg_gui.pixmaps.pix_working
        self.pix[state_ko]    = cfg_gui.pixmaps.pix_ko
        self.pix[state_error] = cfg_gui.pixmaps.pix_ko

        # register slots
        for device in self.device_array:
            AOwindow.Notify( device._varName, self.slotNotify, getNow = True)

        # Watchdog timer
        self.timer = QTimer(self)
        self.timer.start(2000)
        QObject.connect( self.timer, SIGNAL("timeout()"), self.slotTimeout)

    ## +Method: slotNotify
    ##

    def slotNotify(self, args):
        (message) = args
        varname = message['name']
        value = message['value']
        now = time.time()

        ## When one of the watched variables change, update our display

        if self.device_items.has_key(varname):
            state = self.device_items[varname].device.getState( value)
            self.device_items[varname].button.setPixmap( self.pix[state])
            self.device_items[varname].lasttime = now

    def slotTimeout(self):
        now = time.time()
        for k in self.device_items.keys():
            if now - self.device_items[k].lasttime > self.watchdog_timer:
                self.device_items[k].button.setPixmap( self.pix[state_ko])
                 
