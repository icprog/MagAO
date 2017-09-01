##################
#
# Rerotator alignment procedure
#
# Lanciare questo file da aoshell.py
# usando execfile('tozzi.py')

from AdOpt import calib
import signal

############################
#
# Configuration starts

# Directory where files will be stored, without trailing slash



basedir = calib.getTodayMeasDir('MuoviLeggiRotatore')

# No. of steps the rerotator will do for each stages position
n_steps = 16

# Step size in degrees
step =22.5

## SELECT CCD TO USE (one or more)

use_ccd39 = True
use_thorlabs1 = False
use_thorlabs2 = False

# No. of frames to average for each measure 
ccd39_num_frames = 50
thorlabs_num_frames = 1

# XY range to be explored using the stages

# float are not allowed
# xmin...xmax is INCLUSIVE!

#xmin = -500 # micron
#xmax = 500 # micron
#xste = 200 # micron
#ymin = -500 # micron
#ymax = 500 # micron
#yste = 200 # micron

xmin=0
xmax=0
xste=1
ymin=0
ymax=0
yste=1

## ACQUIRE NEW DATA??

do_acquisition = True

## PERFORM IDL ANALYSIS IMMEDIATELY??

idl_analyze = False

#
#
# Configuration ends
#
###########################################

import os, time
from AdOpt import fits_lib, frames_lib
from AdOpt.hwctrl import thorlabs_ccd

xRef = stagex.getPos()
yRef = stagey.getPos()

if use_thorlabs1:
    thorlabs1 = thorlabs_ccd.thorlabs_ccd(app, 1)
if use_thorlabs2:
    thorlabs2 = thorlabs_ccd.thorlabs_ccd(app, 2)

def gotit():
	print 'got'

signal.signal( signal.SIGINT, gotit)

if do_acquisition:
    for ymu in range(ymin,ymax+yste,yste):

        print 'moving y to %g ' % (ymu/1000.)
        stagey.moveTo(yRef + ymu/1000., waitTimeout=20)

        for xmu in range(xmin,xmax+xste,xste):
        
            print 'moving x to %g ' % (xmu/1000.)
            stagex.moveTo(xRef + xmu/1000., waitTimeout=20)

            dirre = os.path.join(basedir,'x%d_y%d' % (xmu,ymu))
            if os.path.exists(dirre) == False:
                os.makedirs(dirre)            

            print 'Saving to '+dirre
            for n in range(n_steps):
                print 'moving rerot to %g ' % (n*step)
                rerot.moveTo( n * step, waitTimeout=20 * 1000)
		time.sleep(3)	 # To wait for thorlabs proxy

                if use_ccd39:
                    filename = os.path.join(dirre, 'ccd39_fr_%d.fits' % n)
                    frame = frames_lib.averageFrames( ccd39.get_frames(ccd39_num_frames))
                    fits_lib.writeSimpleFits(filename, frame, {}, overwrite=True)
                if use_thorlabs1:
                    filename = os.path.join(dirre, 'thorlabs1_fr_%d.fits' % n)
                    frame = frames_lib.averageFrames( thorlabs1.get_frames(thorlabs_num_frames))
                    fits_lib.writeSimpleFits(filename, frame, {}, overwrite=True)
                if use_thorlabs2:
                    filename = os.path.join(dirre, 'thorlabs2_fr_%d.fits' % n)
                    frame = frames_lib.averageFrames( thorlabs2.get_frames(thorlabs_num_frames))
                    fits_lib.writeSimpleFits(filename, frame, {}, overwrite=True)


            print 'homing rerot' 
            rerot.home( waitTimeout = 60 * 1000)
else:
    print 'Skipping data acquisition'


if idl_analyze:

    import idl
    idl_dir  = "/home/labot/AO2.0/proctoz/Allineamento"
    idl_file = "allinea_rirot.pro"
    data_prefix = os.path.abspath(basedir)+os.sep
    idl_cmd  = 'multi_allinea_rirot, x_c, y_c, x, y, ris, errori, m, "%s", IM_TYPE="FITS", 0, %d, /IM_MIG, pix_size=0.0047, "Pupil", ' % (data_prefix, n_steps-1)
    idl_cmd += 'xmin=%d, xmax=%d, xstep=%d, ymin=%d, ymax=%d, ystep=%d, IM_PREFIX="ccd39_fr", /WAITFILE ' % (xmin, xmax, xste, ymin, ymax, yste)

    os.chdir(idl_dir)

    print 'Executing: '+idl_cmd
#    time.sleep(60)
    idl.ex('.r '+idl_file)
    idl.ex(idl_cmd)

else:
    print 'Skipping IDL data analyze'

print 'All data saved to '+dirre

