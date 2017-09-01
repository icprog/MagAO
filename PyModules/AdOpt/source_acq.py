#!/usr/bin/env python

# Source acquisition procedure
#
# API:
#
#  calibMovement()   -    calibration procedure.
#                         The calibration will register the stage movements wrt. the ccd47 acquisition field.
#                         To work, the tv must be on with light from either a reference source or a star. Values
#                          will be written in a calibration file, and read back from the main source acq procedure.
#			  This procedure must be executed at binning 1x1.
#
#  calibReference()  -    calibration procedure.
#                         Registers the current position as the reference position to center a star on the pyramid.
#                         Call this function when a reference source or a star is correctly positioned on the pyramid
#                         and visible as well on the ccd47. Following acquire() calls will bring the source back to this position.
#			  This procedure must be executed at binning 1x1.
#
#  calibFw1()        -    calibration procedure
#                         Registers the PSF offset when selecting different FW1 positions.
#
#  acquire()  -   acquire source.
#                 Move the stages so that the most luminous source in the field is centered on the pyramid.
#
#
# Can be run from command line too
#
# source_acq.py [-calibMovement] [-calibReference] [-acquire]

import time, math
import numpy.numarray as numarray
from AdOpt import cfg, setupDevices, frames_lib, thAOApp, calib
from AdOpt import AOExcept, AOConstants

def calibMovement( app, verbose=True, amount = 0.5):
    '''
    The calibration will register the stage movements wrt. the ccd47 acquisition field.
    To work, the tv must be on with light from either a reference source or a star. Values
    will be written in a calibration file, and read back from the main source acq procedure.

    <amount> is the calibration movement amplitude, in mm. Defaults to 0.5 mm.
    '''
    
    # Load required modules

    setupDevices.loadDevices( app, ['stagex', 'stagey', 'ccd47', 'fw1'])

    xpos = app.stagex.getPos()
    ypos = app.stagey.getPos()

    moveTimeout = 10
    frame = app.ccd47.get_frame()
    x0,y0,mag = frames_lib.findStar(frame)

    app.log('source_acq.calibMovement(): star found at X=%5.2f,Y=%5.2f' % (x0, y0))

    app.log('Moving stage x by %7.3f' % amount)
    app.stagex.moveBy(amount, waitTimeout = moveTimeout)
    time.sleep(10)    ## ccd47 delay
    x1,y1,mag = frames_lib.findStar( app.ccd47.get_frame())

    xratio = (x1-x0)/amount

    app.log('source_acq.calibMovement(): star found at X=%5.2f,Y=%5.2f' % (x1, y1))
    app.log('source_acq.calibMovement(): X ratio is %5.2f pixel/mm' % xratio)

    app.stagey.moveBy(amount, waitTimeout = moveTimeout)
    time.sleep(10) ## ccd47 delay
    x2,y2,mag = frames_lib.findStar( app.ccd47.get_frame())

    yratio = (y2-y0)/amount

    app.log('source_acq.calibMovement(): star found at X=%5.2f,Y=%5.2f' % (x2, y2))
    app.log('source_acq.calibMovement(): Y ratio is %5.2f pixel/mm' % yratio)


    calib.saveCalibData( calib.sourceAcqCalibFile(round(app.fw1.getPos())), {'xratio':xratio, 'yratio':yratio} )
    
    app.stagex.moveTo(xpos, waitTimeout = moveTimeout)
    app.stagey.moveTo(ypos, waitTimeout = moveTimeout)



def calibReference( app, verbose=True):
    '''
    Registers the current position as the reference position to center a star on the pyramid.
    Call this function when a reference source or a star is correctly positioned on the pyramid
    and visible as well on the ccd47. Following acquire() calls will bring the source back to this position.
    '''
    # Load required modules

    setupDevices.loadDevices(app, ['ccd47', 'fw1'])

    frame = app.ccd47.get_frame()
    x,y,mag = frames_lib.findStar(frame)

    app.log('source_acq.calibReference(): star found at X=%5.2f,Y=%5.2f' % (x, y))

    calib.saveCalibData( calib.sourceAcqCalibFile(round(app.fw1.getPos())), {'xrefpos':x, 'yrefpos':y} )

def calibFw1( app, verbose=True):
    '''
    Registers the PSF offset when selecting different FW1 positions.
    A reference source or star must be well visibile on the ccd47.
    '''

    #@C
    # This procedure is currently not used since the reference position
    # is calibrated separately for each filter
    #@
    
    pass


#@Function: ccd47Mag
#
# Estimate magnitude from ccd47 flux.
# Current information about ccd47 and other devices is used when calculating magnitude, so 
# the device state must not be changed in between.
#@

def ccd47Mag( app, TVflux):

    setupDevices.loadDevices( app, ['ccd47', 'fw1', 'fw2'], check=True)
    acqData = calib.loadCalibData( calib.sourceAcqCalibFile(), check=['counts_mag10_ccd47'])

    filter1 = int(round(app.fw1.getPos()))
    filter2 = int(round(app.fw2.getPos()))

    # Correct for filter position
    # [todo...]

    fw1_factor = [ 1, 1, 1, 1, 1, 1]
    fw2_factor = [ 1, 10, 10, 1, 1, 1]

    TVflux *= fw1_factor[filter1] * fw2_factor[filter2]

    c = TVflux * app.ccd47.get_framerate() / (app.ccd47.xbin() * app.ccd47.ybin())
    if c <=0:
        raise AORetryException(errstr = 'Magnitude calculation cannot take place with negative counts')

    return -2.5 * math.log10( c / acqData['counts_mag10_ccd47']) + 10.5


def acquire( app, TVavg=1, TVflux=5000.0, verbose=True, getError = False, fwhm = 0.6):
    '''
    Move the stages so that the most luminous source in the field is centered on the pyramid.


    <TVavg> is the number of ccd47 frames to average for each measure.
    <TVflux> is the expected flux on ccd47 - the procedure will check if it is whithin a configurable threshold
    <getError> if True, only check the current displacement and exit without moving the stages.
    <fhwm> is the estimated PSF fwhm in arcsec 

    Returns an object with the following attributes defined:

    .deltaX   - amount of displacement from original target position, in focal plane mm
    .deltaY 
    .TVframe  - last frame used for star acquisition

    '''
    # Load required modules
    setupDevices.loadDevices(app, ['stagex','stagey','ccd47', 'fw1'])

    acqData = calib.loadCalibData( calib.sourceAcqCalibFile(), check=['maxIterations', 'magError', 'counts_mag10_ccd47', 'counts_mag10_ccd47', 'maxError'])

    calib_filename = calib.sourceAcqCalibFile(round(app.fw1.getPos()))
    data = calib.loadCalibData( calib_filename, check=['xratio', 'yratio', 'xrefpos', 'yrefpos'])

    if (data['xratio'] == 0) or (data['yratio'] == 0):
        raise AOExcept.AdOptError( code = AOConstants.AOErrCode['WFSARB_MISSING_CALIB'], errstr = 'Stage movements X/Y calib missing for fw1 pos %d' % round(app.fw1.getPos()))
    if (data['xrefpos'] == 0) or (data['yrefpos'] == 0):
        raise AOExcept.AdOptError( code = AOConstants.AOErrCode['WFSARB_MISSING_CALIB'], errstr = 'Star position reference missing for fw1 pos %d' % round(app.fw1.getPos()))

    app.log('source_acq.acquire(): starting source acquisition')

    # Return object
    class obj: pass
    ret = obj()
    ret.deltaX=0
    ret.deltaY=0
    ret.mag=0

    app.stagex.enable( True, onaftermove=True)
    app.stagey.enable( True, onaftermove=True)

    gain = 1.0
    moveTimeout=10

    for ii in range(acqData['maxIterations']):
        ret.TVframe = frames_lib.averageFrames( app.ccd47.get_frames( TVavg))
        x, y, flux = frames_lib.findStar( ret.TVframe, fwhm_px=10 * (16/app.ccd47.xbin()))

        # Check flux
        if flux <= 0:
            errstr = 'No stars found in TV frame'
            raise AOExcept.AORetryException( code = AOConstants.AOErrCode['WFSARB_STAR_NOTFOUND'], errstr = errstr)

        app.log('Star found at x=%3.1f, y=%3.1f, flux=%3.1f' % (x,y,flux))

        # Calculate equivalent magnitude
        expected_mag = ccd47Mag( app, TVflux)
        ret.mag = ccd47Mag( app, flux)

        if abs(ret.mag - expected_mag) > acqData['magError']:
            errstr = 'source_acq error: mag %5.3f - expected: %5.3f  difference: %5.3f)' % (ret.mag, expected_mag, abs(ret.mag - expected_mag))
            raise AOExcept.AORetryException( code = AOConstants.AOErrCode['WFSARB_STAR_WRONGFLUX'], errstr = errstr)

        x *= app.ccd47.xbin()  # Transform coordinates to bin 1x1
        y *= app.ccd47.ybin()

        xError = (x-data['xrefpos']) / data['xratio']
        yError = (y-data['yrefpos']) / data['yratio']

        app.log('source_acq.acquire(): star found at X=%5.2f,Y=%5.2f' % (x, y))
        app.log('source_acq.acquire(): current error X=%5.3f mm, Y=%5.3f mm' % (xError, yError))

        if getError:
            ret.deltaX -= xError
            ret.deltaY -= yError
            return ret

        # Done?
        if (abs(xError) < acqData['maxError']) and (abs(yError) < acqData['maxError']):
            if verbose:
                app.log('source_acq.acquire(): target reached.')
            return ret

        # Move towards correct position
        app.stagex.moveBy(-yError * gain)   # Invert X and Y movements
        app.stagey.moveBy(-xError * gain)
        app.stagex.waitTargetReached( moveTimeout)
        app.stagey.waitTargetReached( moveTimeout)

        ret.deltaX -= xError * gain
        ret.deltaY -= yError * gain

        time.sleep(3) # ccd 47 latency


    errstr = 'source_acq error: Star centering procedure failed to converge after %d iterations' % acqData['maxIterations']
    raise AOExcept.AORetryException( code = AOConstants.AOErrCode['WFSARB_STAR_NOTCONVERGED'], errstr = errstr)

    app.stagex.enable( False)
    app.stagey.enable( False)



def usage():
    print 'Usage:  source_acq.py [-v] [-calibMovement | -calibReference | -acquire | -check]'

def run():
    import sys

    verbose = True
    if '-v' in sys.argv:
        del sys.argv[1]
        verbose = True
        

    if len(sys.argv)<2:
        usage()
        import time
        time.sleep(5)
        sys.exit(0)

    app = thAOApp('acq')
   
    if len(sys.argv)>1:
        if sys.argv[1] == '-calibMovement':
            calibMovement(app, verbose = verbose)
        elif sys.argv[1] == '-calibReference':
            calibReference(app, verbose = verbose)
        elif sys.argv[1] == '-acquire':
            acquire(app, verbose = verbose)
        elif sys.argv[1] == '-check':
            acquire(app, verbose = verbose, checkError = True)
        else:
            print sys.argv
            usage()



if __name__ == "__main__":
    run()


    


