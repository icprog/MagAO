#@Class: irtc_ccd
#
# Python IRTC CCD module

from ccd import *
from AdOpt import calib, cfg
import os.path

class irtc_ccd:

    def __init__(self, app):

        # [TODO] configure address from cfg file?

        self._proxy = 'irtcproxy.'+cfg.side
        self._bufname = self._proxy+':FRAME'
        self.app = app
        self._lastDarkName = ""
        self._lastDark = None
        self._framerate = 10

        self._darkVar = self.app.ReadVar(cfg.side + ".IRTC.DARK.REQ")

        # Ask to be notified on dark frame change
        self.app.VarNotif(self._darkVar.Name(), self.__darkVarChanged)
        self.__darkVarChanged( self.app.ReadVar(self._darkVar.Name()))



    #@Method{API}: curDark
    #
    # Returns the current dark file.
    #@

    def curDark(self):     #R: current dark filename (string)
        return self._lastDarkName

    def setDark(self, filename):

        if os.path.basename(filename) != '':
            self._lastDark = fits_lib.readSimpleFits(filename)
            self._lastDarkName = filename

    def __darkVarChanged( self, var):
        self.setDark( calib.BackgDir('irtc', 1) + '/' + var.Value())


    #@Method{API}: dx
    #
    #       Returns the ccd x dimension (not binned).
    #@

    def dx(self):         #R:  ccd dimension in pixels(from 80 to 1024 depending on ccd type)
        return 320

    #@Method{API}: dy
    #
    #       Returns the ccd y dimension (not binned).
    #@

    def dy(self):         #R:  ccd dimension in pixels(from 80 to 1024 depending on ccd type)
        return 256

    def xbin(self):
        return 1

    def ybin(self):
        return 1


    #@Method{API}: set_framerate
    #
    # Sets the framerate for subsequent exposures

    def set_framerate(self, framerate):
        self._framerate = framerate


    #@Method{API}: get_frames
   

    def get_frames(self, num, type = 'pixels', subtractDark=True, average=False, useFloat=False, timeout=1000, callback = None, getHeader=False):

        if num == 1:
            return self.get_frame(type = type, subtractDark = subtractDark, timeout = timeout)

        path = self.app.SendMsg( self._proxy, 50000, '%d' % num, timeout = 180*1000)['body']
        print path

        data = fits_lib.readSimpleFits(path)
        if not getHeader:
            return data
        else:
            hdr = fits_lib.readFitsHdr(path)
            return (data, dict(hdr.items()))


    #@Method: formatCcd
    #
    # Formats a ccd name from a few variants down to the standard names.
    #@

    def formatCcd(self):
        return 'IRTC'

    #@Method{API}: get_detector_string
    
    def get_detector_string(self):
        return self.formatCcd()


    #@Method{API}: get_frame
    #
    # Gets a single frame from IRTC.
    # A dark file can be optionally subtracted.
    #@

    def get_frame(self, timeout=1000,          #@P{timeout}: timeout in milliseconds
                        type = 'pixels',       #@P{type}: can be 'pixels', or 'background'. The last case is identical to 'pixels', except that background parameters are ignored.
                        subtractDark=True):     #@P{subtractDark}: the current slopecomputer background will be be subtracted if <type> if 'pixels', otherwise it has no effects.
                                               #@R: 2D numpy object, of type uint16 (for pixels) or float32 (for slopes)

        data = self.app.BufRead( self._bufname, timeout=15000)
        f = numpy.fromstring( data['data'], dtype=numpy.int16).reshape(256,320)

        # Subtract background if requested
        if (type == 'pixels') and (subtractDark == True):
            if self._lastDarkName != '':
                f = f - self._lastDark
                f[ numpy.where(f <0)] = 0

        return f
        





