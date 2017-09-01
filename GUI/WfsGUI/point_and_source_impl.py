#!/usr/bin/python

from qt import *
import sys, struct

from AdOpt import processControl, cfg, calib
from AdOpt.wrappers import msglib
from AdOpt.QtDesigner.point_and_source import *


class point_and_source_impl(point_and_source):
    
    ## aoGui is an AOGUI, the main gui connected to MsgD, wich wait and 
    ## dispatch rtdb variable changes.
    def __init__(self, aoGui):
        
        self.aoGui = aoGui

        self.name = 'Pointing and source control'

        
        # --- INITITALIZE GRAPHICS --- #
        point_and_source.__init__(self)

        self.stagePosX.setup( 'Stage X pos (mm):', aoGui, cfg.varname( 'baysidex', 'POS'))
        self.stagePosY.setup( 'Stage Y pos (mm):', aoGui, cfg.varname( 'baysidey', 'POS'))
        self.stagePosZ.setup( 'Stage Z pos (mm):', aoGui, cfg.varname( 'baysidez', 'POS'))

        stageStatusVars = []
        stageStatusVars.append( cfg.varname( 'baysidex', 'STATUS'))
        stageStatusVars.append( cfg.varname( 'baysidey', 'STATUS'))
        stageStatusVars.append( cfg.varname( 'baysidez', 'STATUS'))
        self.stageStatus.setup( aoGui, stageStatusVars, callback = self.enableStageButtons, watchdog = aoGui.timer)

        self.stagePositionPreset.setup( 'Position:', aoGui, {})
        self.stagePolicy.setCallbacks( state_callback = None, cmd_callback = self.changeStagePolicy)

        self.stageXlimitPos.setup( '+', aoGui, cfg.varname( 'baysidex', 'POSLIMIT'))
        self.stageXlimitNeg.setup( '-', aoGui, cfg.varname( 'baysidex', 'NEGLIMIT'))
        self.stageYlimitPos.setup( '+', aoGui, cfg.varname( 'baysidey', 'POSLIMIT'))
        self.stageYlimitNeg.setup( '-', aoGui, cfg.varname( 'baysidey', 'NEGLIMIT'))
        self.stageZlimitPos.setup( '+', aoGui, cfg.varname( 'baysidez', 'POSLIMIT'))
        self.stageZlimitNeg.setup( '-', aoGui, cfg.varname( 'baysidez', 'NEGLIMIT'))

        cubeStatusVars = []
        cubeStatusVars.append( cfg.varname( 'cubestage', 'STATUS'))
        cubeStatusVars.append( cfg.varname( 'mercury', 'STATUS'))
        self.cubeStatus.setup( aoGui, cubeStatusVars, callback = self.enableCubeButtons, watchdog = aoGui.timer)

        cubePosDict = {}
        cubePosDict[ cfg.varname( 'cubestage', 'POS')] = 'cubestage'
        cubePosDict[ cfg.varname( 'mercury', 'POS')]   = 'mercury'
        self.cubePositionPreset.setup( 'Position:', aoGui, cubePosDict)
        self.cubePolicy.setCallbacks( state_callback = None, cmd_callback = self.changeCubePolicy)

        self.lampStatus.setup( aoGui, cfg.varname( 'picctrl', 'STATUS'), callback = self.enableLampButtons, watchdog = aoGui.timer)
        self.lampPower.setup( 'Power:',     aoGui, cfg.varname( 'picctrl', 'lamp'), type='int', button='onoff')
        self.lampIntensity.setup( 'Intensity:', aoGui, cfg.varname( 'picctrl', 'LampIntensity'), type='int', button='spinbox')
        self.lampPolicy.setCallbacks( state_callback = None, cmd_callback = self.changeLampPolicy)

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

    def policy2value(self, policy):
        return {'auto':0, 'manual':1}[policy.lower()]

    def changeStagePolicy( self, str):
        multiStatus = self.stageStatus.computeMultiStatus( self.stageStatus._status.values())
        self.enableStageButtons( multiStatus)
        msg = struct.pack('iii', self.policy2value(str.latin1()), 2, 2)

        msglib.SendBinaryMessage( 'wfsarb', msglib.WFS_ARB_SET_AND_POINT_SOURCE_POLICY, msg)
        

    def enableStageButtons( self, multiStatus):
        '''
        Enables/disables the stage buttons depending on stage status and policy.
        '''

        valid, enable = self.computeEnable( multiStatus, self.stagePolicy.get())

        self.stagePolicy.setEnabled(valid)
        self.stagePosX.setValidAndEnabled( valid, enable)
        self.stagePosY.setValidAndEnabled( valid, enable)
        self.stagePosZ.setValidAndEnabled( valid, enable)
        self.stagePositionPreset.setEnabled( valid)

    def changeCubePolicy( self, str):
        multiStatus = self.cubeStatus.computeMultiStatus( self.cubeStatus._status.values())
        self.enableCubeButtons( multiStatus)
        msglib.SendMessage( 'wfsarb', msglib.WFSARB_CMD, 'WfsArbScripts.setPointingPolicy( self, cube="%s")' % lower(str))


    def enableCubeButtons( self, multiStatus):
        '''
        Enables/disables the cube buttons depending on cube status and policy
        '''

        valid, enable = self.computeEnable( multiStatus, self.cubePolicy.get())

        self.cubePolicy.setEnabled( valid)
        self.cubePositionPreset.setValidAndEnabled( valid, enable)

    def changeLampPolicy( self, str):
        multiStatus = self.lampStatus.computeMultiStatus( self.lampStatus._status.values())
        self.enableLampButtons( multiStatus)
        msglib.SendMessage( 'wfsarb', msglib.WFSARB_CMD, 'WfsArbScripts.setPointingPolicy( self, lamp="%s")' % str.lower())

    def enableLampButtons( self, lampStatus):
        '''
        Enables/disables the lamp buttons depending on lamp status and policy.
        '''

        valid, enable = self.computeEnable( lampStatus, self.lampPolicy.get())

        self.lampPolicy.setEnabled(valid)
        self.lampPower.setValidAndEnabled( valid, enable)
        self.lampIntensity.setValidAndEnabled( valid, enable)
    




