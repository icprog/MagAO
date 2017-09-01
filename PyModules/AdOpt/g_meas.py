#!/usr/bin/env python
#@File: g_meas.py
#
# Module with procedures to measure the 'g' (ADU/e- gain) of a ccd.
#
# Three functions are defined:
#
# doMeasure()  - perform a measure and save results in a calibration directory. Returns a tracking number.
# doAnayse()   - analyse a measure.
#
# doAll()         - does a measure plus analyse results.
#
#
# Function arguments:
#
# app: thAOApp object
# ccd: ccd object to use. Must support get_frames() and equalize_quadrants() methods (the second may be a NOP if not required).
# nFrames: how many frames to average for each measure. Defaults to 1000.
# lamp: lamp object. Must support setIntensity() method.
# lamp_min: minimum lamp setting to use
# lamp_max: maximum lamp setting to use
# lamp_step: lamp stepping to use.
# illumination_min: threshold for min. pixel illumination accepted (ccd counts)
# illumination_max: threshold for max. pixel illumination accepted (ccd counts)
# display: show results graphically on screen. Defaults to True. Causes the script to block until the user closes the window.
#
# Pixels that do not go over th_min in any measure are discarded. For each pixel, measurements that go over th_max are discarded.

from AdOpt import calib, fits_lib, WfsArbScripts
import os, threading
import numpy


# Filename to use for dark files
dark_filename = 'dark.fits'

# Prefix for measurement files
meas_prefix = 'measure'

def doAll(  app, ccd, lamp, nFrames = 1000, illumination_min=7000, illumination_max=12000, lamp_min=10, lamp_max=100, lamp_step=10, display=True):

    tracknum = doMeasure( app, ccd, lamp, nFrames, lamp_min, lamp_max, lamp_step)
    doAnalyse( tracknum, illumination_min, illumination_max, display)


def doMeasure( app, ccd, lamp, nFrames = 1000, lamp_min=10, lamp_max=100, lamp_step=10):

    dir = calib.getTodayMeasDir('gMeas')

    # Set lamp to zero and equalize ccd quadrants
    lamp.setIntensity(0, waitTimeout=60)
    ccd.equalize_quadrants(target=1000)

    # Wait a bit for the lamp to go completely off.
    time.sleep(5)

    # Acquire dark
    f= ccd.get_frames(nFrames, useFloat= True, average=True)
    fits_lib.writeSimpleFits( os.path.join(dir, dark_filename), f, WfsSetupScripts.getSetupFits(app))

    # Calculate how many digits we need for the file numbering
    import math
    n_meas = (lamp_max - lamp_min) / lamp_step
    digits = int(math.log10(n_meas))+1

    # Step lamp and acquire cubes. Wait a few seconds after setting lamp for the intensity to stabilize.
    for l in range(lamp_min, lamp_max, lamp_step):
        lamp.setIntensity(l, waitTimeout=60)
        time.sleep(5)
        f = ccd.get_frames( nFrames, useFloat = True, average=False)
        filename = getNumberedFilename( meas_prefix, 'fits', digits)
        fits_lib.writeSimpleFits( os.path.join(dir, filename), f, WfsSetupScripts.getSetupFits(app))
   
    # Return tracking number 
    return os.path.basename(dir)


def doAnalyse( tracknum, illumination_min, illumination_max, display):

    dir = calib.getMeasDir( 'gMeas', tracknum)

    darkfile = os.path.join(dir, dark_filename)
    dark = fits_lib.readSimpleFits(darkfile)

    files = filter( lambda x: x != dark_filename, sorted(os.listdir(dir)))

    nCubes = len(files)

    first = True

    # Read all data files and compute mean / stddev value for each pixel in each data cube.
    for f in range(nCubes):
        hdr  = fits_lib.readFitsHdr(files[f])
        cube = fits_lib.readSimpleFits(files[f])

        nFrames, dx, dy = cube.shape
        if first:
            mm = numpy.zeros(( nCubes, dx, dy))
            ss = numpy.zeros(( nCubes, dx, dy))
            first = False
        
        for n in range(nFrames):
            cube[n, :, :] -= dark

        for x in range(dx):
            for y in range(dy):
                level = cube[:, x, y].mean()
                if level <= illumination_max:
                    mm[f, x, y] = level
                    ss[f, x, y] = cube[:, x, y].std()
    


    # Compute linear fitting for valid pixels

    fit_ron = numpy.zeros((dx,dy))
    fit_g   = numpy.zeros((dx,dy))
    for x in range(dx):
        for y in range(dy):
            x = mm[:,x,y]
            y = ss[:,x,y]
            if x.max() >= illumination_min:
                m, c = linfit(x,y)
                fit_ron[x,y] = c
                fit_g[x,y] = m 

    fits_lib.writeSimpleFits( os.path.join( dir, 'fit_ron.fits'), fit_ron)
    fits_lib.writeSimpleFits( os.path.join( dir, 'fit_g.fits'), fit_g)

    # Compute average linear fitting

    mm_avg = numpy.zeros((nCubes))
    ss_avg = numpy.zeros((nCubes))
    for f in range(nCubes):
        mm_avg[f] = mm[f,:,:].mean()
        ss_avg[f] = ss[f,:,:].std()

    m_avg, c_avg = linfit( mm_avg, ss_avg)

    if display:
        import matplotlib.pyplot as plt
        plt.plot( mm_avg, ss_avg, 'o', label='Data', markersize=5)
        plt.plot( mm_avg, mm_avg*m_avg + c_avg, 'r', label="Fit")
        plt.xlabel('Counts')
        plt.ylabel('Std dev')
        plt.title('Acq %s  - average values: G= %5.3f e-/ADU, RON = %5.3 e-' % (tracknum, c_avg, m_avg))
        plt.legend()
        plt.savefig('plot.png')

        # Blocks here until user closes the window
        plt.show() 


def linfit( x, y):
    A = numpy.vstack( [x, numpy.ones(len(x))]).T
    return numpy.linalg.lstsq(A, y)[0]






    

    
