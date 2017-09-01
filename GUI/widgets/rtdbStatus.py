#!/usr/bin/python

from qt import *

from AdOpt import AOConstants
from AdOpt.WfsGUI import cfg_gui
import time


class rtdbStatus( QWidget):

    def __init__( self, parent, name):


        QWidget.__init__(self, parent)

        self.setMinimumSize(QSize(270,40))
        self.setMaximumSize(QSize(270,40))


        layout2 = QHBoxLayout(self,0,0,"layout2")

        self.stateLabel = QLabel('', self)
        self.stateLabel.setMinimumSize(QSize(200,20))
        self.stateLabel.setMaximumSize(QSize(200,20))
        self.stateLabel.setAlignment(Qt.AlignLeft)
        layout2.addWidget(self.stateLabel)

        self.statePixmap = QPushButton('', self)
        self.statePixmap.setSizePolicy(QSizePolicy(QSizePolicy.Fixed,QSizePolicy.Ignored,0,0,self.statePixmap.sizePolicy().hasHeightForWidth()))
        self.statePixmap.setMinimumSize(QSize(30,30))
        self.statePixmap.setMaximumSize(QSize(30,30))
        layout2.addWidget(self.statePixmap)

        self.resize(QSize(350,40).expandedTo(self.minimumSizeHint()))
        self.clearWState(Qt.WState_Polished)

        cfg_gui.init()

        self._lastUpdated = -1
        self._timeoutExpired = True
        self._eraseColor = cfg_gui.state2color[ AOConstants.AOStatesCode['STATE_INVALID']]
        self._currentStatus = AOConstants.AOStatesCode['STATE_INVALID']



    def setup( self, aoGui, vars, callback = None, watchdog = None):
        '''
        <vars> may be a single variable name (string) or a list of names
        '''

        import types
        if type(vars) != types.ListType: 
            vars = [vars]
        self._status = {}
        for var in vars:
            aoGui.Notify( var, self.slotValueChanged, getNow = True);
            self._status[var] = AOConstants.AOStatesCode['STATE_INVALID']

        self._callback = callback

        if watchdog != None:
            self._timeout = 10
            QObject.connect( watchdog, SIGNAL("timeout()"), self.checkTimeout)

    def slotValueChanged(self, message):

        varname = message['name']
        value = message['value']
        self._lastUpdated = message['mtime']

        self._status[varname] = value

        self._currentStatus = self.computeMultiStatus( self._status.values())
        self.checkTimeout()
        self.redisplay()

        if self._callback:
            self._callback(value)

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
        Redraw widget accordint to current parameters.
        '''

        if self._timeoutExpired:
            enableLabel = False
            eraseColor = cfg_gui.colors.darkgray
        else:
            enableLabel = True
            eraseColor = cfg_gui.state2color[ self._currentStatus]

        self.stateLabel.setEraseColor(eraseColor)
        self.stateLabel.setText( cfg_gui.state2text[ self._currentStatus])
        self.statePixmap.setPixmap( cfg_gui.state2pix[ self._currentStatus])

        self.stateLabel.setEnabled( enableLabel)

        
            

    def computeMultiStatus( self, statusList):
        '''
        Computes the status of multiple devices, intended to be seen
        as a single device. The resulting status is either the lowest
        in the list (for configuration) or the highest (for operation and errors).
        '''

        if AOConstants.AOStatesCode['STATE_HOMING'] in statusList:
            return AOConstants.AOStatesCode['STATE_HOMING']

        mx = max(statusList)
        mn = min(statusList)


        if mn < AOConstants.AOStatesCode['STATE_OFF']:
            return mn
        else:
            return mx


