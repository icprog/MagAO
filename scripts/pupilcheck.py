#!/usr/bin/env python

import sys, os, time

from AdOpt import frames_lib, thAOApp, cfg, calib, AOVar, setupDevices, fits_lib, AOConfig
from AdOpt.hwctrl import ccd


# Default values if not specified in the configuration file

expDelay     = 0.1          # Minimum delay between successive measurements in seconds
expTime      = 1.0          # Integration time in seconds
pupThreshold = 0.3
pupSplit     = 1
Method       = 'separate'   # Can be 'separate' or 'sum'

self = thAOApp( myname='pupilcheck', argv= sys.argv)

if len(sys.argv)>1:
    ccdname = sys.argv[-1]
else:
    ccdname = 'ccd39'

if ccdname == 'ccd39':
    from AdOpt.hwctrl import ccd
    myccd = ccd.ccd('ccd39', self)
elif ccdname == 'thorlabs1':
    from AdOpt.hwctrl import thorlabs_ccd
    myccd = thorlabs_ccd.thorlabs_ccd(self, 1)
elif ccdname == 'thorlabs2':
    from AdOpt.hwctrl import thorlabs_ccd
    myccd = thorlabs_ccd.thorlabs_ccd(self, 2)
else:
    print 'Error: ccd name %s unknown' % ccdname
    sys.exit(0)

setupDevices.loadDevices(self, ['sc'], check=True)

#@ Diameter modification: if requested from outside, modify the measured diameter

self.diameterModify=0
def slotDiameterVar( var):
    self.diameterModify = var.Value()

pup_vars = []
for n in range(4):
    pup_vars.append( AOVar.AOVar( name = cfg.varname( self.identity(), 'PUP%d' % n), tipo = 'REAL_VARIABLE', value=(0.0, 0.0, 0.0, 0.0, 0.0, 0.0)))
 
for n in range(4):
    self.CreaVar(pup_vars[n])

diameterVar = AOVar.AOVar( name = cfg.varname( self.identity(), 'DIAMETER_MODIFY'), tipo = 'INT_VARIABLE', value=(0,))
self.CreaVar(diameterVar)
self.VarNotif( diameterVar.Name(), slotDiameterVar)



self.SetReady()

def fakePupils():
    pupils = []
    for i in range(4):
        p = frames_lib.pupil()
        p.cx = 0
        p.cy = 0
        pupils.append(p)
    return pupils




while not self.TimeToDie():

    # Refresh parameters from cfg file
    mycfg = AOConfig.read_config(cfg.conffile(self.identity()))
    try:
        th1 = mycfg['pupThreshold'].Value()
    except:
        th1 = pupThreshold

    try:
        split = mycfg['pupSplit'].Value()
    except:
        split = pupSplit

    try:
        delay = mycfg['delay'].Value()
    except:
        delay = expDelay

    try:
        exptime = mycfg['expTime'].Value()
    except:
        exptime = expTime

    try:
        curPupils = calib.getCurPupils( self)
    except:
        curPupils = fakePupils()

    method = Method
    try:
        bin = myccd.xbin()
        if bin > 0:
            method = mycfg['methodBin%d' % bin].Value()
    except:
        pass

    self.log('Threshold: %3.1f Split: %d  Exptime: %3.1f  Delay: %3.1f' % (th1, split, exptime, delay))
    self.log('Method: %s' % method)

    if (delay>0):
        time.sleep(delay)

    try:
	num_frames = int(round((exptime*myccd.get_framerate()) / (self.sc.get_masterd_decimation()+1)))
        #if num_frames > 500:
        #    num_frames = 500
        self.log('Reading %s frames' % num_frames)
        frame = frames_lib.averageFrames( myccd.get_frames( num_frames))
	fits_lib.writeSimpleFits('/tmp/acqpupils.fits', frame, overwrite=True);

        pupils = frames_lib.findPupils( frame, th1, th1, fourPupils=split, app=self, method=method, binning=myccd.xbin())

        for n in range(len(pupils)):

            diff_cx = pupils[n].cx - curPupils[n].cx
            diff_cy = pupils[n].cy - curPupils[n].cy
            pupils[n].diameter += self.diameterModify

            print pupils[n].diameter, pupils[n].cx, pupils[n].cy, pupils[n].side, diff_cx, diff_cy
            self.SetVar( pup_vars[n], (pupils[n].diameter, pupils[n].cx, pupils[n].cy, pupils[n].side, diff_cx, diff_cy))


    except Exception,e:
        print 'Error: ',e
        self.log('Exception: '+str(e))
        time.sleep(1)
        continue
    except:
        print 'Unknown exception'
        app.log('Unknown exception')
        time.sleep(1)
        continue

     
     
    
