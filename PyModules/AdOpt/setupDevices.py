#!/usr/bin/env python
#
# Wrapper to load the right setup devices for the current W unit

from AdOpt import cfg

from setupDevices_W1 import *
from setupDevices_W2 import *
from setupDevices_MAG import *

def get(wfs_name = None):
    '''
    Returns a setup object for the specified WFS unit.
    If nothing is specified, the current WFS name (as found in the configuration file) is used.
    '''

    if wfs_name == None:
        wfs_name = cfg.subsystem_name

    if wfs_name == 'W1':
        return setupDevices_W1()

    if wfs_name == 'W2':
        return setupDevices_W2()

    if wfs_name == 'MAG':
        return setupDevices_MAG()
