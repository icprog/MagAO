
# Start this file with execfile('test_dark_ron.py')

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

binning = ccd39.xbin()
speeds = list(set(ccd39.speedlist()))  # Remove duplicates


# Target light level for dark (background level when no light is present)

dark_level = 1000

# g factor (conversione ADU -> e-)

g = 0.50

# No. of frames to take for each measurement

frames_num = 100

# Configuration ends
###########################

from AdOpt import calib, fits_lib
import os, time
import numpy

result = {}

basedir = calib.getTodayMeasDir(type)

# Avoid going too fast when we set 1Khz.
sc.set_masterd_decimation(2)

for speed in speeds:

    print 'Preparing CCD for speed',speed
    ccd39.reprogram(speed = speed, xbin=binning, ybin=binning)
    ccd39.set_rep(0)
    ccd39.equalize_quadrants( target = dark_level, delay=0.1)

    # Leave things to stabilize a bit
    time.sleep(1)

    dir = os.path.join(basedir,'_%d' % speed)
    if not os.path.exists(dir):
        os.makedirs(dir)

    print 'Getting %d frames...' % frames_num
    frames = ccd39.get_frames(frames_num, subtractDark=False)
    filename = calib.getNumberedFilename( os.path.join(dir, name), 'fits')
    fits_lib.writeSimpleFits( filename, frames)
    print 'Done'

    ron = numpy.zeros((ccd39.dx()/binning, ccd39.dy()/binning), dtype=numpy.float)
    for x in range(ccd39.dx()/binning):
        for y in range(ccd39.dy()/binning):
            ron[x,y] = numpy.std(frames[:,x,y])

    ron_mean = numpy.mean(ron)
    result[speed] = ron_mean
    print 'Average RON:', round( ron_mean,2) , 'ADU  (estimated e-: ', round( ron_mean * g, 2) ,')'
    print '\nAutosaved as ', filename


print 'Resetting to max. speed'
#ccd39.set_framerate(1000)
ccd39.equalize_quadrants( target = dark_level, delay=0.1)

print
print '------------------------------ SUMMARY ------------------------------------'
print
print 'ccd39 - binning',binning
print

kk = list(speeds)
kk.sort(lambda x,y:y-x)
for speed in kk:
    print 'Speed: %4d kpix/sec - RON (ADU): %5.2f   -  RON (e-):  %5.2f (estimated)' % (speed, result[speed], result[speed] * g)

