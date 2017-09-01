#!/usr/bin/env python
#
#+File: tests.py
#
# system tests GUI
#
#-

from qt import *
from AdOpt import processControl, cfg
from AdOpt.widgets import LogWidget

#+Class: tests_gui
#
# Main window for the GUI tool
#-

class tests_gui( QDialog):
    
    def __init__(self, AOwindow):
        QDialog.__init__(self)

        if not vars(self).has_key('prefix'):
            self.prefix = 'testsgui'

        self.name = 'System tests'
        self.setGeometry(QRect(20,20,700,350))

        self.AOwindow = AOwindow

        ## Draw the widgets

        mainLayout = QVBoxLayout(self,11,6,"layout1")
        layout2 = QHBoxLayout(None,0,6,"layout2")
        layout1 = QVBoxLayout(None,0,7,"layout1")

        self.label1 = QLabel( self, "System tests")
        layout1.addWidget(self.label1)

        self.ronButton = QPushButton( "ccd39 RON test", self)
        self.connect( self.ronButton, SIGNAL("clicked()"), self.slotRonButton)
        layout1.addWidget(self.ronButton)

        self.ronTableButton = QPushButton( "ccd39 RON AO TABLE test", self)
        self.connect( self.ronTableButton, SIGNAL("clicked()"), self.slotRonTableButton)
        layout1.addWidget(self.ronTableButton)

        self.rerotButton = QPushButton( "tozzi.py", self)
        self.connect( self.rerotButton, SIGNAL("clicked()"), self.slotRerotButton)
        layout1.addWidget(self.rerotButton)

        self.muovileggiButton = QPushButton( "Muovi leggi rerotator", self)
        self.connect( self.muovileggiButton, SIGNAL("clicked()"), self.slotMuoviLeggi)
        layout1.addWidget(self.muovileggiButton)

        self.autocenterStagesButton = QPushButton( "Autocenter stages", self)
        self.connect( self.autocenterStagesButton, SIGNAL("clicked()"), self.slotCenterStages)
        layout1.addWidget(self.autocenterStagesButton)

        self.autocenterTTButton = QPushButton( "Autocenter tip-tilt", self)
        self.connect( self.autocenterTTButton, SIGNAL("clicked()"), self.slotCenterTT)
        layout1.addWidget(self.autocenterTTButton)

        self.autocenterFocusButton = QPushButton( "Autofocus with Z stage", self)
        self.connect( self.autocenterFocusButton, SIGNAL("clicked()"), self.slotAutoFocus)
        layout1.addWidget(self.autocenterFocusButton)

        self.sourceacqButton = QPushButton( "Perform source acquisition", self)
        self.connect( self.sourceacqButton, SIGNAL("clicked()"), self.slotSourceAcq)
        layout1.addWidget(self.sourceacqButton)

        layout2.addLayout( layout1)
        spacer1 = QSpacerItem(330,20,QSizePolicy.Expanding,QSizePolicy.Minimum)
        layout2.addItem(spacer1)
        mainLayout.addLayout(layout2)

        self.logWidget = LogWidget.LogWidget(self, 'title')
        mainLayout.addWidget(self.logWidget)

        self.stopButton = QPushButton( "Stop script", self);
        self.connect( self.stopButton, SIGNAL("clicked()"), self.slotStop)
        layout1.addWidget(self.stopButton)

        self.last = ''

    def slotRonButton(self):
        processControl.startProcessByName('ccd39ron_test', multi=0)
        self.logWidget.setFile( cfg.stdoutfile('ccd39ron_test'))
        self.last = 'ccd39ron_test'

    def slotRonTableButton(self):
        processControl.startProcessByName('ccd39ron_table', multi=0)
        self.logWidget.setFile( cfg.stdoutfile('ccd39ron_table'))
        self.last = 'ccd39ron_table'

    def slotRerotButton(self):
        processControl.startProcessByName('rerotator_test', multi=0)
        self.logWidget.setFile( cfg.stdoutfile('rerotator_test'))
        self.last = 'rerotator_test'

    def slotMuoviLeggi(self):
        processControl.startProcessByName('muovileggirot', multi=0)
        self.logWidget.setFile( cfg.stdoutfile('muovileggirot'))
        self.last = 'muovileggirot'

    def slotCenterStages(self):
        processControl.startProcessByName('autocenter_stages', multi=0)
        self.logWidget.setFile( cfg.logfile('autocenter_stages'))
        self.last = 'autocenter_stages'

    def slotCenterTT(self):
        processControl.startProcessByName('autocenter_tt', multi=0)
        self.logWidget.setFile( cfg.logfile('autocenter_tt'))
        self.last = 'autocenter_tt'

    def slotAutoFocus(self):
        processControl.startProcessByName('autofocus', multi=0)
        self.logWidget.setFile( cfg.logfile('autofocus'))
        self.last = 'autofocus'

    def slotSourceAcq(self):
        processControl.startProcessByName('sourceacq_acquire', multi=0)
        self.logWidget.setFile( cfg.stdoutfile('sourceacq_acquire'))
        self.last = 'sourceacq_acquire'

    def slotStop(self):
        if self.last != '':
            processControl.stopProcessByName(self.last, kill = True)


