#!/usr/bin/env python

import os, time
from AdOpt import AOConfig


#######################
#
# Star acquisition data


# Star finding algorithm. Only one of these must be True

useStarFindPython = False
useStarFindIDL    = True



### Defined configurations (lists of devices to use)

wfs_configs  = {}
wfs_configs['Debug'] = ['ts8sx', 'ts8dx']
wfs_configs['Ccd39'] = ['ts8sx', 'bcu39', 'ccd39']
wfs_configs['WFS_complete_(without_ccd47)'] = ['ts8sx', 'ts8dx', 'bcu39', 'bcu47', 'fw', 'tt', 'cube', 'lamp', 'stages', 'ccd39', 'cameralens', 'rerot', 'adc', 'flowerpot']
wfs_configs['WFS_complete_(with_ccd47)'] = ['ts8sx', 'ts8dx', 'bcu39', 'bcu47', 'fw', 'tt', 'cube', 'lamp', 'stages', 'ccd39', 'cameralens', 'ccd47', 'rerot', 'adc', 'flowerpot']


# Switch 'mirrorctrl' and 'masterdiagnostic' depending on the application wich
# performs the tech viewer frames downloading
techviewer_frames_producer = 'mirrorctrl' # 'masterdiagnostic'

from AdOpt.cfg import * # Circular import, but it works...

ccd_framebuffer = {}
ccd_framebuffer[39] = "%s:%s" % (taskname('masterdiagnostic'), get_cfg_value('masterdiagnostic', 'OptLoopShmBuf'))
#ccd_framebuffer[39] = 'MIRRORCTRL01:OPTLOOPBUF'
ccd_framebuffer[47] =  "%s:%s" % (taskname(techviewer_frames_producer), get_cfg_value('techviewctrl', 'ShmName'))

ccd_darkvariable = {}
ccd_darkvariable['ccd39'] = 'slopecompctrl.%s.DARK' % side
ccd_darkvariable['ccd47'] = 'techviewctrl.%s.DARK' % side
ccd_darkvariable['irtc'] = side+'.IRTC.DARK'

