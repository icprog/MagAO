##################
#
# Rerotator alignment procedure using tt centering
#
# Lanciare questo file da aoshell.py
# usando execfile('muovileggirot.py')


############################
#
# Configuration starts

# Directory where files will be stored, without trailing slash

from AdOpt import centerPupils, calib, fits_lib, frames_lib
from AdOpt.wrappers import drawlib
import os, time

basedir = calib.getTodayMeasDir('MuoviLeggiRotatoreTT')

# No. of steps the rerotator will do for each stages position
n_steps = 15

# Step size in degrees
step =22.5

# No. of frames to average for each measure 
ccd39_num_frames = 10

# Recenter stages at each step
recenterStages = False

if os.path.exists(basedir) == False:
    os.makedirs(basedir)            

print 'Saving to '+basedir

maxttx = -1000
minttx = 1000
maxtty = -1000
mintty = 1000
maxpupx = -1000
minpupx = 1000
maxpupy = -1000
minpupy = 1000

prev_x=None
prev_y=None

for n in range(n_steps):
    print 'moving rerot to %g ' % (n*step)
    rerot.moveTo( n * step, waitTimeout=20 * 1000)

    if recenterStages:
        centerPupils.centerPupils( app, useStages=True, useTT=False) 

    filename = os.path.join(basedir, 'tt_fr%d.txt'%n)
    file(filename, 'w').write('%5.2f %5.2f' % (tt.offx(), tt.offy()))

    if tt.offx() > maxttx:
        maxttx = tt.offx()
    if tt.offy() > maxtty:
        maxtty = tt.offy()
    if tt.offx() < minttx:
        minttx = tt.offx()
    if tt.offy() < mintty:
        mintty = tt.offy()

    frame = frames_lib.averageFrames( ccd39.get_frames(ccd39_num_frames))
    pupils = frames_lib.findPupils( frame, 0.40, 0.40, fourPupils = 1, method='sum',binning=1)
    filename = os.path.join(basedir, 'pupils_fr%d.txt'%n)
    f = file(filename, 'w')
    for pup in pupils:
        f.write('%5.2f %5.2f\n' % (pup.cx, pup.cy))
    f.close()

    if pupils[0].cx > maxpupx:
        maxpupx = pupils[0].cx
    if pupils[0].cy > maxpupy:
        maxpupy = pupils[0].cy
    if pupils[0].cx < minpupx:
        minpupx = pupils[0].cx
    if pupils[0].cy < minpupy:
        minpupy = pupils[0].cy

    #drawlib.sendCircle('ccdviewer_ccd39', pupils[0].cx, pupils[0].cy, pupils[0].diameter, 0xFF0000, 5, 1 )
    #if prev_x != None:
    #    drawlib.sendLine('ccdviewer_ccd39', prev_x, prev_y, pupils[0].cx, pupils[0].cy, 0xFF0000, 30, 1)
    #prev_x = pupils[0].cx
    #prev_y = pupils[0].cy

    filename = os.path.join(basedir, 'ccd39_fr%d.fits' % n)
    fits_lib.writeSimpleFits(filename, frame, {}, overwrite=True)

mm_ttx = maxttx - minttx
mm_tty = maxtty - mintty
mm_pupx = maxpupx - minpupx
mm_pupy = maxpupy - minpupy

# IDL plot
from AdOpt.wrappers import idl

try:
    #idl_file = "muovileggirot_plot.pro"
    idl_cmd = 'muovileggirot_plot, "%s", %d' % (basedir, n_steps)

    print 'Executing: '+idl_cmd
    idl.ex(idl_cmd)
except:
    pass


print
print
print 'Minmax tt X (volts): ',mm_ttx
print 'Minmax tt Y (volts): ',mm_tty
print 'Minmax pup X  (pix): ',mm_pupx
print 'Minmax pup Y  (pix): ',mm_pupy
print
print 'Detailed data saved to: ',basedir
print
print 'homing rerot' 
rerot.home( waitTimeout = 60 * 1000)
print 'done' 

time.sleep(10000)




