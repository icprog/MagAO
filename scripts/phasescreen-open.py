#!/usr/bin/env python2

# Automatic closed loop run

import sys, ll, time
from BCUfunctions import *

time_acq = 20

ll.ConnectToMsgD("PhaseScreen", "127.0.0.1")


ll.WFS_SetRunState(1,1)
ll.RTR_SetLoopState(1, 0)
flatOKO()
startframe = getCurrentFrameNumber()

time.sleep(time_acq)

# Stop DSP to avoid overwriting our frames

ll.WFS_SetRunState(1, 0)
ll.RTR_SetLoopState(1,0)

endframe = getCurrentFrameNumber()

print "Getting commands"
commands = getCommands( startframe, endframe)
print "Getting frames"
frames = getFrames( startframe, endframe)
print "Getting slopes"
slopes = getSlopes( startframe, endframe)


saveFrame( frames, "phasescreen/open-frames")
saveFrame( slopes, "phasescreen/open-slopes")
saveFrame( commands, "phasescreen/open-commands")

ll.WFS_SetRunState(1, 1)
ll.RTR_SetLoopState(1,1)

