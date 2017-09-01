
# Start this file with execfile('graph_ron.py')

###########################
# CCD39 readout noise measurement using only the dark frame. The value found is expressed in ADU.
#
# Assumes that the environment is already completely dark.
###########################

def rontest( g = 0.58, numframes = 100, verbose = False):

	import numarray

	if verbose:
		print 'Getting %d frames...' % numframes

	frames = ccd39.get_frames(numframes)

	ron = numarray.zeros((ccd39.dx(), ccd39.dy()), type=numarray.Float)
	for x in range(ccd39.dx()):
    		for y in range(ccd39.dy()):
        		ron[x,y] = frames[:,x,y].std()

	ron_mean = round( ron.mean(), 2)
	ron_e    = ron_mean * g

	return ron_e



g={}
g[2500]=0.58
g[890]=0.48
g[400]=0.48
g[150]=10.0 #da misurare

lst = []
import pylab
import sys
import time

while 1:
	time.sleep(0.2)
	speed  = ccd39.speed()
	ron_e  = rontest(g=g[speed], numframes=100)
	lst.append (ron_e)

	print 'Ron (e-):', lst[-1], '  g=',g[speed]

	pylab.plot( range(len(lst)), lst, 'ro-', linewidth=1.0)

