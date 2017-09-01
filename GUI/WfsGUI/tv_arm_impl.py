#!/usr/bin/python

from qt import *
import sys

from AdOpt import processControl, cfg, calib
from AdOpt.QtDesigner.tv_arm import *
from AdOpt.wrappers import msglib


class tv_arm_impl(TVarm):
    
    ## aoGui is an AOGUI, the main gui connected to MsgD, wich wait and 
    ## dispatch rtdb variable changes.
    def __init__(self, aoGui):
        
        self.aoGui = aoGui

        self.name = 'TV arm control'
        
        # --- INITITALIZE GRAPHICS --- #
        TVarm.__init__(self)

        self.ccd47Status.setup( aoGui, cfg.varname( 'ccd47', 'STATUS'), callback = self.enableCcd47Buttons, watchdog = aoGui.timer)
        self.ccd47Policy.setCallbacks( state_callback = None, cmd_callback = self.changeCcd47Policy)
        self.ccd47Freq.setup( 'Frequency (Hz):', aoGui, cfg.varname( 'ccd47', 'FRMRT'), type='float')
        self.ccd47Binning.setup( 'Binning:', aoGui, cfg.varname( 'ccd47', 'XBIN'), type='int')

        self.Fw1Status.setup( aoGui, cfg.varname( 'filterwheel1', 'STATUS'), callback = self.enableFw1Buttons, watchdog = aoGui.timer)
        self.Fw1Policy.setCallbacks( state_callback = None, cmd_callback = self.changeFw1Policy)
        fw1PosDict = {}
        fw1PosDict[ cfg.varname( 'filterwheel1', 'POS')] = 'filterwheel1'
        self.Fw1PositionPreset.setup( 'Position:', aoGui, fw1PosDict)

        self.Fw2Status.setup( aoGui, cfg.varname( 'filterwheel2', 'STATUS'), callback = self.enableFw2Buttons, watchdog = aoGui.timer)
        self.Fw2Policy.setCallbacks( state_callback = None, cmd_callback = self.changeFw2Policy)
        fw2PosDict = {}
        fw2PosDict[ cfg.varname( 'filterwheel2', 'POS')] = 'filterwheel2'
        self.Fw2PositionPreset.setup( 'Position:', aoGui, fw2PosDict)

        adcStatusVars = []
        adcStatusVars.append( cfg.varname( 'adc1', 'STATUS'))
        adcStatusVars.append( cfg.varname( 'adc2', 'STATUS'))
        self.AdcStatus.setup( aoGui, adcStatusVars, callback = self.enableAdcButtons, watchdog = aoGui.timer)
        self.AdcPolicy.setCallbacks( state_callback = None, cmd_callback = self.changeAdcPolicy)
        self.AdcPos1.setup( 'Pos #1:', aoGui, cfg.varname( 'adc1', 'POS1'), type='float')
        self.AdcPos2.setup( 'Pos #2:', aoGui, cfg.varname( 'adc2', 'POS2'), type='float')

    def computeEnable( self, status, curPolicy):
        '''
        Computes whether a set of buttons should be enabled, based on
        the current defice status and policy.
        Return True or False.
        '''

        valid = False
        enable = False

        if status < AOConstants.AOStatesCode['STATE_OFF'] or \
           status > AOConstants.AOStatesCode['STATE_OPERATING']:
            return (valid, enable)

        valid = True
        if curPolicy == 'Manual':
        #if 1:
            enable = True

        return (valid, enable)



    def changeCcd47Policy( self, dummy):
        multiStatus = self.ccd47Status.computeMultiStatus( self.ccd47Status._status.values())
        self.enableCcd47Buttons( multiStatus)
        msglib.SendMessage( 'wfsarb', msglib.WFSARB_CMD, 'WfsArbScripts.setTVarmPolicy( self, ccd47="%s")' % str.lower())


    def enableCcd47Buttons( self, multiStatus):
        '''
        Enables/disables the stage buttons depending on stage status and policy.
        '''

        valid, enable = self.computeEnable( multiStatus, self.ccd47Policy.get())

        self.ccd47Policy.setEnabled(valid)
        self.ccd47Freq.setValidAndEnabled( valid, enable)
        self.ccd47Binning.setValidAndEnabled( valid, enable)

    def changeFw1Policy( self, dummy):
        multiStatus = self.Fw1Status.computeMultiStatus( self.Fw1Status._status.values())
        self.enableFw1Buttons( multiStatus)
        msglib.SendMessage( 'wfsarb', msglib.WFSARB_CMD, 'WfsArbScripts.setTVarmPolicy( self, fw1="%s")' % str.lower())

    def enableFw1Buttons( self, multiStatus):
        '''
        Enables/disables the lamp buttons depending on lamp status and policy.
        '''

        valid, enable = self.computeEnable( multiStatus, self.Fw1Policy.get())

        self.Fw1Policy.setEnabled(valid)
        self.Fw1PositionPreset.setValidAndEnabled( valid, enable)


    def changeFw2Policy( self, dummy):
        multiStatus = self.Fw2Status.computeMultiStatus( self.Fw2Status._status.values())
        self.enableFw2Buttons( multiStatus)
        msglib.SendMessage( 'wfsarb', msglib.WFSARB_CMD, 'WfsArbScripts.setTVarmPolicy( self, fw2="%s")' % str.lower())

    def enableFw2Buttons( self, multiStatus):
        '''
        Enables/disables the lamp buttons depending on lamp status and policy.
        '''

        valid, enable = self.computeEnable( multiStatus, self.Fw2Policy.get())

        self.Fw2Policy.setEnabled(valid)
        self.Fw2PositionPreset.setValidAndEnabled( valid, enable)

    def changeRerotPolicy( self, dummy):
        multiStatus = self.RerotStatus.computeMultiStatus( self.RerotStatus._status.values())
        self.enableRerotButtons( multiStatus)
        msglib.SendMessage( 'wfsarb', msglib.WFSARB_CMD, 'WfsArbScripts.setTVarmPolicy( self, rerot="%s")' % str.lower())

    def enableRerotButtons( self, multiStatus):
        '''
        Enables/disables the lamp buttons depending on lamp status and policy.
        '''

        valid, enable = self.computeEnable( multiStatus, self.RerotPolicy.get())

        self.RerotPolicy.setEnabled(valid)
        self.RerotPos.setValidAndEnabled( valid, enable)
    
    def changeAdcPolicy( self, dummy):
        multiStatus = self.AdcStatus.computeMultiStatus( self.AdcStatus._status.values())
        self.enableAdcButtons( multiStatus)
        msglib.SendMessage( 'wfsarb', msglib.WFSARB_CMD, 'WfsArbScripts.setTVarmPolicy( self, adc="%s")' % str.lower())

    def enableAdcButtons( self, multiStatus):
        '''
        Enables/disables the lamp buttons depending on lamp status and policy.
        '''

        valid, enable = self.computeEnable( multiStatus, self.AdcPolicy.get())

        self.AdcPolicy.setEnabled(valid)
        self.AdcPos1.setValidAndEnabled( valid, enable)
        self.AdcPos2.setValidAndEnabled( valid, enable)
    




