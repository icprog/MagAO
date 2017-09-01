
###########################
# CCD39 readout noise measurement using only the dark frame. The value found is expressed in ADU.
#
# Assumes that the environment is already completely dark.
###########################

#############################
# Configuration starts

# Acquisition type (will go in the directory name)

type = 'RON'

# Acquisition name (will go in the file name)

name = 'dark_ron'

# Target light level for dark (background level when no light is present)

target = 1000

# g factor (conversione ADU -> e-)

g = {}
g[1] = {}
g[2] = {}
g[3] = {}
g[4] = {}
g[5] = {}
g[1][5000] = 0.75
g[1][2500] = 0.50
g[1][890]  = 0.46
g[1][400]  = 0.48
g[1][150]  = 0.50
g[2][1500] = 0.50
g[2][800]  = 0.50
g[2][380]  = 0.50
g[2][150]  = 0.50
g[3][380]  = 0.50
g[3][150]  = 0.50
g[3][800]  = 0.50
g[3][1500]  = 0.50
g[4][140]  = 0.50
g[4][335]  = 0.50
g[5][150]  = 0.50
g[5][380]  = 0.50

# Configuration ends
###########################

from AdOpt import calib, fits_lib
import os
import numpy.numarray as numarray


def measureSingle( app, num=100, doSetup=True, saveData = True, filename = None, removeQuadrant=False, binning=None, speed=None, verbose=True):

    if not filename:
        binning = app.ccd39.xbin()
        speed = app.ccd39.speed()

    skip = 2
    if binning != 1:
        skip=1

    if doSetup:
        pos1 = app.fw1.getPos()
        pos2 = app.fw2.getPos()
        app.fw2.moveTo(0.5, waitTimeout=10)
        app.fw1.moveTo(4, waitTimeout=10)
        app.ccd39.equalize_quadrants( target = target)
        

    if saveData:
        basedir = calib.getTodayMeasDir(type)
        dir = os.path.join(basedir,'bin%d_%d' % (binning, speed))
        if not os.path.exists(dir):
            os.makedirs(dir)

    app.log('ron.measure(): getting %d frames' % num)

    if filename != None:
        frames = fits_lib.readSimpleFits(filename)
        s = frames.shape
        dx = s[1]
        dy = s[2]
    else:
        frames = app.ccd39.get_frames(num, subtractDark=False)
        dx = app.ccd39.dx()/binning
        dy = app.ccd39.dy()/binning
        if saveData:
            filename = calib.getNumberedFilename( os.path.join(dir, name), 'fits')
            fits_lib.writeSimpleFits( filename, frames)
            app.log('ron.measure(): frames saved as '+filename)

    ron = numarray.zeros((dx-skip*2, dy-skip*2), type=numarray.Float32)
    for x in range(skip,dx-skip*2):
        for y in range(skip,dy-skip*2):
            ron[x,y] = frames[:,x,y].std()
    s = ron.shape
    ron_dx=s[0]
    ron_dy=s[1]
    if removeQuadrant: 
        ron[ron_dx/2:,ron_dy/2:]=0 # Quadrante
    ron_mean = ron.mean()
    if verbose:
        print ron[0:ron_dx/2,0:ron_dy/2].mean(), ron[0:ron_dx/2,ron_dy/2:].mean(), ron[ron_dx/2:,0:ron_dy/2].mean(), ron[ron_dx/2:,ron_dy/2:].mean()

    if removeQuadrant:
         ron_mean *= 4/3. # Quadrante

    app.log('ron.measure(): speed %4d kpix/sec - RON (ADU): %5.2f   -  RON (e-):  %5.2f (estimated)' % (speed, ron_mean, ron_mean * g[binning][speed]))

    if doSetup:
        app.fw1.moveTo(pos1, waitTimeout=10)
        app.fw2.moveTo(pos2, waitTimeout=10)

    return ron_mean * g[binning][speed]


def measureAll( app, num=100, doSetup=True, saveData=True):

    if doSetup:
        pos1 = app.fw1.getPos()
        pos2 = app.fw2.getPos()
        app.fw2.moveTo(0.5, waitTimeout=10)
        app.fw1.moveTo(4, waitTimeout=10)
        s = app.ccd39.speed()
        b = app.ccd39.get_black()
        f = app.ccd39.get_framerate()

    speeds = app.ccd39.speedlist()
    ron = []
    for s in speeds:
        app.ccd39.reprogram(speed=s)
        ron.append( measureSingle( app, doSetup=False, saveData=saveData))

    if doSetup:
        app.fw1.moveTo(pos1, waitTimeout=10)
        app.fw2.moveTo(pos2, waitTimeout=10)
        app.ccd39.set_speed(s)
        app.ccd39.set_black(b)
        app.ccd39.set_framerate(f)

    return ron

        
