#!/usr/bin/env python
#
#@File: change_binning_gui.py
#
# GUI frontend for the change binning script
#
#@

import sys, os
from qt import *
import cfg


#@Class: changebinning_gui
#
# Main window class
#@

class changebinning_gui( QDialog):
    
    def __init__(self):
        QDialog.__init__(self)

        self.gridlayout = QGridLayout( self, 5, 2, 10)
        self.gridlayout.setColSpacing(1, 100)
        self.gridlayout.setAutoAdd(1)

        self.label1 = QLabel("Change ccd39 binning", self)
        self.dummy = QLabel("", self)

        self.group = QButtonGroup(self)
        self.group.hide()

        self.dummy = QLabel("", self)
        self.bin1 = QPushButton("Bin 1x1", self)
        self.group.insert(self.bin1)

        self.dummy = QLabel("", self)
        self.bin2 = QPushButton("Bin 2x2", self)
        self.group.insert(self.bin2)

        self.dummy = QLabel("", self)
        self.bin3 = QPushButton("Bin 3x3", self)
        self.group.insert(self.bin3)

        self.dummy = QLabel("", self)
        self.bin4 = QPushButton("Bin 4x4", self)
        self.group.insert(self.bin4)

        self.dummy = QLabel("", self)
        self.bin5 = QPushButton("Bin 5x5", self)
        self.group.insert(self.bin5)

        self.connect( self.group, SIGNAL("clicked(int)"), self.slotButton)

    def slotButton(self, id):
        binning = id+1

        command = os.environ['ADOPT_ROOT']+"/scripts/change_binning.py -s %s %s last ; /bin/tcsh" % (self.side, binning)

        command = "xterm -T \"Change binning\" -e \""+command+"\""
        print command

        os.system(command)

# Main program
# Si limita a creare una QApplication, importare il modulo C++ e creare gli oggetti necessari

if __name__ == "__main__":

    app = QApplication(sys.argv)    
    name = "VAR"

    font = QFont("times")
    app.setFont( font)

    main = changebinning_gui()
    main.side = sys.argv[1]

    app.setMainWidget( main)
    main.show()

app.exec_loop()

