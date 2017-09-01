#!/usr/bin/env python

from AdOpt.centerPupils import *
try:
    run()
except Exception, e:
    print 'Exception:',e
    print e
    import time
    time.sleep(30000)
