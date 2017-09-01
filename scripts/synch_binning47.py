#!/usr/bin/env python

# Script to change the ccd47 binning. It requires some care because the diagnostic format must be changed
# at runtime

import sys
from AdOpt import cfg, setupDevices, WfsArbScripts, thAOApp

app = thAOApp('synchbin47')

showError = False
if '-showError' in sys.argv:
    showError = True


try:
    app.mute()
    setupDevices.loadDevices(app)
    app.unmute()

    WfsArbScripts.synchTVbinning(app)
except Exception, e:
    print 'Error:',e
    if showError:
        import time
        time.sleep(100)


