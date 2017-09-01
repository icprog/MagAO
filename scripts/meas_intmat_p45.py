#!/usr/bin/env python2

# Measures a zonal interaction matrix

import sys, dl
sys.setdlopenflags(dl.RTLD_NOW|dl.RTLD_GLOBAL)

import time, getopt
from BCUfunctions import *
import p45lib 
from numarray import *
import ll, fits_lib, os

import idl

#from AdOpt import *

# default values
prefix = os.environ['ADOPT_ROOT'] + "/"
idlroot = os.path.dirname(os.environ['IDL_STARTUP'])+"/"

matrix_name = "p45"
n_modes      = 45
n_act        = 48
n_frames     = 10
ref_position = 0.0
tun_tun_cycles = 1
skip  =0   # Set this to skip the measure and process an already-taken intmat
dummy_mirror   =0   # Set this to avoid moving the mirror at all
dummy_sensor   =0   # Set this to avoid reading the sensor at all
ccdnum       =1
ampfactor_file  = prefix+"scripts/ampfactor.fits"


# get_slopes
#
# Gets a series of slopes from the CCD and optionally
# averages them. The <n_slopes> parameter is the number
# of valid slopes in the slope frame

def get_slopes( num_frames, n_slopes, n_good_slopes, average=1):


    print "Getting",num_frames,"slope frames"

    my_slopes = zeros( (num_frames, n_slopes), Float32)
    if not dummy_sensor:
        for f in range(num_frames):
            sl = ll.CCD_GetCurFrame(ccdnum,1,2,1,1)[0,:]
            sl[n_good_slopes:] = 0
            my_slopes[f,:] = sl

    if average:
        my_slopes = averageFrames(my_slopes)

    return my_slopes




ll.ConnectToMsgD("IntMat", "127.0.0.1")

# Parse command line arguments
opts, args = getopt.getopt( sys.argv[1:], "", ['modes=', 'frames=', 'cycles=', 'nomeasure', 'ampfactorfile='])
if len(args)>0:
    matrix_name = args[0]

for option,value in opts:
    if option == "--modes":
        n_modes = int(value)
    elif option == "--frames":
        n_frames =  int(value)
    elif option == "--cycles":
        tun_tun_cycles =  int(value)
    elif option == "--nomeasure":
        skip = 1
    elif option == "--ampfactorfile":
        ampfactor_file = value
    
print 'Matrix name: ',matrix_name
print 'Modes      : ',n_modes
print 'Frames     : ',n_frames
print 'Cycles     : ',tun_tun_cycles
print 'Skip       : ',skip
print 'Ampfactor  : ',ampfactor_file

# Values derived from command line arguments or default values


slopes_file  = prefix+"scripts/data/measure_%s.fits" % matrix_name
modes_file   = prefix+"scripts/data/modes_%s.fits" % matrix_name
slopes_collapsed_file  = prefix+"scripts/data/intmat_%s.fits" % matrix_name
rec_file     = prefix+"config/reconstructors/rec_%s.fits" % matrix_name
maxamp_file  = idlroot+"adsecP45/max_modal_amplitude.fits"
out_amp_factor_file  = prefix+"scripts/data/ampfactor_%s.fits" % matrix_name
cur_indpup   = prefix+"config/lookuptables/cur_indpup"

exit = 0
files = [slopes_file, modes_file, rec_file, out_amp_factor_file]

if not skip:
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

# Test ampfactor file
if not os.access( ampfactor_file, os.F_OK):
    print "Amplitude factor file not found. Continue using full amplitude?"
    answer = sys.stdin.readline().strip()
    if answer == "y" or answer == "yes":
        maxamp = zeros(48, Float32)
        for i in range(48):
            maxamp[i] = 1.0
    else:
        sys.exit(0)

# Get binning

binning = ll.CCD_GetBinning(1)

print "Binning: ",binning


if skip == 0:
    # Read # of good slopes from the acq pupils result
    real_slopes  = len(file(cur_indpup).readlines()) /2

    # Read modal amplitude file
    maxamp = fits_lib.readSimpleFits(maxamp_file)

    # Read amplitude factor file
    ampfactor = fits_lib.readSimpleFits(ampfactor_file)

    # Initialize BCU communication
    ll.CCD_PrepareGetCurFrame(ccdnum)

    print "Getting a single frame"
    if not dummy_sensor:
        s = ll.CCD_GetCurFrame(ccdnum,1,2,1,1).shape
    else:
        s = array([40,80])
    print "Done"

    total_slopes = s[1]

    print "Total slopes:",total_slopes
#    print "Getting nslopes"
#    n_slopes = ll.WFS_GetNsubap(1)
#    print "N slopes: ", n_slopes
    n_slopes = total_slopes

    data_slopes = zeros((total_slopes, n_modes * tun_tun_cycles), Float32)
    data_modes  = zeros((48, n_modes * tun_tun_cycles), Float32)

    print "Data.shape:",data_slopes.shape

    # Now loop over the modes
    for i in range(n_modes):

     modes = zeros( n_act, Float32)

     for n in range(n_act):
             modes[n] = ref_position

     amp = ampfactor[i]
     if amp<0:
        amp = 0
     if amp>1:
        amp = 1 
     modes[i] = maxamp[i] * amp

     print "Mode %d, amp %f (%d%% of maximum)" % (i, modes[i], int(amp*100))


     time.sleep(0.1)

     for t in range(tun_tun_cycles):
         if not dummy_mirror:
            p45lib.p45_apply_modes(modes)
            time.sleep(0.01)

         up_slopes = get_slopes( n_frames, total_slopes, n_slopes)
         if not dummy_mirror:
            p45lib.p45_apply_modes(-modes)
            time.sleep(0.01)

         down_slopes = get_slopes( n_frames, total_slopes, n_slopes)

         # Save slopes and command
         data_slopes[:,i*tun_tun_cycles+t] = up_slopes - down_slopes
         data_modes [:,i*tun_tun_cycles+t] = modes - (-modes)        # Scritta per esteso......

    # Write the result file

    dict={}
    dict["TYPE"] = "INTMAT"
    dict["SUBTYPE"] = "MODAL"
    dict["N_MODES"] = str(n_modes)
    dict["TUN_TUN"]= str(tun_tun_cycles)
    dict["N_ACT"] = str(n_act)
    dict["N_FRAMES"] = str(n_frames)

    reduced_slopes = data_slopes[0:real_slopes,:] 

    fits_lib.writeSimpleFits( slopes_file, reduced_slopes, dict)
    fits_lib.writeSimpleFits( modes_file,  data_modes, dict)
    fits_lib.writeSimpleFits( out_amp_factor_file, ampfactor, dict)

    print "Saved", slopes_file
    print "Saved", modes_file

    # Leave mirror flat at the end
    if not dummy_mirror:
       p45lib.p45_apply_modes( zeros(n_act, Float32))

# Now calculate reconstruction matrix

idl_file = prefix+"scripts/guarda_intmat.pro"
idl.ex(".r "+idl_file)
idl_cmd = "guarda_intmat, intmat, b, TUN_TUN="+str(tun_tun_cycles)+", INPUT_FILE='"+slopes_file+"', OUTPUT_FILE='"+slopes_collapsed_file+"', NORMALIZE_CMD_FILE='"+maxamp_file+"', AMPFACTOR_FILE='"+out_amp_factor_file+"', BINNING = "+str(binning)
idl.ex(idl_cmd)


idl_file = "/home/labot/idl/adsec_lib/adsec_utilities/cloop_routines_fits_p45.pro"
idl_cmd = "gen_reconstructor, rec, w, /MODAL_MATRIX, INPUT_FILE='"+slopes_collapsed_file+"', OUTPUT_FILE='"+rec_file+"', FINAL_DIM = [1600,48]"
idl.ex(".r "+idl_file)
idl.ex(idl_cmd)



print "Done"



