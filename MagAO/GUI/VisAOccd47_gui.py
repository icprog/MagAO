
from VisAOccd_gui_impl import *

import os
from AdOpt import cfg

class VisAOccd47_gui(VisAOccd_gui_impl):
    
    def __init__(self, aoGui):
        VisAOccd_gui_impl.__init__(self, 'ccd47', aoGui)
        self.liveView_process = 'tvccd'
        self.saveToFile_process = 'ccd47acq'
        self.changeBinning_process = 'changebin47';

        self.newBinning.insertItem("Select")
        self.newBinning.insertItem("1x1")
        self.newBinning.insertItem("2x2")
        self.newBinning.insertItem("16x16")

    def newBinning_activated(self, s):
       if self.curctype != 1:
          QMessageBox.information( self, "ccd47 Control","You must first take Remote Control to change settings on ccd47 from here.")
          return
       s = s.latin1()
       binning = s[0:s.index('x')]
       #bins = { 1:0, 2:1, 16:2}
       print "Binning (int): " + binning
       if binning == '1':
         bins = 1
       if binning == '2':
         bins = 2
       if binning == '16':
         bins = 16
       
       print "Binning (str): " + str(bins)
       command = os.environ['ADOPT_ROOT']+"/bin/change_binning47.py %s -showError" % (binning)

       command = "xterm -T \"Change binning\" -e \""+command+"\""
       print command

       os.system(command)

       #Temp for VisAO development.  Just do once.
       #msglib.SetIntVar(self.varname_reqXbin, bins)
       #msglib.SetIntVar(self.varname_reqYbin, bins)
       
    def enableCustomButtons(self, enable):
        self.equalizeButton.setEnabled(enable)
        self.saveToFileButton.setEnabled(enable)
        self.newWindow.setEnabled(enable)
        self.readoutReqValue.setEnabled(enable)
        self.gainReq.setEnabled(enable)
        
 