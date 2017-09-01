#!/usr/bin/env python
#
#+File: AOprocess.py
#
# AO process control. Starts, stops and restarts AO-related processes.
#

from qt import *
import sys
import os
from AdOpt import processControl, cfg, AOVar, AOExcept
from AdOpt.WfsGUI import cfg_gui
from AdOpt.WfsGUI.thAOAppGUI import thAOAppGUI
from AdOpt.widgets.LogViewer import LogViewer
from AdOpt.wrappers import thrdlib


class process_class:
    pass

#+Class: AOprocess.py
#
# Main window class for the AO gui system
#-

class AOprocess( thAOAppGUI):

    #+Method: __init__
    #
    # Standard constructor
    #
    # The constructor sets the caption, builds the main widgets and connects with the MsgD-RTDB server
    #-
    
    def __init__(self, qApp, multi=False, rowSize = 20, caption = None, showStatus=True):
        thAOAppGUI.__init__( self, qApp, os.path.basename(sys.argv[0]),debug=False,multi=multi)

        cfg_gui.init()

        self.showStatus = showStatus

        if not caption:
            caption = "Adopt process control"

        self.setCaption(caption)
        self._rowSize = rowSize

        a = 1

        if self.showStatus:
            self.gridLayout = QGridLayout( self, 2, 4, 1)
            self.w = 4
        else:
            self.gridLayout = QGridLayout( self, 2, 2, 1)
            self.w = 2


        self.processList = {}
        self.var2process = {}

        self.StartButtonGroup = QButtonGroup(self)
        self.LogButtonGroup = QButtonGroup(self)
        self.process_counter = 0

        # Slowly poll for variables which may be created in the future
        if self.showStatus:
            self._notFoundList = []
            self.timer = QTimer(self)
            self.connect( self.timer, SIGNAL("timeout()"), self.slotTimer)
            self.timer.start(1000)
            self.retry_counter=0


        self.StartButtonGroup.hide()
        self.LogButtonGroup.hide()

        # Enable the button group

        self.gridLayout.setAutoAdd(1)
        self.dummy1 = QLabel("", self)

        self.upText = " Up "
        self.downText = " Down "
        self.initText = " Init "
        self.startText = "Start"
        self.stopText = "Stop"

        self.connect( self.StartButtonGroup, SIGNAL("clicked(int)"), self.slotStartButtonGroup)
        self.connect( self.LogButtonGroup, SIGNAL("clicked(int)"), self.slotLogButtonGroup)

        self.connect( self.qApp, SIGNAL("aboutToQuit()"), self.slotQuit)

    def addTitle( self, title):
       
        if self.showStatus:
            dummy = QLabel("", self)
 
        title = QLabel( title, self)

        if not self.showStatus:
            dummy = QLabel("", self)
        else:
            dummy = QLabel("", self)
            dummy = QLabel("", self)
   
    def addSeparator( self): 
        for i in range(self.w):
            dummy = QLabel("", self)
        
    #+Method: addProcess
    #
    # Adds a process to the monitored array
    
    def addProcess( self, process):

        p = processControl.getProcess(process)
        if p == None:
            print 'Warning: cannot add process %s' % process
            return

        desc, executablename, options, path = p

        startlabel = "Waiting"
        if not self.showStatus:
            startlabel = "Start"

        self.processList[process] = process_class()

        self.processList[process].counter = self.process_counter

        self.processList[process].desc = QLabel( desc, self)
        self.processList[process].desc.setMaximumHeight(self._rowSize)

        self.processList[process].startbutton = QPushButton( startlabel, self)
        self.processList[process].startbutton.setMaximumHeight(self._rowSize)

        self.StartButtonGroup.insert (self.processList[process].startbutton)

        self.process_counter += 1

        if self.showStatus:
            self.processList[process].label = QLabel("", self)
            self.processList[process].label.setAlignment( Qt.AlignHCenter | Qt.AlignVCenter) 
            self.processList[process].label.setMaximumHeight(self._rowSize)
            self.processList[process].oldValue = -1

            logger = LogViewer(self, desc)
            self.processList[process].logger = logger
            self.processList[process].logger.setFile(cfg.logfile(process))
            self.processList[process].logbutton = QPushButton("Log", self)
            self.processList[process].logbutton.setMaximumHeight(self._rowSize)
            self.LogButtonGroup.insert(  self.processList[process].logbutton)

            varname = cfg.clstat(process)
            self.processList[process].varname = varname
            self.var2process[varname] = process

            self._notFoundList.append(varname)

	    self.updateList( varname, useValue=0)


    def processByNumber(self, number):
        for k in self.processList.keys():
            if self.processList[k].counter == number:
                return k
       
    def slotStartButtonGroup(self, number):

        process = self.processByNumber(number)
        if not self.showStatus:
            processControl.startProcessByName( process, multi=1)
            return

        mytext = self.processList[process].label.text()
        if mytext == self.upText or mytext == self.initText:
            processControl.stopAndWait( self, process, timeout=2)
            self.processList[process].logger.addString("Process killed!!")
        else:
            processControl.startProcessByName( process)

    def slotLogButtonGroup( self, number):
        process = self.processByNumber(number)
        if self.processList[process].logger.isVisible():
            self.processList[process].logger.hide()
        else:
            self.processList[process].logger.show()

    def updateList( self, var, useValue= None):

        if isinstance(var, AOVar.AOVar):
        	varname = var.Name()
        else:
        	varname = var

	if useValue == None:
       		value = self.ReadVar(varname).Value()
	else:
		value = useValue

      	process = self.var2process[varname]

        if value == 2:
            self.callAfter( self.processList[process].startbutton.setText, self.stopText)
            self.callAfter( self.processList[process].label.setEraseColor, cfg_gui.colors.green)
            self.callAfter( self.processList[process].label.setText, self.upText)
        elif value == 1:
            self.callAfter( self.processList[process].startbutton.setText,self.stopText)
            self.callAfter( self.processList[process].label.setEraseColor, cfg_gui.colors.yellow)
            self.callAfter( self.processList[process].label.setText, self.initText)
        else:
            self.callAfter( self.processList[process].startbutton.setText,self.startText)
            self.callAfter( self.processList[process].label.setEraseColor, cfg_gui.colors.red)
            self.callAfter( self.processList[process].label.setText, self.downText)

    def slotTimer(self):

        # Retry variable discovery every now and then
        self.retry_counter -=1
        if self.retry_counter <0:
            self.retry_counter= 5
            _newNotFoundList = []
            for varname in self._notFoundList:
                try:
                    self.updateList(varname) 
                    self.VarNotif( varname, self.updateList)
                except AOExcept.AOVarNotInRTDB:
                    _newNotFoundList.append( varname)
                    pass
                except thrdlib.varNotFound:
                    _newNotFoundList.append( varname)
                    pass

            self._notFoundList = _newNotFoundList

    def slotQuit(self):
        ''' Called when the application exits to kill all tail processes '''

        for process in self.processList.keys():
            if hasattr(self.processList[process], 'logger'):
                self.processList[process].logger.kill()


            

