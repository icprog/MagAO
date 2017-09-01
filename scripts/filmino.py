#!/usr/bin/env python2

# Automatic closed loop run

import sys
import ll
import time

from BCUfunctions import *

ll.ConnectToMsgD("CLoop", "127.0.0.1")

print "Press Enter to start"
sys.__stdin__.readline()

startframe = getCurrentFrameNumber()

ll.SendBinaryMessage("BCUCTRL", "STARTDSP", "")

print "Press Enter to end"
sys.__stdin__.readline()

ll.SendBinaryMessage("BCUCTRL", "STOPDSP", "")

endframe = getCurrentFrameNumber()

print "Start "+str(startframe)
print "End "+str(endframe)


frames = getFrames( startframe, endframe)
nomefile = "filmino/frames"
saveFrame( frames, nomefile)

slopes = getSlopes( startframe, endframe)
nomefile = "filmino/slopes"
saveFrame( slopes, nomefile)

print frames.shape
print slopes.shape


