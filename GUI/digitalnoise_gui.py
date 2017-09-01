#!/usr/bin/env python
#
#@File: digitalnoise_gui.py
#
# GUI frontend for the interaction matrix measurement script
#
#@

import sys,os,time
from qt import *
from qttable import *
import idl, fits_lib
from numarray import *
from AdOpt import *
import cfg

# First and foremost, construct the QApplication object

app = QApplication( sys.argv)

## +Class
##
## QModesTable  QTable derived class for a table with a read-only first column
##
## The createEditor() method is reimplemented to prevent
## in-place editing of the table contents

class QModesTable(QTable):

    def __init__(self, row, col, parent):
        QTable.__init__(self, row, col, parent) 

    def createEditor( self, row, col, f):
        if col == 0:
            return None
        else:
            return QTable.createEditor( self, row, col, f)

#@Class: digitalnoise_gui
#
# Main window class
#@

class digitalnoise_gui( QDialog, AOApp):
    
    def __init__(self):
        AOApp.__init__(self, "dnoise")
        QDialog.__init__(self)
        self.resize(400,600)

        self.layout = QVBoxLayout( self, 10)

        self.table = QModesTable( cfg.real_dm_modes, 3, self)
        self.layout.addWidget(self.table, 0)

        self.table.horizontalHeader().setLabel(0, "Mode")
        self.table.horizontalHeader().setLabel(1, "Amplitude (%)")
        self.table.horizontalHeader().setLabel(2, "Periods")
        self.table.verticalHeader().hide()
        self.table.setLeftMargin(0)

        for n in range(cfg.real_dm_modes):
            if cfg.modes.has_key(n):
                self.table.setText( n, 0, cfg.modes[n])
            else:
                self.table.setText( n, 0, 'mode %d' % (n+1))

        self.apply_button = QPushButton("Apply", self)
        self.layout.addWidget( self.apply_button, 1)
        self.connect( self.apply_button, SIGNAL("clicked()"), self.slotApplyButton)

        self.noise_var  = self.ReadVar("WFS01CMD_OFFSETS")
        self.enable_var = self.ReadVar("WFS01CMD_ENABLE")

        self.cmd_stop = QPushButton("Stop", self)
        self.connect( self.cmd_stop, SIGNAL("clicked()"), self.slotStopButton)

        self.layout.addWidget( self.cmd_stop)


    def slotApplyButton(self):

        idl_file = os.environ['ADOPT_ROOT']+'/scripts/modal_noise.pro'
        idl.ex('.r '+idl_file)
        idl_file = os.environ['ADOPT_ROOT']+'/scripts/gen_noise.pro'
        idl.ex('.r '+idl_file)

        amp_file = "/tmp/amp_file.fits"
        per_file = "/tmp/per_file.fits"
        #out_file = "/tmp/out_file.fits"
        out_file = os.environ['ADOPT_ROOT']+'/config/commands/modes_with_atmosphere.fits'

        amp = zeros( cfg.n_dm_modes, Float32)
        per = zeros( cfg.n_dm_modes, Float32)
        for n in range(cfg.real_dm_modes):
            per[n] = float( self.formatString( self.table.text( n, 2).latin1()))
            a      = float( self.formatString( self.table.text( n, 1).latin1()))
            if a<0:
                a=0
            if a >100:
                a = 100
            amp[n] = a / 100.0

        fits_lib.writeSimpleFits( amp_file, amp, {}, overwrite = True)
        fits_lib.writeSimpleFits( per_file, per, {}, overwrite = True)

        idlroot = os.path.dirname(os.environ['IDL_STARTUP'])
        m2c_file    = idlroot+'/adsecP45/data/m2c.fits'
        maxamp_file = idlroot+'/adsecP45/data/max_modal_amplitude.fits'
        ff_file     = idlroot+'/adsecP45/data/ff_matrix.sav'

        idl_cmd = "modal_noise, '%s', AMPFILE='%s', PERIODSFILE='%s', M2C_FILE='%s', FF_FILE='%s', MAXAMP_FILE='%s' " % \
                   (out_file, amp_file, per_file, m2c_file, ff_file, maxamp_file)
        print idl_cmd
        idl.ex(idl_cmd)
    
#        self.SetVar( self.noise_var, out_file)
#        time.sleep(0.5)
#        self.SetVar( self.enable_var, 1)
           
    def slotStopButton(self):
        self.SetVar( self.enable_var, 0) 

    def formatString( self, s):
        try:
            n = float(s)
            return s
        except:
            return "0"

# Main program
# Si limita a creare una QApplication, importare il modulo C++ e creare gli oggetti necessari
    
app.setFont( QFont("times"))

main = digitalnoise_gui()
app.setMainWidget( main)
main.show()

app.exec_loop()

