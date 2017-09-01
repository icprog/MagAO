#!/usr/bin/env python

# Source acquisition procedure
#
# API:
#
#  calibMovement()   -    calibration procedure.
#                         The calibration will register the stage movements wrt. the ccd47 acquisition field.
#                         To work, the tv must be on with light from either a reference source or a star. Values
#                          will be written in a calibration file, and read back from the main source acq procedure.
#
#  calibReference()  -    calibration procedure.
#                         Registers the current position as the reference position to center a star on the pyramid.
#                         Call this function when a reference source or a star is correctly positioned on the pyramid
#                         and visible as well on the ccd47. Following acquire() calls will bring the source back to this position.
#
#  acquire()  -   acquire source.
#                 Move the stages so that the most luminous source in the field is centered on the pyramid.
#
#
# Can be run from command line too
#
# source_acq.py [-calibMovement] [-calibReference] [-acquire]

import time
import numarray
from AdOpt import cfg, setupDevices, frames_lib

def calibMovement( app, verbose=True, amount = 0.5):
    '''
    The calibration will register the stage movements wrt. the ccd47 acquisition field.
    To work, the tv must be on with light from either a reference source or a star. Values
    will be written in a calibration file, and read back from the main source acq procedure.

    <amount> is the calibration movement amplitude, in mm. Defaults to 0.5 mm.
    '''
    
    # Load required modules

    setupDevices.loadDevices( app, ['stagex', 'stagey', 'ccd47', 'fw1'])

    moveTimeout = 10
    frame = app.ccd47.get_frame()
    x0,y0 = frames_lib.findStar(frame)

    app.log('source_acq.calibMovement(): star found at X=%5.2f,Y=%5.2f' % (x0, 1024-y0))

    app.stagex.moveBy(amount, moveTimeout)
    time.sleep(5)    ## ccd47 delay
    x1,y1 = frames_lib.findStar( app.ccd47.get_frame())

    xratio = (x1-x0)/amount

    app.log('source_acq.calibMovement(): star found at X=%5.2f,Y=%5.2f' % (x1, 1024-y1))
    app.log('source_acq.calibMovement(): X ratio is %5.2f pixel/mm' % xratio)

    app.stagey.moveBy(amount, moveTimeout)
    time.sleep(5) ## ccd47 delay
    x2,y2 = frames_lib.findStar( app.ccd47.get_frame())

    yratio = (y2-y0)/amount

    app.log('source_acq.calibMovement(): star found at X=%5.2f,Y=%5.2f' % (x2, 1024-y2))
    app.log('source_acq.calibMovement(): Y ratio is %5.2f pixel/mm' % yratio)


    saveCalibData( calib.sourceAcqCalibFile(round(app.fw1.getPos())), xratio=xratio, yratio=yratio)
    


def calibReference( app, verbose=True):
    '''
    Registers the current position as the reference position to center a star on the pyramid.
    Call this function when a reference source or a star is correctly positioned on the pyramid
    and visible as well on the ccd47. Following acquire() calls will bring the source back to this position.
    '''
    # Load required modules

    setupDevices.loadDevices(app, ['ccd47', 'fw1'])

    frame = app.ccd47.get_frame()
    x,y = frames_lib.findStar(frame)

    app.log('source_acq.calibReference(): star found at X=%5.2f,Y=%5.2f' % (x, 1024-y))

    saveCalibData( calib.sourceAcqCalibFile(round(app.fw1.getPos())), xrefpos=x, yrefpos=y)


def acquire( app, verbose=True):
    '''
    Move the stages so that the most luminous source in the field is centered on the pyramid.
    '''
    # Load required modules
    setupDevices.loadDevices(app, ['stagex','stagey','ccd47', 'fw1'])

    # maximum pos. error in mm

    maxError = 0.030

    data = loadCalibData( calib.sourceAcqCalibFile(round(app.fw1.getPos())))
    if (data['xratio'] == 0) or (data['yratio'] == 0):
        from AdOpt import AOExcept, AOConstants
        raise AdOptError( code = AOConstants.AOErrCode['DIVIDE_BY_ZERO_ERROR'], errstr = 'X/Y ratios are zero')

    app.log('source_acq.acquire(): starting source acquisition')

    moveTimeout = 10

    while 1:
        x,y = frames_lib.findStar( app.ccd47.get_frame())

        xError = (x-data['xrefpos']) / data['xratio']
        yError = (y-data['yrefpos']) / data['yratio']

        app.log('source_acq.acquire(): star found at X=%5.2f,Y=%5.2f' % (x, 1024-y))
        app.log('source_acq.acquire(): current error X=%5.2f mm, Y=%5.2f mm' % (xError, yError))

        # Done?
        if (abs(xError) < maxError) and (abs(yError) < maxError):
            if verbose:
                app.log('source_acq.acquire(): target reached.')
            return

        # Move towards correct position
        app.stagex.moveBy(-xError)
        app.stagey.moveBy(-yError)
        app.stagex.waitTargetReached( moveTimeout=10)
        app.stagey.waitTargetReached( moveTimeout=10)

        time.sleep(5) # ccd 47 latency



def saveCalibData( filename, xratio=None, yratio=None, xrefpos=None, yrefpos=None):
    '''
    Saves calibration data to disk. Reads old values, if any, and only updates the ones given as parameters.
    '''

    # Some harmless default values...
    data =  {'xratio': 1.0, 'yratio': 1.0, 'xrefpos': 0, 'yrefpos': 0} 

    # Read back old values (if any)
    import sys
    oldstderr = sys.__stderr__
    sys.__stderr__ = None
    try:
        data = loadCalibData(filename)
    except:
        pass
    sys.__stderr__ = oldstderr

    # Set new values
    if xratio != None:
        data['xratio'] = xratio
    if yratio != None:
        data['yratio'] = yratio
    if xrefpos != None:
        data['xrefpos'] = xrefpos
    if yrefpos != None:
        data['yrefpos'] = yrefpos

    f = file(filename, 'w')
    s = '''
#
# Parameters for the source acquisition procedure.
#
# THIS FILE IS AUTOMATICALLY GENERATED BY CALIBRATION PROCEDURES.
# DO NOT EDIT MANUALLY!!
#
# Please use the calibration procedures in the source_acq module to update values.

'''
    f.write(s)
    f.write('xratio    float   %5.2f\n' % data['xratio'])
    f.write('yratio    float   %5.2f\n' % data['yratio'])
    f.write('xrefpos   float   %5.2f\n' % data['xrefpos'])
    f.write('yrefpos   float   %5.2f\n' % data['yrefpos'])

    f.close()




def loadCalibData( filename):
    '''
    Loads calibration data from disk. Returns a dictionary of values.
    Throws exceptions if some data are missing.
    '''
    from AdOpt import AOConfig

    vars = AOConfig.read_config(filename)

    xratio = vars['xratio'].Value()
    yratio = vars['yratio'].Value()
    xrefpos = vars['xrefpos'].Value()
    yrefpos = vars['yrefpos'].Value()
    
    return {'xratio': xratio, 'yratio': yratio, 'xrefpos': xrefpos, 'yrefpos': yrefpos} 







def usage():
    print 'Usage:  source_acq.py [-v] [-calibMovement] [-calibReference] [-acquire]'

if __name__ == "__main__":
    import sys
    from AdOpt import *

    verbose = True
    if '-v' in sys.argv:
        del sys.argv[1]
        verbose = True
        

    if len(sys.argv)<2:
        usage()
        import time
        time.sleep(5)
        sys.exit(0)

    app = AOApp('acq')
   
    try: 
        if len(sys.argv)>1:
            if sys.argv[1] == '-calibMovement':
                calibMovement(app, verbose = verbose)
            elif sys.argv[1] == '-calibReference':
                calibReference(app, verbose = verbose)
            elif sys.argv[1] == '-acquire':
                acquire(app, verbose = verbose)
            else:
                print sys.argv
                usage()

    except Exception,e:
        print e
        pass



    


