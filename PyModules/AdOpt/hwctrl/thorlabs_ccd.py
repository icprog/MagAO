#!/usr/bin/env python2

import numpy

class thorlabs_ccd:

    def __init__(self, app, numThorlabs=0):

        self.bufname = 'ThorlabsProxy%02d:FRAME' % (numThorlabs)
        self.app = app

    def get_frame(self, type = None, callback = None):

        data = self.app.BufRead( self.bufname, timeout=10000)
        return numpy.fromstring( data['data'], dtype= numpy.int16).reshape((768,1024))

    def get_frames(self, num, callback = None, type = None):
        '''
        Reads 'num' consecutive frames from ccd, returns a 3D array.
        '''
        f = self.get_frame()
        if num == 1:
            return f

        s = f.shape
        frames = numpy.zeros((num, s[0], s[1]), dtype = numpy.int16)
        frames[0,:,:] = f

        for i in range(num-1):
            frames[i+1, :, :] = self.get_frame()
            if callback != None:
                callback(i)


        return frames








