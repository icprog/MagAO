#!/usr/bin/env python
#
#+File: calib_gui.py
#
# system tests GUI
#
#-

from qt import *
from AdOpt import processControl, cfg
from AdOpt.widgets import LogWidget

#+Class: calib_gui
#
# Main window for the GUI tool
#-

class calib_gui( QDialog):
    
    def __init__(self, AOwindow):
        QDialog.__init__(self)

        if not vars(self).has_key('prefix'):
            self.prefix = 'optcalib'

        self.name = 'Optical calibration'

        self.AOwindow = AOwindow

        ## Draw the widgets

        LayoutWidget = QWidget(self,"layout1")
        LayoutWidget.setGeometry(QRect(20,20,400,500))

        layout1 = QVBoxLayout( LayoutWidget, 11, 2, "layout1")
        layout2 = QHBoxLayout( None, 0, 2, "layout2")
        layout3 = QVBoxLayout( None, 0, 2, "layout3")

        self.label1 = QLabel("Mechanical calibration", LayoutWidget)
        self.label1.setFont( QFont('arial', 20))
        layout3.addWidget(self.label1)

        self.calibAutoCenterStagesButton = QPushButton( "Calibrate pupil autocenter/autofocus with XYZ stages", LayoutWidget)
        self.connect( self.calibAutoCenterStagesButton, SIGNAL("clicked()"), self.slotCalibAutocenterStagesButton)
        layout3.addWidget(self.calibAutoCenterStagesButton)

        self.calibAutoCenterTTButton = QPushButton( "Calibrate pupil autocenter with tip-tilt", LayoutWidget)
        self.connect( self.calibAutoCenterTTButton, SIGNAL("clicked()"), self.slotCalibAutocenterTTButton)
        layout3.addWidget(self.calibAutoCenterTTButton)

        self.calibSourceAcqButton = QPushButton( "Calibrate source acq. movement on technical viewer", LayoutWidget)
        self.connect( self.calibSourceAcqButton, SIGNAL("clicked()"), self.slotCalibSourceAcqButton)
        layout3.addWidget(self.calibSourceAcqButton)

        self.calibSourceAcqPositionButton = QPushButton( "Calibrate source acq. position on technical viewer", LayoutWidget)
        self.connect( self.calibSourceAcqPositionButton, SIGNAL("clicked()"), self.slotCalibSourceAcqPositionButton)
        layout3.addWidget(self.calibSourceAcqPositionButton)

        self.calibFw1PsfTiltButton = QPushButton( "Calibrate fw1 psf tilting", LayoutWidget)
        self.connect( self.calibFw1PsfTiltButton, SIGNAL("clicked()"), self.slotCalibFw1PsfTiltButton)
        layout3.addWidget(self.calibFw1PsfTiltButton)

        self.label2 = QLabel("Optical calibration", LayoutWidget)
        self.label2.setFont( QFont('arial', 20))
        layout3.addWidget(self.label2)



        self.calibPupilsButton = QPushButton( "Acquire new pupils", LayoutWidget)
        self.connect( self.calibPupilsButton, SIGNAL("clicked()"), self.slotCalibPupilsButton)
        layout3.addWidget(self.calibPupilsButton)

        self.calibSlopenullButton = QPushButton( "Acquire slope null", LayoutWidget)
        self.connect( self.calibSlopenullButton, SIGNAL("clicked()"), self.slotCalibSlopenullButton)
        layout3.addWidget(self.calibSlopenullButton)

        self.label3 = QLabel("Stop", LayoutWidget)
        self.label3.setFont( QFont('arial', 20))
        layout3.addWidget(self.label3)

        self.stopButton = QPushButton("Stop script", LayoutWidget);
        self.connect( self.stopButton, SIGNAL("clicked()"), self.slotStop)
        layout3.addWidget(self.stopButton)

        layout2.addLayout( layout3)
        spacer1 = QSpacerItem(110,70,QSizePolicy.Expanding,QSizePolicy.Minimum)
        layout2.addItem(spacer1)
        layout1.addLayout(layout2)

        self.logWidget = LogWidget.LogWidget(LayoutWidget, 'title')
        layout1.addWidget(self.logWidget)


        self.last = ''

    def startCalib( self, process, confirm=True):
        if confirm:
            result = QMessageBox.question( self, "Are you sure?", "Confirm calibration start?", QMessageBox.Yes, QMessageBox.No)
            if result != QMessageBox.Yes:
                return

        self.logWidget.setFile( cfg.stdoutfile(process))
        processControl.startProcessByName(process, multi=0)
        self.last = process
            

    def slotCalibAutocenterStagesButton(self):
        self.startCalib( 'calib_autocenterstages')

    def slotCalibAutocenterTTButton(self):
        self.startCalib( 'calib_autocentertt')

    def slotCalibSourceAcqButton(self):
        self.startCalib( 'calib_sourceacqmovement')

    def slotCalibSourceAcqPositionButton(self):
        self.startCalib( 'calib_sourceacqposition')

    def slotCalibFw1PsfTiltButton(self):
        self.startCalib( 'calib_fw1psftilt')

    def slotCalibSlopenullButton(self):
        self.startCalib( 'calib_slopenull')

    def slotCalibPupilsButton(self):
        self.startCalib( 'calib_pupils')


    def slotStop(self):
        if self.last != '':
            processControl.stopProcessByName(self.last, kill = True)


