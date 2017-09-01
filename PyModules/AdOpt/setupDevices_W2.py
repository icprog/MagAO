#!/usr/bin/env python
#
# Setupdevices overrides for unit W#2

from setupDevices_W1 import *
from processControl import *

class setupDevices_W2(setupDevices_W1):

    def ident(self):
        return 'setup unit W#2'

    def setupBcu39_prepare(self):
        '''
        Prepares bcu39 preconditions
        '''
    
        self.setupPS1()
        processControl.startAndWait( self._app, 'powerboard')
        self._app.power._ctrl['powerboard'].waitActive()
        self._app.power.turnon('boxfans')
        self._app.power.turnon('bcu39')
    
    
    def setupBcu47_prepare(self):
        '''
        Bcu47 preconditions on W#2
        '''
        processControl.startAndWait( self._app, 'powerboard')
        self._app.power._ctrl['powerboard'].waitActive()
        self._app.power.turnon('bcu47')
    
    
    def setupPS1(self):
        '''
        Turn on PS1 (linear) on W#2,
        '''
    
        processControl.startAndWait( self._app, 'powerboard')
        self._app.power._ctrl['powerboard'].waitActive()
        self._app.power.turnon('ps1')
    
    def setupPS2(self):
        '''
        Turn on PS2 on W#2,
        '''
    
        processControl.startAndWait( self._app, 'powerboard')
        self._app.power._ctrl['powerboard'].waitActive()
        self._app.power.turnon('ps2')
    
    def setupLamp_prepare(self):
        '''
        Lamp preconditions on W#2
        '''
        self.setupFlowerpot()
        self.setupPS2()
    
    def setupCCD39_prepare(self):
        '''
        CCD39 preconditions on W#2
        '''
        self.setupPS1()
        processControl.startAndWait( self._app, 'powerboard')
        self._app.power._ctrl['powerboard'].waitActive()
    
    
    def setupFW_prepare(self):
        '''
        Starts up filter wheels preconditions.
        '''
        self.setupPS2()
    
    def setupCube_prepare(self):
        '''
        Cube preconditions - nothing to do on W#2
        '''
        self.setupFlowerpot()
        self.setupPS2()
    
    def setupADC_prepare(self):
        '''
        ADC preconditions
        '''
        self.setupPS2()
    
    def setupCameralens_prepare(self):
        '''
        Prepares camera lens preconditions
        '''
        self.setupPS2()
        self.setupTT()
        self._app.power.turnon('cameralens')

    
    def setupRerotator_prepare(self):
        '''
        Rerotator preconditions.
        '''
        self.setupPS2()
    
    def setupTT_prepare(self):
        '''
        TT preconditions.
        '''
        self.setupBcu47()
        self.setupPS2()
        self._app.power.turnon('tt')
    
    def setupXYZStages_prepare(self):
        '''
        XYZ stages preconditions.
        '''
        self.setupPS2()
        self._app.power.turnon('copley')
           
