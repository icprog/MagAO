#!/usr/bin/env python

from AdOpt.QtDesigner.AutoFocusPisces_gui import *
from AdOpt.autoFocus47 import *
import sys
from AdOpt.WfsGUI.thAOAppGUI import *



class AutoFocusPiscesGui(autoFocusPisces_gui, thAOAppGUI):

    def __init__(self, qApp):
        thAOAppGUI.__init__(self, qApp, 'FocusPisces')
        autoFocusPisces_gui.__init__(self)


    def buttonStart_clicked(self):

        zRange = float(self.editFocusRange.text().latin1())
        numSamples = float(self.editFocusRange.text().latin1()) / float(self.editFocusStep.text().latin1())
        numFrames = float(self.editImageAvg.text().latin1())
        cx = int(self.editPosX.text().latin1())
        cy = int(self.editPosY.text().latin1())

        tracknum, positions = autoFocusPisces( self, zRange = zRange, numSamples = numSamples, numFrames = numFrames)
        findPeakIdl( self, tracknum, positions, cx, cy, interactive=True)



if __name__ == '__main__':
    qApp = QApplication( sys.argv)

    main = AutoFocusPiscesGui(qApp)
    qApp.setMainWidget(main)
    main.show()

    qApp.exec_loop()


