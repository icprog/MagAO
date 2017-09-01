#!/usr/bin/env python
#
#+File: oko_gui
#
# GUI tool for the second OKO
#
#-

import sys,socket
from qt import *
from random import *
import time

# First and foremost, construct the QApplication object
                                                                                                                 
app = QApplication( sys.argv)


#+Class: oko_gui
#
# Main window for the GUI tool.
#-

class oko_gui( QDialog):
	
	def __init__(self):
		QDialog.__init__(self)

		# Configuration

		self.hostname = "localhost"
		self.port = 3456

		## Draw the widgets

		self.gridLayout = QGridLayout( self, 2,3 ,20)
		self.gridLayout.setAutoAdd(1)

		self.label1 = QLabel("Position:", self)

		self.pos_list = QListBox( self)

		self.pos_send = QPushButton("Set position", self)
		self.connect( self.pos_send, SIGNAL("clicked()"), self.slotPos_send)

		self.label2 = QLabel("Turbolence:", self)

		self.turbolence_start = QPushButton("Start", self)
		self.connect( self.turbolence_start, SIGNAL("clicked()"), self.slotTurbolence_start)

		self.turbolence_stop = QPushButton("Stop", self)
		self.connect( self.turbolence_stop, SIGNAL("clicked()"), self.slotTurbolence_stop)

		# fill the position list

		self.fillPos_list()

		# Connect to server

		self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		self.socket.connect((self.hostname, self.port))

		# Prepare the timer
		self.timer = QTimer(self)
		self.connect( self.timer, SIGNAL("timeout()"), self.slotTimeout)

		seed()

	#++Method: fillPos_list
	#
	# Refreshes the position list
	#-

	def fillPos_list(self):

		self.pos_list.clear()
	
		#configs = os.listdir(self.configdir)
		#for c in configs:
		#	self.config_list.insertItem( c, -1)

	#++Method: slotPos_send
	#
	# Handler for the "Send" button: loads a new position
	#-

	def slotPos_send(self):
		config = self.config_list.currentText().latin1()
		#msglib.SendMessage("BCUCTRL", "RTRCONFIG", config)


	def slotTurbolence_start(self):
		self.timer.start(50)
#		for n in range(37):
#			stringa = str(n+1)+" "+str(0)
#			while(len(stringa)<8):
#				stringa = stringa+" "
#			print stringa
#			self.socket.send(stringa)


	def slotTurbolence_stop(self):
		self.timer.stop()
#		for n in range(37):
#			stringa = str(n+1)+" "+str(150)
##			while(len(stringa)<8):
#				stringa = stringa+" "
#			print stringa
#			self.socket.send(stringa)
#			time.sleep(1)


	def slotTimeout(self):
		for n in range(37):
			stringa = str(n)+" "+str( randint(20,180))
			while(len(stringa)<8):
				stringa = stringa+" "
			print stringa
			self.socket.send(stringa)

# Main program
# Si limita a creare una QApplication e creare gli oggetti necessari
                                                                                                                 
font = QFont("times")
app.setFont( font)
                                                                                                                 
main = oko_gui()
app.setMainWidget( main)
main.show()
                                                                                                                 
app.exec_loop()
                                                                                                                 

