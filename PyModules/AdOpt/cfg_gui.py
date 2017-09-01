#!/usr/bin/env python2

#@File: cfg_gui.py
#
# Common configuration options for graphic interfaces
#
# Due to QT limitations, initialization must be deferred to after a QApplication has been constructed.
# Therefore, any code using these functions need to call init() after the QT application has been constructed.
# Calling init() multiple times (for example, once on each class using the configuration values) is allowed and has no effect.
#
#@

import os

from qt import *
from AdOpt import AOConstants

class holder:
    pass

pixmaps = holder()
colors = holder()
state2pix = {}
state2text = {}

def init():
    # Only load objects once
    if not ('pix_ok') in dir(pixmaps):
        prefix = os.getenv('ADOPT_ROOT')+'/GUI/images/'
        pixmaps.pix_ok = QPixmap( prefix + 'small_ok.png')
        pixmaps.pix_ko = QPixmap( prefix + 'small_ko.png')
        pixmaps.pix_working = QPixmap( prefix + 'small_working.png')


        state2pix[ AOConstants.AOStatesCode['STATE_INVALID']] = pixmaps.pix_ko
        state2pix[ AOConstants.AOStatesCode['STATE_UNDEFINED']] = pixmaps.pix_ko
        state2pix[ AOConstants.AOStatesCode['STATE_NOCONNECTION']] = pixmaps.pix_ko
        state2pix[ AOConstants.AOStatesCode['STATE_CONNECTED']] = pixmaps.pix_working
        state2pix[ AOConstants.AOStatesCode['STATE_CONFIGURING']] = pixmaps.pix_working
        state2pix[ AOConstants.AOStatesCode['STATE_CONFIGURED']] = pixmaps.pix_working
        state2pix[ AOConstants.AOStatesCode['STATE_OFF']] = pixmaps.pix_ok
        state2pix[ AOConstants.AOStatesCode['STATE_READY']] = pixmaps.pix_ok
        state2pix[ AOConstants.AOStatesCode['STATE_OPERATING']] = pixmaps.pix_ok
        state2pix[ AOConstants.AOStatesCode['STATE_BUSY']] = pixmaps.pix_working
        state2pix[ AOConstants.AOStatesCode['STATE_ERROR']] = pixmaps.pix_ko
        state2pix[ AOConstants.AOStatesCode['STATE_HOMING']] = pixmaps.pix_working
        state2pix[ AOConstants.AOStatesCode['STATE_POLEFINDING']] = pixmaps.pix_working
        state2pix[ AOConstants.AOStatesCode['STATE_LMRECOVER']] = pixmaps.pix_working


        state2text[ AOConstants.AOStatesCode['STATE_INVALID']] = 'INVALID'
        state2text[ AOConstants.AOStatesCode['STATE_UNDEFINED']] = 'UNDEFINED STATE'
        state2text[ AOConstants.AOStatesCode['STATE_NOCONNECTION']] = 'NO CONNECTION'
        state2text[ AOConstants.AOStatesCode['STATE_CONNECTED']] = 'CONNECTED'
        state2text[ AOConstants.AOStatesCode['STATE_CONFIGURING']] = 'CONFIGURING'
        state2text[ AOConstants.AOStatesCode['STATE_CONFIGURED']] = 'CONFIGURED'
        state2text[ AOConstants.AOStatesCode['STATE_OFF']] = 'READY, OFF'
        state2text[ AOConstants.AOStatesCode['STATE_READY']] = 'READY'
        state2text[ AOConstants.AOStatesCode['STATE_OPERATING']] = 'OPERATING'
        state2text[ AOConstants.AOStatesCode['STATE_BUSY']] = 'BUSY'
        state2text[ AOConstants.AOStatesCode['STATE_ERROR']] = 'ERROR'
        state2text[ AOConstants.AOStatesCode['STATE_HOMING']] = 'HOMING'
        state2text[ AOConstants.AOStatesCode['STATE_POLEFINDING']] = 'POLE FINDING'
        state2text[ AOConstants.AOStatesCode['STATE_LMRECOVER']] = 'RECOVERING LIMIT'


    if not ('green') in dir(colors):
        colors.green = QColor(0, 255, 0)
        colors.red = QColor(255,0,0)
        colors.yellow = QColor(255,255,0)
        colors.darkgray = QColor(192,192,192)





