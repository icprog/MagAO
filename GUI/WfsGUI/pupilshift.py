#!/usr/bin/env python

from AdOpt.QtDesigner.pupilshift import *
from AdOpt.WfsArbScripts import *
from AdOpt.makePupils import *
import sys, thread, time, glob
from AdOpt.WfsGUI.thAOAppGUI import *
from AdOpt import cfg


class PupilShiftGui(PupilShift):

    def __init__(self, app):
        PupilShift.__init__(self)

        self.app = app
        self.varname_curxbin = cfg.varname( 'ccd39', 'XBIN.CUR')
        self.varname_pixellut = cfg.varname( 'slopecompctrl', 'PIXELLUT.CUR')

    def buttonUp_clicked(self):
        self.doShift( 1, 0, "up")

    def buttonDown_clicked(self):
        self.doShift( -1, 0, "down")

    def buttonLeft_clicked(self):
        self.doShift( 0, -1, "to the left")

    def buttonRight_clicked(self):
        self.doShift( 0, 1, "to the right")

    def doShift( self, x, y, dirStr):

        pix = self.spinPixels.value()
        x *= pix
        y *= pix

        answ = QMessageBox.warning( self, "Confirm", "This will move the current pupils by %d pixels %s\nDo you really want to do this?" % (pix, dirStr), QMessageBox.Yes, QMessageBox.No)
        if answ != QMessageBox.Yes:
            return

        binning = self.app.ReadVar( self.varname_curxbin).Value()
        pixellut = self.app.ReadVar( self.varname_pixellut).Value()
        old_tracknum = pixellut.split('/')[-2]

        new_tracknum = movePupils (old_tracknum, x, y, 80, binning)
        updateHOpupil( self.app, binning, new_tracknum)

class MainGui( PupilShiftGui, thAOAppGUI):

    def __init__(self, qApp):
       thAOAppGUI.__init__(self, qApp, 'PupilShift')
       PupilShiftGui.__init__(self, self)


if __name__ == '__main__':
    qApp = QApplication( sys.argv)

    main = MainGui(qApp)
    qApp.setMainWidget(main)
    main.show()

    qApp.exec_loop()


