#!/usr/bin/env python
#
# Set ccd39 slopenull from an interaction matrix mode
#
# syntax:
#
# set_slopenull.py <modal basis> <intmat_tracknum> <mode no.> <coefficient>

import sys, os
from AdOpt import thAOApp, setupDevices, cfg, calib, WfsArbScripts, AOExcept, fits_lib

side = cfg.myside

if len(sys.argv)!=6:
    print "set_slopenull.py <binning> <modal basis> <intmat_tracknum> <mode no.> <coefficient>"
    sys.exit(-1)

app = thAOApp('setsn_', side = side)

binning     = int(sys.argv[1])
modalbasis  = sys.argv[2]
im_tracknum = sys.argv[3]
modeno      = int(sys.argv[4])
coeff       = float(sys.argv[5])


impath = os.getenv('ADOPT_ROOT')+'/calib/right/adsec/M2C/'+modalbasis+'/RECs/Intmat_'+im_tracknum+'.fits'
try:
   im = fits_lib.readSimpleFits(impath)
except IOError:
   print 'File %s not found' % impath
   sys.exit(-1)

mode = im[:,modeno] * coeff

snfilename = 'im%s_Z%d_c%03.2f.fits' % (im_tracknum, modeno, coeff)
snpath = os.path.join(calib.slopenullDir(binning), snfilename)

fits_lib.writeSimpleFits( snpath, mode, overwrite=True)

setupDevices.loadDevices( app, ['sc'], check=True)

app.sc.set_slopenull(snfilename)


