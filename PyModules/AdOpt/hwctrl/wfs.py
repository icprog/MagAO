#+Class: wfs.py

# Python class for a wfs BCU

from hw_ctrl import *
from AdOpt import cfg

class wfs(hw_ctrl):

    def __init__(self, name, aoapp, ctrl_check = True, power_check = False):

        hw_ctrl.__init__( self, name, aoapp, ctrl_check=False, power_check=False)

        try:
            self.v_dsp_enable    = self.app.ReadVar( self.varname('GO', 'REQ'))
            self.v_pixellut      = self.app.ReadVar( self.varname('PIXELLUT', 'REQ'))
            self.v_curpixellut   = self.app.ReadVar( self.varname('PIXELLUT', 'CUR'))

        except AOVarNotInRTDB, e:
            errstr =  "Controller process %s is not working properly" % cfg.taskname(name)
            print errstr
            raise AOMissingProcess, ( errstr, e.message, cfg.taskname(name))


    def start( self):

        if not self.checkPolicy():
            return
        self.v_dsp_enable.SetValue(1)
        self.app.WriteVar( self.v_dsp_enable, nocreate = True)
        self.app.WaitVarValue( self.v_status, AOStatesCode['STATE_OPERATING'], timeout = 1)

    def stop( self):

        if not self.checkPolicy():
            return
        self.v_dsp_enable.SetValue(0)
        self.app.WriteVar( self.v_dsp_enable, nocreate = True)
        self.app.WaitVarValue( self.v_status, AOStatesCode['STATE_READY'], timeout = 1)

    def set_pixellut( self, filename):

        if not self.checkPolicy():
            return
        self.v_pixellut.SetValue( filename)
        self.app.WriteVar( self.v_pixellut, nocreate = True)
        self.app.WaitVarValue( self.v_curpixellut, filename, timeout = 4)

    def get_pixellut( self):
        return self.app.ReadVar( self.v_curpixellut).Value()
