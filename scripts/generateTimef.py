#!/usr/bin/env python2

# generates the Timef matrix

import os
from BCUfunctions import *


# Create the timefiltering matrix

timef = array.array('L')

for y in range(40):
	for x in range(40):
		if x == y:
			if x != 1:
				timef.append(0x3F800000)	 # 1.0 in float
			else:
				timef.append(0x00000000)	 # 0.0 in float (for the BIAS channel)
		else:
			timef.append(0x00000000)	 # 0.0 in float

timef_BCU = matrixToREC( timef.tolist(), 40, 40)

writeBCUfile("timef-identity.dat", timef_BCU)

# Move the files in the correct position

os.system("/bin/cp timef-identity.dat ../config/cloop/timef-identity.dat")


