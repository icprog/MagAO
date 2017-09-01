#!/usr/bin/env python
#@File: runGMeas.py
#
# Script to run the g_meas.py module.

###########################
# Configuration starts
# Each of these parameters can be overriden by a commandline switch with the same name

# Ccd to use ('ccd39', or 'ccd47')
ccd = 'ccd39'

# no. of frames to take at each illumination level
nFrames = 1000

# Min and max illumination thresholds
illumination_min = 7000
illumination_max = 10000

# Lamp settings
lamp_min = 10
lamp_max = 100
lamp_step = 10

# Show plot at the end
display = True


# Configuration ends
#######################à

from AdOpt import thAOApp, setupDevices, g_meas
import getopt, sys

# Load command line parameters
try:
    opts, args = getopt.getopt( sys.argv[1:], "",  
     ["display", "nodisplay", "ccd=", "nFrames=", "illumination_min=", "illumination_max=",
      "lamp_min=", "lamp_max=", "lamp_step="])
except getopt.GetoptError, err:
    print str(err)
    sys.exit(2)

for o, a in opts:
    if o == "--display":
        display = True
    elif o == "--nodisplay":
        display = False
    elif o == "--ccd":
        ccd = a
    elif o == "nFrames":
        nFrames = int(a)
    elif o == "illumination_min":
        illumination_min = int(a)
    elif o == "illumination_max":
        illumination_max = int(a)
    elif o == "lamp_min":
        lamp_min = int(a)
    elif o == "lamp_max":
        lamp_max = int(a)
    elif o == "lamp_step":
        lamp_step = int(a)

app = thAOApp('gmeas')

setupDevices.loadDevices( app, [ccd, 'lamp'], check=True)

g_meas.doAll( app, app.__dict__[ccd], app.lamp, nFrames, illumination_min, illumination_max, lamp_min, lamp_max, lamp_step, display)

