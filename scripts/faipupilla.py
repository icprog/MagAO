#!/usr/bin/env python2

# makes new pupils

import os,sys
from math import *
from BCUfunctions import *

if len(sys.argv) != 6:
	print "Usage: faipupilla.py <binning> <quadrant_width> <centerx> <centery> <radius>"
	sys.exit(0)

# Get parameters

binning = int(sys.argv[1])
width = int(sys.argv[2])
cx = float(sys.argv[3])
cy = float(sys.argv[4])
diameter = float(sys.argv[5])

# We'll build a pupil mask for 1 quadrant of the CCD
# Pixel selected with the pupils will be set to 1

# Build an empty map
n_pixels = width * width
quadrant = [0] * n_pixels

# Loop on each pixel
for x in range(width):
	for y in range(width):
		radius = (x-cx)*(x-cx) + (y-cy)*(y-cy)
		radius = sqrt(radius)

		# Skip pupils outside the chosen diameter
		if (radius > diameter):
			continue

		# Select pupils inside
		quadrant[ y*width+x] = 1

# build a filename where to store results
outfile = file("../GUI/pupilsfile"+str(binning), "w")

# Save results
for n in range(len(quadrant)):
	if quadrant[n] == 1:
		outfile.write(str(n)+"\n")

outfile.close()




