#!/usr/bin/env python
#
#@File: diagbuf.py
#
# Provides access to realtime diagnostic data in the AdOpt system
#
# API from Python:
#
# obj = diagbuf( app, ccd)  - initializes an interace to a shared memory buffer, among the known types. Returns an interface object.
#
# obj.pixelsRaster( refresh = True)   - returns an array with pixels in raster order - refreshes data from shm by default
# obj.pixelsRaw( refresh = True)      - returns an array with pixels in raw order - refreshes data from shm by default
# obj.slopesRaster( refresh = True)   - returns an array with slopes in raster order (if available) - refreshes data from shm by default
# obj.slopesRaw( refresh = True)      - returns an array with pixels in raw order    (if available) - refreshes data from shm by default
#
# Usage:
#
# optloop = diagbuf.diagbuf( app, 39, 'left')    // Init. interface
# pixels = optloop.pixelsRaster()                // Read pixels
#
#@

from AdOpt.wrappers import diagbuf_wrapper
from AdOpt import cfg, AOExcept
import time

class diagbuf:

    def __init__(self, app, ccd, bufname = None):

        self._app = app
        if bufname:
            self._bufname = bufname
        else:
            self._bufname = cfg.ccd_framebuffer[ccd]
        self._lastBuf = None
        self._intf = diagbuf_wrapper.Diagbuf( ccd)

    def __del__(self):
        diagbuf_wrapper.release( self._intf)

    def release(self):
        diagbuf_wrapper.release( self._intf)

    def readBuf( self, refresh, timeout = 1000):
        acceptableErrors = 2
        if refresh:
            done=False
            while not done:
                try:
                    self._lastBuf = self._app.BufRead( self._bufname, timeout = timeout)
                    done = True
                except AOExcept.AdOptError, e:
                    acceptableErrors -= 1
                    if acceptableErrors <=0:
                        raise
                    time.sleep(0.1)

    def pixelsRaster( self, refresh = True, timeout = 1000):
        self.readBuf(refresh, timeout = timeout)
        return diagbuf_wrapper.pixelsRaster( self._intf, self._lastBuf['data'])

    def pixelsRaw( self, refresh = True, timeout = 1000):
        self.readBuf(refresh, timeout = timeout)
        return diagbuf_wrapper.pixelsRaw( self._intf, self._lastBuf['data'])

    def slopesRaster( self, refresh = True, timeout = 1000):
        self.readBuf(refresh, timeout = timeout)
        return diagbuf_wrapper.slopesRaster( self._intf, self._lastBuf['data'])

    def slopesRaw( self, refresh = True, timeout = 1000):
        self.readBuf(refresh, timeout = timeout)
        return diagbuf_wrapper.slopesRaw( self._intf, self._lastBuf['data'])


