#!/usr/bin/env python
#
#+File: rtr_gui
#
# GUI tool for RTR parameters
#
# This window will let the user choose an available configuration for the RTR parameters
# and send it to the BCU control program.
#-

import os
from qt import *

# Import the C library wrapper

import msglib

#+Class: rtr_gui
#
# Main window for the GUI tool.
#-

class rtr_gui( QDialog):
	
	def __init__(self, AOwindow):
		QDialog.__init__(self)

		self.name = "RTR/WFS configuration"

		# Directory where we can find configurations

		self.configdir = "../config/modes/"

		self.Position = {}
		self.status = {}

		## Draw the widgets

		self.gridLayout = QGridLayout( self, 10, 2, 20)

		self.label1 = QLabel("Current configuration:", self)
		self.gridLayout.addWidget( self.label1, 0, 0)

		self.cur_config = QLabel("None", self)
		self.gridLayout.addWidget( self.cur_config, 0, 1)

		self.label2 = QLabel("Select a new configuration:", self)
		self.gridLayout.addWidget( self.label2, 1, 0)

		self.config_list = QListBox( self)
		self.gridLayout.addWidget( self.config_list, 1, 1)

		self.config_send = QPushButton("Send config", self)
		self.gridLayout.addWidget( self.config_send, 1, 2)
		self.connect( self.config_send, SIGNAL("clicked()"), self.slotConfigSend)

		# fill the configuration list

		self.fillConfigList()

		# register slots

		QObject.connect( AOwindow, PYSIGNAL("CLNREADY"), self.slotNotify)
		QObject.connect( AOwindow, PYSIGNAL("VARREPLY"), self.slotNotify)
		QObject.connect( AOwindow, PYSIGNAL("RTR_NEW"), self.fillConfigList)

		# Wait for BCU controller to be ready

		msglib.SendMessage( "", "WAITCLRDY", "BCUCTRL")

	## +Method: slotNotify
	##
	## This method is the network event handler for the whole application. It's called by QSocketNotifier when
	## some data is waiting in the server-connected socked. Thus WaitMessage() is guaranteed to return some
	## useful data. Different actions are taken based on the message contents.

        def slotNotify(self, args):
		(message) = args
		code = message['code']

		## CLNREADY
		##
		## When the Stage control client is ready, start the variable management

		if code == "CLNREADY":
			msglib.SendMessage("", "VARNOTIF", "RTRCONFIG")
			msglib.GetVar("RTRCONFIG")
			return

		## VARREPLY
		##
		## When one of the watched variables change, update our display

		if code == "VARREPLY":

			varname = message['name']
			type = message['type']
			owner = message['owner'] or 'global'
			value = message['value']

			if varname == "RTRCONFIG":
				self.cur_config.setText( value)
				return

		## RTR_NEW
		##
		## Inform us that the configurations on disk have been changed

		if code == "RTR_NEW":
			self.fillConfigList()


	#++Method: fillConfigList
	#
	# Refreshes the configuration list
	#-

	def fillConfigList(self):

		self.config_list.clear()

		configs = os.listdir(self.configdir)
		for c in configs:
			self.config_list.insertItem( c, -1)

	#++Method: slotConfigSend
	#
	# Handler for the "Send" button: loads a new configuration
	#-

	def slotConfigSend(self):
		config = self.config_list.currentText().latin1()
		self.cur_config.setText("working...")
		msglib.SendMessage("BCUCTRL", "RTRCONFIG", config)

