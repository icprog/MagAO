#+Class: wfs.py

# Python class for a wfs BCU

from hw_ctrl import *
from AdOpt import cfg
import time

class gimbal(hw_ctrl):

    def __init__(self, name, aoapp, ctrl_check = True, power_check = False):

        hw_ctrl.__init__( self, name, aoapp, ctrl_check=False, power_check=False)

        try:
            self.v_center = self.app.ReadVar( self.varname('center', 'REQ'))
            self.v_dark   = self.app.ReadVar( self.varname('dark', 'REQ'))

        except AOVarNotInRTDB, e:
            errstr =  "Controller process %s is not working properly" % cfg.taskname(name)
            print errstr
            raise AOMissingProcess, ( errstr, e.message, cfg.taskname(name))


    def center( self, wait=True):
        self.v_center.SetValue(1)
        self.app.WriteVar(self.v_center)
        if wait:
            time.sleep(10)

    def dark(self, wait=True):
        self.v_dark.SetValue(1)
        self.app.WriteVar(self.v_dark)
        if wait:
            time.sleep(10)
