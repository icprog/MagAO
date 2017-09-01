#!/usr/bin/env python

#############################
# ADC autozero
#
# System needs to be aligned with the PSF in focus on the ccd47.
#
# It will move one ADC wheel to find the zero dispersion position.
#
# Care must be taken to avoid saturation of the ccd47.
# 
#
# 1) Move the ADC on an angle range and register the peak value
# 2) Fit a curve to the values and find the maximum - that's the best ADC position
# 3) Move to the best ADC position

import numarray
import setupDevices
import frames_lib
import time

def getPeak( frameCube, positions):
    '''
    Finds the peak position over a series of measures.

    Parameters:
    frameCube: numarray cube of frames taken at various coordinates [X,Y,Z]
    positions: numarray or sequence of Z coordinates, one for each frame. [Z]

    Returns:
    Z coordinate for best peak - fitted to the available data.
    '''


    maxIntensity = numarray.zeros( len(positions), type = numarray.Float32 )
    for x in range(len(positions)):
        maxIntensity[x] = frameCube[:,:,x].max()

    # Here we should fit a x^2 function to the max values...

    # For now just take the maximum value.

    print positions
    print maxIntensity

    bestPos = int( numarray.where( maxIntensity == maxIntensity.max())[0])
    return positions[bestPos]


def autoZeroADC( app, adcRange = 5.0, numSamples = 5, numFrames = 10):
    '''
    Searches for adc zero dispersion position. Moving a wheel, it will take
    measures at different positions and pick the one with the higher peak.

    Parameters:

    app: current AOApp
    adcRange: range of movement of one ADC wheel, in degrees
    numSamples: number of samples to take over the movement range
    numFrames:  number of ccd frames to average for each measure.
    '''

    # Load required modules
    setupDevices.loadDevices( app, ['ccd47','adc'])

    adc1 = app.adc._motor1.getPos()
    adc2 = app.adc._motor2.getPos()

    app.log('autoCenterADC(): starting from wheel positions #1=%5.3f, #2=%5.3f' % (adc1, adc2))

    frameCube = numarray.zeros( (app.ccd47.dx(), app.ccd47.dy(), numSamples), type = numarray.Int32)
    positions = numarray.zeros( numSamples, type = numarray.Float32)


    for pos in range(numSamples):
        p = adc1 - (adcRange/2) + (adcRange/numSamples * pos)
        app.adc._motor1.moveTo( p, waitTimeout = 20)
        positions[pos] = app.adc._motor1.getPos()
        time.sleep(4)
        frameCube[:,:,pos] = frames_lib.averageFrames( app.ccd47.get_frames( numFrames, type='pixels'))
        
    bestZero = getPeak( frameCube, positions)

    app.log('autoCenterAdc(): found best zero position position: %5.3f ' % bestZero)
    app.adc._motor1.moveTo( bestZero, waitTimeout = 20)

def usage():
    print 'Usage: autoZeroADC.py'

   
if __name__ == "__main__":
    import sys
    from AdOpt import *

    if len(sys.argv) != 1:
        usage()
        sys.exit(0)

    app = AOApp('zeroADC')

    autoZeroADC(app)



 
