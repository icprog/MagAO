#!/usr/bin/env python
#
#@File: cloop_gui.py
#
# GUI frontend for the closed loop acquisition script
#
#@

import sys,os, fits_lib
from qt import *
from qttable import *
from numarray import *
import cfg


# First and foremost, construct the QApplication object

app = QApplication( sys.argv)

#@Class: cloop_gui
#
# Main window class
#@

class cloop_gui( QDialog):
    
    def __init__(self):
        QMainWindow.__init__(self)

        self.gridlayout = QGridLayout( self, 5, 2, 10)
        self.gridlayout.setColSpacing(1, 100)
        self.gridlayout.setAutoAdd(1)

        ampfile = os.environ['ADOPT_ROOT']+"/config/modes_amp/full.fits"

        self.label1 = QLabel("Loop name", self)
        self.loopname = QLineEdit("",self)

        self.label1 = QLabel("Gain", self)
        self.gain = QLineEdit("-1.0",self)

        self.label5 = QLabel("Start modes file:", self)
        self.hbox1 = QLabel("", self)
        self.hbox1_lay  = QHBoxLayout( self.hbox1)

        self.dummy4 = QLabel("", self)
        self.hbox2 = QLabel("", self)
        self.hbox2_lay = QHBoxLayout(self.hbox2)

        self.dummy2 = QLabel("", self)
        self.startbutton = QPushButton("Start measure", self)
        self.connect( self.startbutton, SIGNAL("clicked()"), self.slotStartButton)

        self.exitbutton = QPushButton("Exit", self)
        self.connect( self.exitbutton, SIGNAL("clicked()"), self.slotExitButton)

        self.ampfile = QLineEdit( ampfile, self.hbox1)
        self.browse = QPushButton("...", self.hbox1)
        self.browse.setMaximumSize(QSize(30, 40))
        self.hbox1_lay.addWidget( self.ampfile)
        self.hbox1_lay.addWidget( self.browse)


        self.edit = QPushButton("Edit", self.hbox2)
        self.new = QPushButton("Create new", self.hbox2)
        self.hbox2_lay.addWidget(self.edit)
        self.hbox2_lay.addWidget(self.new)

        self.connect( self.browse, SIGNAL("clicked()"), self.slotBrowseButton)
        self.connect( self.edit, SIGNAL("clicked()"), self.slotEditButton)
        self.connect( self.new, SIGNAL("clicked()"), self.slotNewButton)

    def slotExitButton(self):
        self.close()

    def slotNewButton(self):
        a = zeros(cfg.n_dm_modes, Float32)
        dlg = amp_table( self, a, None)
        dlg.exec_loop()
        
    def slotEditButton(self):
        filename = self.ampfile.text().latin1()
        if filename == "":
            QMessageBox.warning( self, "Error", "Invalid filename", QMessageBox.Ok)
            return
            
        a = fits_lib.readSimpleFits(filename)
        if len(a) != cfg.n_dm_modes and len(a) != cfg.real_dm_modes:
            QMessageBox.warning( self, "Error", "This is not a valid amplitude file (len=%d instead of %d/%d)" % len(a), QMessageBox.Ok, cfg.n_dm_modes, cfg.n_real_dm_modes)
        else:
            dlg = amp_table( self, a, filename)
            dlg.exec_loop()


    def slotBrowseButton(self):
        dir = os.environ['ADOPT_ROOT']+"/config/modes_amp" 
        filename = QFileDialog.getOpenFileName( dir, "Fits files (*.fits)", self, "", "Select an amplitude file")

        if filename != QString.null:
            # check file valid
            a = fits_lib.readSimpleFits(filename.latin1())
            if len(a) != cfg.n_dm_modes and len(a) != cfg.real_dm_modes:
                QMessageBox.warning( self, "Error", "This is not a valid amplitude file (len=%d instead of %d/%d)" % len(a), QMessageBox.Ok, cfg.n_dm_modes, cfg.real_dm_modes)
            else:
                self.ampfile.setText(filename)
        

    def slotStartButton(self):
        try:
            loopname = self.loopname.text().latin1()
            ampfile    = self.ampfile.text().latin1()
            gain      = float(self.gain.text().latin1())
 

            if loopname == "" or ampfile == "":
                raise

            # Traps both int() exceptions and raised exceptions
        except:
            QMessageBox.warning(self,"Error in measurement parameters","Error in measurement parameters")
            return

        command = os.environ['ADOPT_ROOT']+"/scripts/cloop.py --modes='%s' --gain='%5.2f' %s ; /bin/tcsh" % (ampfile, gain, loopname)

        command = "xterm -T \"Closed loop\" -e \""+command+"\""
        print command

        os.system(command)

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


#@class amp_table

class amp_table( QDialog):
    
    def __init__(self, parent, amps, filename = None):
        QDialog.__init__(self, parent)
        self.resize(400,600)

        self.layout = QVBoxLayout( self, 10)

        self.table = QModesTable( cfg.real_dm_modes, 2, self)
        self.layout.addWidget(self.table, 0)

        self.table.horizontalHeader().setLabel(0, "Mode")
        self.table.horizontalHeader().setLabel(1, "Amplitude (0-1)")
        self.table.verticalHeader().hide()
        self.table.setLeftMargin(0)

        for n in range(cfg.real_dm_modes):
            if cfg.modes.has_key(n):
                self.table.setText( n, 0, cfg.modes[n])
            else:
                self.table.setText( n, 0, 'mode %d' % n)

            self.table.setText( n, 1, "%1.3f" % amps[n])

        self.save = QPushButton("Save", self)
        self.layout.addWidget( self.save, 1)
        self.connect( self.save, SIGNAL("clicked()"), self.slotSaveButton)

        self.filename = filename

    def slotSaveButton(self):
        if self.filename:
            if os.access(self.filename, os.F_OK):
                answer = QMessageBox.question(self, "Overwrite?", "%s already exists.\nOverwrite?" % self.filename, QMessageBox.Yes, QMessageBox.No)
                if answer == QMessageBox.No:
                    return

        else:
            dir = os.environ['ADOPT_ROOT']+"/config/modes_amp"
            f = QFileDialog.getSaveFileName(dir, "Fits files (*.fits)", self, "Save", "Choose a filename")
            if f == QString.null:
                return
            else:
                self.filename = f.latin1()
           
        data = zeros( cfg.n_dm_modes, Float32)
        for n in range(cfg.real_dm_modes):
            data[n] = float( self.table.text(n,1).latin1())

        fits_lib.writeSimpleFits( self.filename, data, {}, overwrite = True)
        self.parentWidget().ampfile.setText(self.filename)






# Main program
# Si limita a creare una QApplication, importare il modulo C++ e creare gli oggetti necessari
    
font = QFont("times")
app.setFont( font)

main = cloop_gui()
app.setMainWidget( main)
main.show()

app.exec_loop()

