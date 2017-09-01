#!/usr/bin/env python
#
# This is a sample python GUI application for use with the MsgD-RTDB
#
 
import sys
import string
import time
from Logger import *
from qt import *
from qtnetwork import *

## First and foremost, construct the QApplication object

app = QApplication( sys.argv)

## Import the C library wrapper

import msglib


## +Class: GUIMain
##
## Main window for the Variable tool.

class GUIMain( QMainWindow):
	
	def __init__(self):
		QMainWindow.__init__(self)
		self.resize(650,100)
		self.setCaption("Base gui")

		## Draw the widgets

		self.gridLayout = QGridLayout( self, 2, 3, 6)

		self.label1= QLabel("example1", self)
		self.gridLayout.addWidget( self.label1, 0, 0)

		self.label2= QLabel("example2", self)
		self.gridLayout.addWidget( self.label2, 1, 0)

		self.label3= QLabel("example3", self)
		self.gridLayout.addWidget( self.label3, 0, 1)

		self.label4= QLabel("example4", self)
		self.gridLayout.addWidget( self.label4, 1, 1)

		self.label5= QLabel("example5", self)
		self.gridLayout.addWidget( self.label5, 0, 2)

		self.ExitButton= QPushButton("Exit", self)
		self.gridLayout.addWidget( self.ExitButton, 1, 2)
		self.connect( self.ExitButton, SIGNAL("clicked()"), self.slotExitButton)

		## Prepare the QTimer object

		self.timer = QTimer(self)
		self.connect( self.timer, SIGNAL("timeout()"), self.slotTimeout)

		## Prepare the Logger object

		self.logger = Logger()
		self.logger.SetOutputFile('base_gui.log')
		self.logger.SetActive(1)
	
		## Connect to MsgD-RTDB server using an unique name

		self.ServerFD = -1
		self.name = "STAGESTOOL"
		counter=0	
		while self.ServerFD <0:
			self.ServerFD = msglib.ConnectToServer( self.name +str(counter))
			if self.ServerFD == -5000:
				counter = counter+1
				continue
	
			if self.ServerFD < 0:
				errstring = msglib.GetErrorString( self.ServerFD)
				QMessageBox.warning( self, "Base gui Error", "Error "+str(self.ServerFD)+" connecting to MsgD-RTDB server: "+errstring +"\n\n")
				break

                self.notify = QSocketNotifier( self.ServerFD, QSocketNotifier.Read, self)
                self.notify.setEnabled(1)
                self.connect( self.notify, SIGNAL("activated(int)"), self.slotNotify)

		self.logger.LogString('Connected to MsgD-RTDB server')
	

	## +Method: slotTimeout
	##
	## Handler for the timer object. (if some periodic operation is needed)

	def slotTimeout(self):
		pass
	

	## +Method: slotNotify
	##
	## This method is the network event handler for the whole application. It's called by QSocketNotifier when
	## some data is waiting in the server-connected socked. Thus WaitMessage() is guaranteed to return some
	## useful data. Different actions are taken based on the message contents.

        def slotNotify(self, fd):
		message = msglib.WaitMessage()
		code = message['code']

		if code == self.config.msg_varreply: 
			varname = message['name']
			type = message['type']
			owner = message['owner'] or 'global'
			value = message['value']
	
		else:	
			print "Got message: "+message['code']+"  -  "+message['body'] 
			

	## +Method: slotExitButton
	##
	## Handler for the Exit button. It terminates the application immediately
 
	def slotExitButton(self):
		self.logger.LogString('Exiting')
		self.close()

	




# Main program
# Creates the top-level window and starts the event loop
	
font = QFont("times")
app.setFont( font)

main = GUIMain()
app.setMainWidget( main)
main.show()

app.exec_loop()

