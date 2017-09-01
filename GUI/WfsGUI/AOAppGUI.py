#!/usr/bin/env python

from qt import *
from AdOpt.AOApp import AOApp

#@Class: AOAppGUI
#
# QT-ready version of AOApp
#
# This class derives from AOApp and adds a few methods necessary to build QT graphic applications.
#
# Short version: 
# * Any worker thread or AOApp-called slot _MUST_ use callAfter() to access any QT graphic function.
#
# Long version:
# Some methods are provided to enable the heavily multithreaded AOApp to access QT non-thread-safe
# graphic functions: while the main thread can call QT functions directly, other threads and all AOApp's slots
# (like the one called by VarNotif()) must use the callAfter() method. This will send a message to the main thread
# calling the required function as soon as possible.

class AOAppGUI( QMainWindow, AOApp):

    def __init__(self, qApplication, name='AOAppGUI', debug=True):
        QMainWindow.__init__(self)
        AOApp.__init__(self,name, debug)
	self.qApp = qApplication

    #@Method{API}: callAfter
    #
    # Method to call a generic function from the main (GUI) thread instead
    # of the current thread.
    # QT IS NOT THREAD-SAFE: any worker thread wishing to manipulate GUI objects
    # must do it through this function.
    #@

    def callAfter( self, func, args):
	self.qApp.postEvent( self, QCustomEvent( QEvent.User, (func, args)))

    #@Method: customEvent
    #
    # Handler for the custom event posted by callAfter()
    #@

    def customEvent(self, ev):
	f, args = ev.data()
	f(args)


if __name__ == "__main__":
    import sys

    qApp = QApplication( sys.argv)
    main = AOAppGUI(qApp)
    qApp.setMainWidget(main)
    main.show()
    qApp.exec_loop()

