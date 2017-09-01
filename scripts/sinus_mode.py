#!/usr/bin/env python2

# Sinus mode acquisition

import sys, dl
sys.setdlopenflags(dl.RTLD_NOW|dl.RTLD_GLOBAL)

import sys, ll, time, getopt, os
import msglib, fits_lib, cfg
from BCUfunctions import *
from numarray import *
import p45lib

idlroot = os.path.dirname(os.environ['IDL_STARTUP'])+"/"

# default values

n_frames = 1000          # N frames to acquire in closed loop
sinus_modes = "modes.fits"             # Fits file with the digital disturbance
max_sc_frames = 0x1026   # Slope computer memory capacity
frames_allowance = 500   # Frames allowance for delays, etc.
modename = ""
maxamp_file  = idlroot+"adsecP45/max_modal_amplitude.fits"
msg_timeout = 60*1000
ccdnum = 1
wfsnum = 1
loop_gain = -1.0

sc_delay = 0.5

# Variables
fl_enable  = "WFS%02dFL_ENABLE" % wfsnum
wfs_enable = "WFS%02dGO" % wfsnum

# Parse command line arguments
opts, args = getopt.getopt( sys.argv[1:], "", ['modes=', 'frames='])
if len(args)>0:
    modename = args[0]

for option,value in opts:
    if option == "--modes":
        sinus_modes = value
    elif option == "--frames":
        n_frames =  int(value)

print "Mode name : ", modename
print "Sinus mode: ", sinus_modes
print "N frames  : ", n_frames


if n_frames > max_sc_frames:
    "An acquisition must be smaller than %d frames in total" % n_frames
    sys.exit(0)

if modename == "":
    print "Usage: sinus_mode.py <filename>"
    sys.exit()

ll.ConnectToMsgD("Sinus", "127.0.0.1")

outprefix = os.environ['ADOPT_ROOT'] + "/scripts/data/sinus/"
outfile = outprefix+modename+".fits"
out_lutx = outprefix+modename+"_slopex.txt"
out_luty = outprefix+modename+"_slopey.txt"
out_modes = outprefix+modename+"_modes.fits"
idl_save_file = outprefix+modename+".sav"

slopex_lut = os.environ['ADOPT_ROOT'] + "/config/lookuptables/cur_slopex"
slopey_lut = os.environ['ADOPT_ROOT'] + "/config/lookuptables/cur_slopey"

exit =0
files = [outfile, out_lutx, out_luty, out_modes, idl_save_file]
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

# Turn off everything
msglib.SetIntVar(wfs_enable, 0)
time.sleep(sc_delay)
msglib.SetIntVar(fl_enable, 0)
time.sleep(sc_delay)
ll.WFS_EnableDigitalNoise(wfsnum,0)
time.sleep(sc_delay)
p45lib.p45_set_gain(0)


# 4. Wait until frame counter has wrapped (if necessary). Record slopecomp frame counter (start)
start = getCurrentFrameNumber_bypointer()
print "start: ",start

end = start + n_frames
print "end: ",end

if end + frames_allowance >= max_sc_frames:
    print "Waiting for %d frames" % (max_sc_frames-start)

    # Re-enable DSP
    msglib.SetIntVar(wfs_enable, 1)

    while getCurrentFrameNumber_bypointer() >= start:
        pass

    # Stop
    msglib.SetIntVar(wfs_enable, 0)
    time.sleep(sc_delay)

    start = getCurrentFrameNumber_bypointer()
    print "nuovo start: ",start

# 6. enable fastlink
p45lib.p45_set_gain( loop_gain)
msglib.SetIntVar(fl_enable, 1)
time.sleep(sc_delay)


# 3. Apply sinus mode
ll.WFS_SetDigitalNoise(wfsnum, sinus_modes)
time.sleep(sc_delay)
ll.WFS_EnableDigitalNoise(wfsnum, 1)
time.sleep(sc_delay)

# 2. Start slopecomp DSP
msglib.SetIntVar(wfs_enable, 1)
time.sleep(sc_delay)


# Get a slope frame to see the shape
s = ll.CCD_GetCurFrame(ccdnum,1,2).shape


# 7. wait until n_frames are passed
while 1:
    stop = getCurrentFrameNumber_bypointer()
    if stop > start + n_frames:
        break

print "stop: ",stop

# 10. stop slopecomp DSP
msglib.SetIntVar(wfs_enable, 0)
time.sleep(sc_delay)

# 9. disable fastlink
msglib.SetIntVar(fl_enable, 0)
time.sleep(sc_delay)
ll.WFS_EnableDigitalNoise(wfsnum,0)
time.sleep(sc_delay)
p45lib.p45_set_gain(0)

# Flat mirror at the end
modes = zeros( 48, Float32)
p45lib.p45_apply_modes(modes)

# 11. download data
slopes = zeros(( n_frames, s[1]), Float32)

print "Getting %d slope frames...." % n_frames

# Initialize BCU communication
ll.CCD_PrepareGetCurFrame(ccdnum)

last_frame = start + n_frames
for n in range(start, last_frame):
    slopes[n-start] = ll.CCD_GetPreviousFrame( ccdnum, n, 2, 0, 1)

# 12. save data
fits_lib.writeSimpleFits(outfile, slopes, {}, overwrite=True)
os.system("/bin/cp %s %s" % (slopex_lut, out_lutx))
os.system("/bin/cp %s %s" % (slopey_lut, out_luty))
os.system("/bin/cp %s %s" % (sinus_modes, out_modes))

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
#msglib.SetIntVar(wfs_enable, 1)

print "Done"
