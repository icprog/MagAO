#!/usr/bin/env python

#############################
# CCD47 autofocus.
#
# System needs to be roughly aligned with light on the ccd47 and
# close to the real focus (for example focusing the ccd39 first).
#
# Also will move the X and Y stages to center the peak on a pixel.
#
# Care must be taken to avoid saturation of the ccd47.
# 
#
# 1) Move the Z stages around the current position and register the peak value.
# 2) Fit a curve to the values and find the maximum - that's the best Z position.

# 3) Move to the best Z, and repeat with X and Y over a range of 1 pixel - to center the PSF on a pixel.


import time, numpy, os.path
from AdOpt import setupDevices, frames_lib

def getPeak( frameCube, positions):
    '''
    Finds the peak position over a series of measures.

    Parameters:
    frameCube: numarray cube of frames taken at various coordinates [X,Y,Z]
    positions: numarray or sequence of Z coordinates, one for each frame. [Z]

    Returns:
    Z coordinate for best peak - fitted to the available data.
    '''


    maxFlux = numpy.zeros( len(positions), dtype = numpy.Float32 )
    for x in range(len(positions)):
        s = frames_lib.findStar( frameCube[:,:,x], completeInfo = True)
        if len(s) == 0:
            maxFlux[x] =0
        else:
            maxFlux[x] = s[0].flux

    # Here we should fit a x^2 function to the max values...

    # For now just take the maximum value.

    print positions
    print maxFlux

    bestPos = int( numpy.where( maxFlux == maxFlux.max())[0])
    return positions[bestPos]


def autofocus47( app, zRange = 1.0, numSamples = 5, numFrames = 10, saveImages = False, findPeak = True, useCCD = None):
    '''
    Searches for ccd47 best focus. For each Z position will search the frame for stars
    and peak the one with the best flux.

    Parameters:

    app: current AOApp
    zRange:  movement range, in mm.
    numSamples: number of samples to take for each axis.
    numFrames:  number of ccd frames to average for each measure.
    saveImages: if True, will save the images obtained during calibration
    '''

    if useCCD != None:
        ccd = useCCD
    else:
        ccd = app.ccd47

    if saveImages:
        import os
        from AdOpt import calib, fits_lib, WfsArbScripts
        savedir = calib.getTodayMeasDir('autoFocus47', addTimestamp=True)

    # Load required modules
    setupDevices.loadDevices( app, ['ccd47', 'stagez'])

    zPos = app.stagez.getPos()

    app.log('autofocus47(): starting focus from position Z=%5.3f' % zPos)

    frameCube = numpy.zeros( (ccd.dy(), ccd.dx(), numSamples), dtype = numpy.int32)
    positions = numpy.zeros( numSamples, dtype = numpy.float32)


    app.stagez.enable( onaftermove = True)

    app.log('autofocus47(): scanning Z stage over %5.3f mm (%d samples)' % (zRange, numSamples))

    for pos in range(numSamples):
        p = zPos - (zRange/2) + (zRange/numSamples * pos)
        print 'commanded pos: ',p
        accuracy = zRange/(numSamples*2)
        if accuracy < 0.001:
            accuracy = 0.001
        app.stagez.moveTo( p, accuracy = accuracy, waitTimeout = 40)
        positions[pos] = app.stagez.getPos()
        print 'real pos: ',positions[pos]
        time.sleep(4)
        frameCube[:,:,pos] = frames_lib.averageFrames( ccd.get_frames( numFrames, type='pixels'), useFloat=True)
        print 'peak: ', frameCube[:,:,pos].max()

        if saveImages:
            hdr, errstr = WfsArbScripts.getSetupFits(app)
            fits_lib.writeSimpleFits( os.path.join(savedir,'zpos_%d.fits' % pos), frameCube[:,:,pos], hdr)

    app.stagez.moveTo( zPos)
       
    if findPeak: 
        bestZ = getPeak( frameCube, positions)

        app.log('autofocus47(): found best Z position: %5.3f mm ' % bestZ)
        app.stagez.moveTo( bestZ, accuracy = zRange/(numSamples*2), waitTimeout = 40)
        app.stagez.enable( onaftermove = False)

    app.log('autofocus47(): done')
    return (savedir, positions)

def autoFocusIrtc( app, zRange = 2.0, numSamples = 10, numFrames = 1, interactive = False):

    from AdOpt.hwctrl import irtc_ccd
    irtc = irtc_ccd.irtc_ccd(app)

    savedir, positions = autofocus47( app, zRange=zRange, numSamples=numSamples, numFrames=numFrames, saveImages=True, findPeak = False, useCCD = irtc)
    return (os.path.basename(savedir), positions)


def autoFocusPisces( app, zRange = 2.0, numSamples = 10, numFrames = 1, interactive = False):

    from AdOpt.hwctrl import pisces_ccd
    pisces = pisces_ccd.pisces_ccd(app)

    savedir, positions = autofocus47( app, zRange=zRange, numSamples=numSamples, numFrames=numFrames, saveImages=True, findPeak = False, useCCD = pisces)
    return (os.path.basename(savedir), positions)


def findPeakIdl( app, tracknum, positions, cx, cy, interactive = False):

    from AdOpt.wrappers import idl
    from AdOpt import InterpolatedArray

    n_images = len(positions)

    cmd = "psf_focus, '%s', %d, peak, cx=%d, cy=%d, MASK=30" % (tracknum, n_images, cx, cy)
    print cmd
    idl.ex(cmd)

    peak = idl.var.peak
    pos = map( lambda x: (x, positions[x]) , range(n_images))
    z = InterpolatedArray(pos)
    zPos = z[peak]

    if interactive:
        import qt, sys
        qapp = qt.QApplication(sys.argv)
        str = 'Best pos found at z=%5.3f  (plot position: %5.3f)\nAccept?' % (zPos, peak)
        ret = qt.QMessageBox.question(qapp.mainWidget(), 'Accept position?', str, qt.QMessageBox.Yes, qt.QMessageBox.No)
        if ret == qt.QMessageBox.Yes:
            app.stagez.moveTo(zPos)
    else:
        app.stagez.moveTo(zPos)

    

  
def run(): 
    from AdOpt import thAOApp

    app = thAOApp('focus47')

    zRange = 2.0
    numSamples = 10
    numFrames = 1
    cx = 230
    cy = 120
    tracknum, positions = autoFocusIrtc( app, zRange = zRange, numSamples = numSamples, numFrames = numFrames)
    findPeakIdl( app, tracknum, positions, cx, cy, interactive=True)



 


