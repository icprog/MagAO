#!/usr/bin/python

from qt import *
import sys

from AdOpt import processControl, cfg, calib
from AdOpt.wrappers import msglib
from AdOpt.QtDesigner.ho_arm import *


class ho_arm_impl(HOarm):
    
    ## aoGui is an AOGUI, the main gui connected to MsgD, wich wait and 
    ## dispatch rtdb variable changes.
    def __init__(self, aoGui):
        
        self.aoGui = aoGui

        self.name = 'HO arm control'
        

        # --- INITITALIZE GRAPHICS --- #
        HOarm.__init__(self)

        self.ccd39Status.setup( aoGui, cfg.varname('ccd39', 'STATUS'), callback = self.enableCcd39Buttons, watchdog = aoGui.timer)
        self.ccd39Policy.setCallbacks( state_callback = None, cmd_callback = self.changeCcd39Policy)
        self.ccd39Freq.setup( 'Frequency (Hz):', aoGui, cfg.varname('ccd39', 'FRMRT'), type='float')
        self.ccd39Binning.setup( 'Binning:', aoGui, 'ccd39', [1,2,3,4,5])
        self.ccd39Dark.setup( 'Dark:', aoGui, cfg.varname( 'slopecompctrl', 'DARK'), type='char', button='browse', dir = calib.BackgDir( 'ccd39', 1))

        self.TTStatus.setup( aoGui, cfg.varname( 'ttctrl', 'STATUS'), callback = self.enableTTButtons, watchdog = aoGui.timer)
        self.TTPolicy.setCallbacks( state_callback = None, cmd_callback = self.changeTTPolicy)
        self.TTAmp.setup( 'Amp (Volts):', aoGui, cfg.varname( 'ttctrl', 'AMP'), type='float')
        self.TTFreq.setup( 'Frequency (Hz):', aoGui, cfg.varname( 'ttctrl', 'FREQ'), type='float')
        self.TTOffX.setup( 'Off X (Volts):', aoGui, cfg.varname( 'ttctrl', 'OFFX'), type='float')
        self.TTOffY.setup( 'Off Y (Volts):', aoGui, cfg.varname( 'ttctrl', 'OFFY'), type='float')

        self.Fw1Status.setup( aoGui, cfg.varname( 'filterwheel1', 'STATUS'), callback = self.enableFw1Buttons, watchdog = aoGui.timer)
        self.Fw1Policy.setCallbacks( state_callback = None, cmd_callback = self.changeFw1Policy)
        fw1PosDict = {}
        fw1PosDict[ cfg.varname( 'filterwheel1', 'POS')] = 'filterwheel1'
        self.Fw1PositionPreset.setup( 'Position:', aoGui, fw1PosDict)

        adcStatusVars = []
        adcStatusVars.append( cfg.varname( 'adc1', 'STATUS'))
        adcStatusVars.append( cfg.varname( 'adc2', 'STATUS'))
        self.AdcStatus.setup( aoGui, adcStatusVars, callback = self.enableAdcButtons, watchdog = aoGui.timer)
        self.AdcPolicy.setCallbacks( state_callback = None, cmd_callback = self.changeAdcPolicy)
        self.AdcPos1.setup( 'Pos #1:', aoGui, cfg.varname( 'adc1', 'POS1'), type='float')
        self.AdcPos2.setup( 'Pos #2:', aoGui, cfg.varname( 'adc2', 'POS2'), type='float')

        self.RerotStatus.setup( aoGui, cfg.varname( 'rerotator', 'STATUS'), callback = self.enableRerotButtons, watchdog = aoGui.timer)
        self.RerotPolicy.setCallbacks( state_callback = None, cmd_callback = self.changeRerotPolicy)
        self.RerotPos.setup( 'Rotation (degrees):', aoGui, cfg.varname( 'rerotator', 'POS'), type='float')

        self.LensStatus.setup( aoGui, cfg.varname( 'cameralensctrl', 'STATUS'), callback = self.enableLensButtons, watchdog = aoGui.timer)
        self.LensPolicy.setCallbacks( state_callback = None, cmd_callback = self.changeLensPolicy)
        self.LensPosX.setup( 'Pos Y (um):', aoGui, cfg.varname( 'cameralensctrl', 'POS_X'), type='float')
        self.LensPosY.setup( 'Pos X (um):', aoGui, cfg.varname( 'cameralensctrl', 'POS_Y'), type='float')

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



    def changeCcd39Policy( self, dummy):
        multiStatus = self.ccd39Status.computeMultiStatus( self.ccd39Status._status.values())
        self.enableCcd39Buttons( multiStatus)
        msglib.SendMessage( 'wfsarb', msglib.WFSARB_CMD, 'WfsArbScripts.setHOarmPolicy( self, ccd39="%s")' % str.lower())

    def enableCcd39Buttons( self, multiStatus):
        '''
        Enables/disables the stage buttons depending on stage status and policy.
        '''

        valid, enable = self.computeEnable( multiStatus, self.ccd39Policy.get())

        self.ccd39Policy.setEnabled(valid)
        self.ccd39Freq.setValidAndEnabled( valid, enable)
        self.ccd39Binning.setValidAndEnabled( valid, enable)

    def changeTTPolicy( self, dummy):
        multiStatus = self.TTStatus.computeMultiStatus( self.TTStatus._status.values())
        self.enableTTButtons( multiStatus)
        msglib.SendMessage( 'wfsarb', msglib.WFSARB_CMD, 'WfsArbScripts.setHOarmPolicy( self, tt="%s")' % str.lower())

    def enableTTButtons( self, multiStatus):
        '''
        Enables/disables the cube buttons depending on cube status and policy
        '''

        valid, enable = self.computeEnable( multiStatus, self.TTPolicy.get())

        self.TTPolicy.setEnabled( valid)
        self.TTAmp.setValidAndEnabled( valid, enable)
        self.TTFreq.setValidAndEnabled( valid, enable)
        self.TTOffX.setValidAndEnabled( valid, enable)
        self.TTOffY.setValidAndEnabled( valid, enable)

    def changeFw1Policy( self, dummy):
        multiStatus = self.Fw1Status.computeMultiStatus( self.Fw1Status._status.values())
        self.enableFw1Buttons( multiStatus)
        msglib.SendMessage( 'wfsarb', msglib.WFSARB_CMD, 'WfsArbScripts.setHOarmPolicy( self, fw1="%s")' % str.lower())

    def enableFw1Buttons( self, multiStatus):
        '''
        Enables/disables the lamp buttons depending on lamp status and policy.
        '''

        valid, enable = self.computeEnable( multiStatus, self.Fw1Policy.get())

        self.Fw1Policy.setEnabled(valid)
        self.Fw1PositionPreset.setValidAndEnabled( valid, enable)

    def changeRerotPolicy( self, dummy):
        multiStatus = self.RerotStatus.computeMultiStatus( self.RerotStatus._status.values())
        self.enableRerotButtons( multiStatus)
        msglib.SendMessage( 'wfsarb', msglib.WFSARB_CMD, 'WfsArbScripts.setHOarmPolicy( self, rerot="%s")' % str.lower())

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
        msglib.SendMessage( 'wfsarb', msglib.WFSARB_CMD, 'WfsArbScripts.setHOarmPolicy( self, adc="%s")' % str.lower())

    def enableAdcButtons( self, multiStatus):
        '''
        Enables/disables the lamp buttons depending on lamp status and policy.
        '''

        valid, enable = self.computeEnable( multiStatus, self.AdcPolicy.get())

        self.AdcPolicy.setEnabled(valid)
        self.AdcPos1.setValidAndEnabled( valid, enable)
        self.AdcPos2.setValidAndEnabled( valid, enable)
    
    def changeLensPolicy( self, dummy):
        multiStatus = self.LensStatus.computeMultiStatus( self.LensStatus._status.values())
        self.enableLensButtons( multiStatus)
        msglib.SendMessage( 'wfsarb', msglib.WFSARB_CMD, 'WfsArbScripts.setHOarmPolicy( self, lens="%s")' % str.lower())

    def enableLensButtons( self, multiStatus):
        '''
        Enables/disables the lamp buttons depending on lamp status and policy.
        '''

        valid, enable = self.computeEnable( multiStatus, self.LensPolicy.get())

        self.LensPolicy.setEnabled(valid)
        self.LensPosX.setValidAndEnabled( valid, enable)
        self.LensPosY.setValidAndEnabled( valid, enable)
    




