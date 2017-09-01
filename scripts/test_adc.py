
# Start this file with execfile('test_adc.py')

###########################
# ADC test measurement
###########################


#############################
# Configuration starts

# Acquisition type (will go in the directory name)

type = 'ADC'

# Acquisition name (will go in the file name)

name = 'ADC_axisrot'


# ADC positions to test (in degrees)

positions = [0, 90, 180, 270, 360]

# No. of slope frames to average for each measurement

ccd39_frames_num = 100

# No. of tv frames to average for each measurements

ccd47_frames_num = 5

# Offset to give to wheel#1 to have zero dispersion (degrees)

wheel1_offset = -8.0

# Configuration ends
###########################

import calib, os

import fits_lib
import os, time

import frames_lib
import numarray

result = {}

basedir = calib.getTodayMeasDir(type, addTimestamp = True)

for pos in positions:

    adc._motor1.moveTo(pos + wheel1_offset)
    adc._motor2.moveTo(-pos)
    adc._motor1.waitTargetReached()
    adc._motor2.waitTargetReached()


    if not os.path.exists(basedir):
        os.makedirs(basedir)

    ccd39_filename = os.path.join(basedir,'slopes_%s_%d.fits' % (name, pos))

    print 'Averaging %d slope frames...' % ccd39_frames_num
    ccd39_slopes = frames_lib.averageFrames( ccd39.get_frames(ccd39_frames_num, type = 'slopes'))
    fits_lib.writeSimpleFits( ccd39_filename, ccd39_slopes)
    print 'Saved as ', ccd39_filename

    time.sleep(2)   # ccd47 delay
    ccd47_filename = os.path.join(basedir,'tv_frame_%s_%d.fits' % (name, pos))

    print 'Averaging %d technical viewer frames...' % ccd47_frames_num
    ccd47_frames = frames_lib.averageFrames( ccd47.get_frames(ccd47_frames_num, type = 'pixels'))
    fits_lib.writeSimpleFits( ccd47_filename, ccd47_frames)
    print 'Saved as ', ccd47_filename



