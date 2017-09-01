#!/usr/bin/env python2

# Automatic closed loop run

import sys, ll, time
from BCUfunctions import *

rec = str(sys.argv[1])

time_acq = 20

ll.ConnectToMsgD("PhaseScreen", "127.0.0.1")


ll.WFS_SetRunState(1,0)
ll.RTR_SetLoopState(1, 0)
ll.RTR_SetREC(1, rec)
ll.WFS_SetRunState(1,1)
ll.RTR_SetLoopState(1, 1)
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


saveFrame( frames, "phasescreen/close-frames")
saveFrame( slopes, "phasescreen/close-slopes")
saveFrame( commands, "phasescreen/close-commands")

ll.WFS_SetRunState(1, 1)
ll.RTR_SetLoopState(1,1)

