
# Start this file with execfile('test_dark_ron_table.py')

###########################
# CCD39 readout noise measurement using only the dark frame. The value found is expressed in ADU.
#
# Assumes that the environment is already completely dark.
#
# This version only tries the binnings and readout speeds that will be actually used 
###########################


#############################
# Configuration starts

# Acquisition type (will go in the directory name)

type = 'RON'

# Acquisition name (will go in the file name)

name = 'dark_ron'

# Set of binning and framerates to test

# [TODO] Read this info from the AO table
bins = [1,2,3,4,5]
framerates = {}
framerates[1] = [990]
framerates[2] = [990,625,400]
framerates[3] = [200]
framerates[4] = [300,200,105]
framerates[5] = [105]
gains = {}
gains[1] = [0,1,2,3]
gains[2] = [0]
gains[3] = [0]
gains[4] = [0]
gains[5] = [0]

results = {}
readouts = {}
for bin in bins:
  results[bin] = {}
  readouts[bin] = {}
  for f in framerates[bin]:
    results[bin][f] = {}

# Target light level for dark (background level when no light is present)

dark_level = 1000

# g factor (conversione ADU -> e-)

g = 0.50

# No. of frames to take for each measurement

frames_num = 100

# Configuration ends
###########################

from AdOpt import calib, fits_lib, WfsArbScripts, ron
import os, time
import numpy

basedir = calib.getTodayMeasDir(type)

# Setup the ccd39 gain conversion
calib_filename = calib.sourceAcqCalibFile()
data = calib.loadCalibData( calib_filename, check=['gain_0', 'gain_1', 'gain_2', 'gain_3'])

# Avoid going too fast when we set 1Khz.
sc.set_masterd_decimation(2)

for bin in bins:
  for framerate in framerates[bin]:
    for gain in gains[bin]:
      print 'Preparing CCD for bin %d, framerate %d Hz, gain %d' % (bin, framerate, gain)

      WfsArbScripts.doSetLoopParams( app, 'FLAO', 'IRTC', framerate, 2, bin, 1, takeDark = False, HOgain=gain)
      ccd39.equalize_quadrants( target = dark_level, delay=0.1)

      # Leave things to stabilize a bit
      time.sleep(1)

      ron_mean = ron.measureSingle( app, doSetup=False, saveData = True)
      ron_mean *= data['gain_%d'%gain]

      results[bin][framerate][gain] = ron_mean
      readouts[bin][framerate] = ccd39.speed() 
      print 'Average RON:', round( ron_mean, 2) ,'e- (estimated)'


print
print '------------------------------ SUMMARY ------------------------------------'
print

for bin in bins:
  for framerate in sorted(results[bin].keys(), reverse=True):
    for gain in sorted(results[bin][framerate].keys()):
       print 'Bin: %d - Framerate %4d Hz (%4d kpix/sec) - gain %d - RON (e-):  %5.2f (estimated)' % (bin, framerate, readouts[bin][framerate], gain, results[bin][framerate][gain])

