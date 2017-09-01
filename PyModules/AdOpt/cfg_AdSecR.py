#!/usr/bin/env python

import os, time
from AdOpt import AOConfig


# Mode names
#modes = dict([
#             [0, 'Piston'],
#             [1, 'Tip'],
#             [2, 'Tilt'],
#             [3, 'Astigmatism 1'],
#             [4, 'Astigmatism 2']
#             ])

# End of DM configuration
#############################

#######################
#
# Star acquisition data


# Star finding algorithm. Only one of these must be True

useStarFindPython = False
useStarFindIDL    = True




## Varius process lists

sys_processes = [ 'aoarb', 'adsecarb', 'mvar']

adsec_processes = ['idlctrl', 'housekeeper', 'fastdiagnostic', 'masterdiagnostic', 'mirrorctrl', 'adamHousekeeper']

interfaces = [ 'adseccontrol', 'housekeepergui', 'adsecmirrorgui', 'vartool', 'sysprocesses', 'idlterm']


