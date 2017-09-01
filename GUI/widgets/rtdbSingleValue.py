#!/usr/bin/python

from qt import *

import time, os.path
from AdOpt import calib,cfg
from AdOpt.wrappers import msglib
from AdOpt.WfsGUI import cfg_gui


class rtdbSingleValue( QWidget):

    def __init__( self, parent, name):
        
        QWidget.__init__(self, parent)

        self.setMinimumSize(QSize(320,20))
        self.setMaximumSize(QSize(320,20))

    def setup( self, name, aoGui, varname, type = 'real', button = 'pushbutton', spin_min=0, spin_max=100, spin_step = 1, watchdog = None, dir=None):

        # Accept a synonim for 'real'
        if type == 'float':
            type = 'real'
        if dir:
            self.dir = dir

        layout2 = QHBoxLayout(self,0,0,"layout2")

        self.nameLabel = QLabel('', self)
        self.nameLabel.setMinimumSize(QSize(110,20))
        self.nameLabel.setMaximumSize(QSize(110,20))
        layout2.addWidget( self.nameLabel)

        self.valueLabel = QLabel('', self)
        self.valueLabel.setMinimumSize(QSize(70,20))
        self.valueLabel.setMaximumSize(QSize(70,20))
        self.valueLabel.setBackgroundMode(QLabel.PaletteBackground)
        self.valueLabel.setFrameShape(QLabel.LineEditPanel)
        layout2.addWidget( self.valueLabel)

        if button == 'browse':
            self.valueLabel.setMinimumSize(QSize(140,20))
            self.valueLabel.setMaximumSize(QSize(140,20))

            self.valueEdit = QLineEdit("",self)
            self.valueEdit.setMinimumSize(QSize(0,20))
            self.valueEdit.setMaximumSize(QSize(0,20))
            layout2.addWidget(self.valueEdit)

            self.valueSet = QPushButton("...", self)
            self.valueSet.setMinimumSize(QSize(40,20))
            self.valueSet.setMaximumSize(QSize(40,20))
            layout2.addWidget(self.valueSet)

            self.connect( self.valueSet, SIGNAL("clicked()"), self.slotButtonPressed)
            self.connect( self.valueEdit, SIGNAL("returnPressed()"), self.slotButtonPressed)

        if button == 'pushbutton':
            self.valueEdit = QLineEdit("New value", self)
            self.valueEdit.setMinimumSize(QSize(70,20))
            self.valueEdit.setMaximumSize(QSize(70,20))
            layout2.addWidget(self.valueEdit)

            self.valueSet = QPushButton("Set", self)
            self.valueSet.setMinimumSize(QSize(40,20))
            self.valueSet.setMaximumSize(QSize(40,20))
            layout2.addWidget(self.valueSet)

            self.connect( self.valueSet, SIGNAL("clicked()"), self.slotButtonPressed)
            self.connect( self.valueEdit, SIGNAL("returnPressed()"), self.slotButtonPressed)

        elif button == 'spinbox':

            self.valueSet = QLabel('Set:', self)
            self.valueSet.setMinimumSize(QSize(40,20))
            self.valueSet.setMaximumSize(QSize(40,20))
            self.valueSet.setAlignment( Qt.AlignRight)
            layout2.addWidget(self.valueSet)

            self.valueEdit = QSpinBox( spin_min, spin_max, spin_step, self)
            self.valueEdit.setMinimumSize(QSize(30,20))
            self.valueEdit.setMaximumSize(QSize(30,20))
            layout2.addWidget(self.valueEdit)

            self.spacer = QLabel('', self)
            self.spacer.setMinimumSize(QSize(40,20))
            self.spacer.setMaximumSize(QSize(40,20))
            layout2.addWidget(self.spacer)

            self.connect( self.valueEdit, SIGNAL("valueChanged(int)"), self.slotButtonPressed)

        elif button == 'onoff':

            self.valueEdit = QComboBox(0, self, "ComboOnOff")
            self.valueEdit.setMinimumSize(QSize(80,20))
            self.valueEdit.setMaximumSize(QSize(80,20))
            self.valueEdit.insertItem('--select--')
            self.valueEdit.insertItem('On')
            self.valueEdit.insertItem('Off')
            layout2.addWidget(self.valueEdit)

            self.spacer = QLabel('', self)
            self.spacer.setMinimumSize(QSize(30,20))
            self.spacer.setMaximumSize(QSize(30,20))
            layout2.addWidget(self.spacer)

            self.connect( self.valueEdit, SIGNAL("activated( const QString &)"), self.slotButtonPressed)

        #self.resize(QSize(320,40).expandedTo(self.maximumSizeHint()))
        #self.clearWState(Qt.WState_Polished)


        self.nameLabel.setText(name)
        self.varname = varname
        self.type = type
        self.button = button
        aoGui.Notify( varname+'.CUR', self.slotValueChanged, getNow = True);

        self._lastUpdated = -1
        self._eraseColor = self.nameLabel.eraseColor()
        self._valid = False
        self._enabled = False

        if watchdog != None:
           self._timeout = 10
           QObject.connect( watchdog, SIGNAL("timeout()"), self.checkTimeout)

    def checkTimeout(self):
        '''
        Checks whether too much time has passed from the last update.
        '''
        if time.time() - self._lastUpdated > self._timeout:
            self._timeoutExpired = True
        else:
            self._timeoutExpired = False

        self.redisplay()


    def redisplay(self):
        '''
        Redraw widget according to current parameters.
        '''

        if self._timeoutExpired:
            enableLabel = False
            eraseColor = cfg_gui.colors.darkgray
        else:
            enableLabel = True
            eraseColor = cfg_gui.state2color[ self._currentStatus]

        self.nameLabel.setEraseColor(eraseColor)
        self.valueLabel.setEraseColor(eraseColor)



    def slotValueChanged(self, message):

        self._lastUpdated = message['mtime']

        if self.button == 'onoff':
            if message['value'] == 0:
                self.valueLabel.setText('Off')
            if message['value'] == 1:
                self.valueLabel.setText('On')
       
        else: 
            if self.type == 'real':
                self.valueLabel.setText( "%5.3f" % message['value'])
            if self.type == 'int':
                self.valueLabel.setText( "%d" % message['value'])
            if self.type == 'char':
                self.valueLabel.setText( "%s" % message['value'])

    def slotButtonPressed(self, dummy=0):

        if self.button == 'browse':
            txt =  QFileDialog.getOpenFileName( self.dir, "Dark files (*.fits)", self, "Dark file dialog", "Choose a dark file").latin1()
            if not txt:
                return
	    txt = os.path.basename(txt)

        elif self.button == 'onoff':
            txt = dummy.latin1() 
            try:
                txt = {'Off':0, 'On':1}[txt]
            except KeyError:
                return
        else:
            txt = self.valueEdit.text().latin1()

        if self.type == 'real':
            try:
                value = float(txt)
            except ValueError, e:
                MessageBox("Error: value must be a number", MessageBox.Ok)
            msglib.SetRealVar( self.varname+'.REQ', value)

        if self.type == 'int':
            try:
                value = int(txt)
            except ValueError, e:
                MessageBox("Error: value must be an interger number", MessageBox.Ok)
            msglib.SetIntVar( self.varname+'.REQ', value)

        if self.type == 'char':
            try:
                value = str(txt)
            except ValueError, e:
                MessageBox("Error: value must be a string", MessageBox.Ok)
            msglib.SetVar( self.varname+'.REQ', msglib.CHAR_VARIABLE, len(value), value)

    def setEnabled(self, enable):
        self.valueEdit.setEnabled(enable)
        if hasattr(self, 'valueSet'):
            self.valueSet.setEnabled(enable)
        self._enabled = enable

    def setValid( self, valid):
        self.nameLabel.setEnabled(valid)
        self.valueLabel.setEnabled(valid)
        self._valid = valid

    def setValidAndEnabled( self, valid, enable):
        self.setValid(valid)
        self.setEnabled(enable)
        

