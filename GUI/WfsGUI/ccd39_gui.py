#!/usr/bin/env python

from ccd_gui_impl import *

import os
from AdOpt import cfg

class ccd39_gui(ccd_gui_impl):
    
    def __init__(self, aoGui):
        ccd_gui_impl.__init__(self, 'ccd39', aoGui)
        self.liveView_process = 'hoccd'
        self.saveToFile_process = 'ccd39acq'
        self.changeBinning_process = 'changebin39';
        self.equalizeQuadrants_process = 'quadranteq';

        self.newBinning.insertItem("Select")
        self.newBinning.insertItem("1x1")
        self.newBinning.insertItem("2x2")
        self.newBinning.insertItem("3x3")
        self.newBinning.insertItem("4x4")
        self.newBinning.insertItem("5x5")

        self.newGain.insertItem("Select")
        self.newGain.insertItem("High")
        self.newGain.insertItem("Med. Hi")
        self.newGain.insertItem("Med. Low")
        self.newGain.insertItem("Low")

    def enableCustomButtons(self, enable):
        self.equalizeButton.setEnabled(enable)
        self.saveToFileButton.setEnabled(enable)   

    def newBinning_activated(self, str):
        binning = str[0]
        if binning == 'S':
            return

        command = os.environ['ADOPT_ROOT']+"/bin/change_binning.py %s last -showError" % binning
        command = "xterm -T \"Change binning\" -e \""+command+"\""
        print command

        os.system(command)

    def newGain_activated(self, str):
        print "newGain activated"
        print str
        ng = 0
        if str == 'Med. Hi':
          ng = 1
        elif str == 'Med. Low':
          ng = 2
        elif str == 'Low':
          ng = 3

        print ng  
        msglib.SetIntVar(self.varname_reqGain, ng)
        
     
# Main
if __name__ == "__main__":
    from engineeringGui import EngineeringGui

    app = QApplication(sys.argv) 
    g = EngineeringGui( name='ccd39gui')
    myGui = ccd39_gui(g)
    app.setMainWidget(myGui)
    myGui.show()
    app.exec_loop()
