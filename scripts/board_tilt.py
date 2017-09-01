##################
#
# Board alignment check with Thorlabs
#
# usando execfile('board_tilt.py')


############################
#
# Configuration starts

# Directory where files will be stored, without trailing slash

import calib

basedir = calib.getTodayMeasDir('BoardAlign')

## SELECT CCD TO USE (only one of these can be True)

use_thorlabs = True

# No. of frames to average for each measure
num_frames = 1

# Z range to be explored using the stages
# Positions are in millimeters

# float are not allowed
# zmin...zmax is INCLUSIVE!

zmin=40
zmax=54
zste=2

## ACQUIRE NEW DATA??

do_acquisition = True

## PERFORM IDL ANALYSIS IMMEDIATELY??

idl_analyze = True


import os, time
import fits_lib
import frames_lib
import thorlabs_ccd

zRef = stagez.getPos()

if use_thorlabs:
    thorlabs = thorlabs_ccd.thorlabs_ccd(app)

if do_acquisition:
    n = 0
    for zmu in range(zmin,zmax+zste,zste):

        print 'moving z to %d ' % zmu
        stagez.moveTo(zmu, waitTimeout=20)


        dirre = basedir
        if os.path.exists(dirre) == False:
           os.makedirs(dirre)            

        filename = os.path.join(dirre, 'fr%d.fits' % n)
        print 'Saving to '+filename

        if use_thorlabs:
            time.sleep(2)
            frame = frames_lib.averageFrames( thorlabs.get_frames(num_frames))

        fits_lib.writeSimpleFits(filename, frame, {}, overwrite=True)
        n = n+1

else:
    print 'Skipping data acquisition'


if idl_analyze:

    import idl
    idl_dir  = "/home/labot/AO2.0/proctoz/Allineamento"
    idl_file = "allinea_rirot.pro"
    data_prefix = os.path.join(dirre, "fr")
    idl_cmd  = 'allinea_rirot, x_c, y_c, x, y, ris, errori, m, "%s", IM_TYPE="FITS", 0, %d, pix_size=0.0047, dist_lin=%d, "Spot position" ' % (data_prefix, n-1, zste)

    os.chdir(idl_dir)

    print 'Executing: '+idl_cmd
    idl.ex('.r '+idl_file)
    idl.ex(idl_cmd)

else:
    print 'Skipping IDL data analyze'



