#!/usr/bin/env python2

# Automatic closed loop run with P45

import sys, dl
sys.setdlopenflags(dl.RTLD_NOW|dl.RTLD_GLOBAL)

import ll, time, getopt, os
import msglib, fits_lib, cfg
from BCUfunctions import *
from numarray import *
import p45lib

idlroot = os.path.dirname(os.environ['IDL_STARTUP'])+"/"


n_previous_frames =0  # N frames before closing the loop
n_frames = 500           # N frames to acquire in closed loop
start_modes = "startmodes.fits"        # Fits file with start modes position
max_sc_frames = 0x1026   # Slope computer memory capacity
frames_allowance = 500   # Frames allowance for delays, etc.
loopname = ""
maxamp_file  = idlroot+"adsecP45/max_modal_amplitude.fits"
msg_timeout = 60*1000
gain = -1.5
ccdnum=1
wfsnum=1


# Variables
fl_enable  = "WFS%02dFL_ENABLE" % wfsnum
wfs_enable = "WFS%02dGO" % wfsnum

# Parse command line arguments
opts, args = getopt.getopt( sys.argv[1:], "", ['modes=', 'frames=', 'gain='])
if len(args)>0:
    loopname = args[0]

for option,value in opts:
    if option == "--modes":
        start_modes = value
    elif option == "--frames":
        n_frames =  int(value)
    elif option == "--gain":
        gain =  float(value)

print "Loop name : ", loopname
print "Start mode: ", start_modes
print "N frames  : ", n_frames
print "gain      : ", gain



if n_frames + n_previous_frames > max_sc_frames:
    "An acquisition must be smaller than %d frames in total" % (n_frames + n_previous_frames)
    sys.exit(0)

if loopname == "":
	print "Usage: cloop.py <filename>\n"
	sys.exit()

ll.ConnectToMsgD("CLoop", "127.0.0.1")

outprefix = os.environ['ADOPT_ROOT'] + "/scripts/data/cloop/"
outfile = outprefix+loopname+".fits"
out_lutx = outprefix+loopname+"_slopex.txt"
out_luty = outprefix+loopname+"_slopey.txt"
idl_save_file = outprefix+loopname+".sav"

slopex_lut = os.environ['ADOPT_ROOT'] + "/config/lookuptables/cur_slopex"
slopey_lut = os.environ['ADOPT_ROOT'] + "/config/lookuptables/cur_slopey"

exit =0
files = [outfile, out_lutx, out_luty]
for f in files:
    if os.access( f, os.F_OK):
        print "Output file %s already exist" % f
        exit = 1

if exit:
    print "Overwrite?"
    answer = sys.stdin.readline().strip()
    if answer == "y" or answer == "yes":
        exit = 0
        for f in files:
            if os.access( f, os.F_OK):
                 os.unlink(f)

if exit:
    sys.exit()

# Read modal amplitude file
maxamp = fits_lib.readSimpleFits(maxamp_file)


# 1. disable fastlink
msglib.SetIntVar(fl_enable, 0)

# 2. Start slopecomp DSP
msglib.SetIntVar(wfs_enable, 1)
time.sleep(0.5)

# Get a slope frame to see the shape
s = ll.CCD_GetCurFrame(ccdnum,1,2).shape

# 3. Apply mode
modes = fits_lib.readSimpleFits(start_modes)
for n in range(len(modes)):
    modes[n] = modes[n] * maxamp[n]

p45lib.p45_apply_modes(modes)

# 6. set zero gain and enable fastlink

p45lib.p45_set_gain(0)

# Raise gain

print "Setting gain to "+str(gain)
p45lib.p45_set_gain(gain)

msglib.SetIntVar(fl_enable, 1)


# 4. Wait until frame counter has wrapped (if necessary). Record slopecomp frame counter (start)
real_start = getCurrentFrameNumber_bypointer()
start = real_start % max_sc_frames 

print "start: ",start
print "real_start: ",real_start

end = start + n_previous_frames + n_frames
print "end: ",end

if end + frames_allowance >= max_sc_frames:
    print "Waiting for %d frames" % (end-start)
    while (getCurrentFrameNumber_bypointer() % max_sc_frames) >= start:
        pass
    real_start = getCurrentFrameNumber_bypointer()
    start = real_start % max_sc_frames

print "start: ",start
print "real_start: ",start

# 5. wait until n_previous_frames have passed
#while 1:
#    if (getCurrentFrameNumber_bypointer() % max_sc_frames) >= start + n_previous_frames:
#        break

# 7. wait until n_frames are passed
while 1:
    if (getCurrentFrameNumber_bypointer() % max_sc_frames) >= start + n_previous_frames + n_frames:
        break

# 8. record slopecomp frame counter (stop)
real_stop = getCurrentFrameNumber_bypointer()
stop = real_stop % max_sc_frames 

print "stop: ",stop
print "real_stop: ",real_stop

# 10. stop slopecomp DSP
msglib.SetIntVar(wfs_enable, 0)

time.sleep(0.5)
# 9. disable fastlink

msglib.SetIntVar(fl_enable, 0)

p45lib.p45_set_gain(0)



# 11. download data
slopes = zeros(( stop-start, s[1]), Float32)

print "Getting %d frames...." % (stop-start)

# Initialize BCU communication
#ll.CCD_PrepareGetCurFrame(ccdnum)

for n in range(start, stop):
    slopes[n-start] = ll.CCD_GetPreviousFrame( ccdnum, n, 2, 0, 0)

# 12. save data
fits_lib.writeSimpleFits(outfile, slopes, {})
os.system("/bin/cp %s %s" % (slopex_lut, out_lutx))
os.system("/bin/cp %s %s" % (slopey_lut, out_luty))


print "Saving IDL data..."

idl_cmd = "print,load_diag_data(%d, data, /READFC, /NOFA)" % (n_frames+100)
msglib.SendMessage( cfg.taskname_IDLCtrl, cfg.msg_idlcmd, idl_cmd)
msg = msglib.WaitMessage(msg_timeout)
if msg['status'] != 'NO_ERROR':
    print "Error saving IDL data"
    sys.exit()

idl_cmd = "save,data,filename='%s'" % idl_save_file
msglib.SendMessage( cfg.taskname_IDLCtrl, cfg.msg_idlcmd, idl_cmd)
msg = msglib.WaitMessage(msg_timeout)
if msg['status'] != 'NO_ERROR':
    print "Error applying modes vector"
    sys.exit()



# 13. restart slopecomp DSP
msglib.SetIntVar(wfs_enable, 1)

print "Done"


