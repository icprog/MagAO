##
## Test TestDialog

from qt import *

class TestDialog( QDialog):
	
	def __init__(self, num, AOwindow):
		QDialog.__init__(self)

		self.name = 'finestra numero '+str(num)

		## Draw the widgets

		self.gridLayout = QGridLayout( self, 2, 3, 6)

		if num == 1:
			self.label1= QLabel("example1", self)
			self.gridLayout.addWidget( self.label1, 0, 0)

			self.label2= QLabel("example2", self)
			self.gridLayout.addWidget( self.label2, 1, 0)

			self.label3= QPushButton("example3", self)
			self.gridLayout.addWidget( self.label3, 0, 1)

			self.connect( self.label3, SIGNAL("clicked()"), self.slotButton3)

			self.connect( AOwindow, PYSIGNAL("ECHO"), self.slotEcho)

		if num == 2:
			self.label4= QLabel("example4", self)
			self.gridLayout.addWidget( self.label4, 1, 1)

			self.label5= QPushButton("example5", self)
			self.gridLayout.addWidget( self.label5, 0, 2)

			self.connect( self.label5, SIGNAL("clicked()"), self.slotButton5)

			## Prepare the QTimer object

			self.timer = QTimer(self)
			self.connect( self.timer, SIGNAL("timeout()"), self.slotTimeout)
			self.timer.start(10000)


	## +Method: slotTimeout
	##
	## Handler for the timer object. (if some periodic operation is needed)

	def slotTimeout(self):
		print "Il timer funziona"
		pass


	def slotButton3(self):
		print "Pressed button3"

	def slotButton5(self):
		print "Pressed button5"

	def slotEcho(self):
		print "Signal ECHO received from GUI application #1"
	
