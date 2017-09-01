#!/usr/bin/env python
#
#@File: mirror_tt_gui.py
#
# GUI frontend for the mirror TT settings
#
#@

import sys, os, time
from numarray import *
import fits_lib
from qt import *
from AdOpt import *


# First and foremost, construct the QApplication object

app = QApplication( sys.argv)

#@Class: mirror_tt_gui
#
# Main window class
#@

class mirror_tt_gui( QDialog, AOApp):
    
    def __init__(self):
        AOApp.__init__(self, "mirrortt")
        QDialog.__init__(self)

        self.gridlayout = QGridLayout( self, 4, 6, 10)
        self.gridlayout.setColSpacing(1, 100)
        self.gridlayout.setAutoAdd(1)

#        self.dummy1 = QLabel("", self)       # To align things

        self.label1 = QLabel("Tilt X (nm):", self)
        self.cur_x  = QLabel("0", self)
        self.tilt_x = QLineEdit("0",self)

        self.dummy1   = QLabel("", self)
        self.u_button = QPushButton("Up", self)
        self.dummy2   = QLabel("", self)

        self.label2 = QLabel("Tilt Y (nm):", self)
        self.cur_y  = QLabel("0", self)
        self.tilt_y = QLineEdit("0",self)

        self.l_button = QPushButton("Left", self)
        self.dummy3   = QLabel("", self)
        self.r_button = QPushButton("Right", self)

        self.dummy4   = QLabel("", self)
        self.dummy5   = QLabel("", self)
        self.set_button   = QPushButton("Set", self)
        self.dummy7   = QLabel("", self)
        self.d_button = QPushButton("Down", self)
        self.dummy8   = QLabel("", self)

        self.dummy9    = QLabel("", self)
        self.dummy10   = QLabel("", self)
        self.bias_check= QCheckBox("Set as new bias", self)
        self.dummy11   = QLabel("", self)
        self.dummy12   = QLabel("", self)
        self.dummy13   = QLabel("", self)

        self.u_button.setPixmap( QPixmap("up.png"))
        self.d_button.setPixmap( QPixmap("down.png"))
        self.l_button.setPixmap( QPixmap("left.png"))
        self.r_button.setPixmap( QPixmap("right.png"))

        self.u_button.setAutoRepeat(1)
        self.d_button.setAutoRepeat(1)
        self.l_button.setAutoRepeat(1)
        self.r_button.setAutoRepeat(1)

        self.zernike_var   = self.ReadVar("L.ADSEC.USE_ZERNIKE")
        self.cmd2bias_var  = self.ReadVar("L.ADSEC.CMD2BIAS")
        self.file_var      = self.ReadVar("L.ADSEC.APPLY_MODES_FILE")
        self.trigger_var   = self.ReadVar("L.ADSEC.TRIG_APPLY_MODES")

        self.connect( self.u_button, SIGNAL("clicked()"), self.slotUButton)
        self.connect( self.d_button, SIGNAL("clicked()"), self.slotDButton)
        self.connect( self.r_button, SIGNAL("clicked()"), self.slotRButton)
        self.connect( self.l_button, SIGNAL("clicked()"), self.slotLButton)
        self.connect( self.bias_check, SIGNAL("clicked()"), self.slotBiasCheck)
        
        self.connect( self.set_button, SIGNAL("clicked()"), self.slotSet)


        self.step = 10

    def slotUButton(self):
        x = float( self.cur_x.text().latin1())
        y = float( self.cur_y.text().latin1()) - self.step
        self.send_and_update(x,y)
        
    def slotDButton(self):
        x = float( self.cur_x.text().latin1())
        y = float( self.cur_y.text().latin1()) + self.step
        self.send_and_update(x,y)

    def slotLButton(self):
        x = float( self.cur_x.text().latin1()) - self.step
        y = float( self.cur_y.text().latin1())
        self.send_and_update(x,y)

    def slotRButton(self):
        x = float( self.cur_x.text().latin1()) + self.step
        y = float( self.cur_y.text().latin1())
        self.send_and_update(x,y)

    def slotSet(self):
        try:
            x = float( self.tilt_x.text().latin1())
        except:
            x = 0
            self.tilt_x.setText("0")
        try:
            y = float( self.tilt_y.text().latin1())
        except:
            y = 0
            self.tilt_y.setText("0")

        self.send_and_update(x,y)

    def slotBiasCheck(self):
        if self.bias_check.isChecked():
            self.SetVar( self.cmd2bias_var, 1)
        else:
            self.SetVar( self.cmd2bias_var, 0)
        

    def send_and_update(self, x, y):

        # Write vector file
        tmp_file = "/tmp/tt_cmd.fits"
        z = zeros(48, Float32)
        z[1] = x
        z[2] = y

        # Ensure that no previous handshake file is present,
        # and wait a bit if one is found
        counter = 0
        while counter < 20:
            if os.access( tmp_file, os.F_OK):
                time.sleep(0.1)
                counter = counter+1
            else:
                break

        # If the handshake file doesn't go away, ask what to do
        if counter == 20:
            result = QMessageBox.warning( self, "Problem", "A previous mirror handshake failed. Force this command?", QMessageBox.Yes, QMessageBox.No)
            if result == QMessageBox.Yes:
                os.unlink( tmp_file)
            else:
                return

        fits_lib.writeSimpleFits( tmp_file, z, {})

        # Set options vars
        self.SetVar( self.zernike_var, 1)
        self.SetVar( self.file_var, "'%s'" % tmp_file)

        # Trigger movement
        #time.sleep(0.1)
        self.SetVar( self.trigger_var, 1)

        self.cur_x.setText(str(x))
        self.cur_y.setText(str(y))


# Main program
# Si limita a creare una QApplication, importare il modulo C++ e creare gli oggetti necessari
    
font = QFont("times")
app.setFont( font)

main = mirror_tt_gui()
app.setMainWidget( main)
main.show()

app.exec_loop()


