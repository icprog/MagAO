#+Class: slopecomp.py

# Python class for slopecomputer control

from wfs import *
from AdOpt import cfg

class slopecomp(wfs):

    def __init__(self, name, aoapp, ctrl_check = True, power_check = False):

        wfs.__init__( self, name, aoapp, ctrl_check=False, power_check=False)

        try:
            self.v_fl_enable_req = self.app.ReadVar( self.varname('FASTLINK.ENABLE', 'REQ'))
            self.v_fl_enable_cur = self.app.ReadVar( self.varname('FASTLINK.ENABLE', 'CUR'))
            self.v_fl_cmd_req    = self.app.ReadVar( self.varname('FASTLINK.CMD', 'REQ'))
            self.v_fl_cmd_cur    = self.app.ReadVar( self.varname('FASTLINK.CMD', 'CUR'))
            self.v_nsubap_req    = self.app.ReadVar( self.varname('NSUBAPS', 'REQ'))
            self.v_nsubap_cur    = self.app.ReadVar( self.varname('NSUBAPS', 'CUR'))
            self.v_fluxgain_req  = self.app.ReadVar( self.varname('FLUXGAIN', 'REQ'))
            self.v_fluxgain_cur  = self.app.ReadVar( self.varname('FLUXGAIN', 'CUR'))
            self.v_pixelgain_req = self.app.ReadVar( self.varname('PIXELGAINS', 'REQ'))
            self.v_pixelgain_cur = self.app.ReadVar( self.varname('PIXELGAINS', 'CUR'))
            self.v_thoffsets_req = self.app.ReadVar( self.varname('TIMEHISTORY.OFFSETS', 'REQ'))
            self.v_thoffsets_cur = self.app.ReadVar( self.varname('TIMEHISTORY.OFFSETS', 'CUR'))
            self.v_slopenull_req = self.app.ReadVar( self.varname('SLOPENULL', 'REQ'))
            self.v_slopenull_cur = self.app.ReadVar( self.varname('SLOPENULL', 'CUR'))
            self.v_decimation_req = self.app.ReadVar( self.varname('MASTERD.DECIMATION', 'REQ'))
            self.v_decimation_cur = self.app.ReadVar( self.varname('MASTERD.DECIMATION', 'CUR'))
            self.v_dark_req = self.app.ReadVar( self.varname('DARK', 'REQ'))
            self.v_dark_cur = self.app.ReadVar( self.varname('DARK', 'CUR'))
            self.v_disturb_enable_req = self.app.ReadVar( self.varname('DISTURBANCE.WFS.ENABLE', 'REQ'))
            self.v_disturb_enable_cur = self.app.ReadVar( self.varname('DISTURBANCE.WFS.ENABLE', 'CUR'))

        except AOVarNotInRTDB, e:
            errstr =  "Controller process %s is not working properly" % cfg.taskname(name)
            print errstr
            raise AOMissingProcess, ( errstr, e.message, cfg.taskname(name))


    def fl_start( self):

        if not self.checkPolicy():
            return
        self.v_fl_enable_req.SetValue(1)
        self.app.WriteVar( self.v_fl_enable_req, nocreate = True)
        self.app.WaitVarValue( self.v_fl_enable_cur, 1, timeout = 1)

    def enable_disturb(self, wfs, ovs):

        if not self.checkPolicy():
            return

        if (not wfs) and (not ovs):
            v=0
        elif (wfs) and (not ovs):
            v=1
        elif (not wfs) and (ovs):
            v=2
        else:
            v=3

        self.v_disturb_enable_req.SetValue(v)
        self.app.WriteVar( self.v_disturb_enable_req, nocreate = True)
        self.app.WaitVarValue( self.v_disturb_enable_cur, v, timeout = 1)

    def get_disturb(self):
        v = self.get_disturb_status()
        return ( (v&1)==1, (v&2)==2)

    def get_disturb_status( self):
        return self.app.ReadVar( self.v_disturb_enable_cur).Value()
        

    def fl_stop( self):

        if not self.checkPolicy():
            return
        self.v_fl_enable_req.SetValue(0)
        self.app.WriteVar( self.v_fl_enable_req, nocreate = True)
        self.app.WaitVarValue( self.v_fl_enable_cur, 0, timeout = 1)

    def get_fl_status( self):
        return self.app.ReadVar( self.v_fl_enable_cur).Value()

    def set_fl_cmd( self, filename):

        if not self.checkPolicy():
            return
        self.v_fl_cmd_req.SetValue( filename)
        self.app.WriteVar( self.v_fl_cmd_req, nocreate = True)
        self.app.WaitVarValue( self.v_fl_cmd_cur, filename, timeout = 1)

    def get_fl_cmd( self):
        return self.app.ReadVar( self.v_fl_cmd_cur).Value()

    def set_nsubap( self, n):

        if not self.checkPolicy():
            return
        self.v_nsubap_req.SetValue(n)
        self.app.WriteVar( self.v_nsubap_req, nocreate = True)
        self.app.WaitVarValue( self.v_nsubap_cur, n, timeout = 1)

    def setfluxgain( self, value):

        if not self.checkPolicy():
            return
        self.app.SetVar( self.v_fluxgain_req, float(value))

    def setpixgain( self, filename):

        if not self.checkPolicy():
            return
        self.app.SetVar( self.v_pixelgain_req, filename)

    def set_masterd_decimation( self, decimation):

        if not self.checkPolicy():
            return
        self.v_decimation_req.SetValue(decimation)
        self.app.WriteVar( self.v_decimation_req, nocreate = True)
        self.app.WaitVarValue( self.v_decimation_cur, decimation, timeout=1)

    def get_masterd_decimation( self):
        return self.app.ReadVar( self.v_decimation_cur).Value()

    def set_thoffsets( self, filename):

        if not self.checkPolicy():
            return
        self.v_thoffsets_req.SetValue(filename)
        self.app.WriteVar( self.v_thoffsets_req, nocreate = True)
        self.app.WaitVarValue( self.v_thoffsets_cur, filename, timeout = 1)

    def set_slopenull( self, filename):
        if not self.checkPolicy():
            return
        self.v_slopenull_req.SetValue(filename)
        self.app.WriteVar( self.v_slopenull_req, nocreate = True)
        self.app.WaitVarValue( self.v_slopenull_cur, filename, timeout = 1)

    def get_slopenull( self):
        return self.app.ReadVar( self.v_slopenull_cur).Value()

    def set_dark( self, filename):
        if not self.checkPolicy():
            return
        self.v_dark_req.SetValue(filename)
        self.app.WriteVar( self.v_dark_req, nocreate = True)
        self.app.WaitVarValue( self.v_dark_cur, filename, timeout = 1)

    def get_dark( self):
        return self.app.ReadVar( self.v_dark_cur).Value()

