#!/usr/bin/env python

import sys
import string
import time

## +Class: Logger
##
## This is a simple logger class to simplify writing of log strings to a file.
## The logging can be stopped and started at will with the SetActive() method. In the non-active state,
## calls to LogString() are ignored.


class Logger:

	def __init__(self):
		self.outfile = ''
		self.active=0
		self.fobj=0

	def SetOutputFile( self, file):
		self.outfile = file

	def GetOutputFile( self):
		return self.outfile

	def GetFileObj( self):
		return self.fobj

	def SetActive( self, active):
		if self.active ==1 and active==0:
			if self.fobj != 0:
				self.fobj.close()
				self.fobj=0

		self.active = active

	def isActive(self):
		return self.active

	def LogString( self, line):
		if not self.active:
			return

		if self.outfile == '':
			return

		if self.fobj ==0:
			self.fobj = open( self.outfile, 'a')
			
		timestamp = time.asctime(time.localtime(int(time.time())))
		line = timestamp + ' ' + string.rstrip(line) + '\n'
		self.fobj.write(line)



