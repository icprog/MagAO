#!/usr/bin/env python

from AdOpt.boardSetup import *
try:
    run()
except Exception, e:
    print e
    import time
    time.sleep(30000)

