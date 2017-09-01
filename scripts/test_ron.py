
# Start this file with execfile('test_ron.py')

###########################
# CCD39 readout noise measurement
###########################

import calib, os

#############################
# Configuration starts

# Output directory

basedir = calib.getTodayMeasDir('RON')

# Acquisition name

name = 'test'

to_test = {}

# Readout speeds to test (in kpixel/sec), with associated reps (to set the frame rate)
#      speed    reps
to_test[2500] = 4050

# Target light level for dark (background level when no light is present)

dark_level = 1000

# Target light levels on CCD for each measurement

light_levels = [11000, 9000, 7000, 5000, 3000, 2000, 1500, 1200]

# No. of frames to take for each measurement

frames_num = 100

# modulation during measurements

# TT frequency in Hz

tt_freq = 200

# TT amplitude in volts

tt_amp   = 0

# CCD regions to look at (for luminosity level), can be any number of rectangular regions.

regions = []
#regions.append({'x1': 15, 'y1':15, 'x2':25, 'y2':25})
#regions.append({'x1': 55, 'y1':15, 'x2':65, 'y2':25})
#regions.append({'x1': 15, 'y1':55, 'x2':25, 'y2':65})
#regions.append({'x1': 55, 'y1':55, 'x2':65, 'y2':65})
regions.append({'x1': 35, 'y1':40, 'x2':50, 'y2':45})

# Configuration ends
###########################

import fits_lib
import os, time

def setLightLevel(level):
    '''
    Sets the lamp intensity so that the mean illumination level on the selected regions
    is as close as possible to the requested level.
    '''

    i = l.getIntensity()
    rebounds=0
    direction = 1
    while 1:
        cur_level = calcRegionIntensity( ccd39.get_frame())
        print 'Current level: %d  - target: %d' % (cur_level, level)
        if (cur_level < level) and (direction == -1):
            direction = 1
            rebounds+=1
        elif (cur_level > level) and (direction == 1):
            direction = -1
            rebounds+=1

        # Dopo due rimbalzi ci siamo di sicuro
        if rebounds>=2:
            break
        
        i = i + direction
        l.setIntensity(i, waitTimeout=10) 

        # Wait a bit to allow the lamp to stabilize
        time.sleep(0.2)


def calcRegionIntensity(frame):

    intensity = 0
    for region in regions:
        f = frame[ region['x1']:region['x2'], region['y1']:region['y2'] ]
        intensity += f.mean()

    return intensity/ len(regions)
        
        

ttmirror.set( amp = tt_amp, freq = tt_freq, offx=0, offy=0)

for speed in to_test.keys():

    ccd39.reprogram(speed = speed)
    ccd39.set_rep(to_test[speed])

    l.setIntensity(0, waitTimeout=60)
    ccd39.equalize_quadrants( target = dark_level)

    dir = os.path.join(basedir,'%d' % speed)
    if not os.path.exists(dir):
        os.makedirs(dir)

    for level in range(len(light_levels)):
        setLightLevel(light_levels[level])

        frames = ccd39.get_frames(frames_num)
        filename = os.path.join(dir, '%s_%d.fits' % (name, level))
        fits_lib.writeSimpleFits( filename, frames)

    l.setIntensity(0, waitTimeout = 60)
    dark_filename = os.path.join(dir, '%s_%d.fits' % (name, len(light_levels)))
    dark_frames = ccd39.get_frames( frames_num)
    fits_lib.writeSimpleFits( dark_filename, dark_frames)


