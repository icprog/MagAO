#+Class: wfs.py

# Python class for VisAO shutter

from hw_ctrl import *
from AdOpt import cfg
import time

class shutter(hw_ctrl):

    def __init__(self, name, aoapp, ctrl_check = True, power_check = False):

        hw_ctrl.__init__( self, name, aoapp, ctrl_check=False, power_check=False)

        try:
            self.v_state = self.app.ReadVar( self.varname('STATE', 'REQ'))

        except AOVarNotInRTDB, e:
            errstr =  "Controller process %s is not working properly" % cfg.taskname(name)
            print errstr
            raise AOMissingProcess, ( errstr, e.message, cfg.taskname(name))


    def close( self, wait=True):
        self.v_state.SetValue(-1)
        self.app.WriteVar(self.v_state)
        if wait:
            time.sleep(2)

    def open( self, wait=True):
        self.v_state.SetValue(1)
        self.app.WriteVar(self.v_state)
        if wait:
            time.sleep(2)

