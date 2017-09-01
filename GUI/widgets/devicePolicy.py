#!/usr/bin/python

from qt import *

from AdOpt.wrappers import msglib

class devicePolicy( QWidget):

    def __init__( self, parent, name):
        
        QWidget.__init__(self, parent)

        self.setMinimumSize(QSize(320,20))
        self.setMaximumSize(QSize(320,20))

        layout2 = QHBoxLayout(self,0,0,"layout2")

        self.nameLabel = QLabel('Policy:', self)
        self.nameLabel.setMinimumSize(QSize(110,20))
        self.nameLabel.setMaximumSize(QSize(110,20))
        layout2.addWidget( self.nameLabel)

        self.curPos = QLabel('Auto', self)
        self.curPos.setMinimumSize(QSize(70,20))
        self.curPos.setMaximumSize(QSize(70,20))
        self.curPos.setBackgroundMode(QLabel.PaletteBackground)
        self.curPos.setFrameShape(QLabel.LineEditPanel)
        layout2.addWidget( self.curPos)

        self.comboPosition = QComboBox(0,self,"comboPosition")
        self.comboPosition.setMinimumSize(QSize(80,20))
        self.comboPosition.setMaximumSize(QSize(80,20))
        layout2.addWidget(self.comboPosition)

        self.spacer = QLabel('', self)
        self.spacer.setMinimumSize(QSize(30,20))
        self.spacer.setMaximumSize(QSize(30,20))
        layout2.addWidget(self.spacer)

        self.comboPosition.insertItem('Auto')
        self.comboPosition.insertItem('Manual')

        self.resize(QSize(350,40).expandedTo(self.minimumSizeHint()))
        self.clearWState(Qt.WState_Polished)

    def setCallbacks( self, state_callback=None, cmd_callback = None):
        '''
        <state_callback> will be called when the policy state changes
        <cmd_callback> will be called with the policy combobox has been changed by the user.
        '''

        if state_callback:
            self.connect( self.curPos, SIGNAL("valueChanged()"), state_callback)

        if cmd_callback:
            self.connect( self.comboPosition, SIGNAL("activated(const QString&)"), cmd_callback)

    def get( self):
        return self.comboPosition.currentText().latin1()
        return self.curPos.text().latin1()

        

