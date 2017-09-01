
from ccd_gui_impl import *

import os
from AdOpt import cfg

class ccd47_gui(ccd_gui_impl):
    
    def __init__(self, aoGui):
        ccd_gui_impl.__init__(self, 'ccd47', aoGui)
        self.liveView_process = 'tvccd'
        self.saveToFile_process = 'ccd47acq'
        self.changeBinning_process = 'changebin47';

        self.newBinning.insertItem("Select")
        self.newBinning.insertItem("1x1")
        #self.newBinning.insertItem("2x2")
        #self.newBinning.insertItem("4x4")
        self.newBinning.insertItem("16x16")

    def newBinning_activated(self, s):
       s = s.latin1()
       binning = s[0:s.index('x')]

       command = os.environ['ADOPT_ROOT']+"/bin/change_binning47.py %s -showError" % binning

       command = "xterm -T \"Change binning\" -e \""+command+"\""
       print command

       os.system(command)

      
    def enableCustomButtons(self, enable):
        self.equalizeButton.setEnabled(False)
        self.saveToFileButton.setEnabled(enable)
 
