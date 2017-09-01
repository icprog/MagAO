#!/usr/bin/python

from qt import *

from AdOpt import cfg
from AdOpt.wrappers import msglib


class positionPreset( QWidget):

    def __init__( self, parent, name):
        
        QWidget.__init__(self, parent)

        self.setMinimumSize(QSize(320,20))
        self.setMaximumSize(QSize(320,20))

        layout2 = QHBoxLayout(self,0,0,"layout2")

        self.nameLabel = QLabel('', self)
        self.nameLabel.setMinimumSize(QSize(110,20))
        self.nameLabel.setMaximumSize(QSize(110,20))
        layout2.addWidget( self.nameLabel)

        self.curPos = QLabel('', self)
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
        layout2.addWidget( self.spacer)

        self.resize(QSize(350,40).expandedTo(self.minimumSizeHint()))
        self.clearWState(Qt.WState_Polished)

        self.selectText = '--select--'
        self._enabled = False
        self._valid   = False
        self._posValid = False

    def setup( self, name, aoGui, varDict):
        '''
        varDict: 'varname1' => 'custom pos conf file'
                 'varname2' => 'custom pos conf file'
                 ...

        Custom positions are specified in the configuration files. Names must match across all configuration files
        to be considered the same position.
        The curPos display will assume the position name when all variables satisfy the position,
        and a combobox set will tell all the variables to move in the specified position.
        '''

        self.nameLabel.setText(name)

        # Build an internal dictionary with two-way mappings
        self._varDict = {}
        for var in varDict.keys():
            self._varDict[var] = self.readCustomPos( varDict[var])

        # Fill combobox
        self.comboPosition.insertItem( self.selectText)
        if len(self._varDict.keys()) > 0:
            for key in self._varDict[ self._varDict.keys()[0]]['byName'].keys():
                self.comboPosition.insertItem(key)

        self._varValues = {}
        # Request variable notifications
        for var in self._varDict.keys():
            self._varValues[var] = 0
            aoGui.Notify( var+'.CUR', self.slotValueChanged, getNow = True);

        self.connect( self.comboPosition, SIGNAL("activated( const QString &)"), self.slotComboActivated)


    def slotValueChanged(self, message):
        '''
        Update local copy of positions and see if a preset name matches all current positions.
        '''

        self._varValues[ message['name']] = message['value']

        aVar = self._varDict.keys()[0]

        posValid = False
        posLabel = self.curPos.text().latin1()
        for posName in self._varDict[aVar]['byName'].keys():

            match=1
            for var in self._varDict.keys():
                pos = self._varDict[var]['byName'][posName]
                try:
                    mypos = self._varValues[var+'.CUR']
                except KeyError:
                    match=0
                    break

                if not (abs(pos-mypos) < self._varDict[var]['precision']):
                    match=0
                    break
 
            if match:
                posLabel = posName
                posValid = True
                break

        self._posValid = posValid                 

        self.curPos.setText(posLabel)
        self.curPos.setEnabled( self._posValid and self._valid)


    def slotComboActivated(self, posName):
        posName = posName.latin1()
        if posName == self.selectText:
            return
        for var in self._varDict.keys():
            value = self._varDict[var]['byName'][posName]
            msglib.SetRealVar( var+'.REQ', float(value))

    def setEnabled(self, enable):
        self.comboPosition.setEnabled(enable)
        self._enabled = enable

    def setValid( self, valid):
        self._valid = valid
        self.nameLabel.setEnabled(valid)
        self.comboPosition.setEnabled(valid)
        self.curPos.setEnabled( self._valid and self._posValid)

    def setValidAndEnabled( self, valid, enable):
        self.setValid(valid)
        self.setEnabled(enable)

    def readCustomPos( self, prefix):
        '''
        Read a custom position list from a motor configuration file.
        Returns two dictionaries in the form [name] -> position
        and [position] -> name
        '''

        posDict = {}
        nameDict = {}
        try:
            num = cfg.cfg[prefix]['customPositionNum'].Value()
            for n in range(num):
                name = cfg.cfg[prefix]['pos%d_name' % n].Value()
                pos = cfg.cfg[prefix]['pos%d_pos' % n].Value()
                posDict[name] = pos
                nameDict[pos] = name
        except Exception, e:
            # May happen if no custom pos. are defined
            print 'Exception! [positionPreset.py:135]: ',e
            pass

        return {'byName': posDict, 'byValue': nameDict, 'precision': cfg.cfg[prefix]['GoodWindow'].Value()}

        

