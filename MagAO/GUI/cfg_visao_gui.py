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

#from AdOpt import cfg_gui

class holder:
    pass

pixmaps = holder()
colors = holder()
state2pix = {}
state2text = {}

def init():
   
    #cfg_gui.init()
    
    if not ('pix_ok') in dir(pixmaps):
        prefix = os.getenv('ADOPT_ROOT')+'/images/'
        
        pixmaps.shutt_open = QPixmap( prefix + 'shutt_open_sm.png' )
        pixmaps.shutt_shut = QPixmap( prefix + 'shutt_shut_sm.png' )
        pixmaps.shutt_unk = QPixmap( prefix + 'shutt_unk_sm.png' )
        








