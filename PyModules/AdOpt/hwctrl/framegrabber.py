#+Class: framegrabber.py

# Python class for framegrabber control

from wfs import *

from AdOpt import cfg, AOVar
from AdOpt.wrappers import thrdlib

class framegrabber(wfs):

    def __init__(self, name, aoapp, ctrl_check = True, power_check = False):

        hw_ctrl.__init__( self, name, aoapp, ctrl_check=False, power_check=False)


        try:
            self.v_dark_req = self.app.ReadVar( self.varname('DARK', 'REQ'))
            self.v_dark_cur = self.app.ReadVar( self.varname('DARK', 'CUR'))



        except AOVarNotInRTDB, e:
            errstr =  "Controller process %s is not working properly" % cfg.taskname(name)
            print errstr
            raise AOMissingProcess, ( errstr, e.message, cfg.taskname(name))
        
        # RTDBVar to notify MirrorCtrl Diagnostic of the frame size change
        try:
            self.mirrorCtrl_tv_npixels = self.app.ReadVar( cfg.varname('mirrorctrl', "TV_NPIXELS"))
        except AOVarNotInRTDB, e:
            print "WARNING: MirrorCtrl is not working properly. Impossible to notify it if frame size changes !" 
        
        # RTDBVar to notify MasterDiagnostic Diagnostic of the frame size change
        try:
            self.mastDiagn_npixels = self.app.ReadVar( cfg.varname('masterdiagnostic', "TV_NPIXELS"))
            print self.mastDiagn_npixels
        except AOVarNotInRTDB, e: 
            print "WARNING: MasterDiagnostic is not working properly. Impossible to notify it if frame size changes !" 
        except thrdlib.varNotFound, e: 
            print "WARNING: MasterDiagnostic is not working properly. Impossible to notify it if frame size changes !" 


    def set_npixels( self, n):

        if not self.checkPolicy():
            return

        self.v_npixels_req.SetValue(n)
        self.app.WriteVar( self.v_npixels_req, nocreate = True)
        self.app.WaitVarValue( self.v_npixels_cur, n, timeout = 1)
        
        if cfg.techviewer_frames_producer == "masterdiagnostic":
            print "Notifying MasterDiagnostic for frame size change..."
            self.mastDiagn_npixels.SetValue(n)
            self.app.WriteVar(self.mastDiagn_npixels, nocreate = True)
        elif cfg.techviewer_frames_producer == "mirrorctrl":
            print "Notifying MirrorCtrl..."
            self.mirrorCtrl_tv_npixels.SetValue(n)
            self.app.WriteVar(self.mirrorCtrl_tv_npixels, nocreate = True)
        else:
             print "ERROR: Neither MasterDiagnostic and MirrorCtrl aren't performing the TechnicalViewer frames download!"
    

    def get_npixels( self):
        return self.app.ReadVar( self.v_npixels_cur).Value()

    def set_dark( self, filename):
        if not self.checkPolicy():
            return
	self.v_dark_cur.SetValue(filename)
        self.app.WriteVar( self.v_dark_cur)

    def get_dark( self):
        return self.app.ReadVar( self.v_dark_cur).Value()


