#@File: calib_unit.py
#
# Calibration unit functionality
#
#
# Defines two operating states for the calibration unit:
#
# calibration: unit is on and optical devices are available.
# observation: unit is off and optical devices are out of the telescope beam.
#
#@

from hw_ctrl import *

from AdOpt import AOVar, AOExcept

class calib_unit(hw_ctrl):

    # Operating states
    OPSTATE_UNDEFINED  =0
    OPSTATE_OBSERVATION=1
    OPSTATE_CALIBRATION=2
    OPSTATE_INTERNALSOURCE=3

    opStateStr={}
    opStateStr[ OPSTATE_UNDEFINED]  = 'UNDEFINED'
    opStateStr[ OPSTATE_OBSERVATION]= 'OBSERVATION'
    opStateStr[ OPSTATE_CALIBRATION]= 'CALIBRATION'
    opStateStr[ OPSTATE_INTERNALSOURCE]= 'INTERNAL SOURCE'

    def __init__(self, aoapp, ctrl_check = True, power_check = False):

        if not hasattr(aoapp, 'power'):
            raise AOExcept.HwCtrlException( code=AOConstants.AOErrCode['NOT_INIT_ERROR'], errstr="You must load the Agw_Power device before constructing a calib_unit device.")

        hw_ctrl.__init__( self, "picctrl", aoapp, ctrl_check, power_check)
        self.opState = self.OPSTATE_UNDEFINED

        # Show our current state in the RTDB
        self.opStateVar = AOVar.AOVar(name="%s.flowerpot.OPSTATE" % cfg.side, tipo="CHAR_VARIABLE", value= self.opStateStr[self.opState])
        self.app.CreaVar( self.opStateVar)


    def getOperatingState(self):
        return self.opState

    def setOperatingState( self, opState):

        from AdOpt import setupDevices

        if opState == self.opState:
            return

        if opState == self.OPSTATE_UNDEFINED:
            return

        # Calibration: turn on everything and leave things in default state
        if opState == self.OPSTATE_CALIBRATION:
            if not self.isOnline(): 
                setup = setupDevices.get()
                faults, errstr = setup.setup( self.app, ['flowerpot', 'lamp', 'cube'])

        # Internal source: turn on everything and setup cube position
        if opState == self.OPSTATE_INTERNALSOURCE:
            if not self.isOnline(): 
                setup = setupDevices.get()
                faults, errstr = setup.setup( self.app, ['flowerpot', 'lamp', 'cube'])

            # Questo elenco puo' essere spostato in un board setup
            self.app.cubestage.moveTo(-4.4)
            self.app.cuberot.moveTo(181.78)
            self.app.cubestage.waitTargetReached()
            self.app.cuberot.waitTargetReached()


        

        # Observation: ensure that the cube is out of the way.
        # If the cube and/or the picctrl is off, we must turn it on, move to the 'away' position, and turn it off again.

        if opState == self.OPSTATE_OBSERVATION:
            setup = setupDevices.get()
            if not self.isOnline():
                faults, errstr = setup.setup( self.app, ['flowerpot'])
            setupDevices.loadDevices( self.app, ['cubestage', 'cuberot'], check=True)
            if self.app.cubestage.isOnline() == False:
                faults, errstr = setup.setup( self.app, ['cube'])
            self.app.cubestage.moveTo('away', waitTimeout=60)
            self.app.power.turnoff('flowerpot')

        self.opState = opState
        self.opStateVar.SetValue( self.opStateStr[opState])
        self.app.WriteVar( self.opStateVar)




