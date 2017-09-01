#!/usr/bin/env python
#
#+File: rtr_newconfig_gui
#
# GUI tool to create a new RTR configuration
#
# This window permits an user to select the files to build a new RTR configuration
#-

import os
from qt import *

# Import the C library wrapper

import msglib

#+Class: rtr_gui
#
# Main window for the GUI tool.
#-

class rtr_newconfig_gui( QDialog):
	
	def __init__(self, AOwindow):
		QDialog.__init__(self)

		self.name = "New configuration"
		self.AOwindow = AOwindow;

		# Directory where we can find configurations

		self.configdir = "../config/"

		self.Position = {}
		self.status = {}

		## Draw the widgets

		self.gridLayout = QGridLayout( self, 10, 3, 20)
		self.gridLayout.setAutoAdd(1)

		self.label1 = QLabel("DSP program:", self)
		self.dspprogram = QLineEdit( self)
		self.dspprogramButton = QPushButton("Select", self)
		self.connect( self.dspprogramButton, SIGNAL("clicked()"), self.slotDspprogramButton)

		self.label2 = QLabel("Pixel lookup table:", self)
		self.pixellut = QLineEdit( self)
		self.pixellutButton = QPushButton("Select", self)
		self.connect( self.pixellutButton, SIGNAL("clicked()"), self.slotPixellutButton)

		self.label3 = QLabel("Reconstructor matrix:", self)
		self.rtr = QLineEdit( self)
		self.rtrButton = QPushButton("Select", self)
		self.connect( self.rtrButton, SIGNAL("clicked()"), self.slotRtrButton)

		self.label4 = QLabel("Time filter matrix:", self)
		self.filtermatrix = QLineEdit( self)
		self.filtermatrixButton = QPushButton("Select", self)
		self.connect( self.filtermatrixButton, SIGNAL("clicked()"), self.slotFiltermatrixButton)

		self.label5 = QLabel("Pixel background:", self)
		self.pixelbackground = QLineEdit( self)
		self.pixelbackgroundButton = QPushButton("Select", self)
		self.connect( self.pixelbackgroundButton, SIGNAL("clicked()"), self.slotPixelbackgroundButton)

		self.label6 = QLabel("Slopes null:", self)
		self.slopesnull = QLineEdit( self)
		self.slopesnullButton = QPushButton("Select", self)
		self.connect( self.slopesnullButton, SIGNAL("clicked()"), self.slotSlopesnullButton)


		self.config_create = QPushButton("Create new", self)
		self.gridLayout.addWidget( self.config_create, 9, 2)
		self.connect( self.config_create, SIGNAL("clicked()"), self.slotConfigCreate)

	## +Method: slotNotify
	##
	## This method is the network event handler for the whole application. It's called by QSocketNotifier when
	## some data is waiting in the server-connected socked. Thus WaitMessage() is guaranteed to return some
	## useful data. Different actions are taken based on the message contents.

        def slotNotify(self, args):
		(message) = args
		code = message['code']

	def slotConfigCreate(self):
		config = QFileDialog.getSaveFileName( self.configdir, "", self, "", "Choose a configuration name")

		if config != "":
			os.mkdir(config.latin1())

			configfile = file( config.latin1()+"/config", "w")

			path = self.dspprogram.text().latin1()
			configfile.write("dsp_program:"+os.path.basename(path)+"\n")
			os.system( "/bin/cp "+path+" "+config.latin1()+"/"+os.path.basename(path))

			path = self.pixellut.text().latin1()
			configfile.write("pixel_lut:"+os.path.basename(path)+"\n")
			os.system( "/bin/cp "+path+" "+config.latin1()+"/"+os.path.basename(path))

			path = self.rtr.text().latin1()
			configfile.write("rtr0:"+os.path.basename(path)+"\n")
			os.system( "/bin/cp "+path+" "+config.latin1()+"/"+os.path.basename(path))

			path = self.filtermatrix.text().latin1()
			configfile.write("time_filter:"+os.path.basename(path)+"\n")
			os.system( "/bin/cp "+path+" "+config.latin1()+"/"+os.path.basename(path))

			path = self.pixelbackground.text().latin1()
			configfile.write("background:"+os.path.basename(path)+"\n")
			os.system( "/bin/cp "+path+" "+config.latin1()+"/"+os.path.basename(path))

			path = self.slopesnull.text().latin1()
			configfile.write("slopesnull:"+os.path.basename(path)+"\n")
			os.system( "/bin/cp "+path+" "+config.latin1()+"/"+os.path.basename(path))

			configfile.close()

			self.AOwindow.emit( PYSIGNAL("RTR_NEW"), ("",))

#			msglib.SendMessage("ADOPTGUI", "RTR_NEW", "")	


	def slotPixellutButton(self):
		filename = QFileDialog.getOpenFileName( "", "", self, "", "Choose a file")
		if filename != QString.null:
			self.pixellut.setText( filename)

	def slotDspprogramButton(self):
		filename = QFileDialog.getOpenFileName( "", "", self, "", "Choose a file")
		if filename != QString.null:
			self.dspprogram.setText( filename)

	def slotRtrButton(self):
		filename = QFileDialog.getOpenFileName( "", "", self, "", "Choose a file")
		if filename != QString.null:
			self.rtr.setText( filename)

	def slotFiltermatrixButton(self):
		filename = QFileDialog.getOpenFileName( "", "", self, "", "Choose a file")
		if filename != QString.null:
			self.filtermatrix.setText( filename)

	def slotPixelbackgroundButton(self):
		filename = QFileDialog.getOpenFileName( "", "", self, "", "Choose a file")
		if filename != QString.null:
			self.pixelbackground.setText( filename)

	def slotSlopesnullButton(self):
		filename = QFileDialog.getOpenFileName( "", "", self, "", "Choose a file")
		if filename != QString.null:
			self.slopesnull.setText( filename)


