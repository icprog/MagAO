#!/usr/bin/env python

# Script to change the ccd47 binning. It requires some care because the diagnostic format must be changed
# at runtime

import sys
from AdOpt import cfg, setupDevices, WfsArbScripts, thAOApp

app = thAOApp('chngbin47')

showError = False
if '-showError' in sys.argv:
    showError = True

if len(sys.argv)<2:
    print "Please specify the new binning"
    sys.exit(-1)
else:
    binning = int(sys.argv[1])
    print "Setting binning: %d" % binning

try:
    app.mute()
    setupDevices.loadDevices(app)
    app.unmute()

    WfsArbScripts.setTVbinning( app, binning)
except Exception, e:
    print 'Error:',e
    if showError:
        import time
        time.sleep(100)


