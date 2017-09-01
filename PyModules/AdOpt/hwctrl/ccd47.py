#+Class: ccd47
#
# Python ccd47 module

from ccd import *
from AdOpt import cfg, frames_lib

class ccd47(ccd):

    def __init__(self, aoapp, name = 'ccd47', timeout_s = 20, ctrl_check = True, power_check = True):
        ccd.__init__(self, name, aoapp, timeout_s, ctrl_check, power_check)

    def get_sharedbuf(self):
        '''
        Overridden to calculate shared buffer name on the fly.
        '''
        npix = (self.dx() * self.dy()) / (self.xbin() * self.ybin())
        return '%s%d' % ( cfg.ccd_framebuffer[ self.ccdnum], npix)

    def get_frames(self, num, type = 'pixels', diagInterface = None, callback = None, subtractDark = True, average=False, useFloat=False, timeout=10000, nextFrame=False):
        '''
        Overridden to specify the buffer name in the diagnostic interface
        If the <nextFrame> is set, it will wait for a frame which surely has been integrated after the calling instant.
        The waiting time can be significant (about 8 seconds) at slow readouts.
        '''

        if not self.isActive():
            raise InvalidStateException(ctrl=self)

        if not diagInterface:
            diagInterface = diagbuf.diagbuf( self.app, self.ccdnum, self.get_sharedbuf())

        if nextFrame:
            waitTime = (1.0 / self.get_framerate()) 
            networkDelay = (self.dx()/self.xbin())*(self.dy()/self.ybin())  / (1024*1024) * 1.0   ## [TODO] 1.0 is the estimated network delay for full (1024x1024) frames
            import time
            time.sleep(waitTime + networkDelay)

        return ccd.get_frames( self, num, type = type, diagInterface = diagInterface, callback = callback, subtractDark = subtractDark, average = average, useFloat = useFloat, timeout = timeout)

    def get_frame(self, diagInterface = None, timeout=10000, type = 'pixels', subtractDark = True, nextFrame=False):
        '''
        Overridden to have a longer default timeout (10000 ms).
        If the <nextFrame> is set, it will wait for a frame which surely has been integrated after the calling instant.
        The waiting time can be significant (about 8 seconds) at slow readouts.
        '''

        if not self.isActive():
            raise InvalidStateException(ctrl=self)

        if not diagInterface:
            diagInterface = diagbuf.diagbuf( self.app, self.ccdnum, self.get_sharedbuf())

        if nextFrame:
            waitTime = (1.0 / self.get_framerate()) 
            networkDelay = (self.dx()/self.xbin())*(self.dy()/self.ybin())  / (1024*1024) * 1.0   ## [TODO] 1.0 is the estimated network delay for full (1024x1024) frames
            import time
            time.sleep(waitTime + networkDelay)

        while 1:
            f =  ccd.get_frame( self, diagInterface = diagInterface, timeout = timeout, type = type, subtractDark = subtractDark)
            if f[300:400,300:400].mean() == 65535:
                print 'Rejected white frame'
                continue
            break
        return f

    def split_quadrants(self, frame):
        '''
        Overridden to split into two halves instead of four quadrants.
        '''
        return frames_lib.splitHalves(frame)

    def equalize_quadrants(self, target=1000, target_threshold=100.0, delay=7.0):
        '''
        Overridden to have a default delay before acting - ccd47 is slow.
        '''
        return ccd.equalize_quadrants( self, target, target_threshold, delay)




