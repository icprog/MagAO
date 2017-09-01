#!/usr/bin/env python
#
#+File: EngineeringGui.py
#

# Enable symbol sharing across modules (to have only one copy of msglib)

import sys, dl
sys.setdlopenflags(dl.RTLD_NOW|dl.RTLD_GLOBAL)

# import all modules

import types, os
from qt import *
from qtnetwork import *
from AdOpt import AOMsgCode, cfg
from AdOpt.wrappers import msglib

from AdOpt.QtDesigner.engineeringGui_template import EngineeringGui_template

#+Class: EngineeringGui.py
#
# Main window class for the AO gui system
#-
class EngineeringGui( EngineeringGui_template):

    #+Method: __init__
    #
    # Standard constructor
    #
    # The constructor sets the caption, builds the main widgets and connects with the MsgD-RTDB server
    #-
    def __init__(self, name = "EngGUI"):

        EngineeringGui_template.__init__(self)

        # Clear default elements set up by QT Designer
        self.windowList.clear()
        self.stack.removeWidget( self.WStackPage)
        self.windowCount=0

        self.wName.setText( 'Unit: '+cfg.subsystem_name)
        self.wSide.setText( 'Side: '+cfg.side_to_string(cfg.side).upper())


        # Connect to MsgD-RTDB server
        if not name:
            self.name = cfg.clientname
        else:
            self.name = name

        try:
            addr = os.environ['ADOPT_SERVER']
	    addrDisp = addr
            print 'Using server:',addr
        except KeyError:
            addr = '127.0.0.1'
            addrDisp = 'localhost'


	self.wServer.setText( 'Server: '+addrDisp)

        counter=0
        while counter <10:

            self.ServerFD = msglib.ConnectToServer( "%s%d" % (self.name, counter), addr )
  
            if self.ServerFD >=0:
                break

            counter = counter+1
  
        if counter == 10:
            errstring = msglib.GetErrorString( self.ServerFD)
            QMessageBox.warning( self, "Adopt GUI error", "Error "+str(self.ServerFD)+" connecting to MsgD-RTDB server: "+errstring +"\n\n")
            exit

        # Enable the notify function
        
        self.notify = QSocketNotifier( self.ServerFD, QSocketNotifier.Read, self)
        self.notify.setEnabled(1)
        self.connect( self.notify, SIGNAL("activated(int)"), self.slotNotify)

        # Watchdog timer
        self.timer = QTimer(self)
        self.timer.start(10000)


        
    # +Method: Notify
    #   
    # Ask to be notified on a var change
    #
    def Notify(self, varName, notifyHdlr = None, getNow = False):
        if notifyHdlr == None:
            notifyHdlr = self.slotNotify

        QObject.connect(self, PYSIGNAL(varName), notifyHdlr)
        msglib.VarNotif(varName, 0)

        if getNow:
            msglib.GetVar(varName)
        
        
    # +Method: WaitCtlrReady
    #
    # Ask to be notified when a certain controller is ready.
    #
    # [todo] Doesn't allow to wait for a queue of controllers: notify
    # only the last one !!!
    #
    def WaitCtlrReady(self, ctrlName, notifyHdlr):
        QObject.connect(self, PYSIGNAL(ctrlName), notifyHdlr)
        msglib.SendMessage("", AOMsgCode['WAITCLRDY'], ctrlName)

    # +Method: slotNotify
    #   
    # Handles and routes messages from the MsgD-RTDB to the various windows
    #
    # This method is the network event handler for the whole application. It's called by QSocketNotifier when
    # some data is waiting in the server-connected socked. Thus WaitMessage() is guaranteed to return some
    # useful data.
    # Each message triggers a different signal that will be received by the interested slots in the various windows.
    def slotNotify(self, fd):
        message = msglib.WaitMessage()
        #print "Got message ", message['code']

        # Emit the signal corresponding to the given code
        if type(message['code']) != types.IntType:
            #print 'Wrong message code'
            #print message['code']
            return

        self.emit(PYSIGNAL("%d" % message['code']), (message,) )
        
        # Notify for a client ready
        if message['code'] == AOMsgCode['CLNREADY']:
            print "Signaling %s ready" % message['sender']
            self.emit(PYSIGNAL(message['sender']), (message,) )
   
        # Notify a var changed
        if message['code'] == AOMsgCode['VARREPLY'] or message['code'] == AOMsgCode['VARCHANGD']:
            self.emit(PYSIGNAL(message['name']), (message,) )
            
        # Terminate message
        if message['code'] == AOMsgCode['TERMINATE']:
            self.close()
            
            
    

    #+Method: AddWindow
    #
    # Adds an interface window to the list
    #
    # Interfaces are identified with their name: each class must have a member variable called "name", that will
    # be used as the interface name. If this variable does not exist, the interface will be reported as "unknown"
    #
    # Interfaces are displayed in the same order as they were added.
    def AddWindow( self, window, id = None, addToList = True):
        if type( window.name) is str:
            n = window.name
        else:
            n = "unknown"

        if addToList:

            self.windowList.insertItem(n)
            scrollview = QScrollView( self)
            scrollview.addChild(window)
#            scrollview.setResizePolicy( QScrollView.AutoOne)
            self.stack.addWidget( scrollview, self.windowCount)
            self.windowCount += 1


    def windowList_selectionChanged(self):
        id = self.windowList.currentItem()
        if id >= 0:
            self.stack.raiseWidget(id)

    def exitButton_clicked(self):
        sys.exit(0)
 
