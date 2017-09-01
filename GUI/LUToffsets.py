#!/usr/bin/env python
#
#+File: LUToffsets.py
#
# GUI tool to quickly modify the pixel LUT offsets
#
#-

from qt import *
import BCUfunctions
import cfg

# Import the C library wrapper

import msglib

#+Class: LUToffsets.py
#
# Main window for the GUI tool.
#-

class LUToffsets( QDialog):
	
	def __init__(self, AOwindow):
		QDialog.__init__(self)

		self.name = "LUT offsets"

		# Default ccd dimension

		self.ccd_width_binned = 80

		# File to memorize offsets

		self.buildOffsetFilename()

		## Draw the widgets

		self.gridLayout = QGridLayout( self, 4, 4, 20)
		self.gridLayout.setAutoAdd(1)

		self.EditX = {}
		self.EditY = {}

		self.label1 = QLabel("Quadrants:", self)
		self.label2 = QLabel("B", self)
		self.label3 = QLabel("C", self)
		self.label4 = QLabel("D", self)

		self.label5 = QLabel("Offset X:", self)
		self.EditX['b'] = QLineEdit("0", self)
		self.EditX['c'] = QLineEdit("0", self)
		self.EditX['d'] = QLineEdit("0", self)

		self.label6 = QLabel("Offset Y:", self)
		self.EditY['b'] = QLineEdit("0", self)
		self.EditY['c'] = QLineEdit("0", self)
		self.EditY['d'] = QLineEdit("0", self)

		self.setbutton = QPushButton("Set", self)
		self.connect(self.setbutton, SIGNAL("clicked()"), self.slotSendOffsets)
		self.dummy1  = QLabel("", self)
		self.dummy2  = QLabel("", self)
		self.dummy3  = QLabel("", self)

		## Read previous offset files

		(offsetx, offsety) = BCUfunctions.readOffsetsFile( self.offset_filename)
		for q in ['b','c','d']:
			self.EditX[q].setText(str(offsetx[q]))
			self.EditY[q].setText(str(offsety[q]))

		# We are interested in CCD dim changing...

		QObject.connect( AOwindow, PYSIGNAL("%d" % cfg.msg_varreply), self.slotNotify)

		msglib.VarNotif( "CCD01XBIN", 1)
		msglib.VarNotif( "CCD01YBIN", 1)
		msglib.VarNotif( "CCD01DX", 1)
		msglib.VarNotif( "CCD01DY", 1)

		self.ccd_dx = -1
		self.ccd_dy = -1
		self.ccd_xbin = -1
		self.ccd_ybin = -1

	#++Method: slotNotify
	#
	# Receives variable notifications for CCD dimensions
	#-
	def slotNotify(self, args):	
		(message) = args
		code = message['code']
		seqnum = message['seqnum']

		if code == cfg.msg_varreply:
			varname = message['name']
			type = message['type']
			owner = message['owner'] or 'global'
			value = message['value']

			if varname == "CCD01DX":
				self.ccd_dx = int(value)
				self.computeBinning()

			if varname == "CCD01DY":
				self.ccd_dy = int(value)
				self.computeBinning()

			if varname == "CCD01XBIN":
				self.ccd_xbin = int(value)
				self.computeBinning()

			if varname == "CCD01YBIN":
				self.ccd_ybin = int(value)
				self.computeBinning()


	#++Method: computeBinning
	#
	# Recalc binning based on the ccd dimension and binning infos
	# Currently it only uses the X bin (assumes a square CCD)

	def computeBinning(self):
		if (self.ccd_dx>0) and (self.ccd_dy>0) and (self.ccd_xbin>0) and (self.ccd_ybin>0):
			self.ccd_width_binned = self.ccd_dx / self.ccd_xbin
			self.buildOffsetFilename()

	#++Method buildOffsetFilename
	#
	# Build the correct filename where to store the current offset values
	#-
	
	def buildOffsetFilename(self):
		self.offset_filename = "offsets-"+str(self.ccd_width_binned)+".txt"

	#++Method: slotSendOffsets
	#
	# Writes a new BCU lookup table and loads it
	#-

	def slotSendOffsets(self):
		offsetx = {}
		offsety = {}

		for q in ['b','c','d']:
			offsetx[q] = int( self.EditX[q].text().latin1())
			offsety[q] = int( self.EditY[q].text().latin1())

#		BCUfunctions.movePupils( 'bin'+str(self.ccd_xbin),'binning', offsetx, offsety, self.ccd_width_binned)

		nomefile = BCUfunctions.createBCUTables('bin'+str(self.ccd_xbin),'binning', offsetx, offsety, self.ccd_width_binned)
		msglib.SetVar( cfg.pixellut_variable, msglib.CHAR_VARIABLE(), len(nomefile), nomefile)
		
		# Save offsets in file

		BCUfunctions.writeOffsetsFile( self.offset_filename, offsetx, offsety)


		
