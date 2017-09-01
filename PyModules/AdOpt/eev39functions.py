#!/usr/bin/env python2
#
#+File: eev39functions.py
#
# Python utility functions for the EEV39 ccd
#-

import math

#+Function: generateEEV39table
#
# Generates the EEV39 pixel reorder table
#
# This function can generate a reorder table for any
# EEV39-style CCD. The CCD must be SQUARE, with an
# even number of pixel in each dimension
#
#
# input stream
#
# 0  1  2  3     4  5  6  7
# 8  9  10 11    12 13 14 15
# ...
# ...
#
# ...
# ...
# 16 17 18 19    20 21 22 23
# 24 25 26 27    28 29 30 31
#
# output stream
#
# 0  4  8  12    13 9  5  1
# 16 20 24 28    29 25 21 17
# ...
# ...
#
# ...
# ...
# 19 23 27 31    30 26 22 18
# 3  7  11 15    14 10 6  2
#
# f(8) = 2
#
#
#-

def generateEEV39table( ccd_dx):

	print "generating table for DX = %d " % ccd_dx
	table = [0] * ( ccd_dx*ccd_dx)
	
	quadrant = ccd_dx/2
	ccd_dy = ccd_dx

	counter=0
	for pixel in range(ccd_dx*ccd_dx/4):

		dest = (counter % quadrant) + (counter/quadrant)*ccd_dx
		table[ counter*4 +0] = dest

		dest = ccd_dx-1 - (counter % quadrant) + (counter/quadrant)*ccd_dx
		table[ counter*4 +1] = dest

#		dest = ccd_dx-1 - (counter % quadrant) + (ccd_dy-1 - (counter/quadrant))*ccd_dx
#		table[ counter*4 +2] = dest

#		dest = (counter % quadrant) + (ccd_dy-1 - (counter/quadrant))*ccd_dx
#		table[ counter*4 +3] = dest 

		dest = ccd_dx-1 - (counter % quadrant) + (ccd_dy-1 - (counter/quadrant))*ccd_dx
		table[ counter*4 +3] = dest

		dest = (counter % quadrant) + (ccd_dy-1 - (counter/quadrant))*ccd_dx
		table[ counter*4 +2] = dest 

		counter = counter+1
		
	return table
	
	
#+Function: EEV39ToFrame
#
# Converts a frame from EEV39 format to standard raster format
#-

def EEV39ToFrame(frame):
	
	numpixels = len(frame)
	dx = int(math.sqrt(numpixels))
	
	table = generateEEV39table(dx)

	outframe = [0] * numpixels
	for pixel in range(numpixels):
		outframe[ table[pixel]] = frame[pixel]

	return outframe

#+Function: FrameToEEV39
#
# Converts a frame from standard raster format to EEV39 format
#-
	
def FrameToEEV39(frame):

	numpixels = len(frame)
	dx = int(math.sqrt(numpixels))
	
	table = generateEEV39table(dx)

	outframe = [0] * numpixels
	for pixel in range(numpixels):
		outframe[ pixel] = frame[ table[pixel]]

	return outframe


#+Function: printframe
#
# Debug function that prints out the a reorder table in matrix format
#-

def printframe(frame,dx):
	for x in range(dx):
		start = x*dx
		end = (x+1)*dx
		print frame[start:end]


#+Function: test
#
# Test function of generateEEV39table() function and related utilities
#-

def test():
	dx = 8

	frame=[]
	for x in range(dx*dx):
		frame.append(x)

	printframe(frame, dx)
	print ""

	#frame = FrameToEEV39(frame)
	frame = EEV39ToFrame(frame)

	printframe(frame, dx)	

#+Function: createTables
#
# Uses the generateEEV39table() function to write the most
# used tables on disk
#-

def createTables():

	dims = [80, 40, 26, 12]

	for dx in dims:
		nomefile = "EEV39table"+str(dx)+".txt"
		outfile = file(nomefile, "w")
		table = generateEEV39table(dx)
		for t in table:
			outfile.write(str(t)+"\n")	
		outfile.close()
		print "Written "+nomefile

#createTables()
#test()
