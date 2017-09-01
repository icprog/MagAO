##################
#
# Tip-tilt alignment procedure
#
# Lanciare questo file da aoshell.py
# usando execfile('calibra_tt.py')

from AdOpt import calib
import os, time

############################
#
# Configuration starts

# Directory where files will be stored, without trailing slash



basedir = calib.getTodayMeasDir('CalibTT')
if os.path.exists(basedir) == False:
    os.makedirs(basedir)            

print 'Saving to ',basedir

# Frequencies to set with ccd39
freqs = [100, 200, 300, 400, 500, 600, 700, 800, 900, 1000]
#freqs = [900, 1000]
#freqs = [100,200,300,400]

# Amplitudes to set with TT (0-5)
#amps = [2.0, 2.5, 3.0, 3.5, 4.0]

amps = [0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0, 1.1, 1.2, 1.3, 1.4, 1.5]

# No. of frames to average for each measure 
thorlabs_num_frames = 3

#
# Configuration ends
#
###########################################

from AdOpt import fits_lib, frames_lib
from AdOpt.hwctrl import thorlabs_ccd

thorlabs1 = thorlabs_ccd.thorlabs_ccd(app, 1)


for freq in freqs:

    print 'Setting %d Hz' % freq
    ccd39.set_framerate(freq)

    for amp in amps:

        print 'Setting amp %f volts' % (amp*2)

        tt.set(amp = amp)

        time.sleep(5)	 # To wait for thorlabs proxy

        filename = os.path.join(basedir, 'thorlabs1_%dhz_%03.1fv.fits' % (freq, amp*2))
        frame = frames_lib.averageFrames( thorlabs1.get_frames(thorlabs_num_frames))
        fits_lib.writeSimpleFits(filename, frame, {}, overwrite=True)

print 'All data saved to '+basedir

tt.set(amp=0)

