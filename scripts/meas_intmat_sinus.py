#!/usr/bin/env python

import sys, dl
sys.setdlopenflags(dl.RTLD_NOW|dl.RTLD_GLOBAL)

import idl, p45lib, ll, fits_lib
import getopt, os, sys
from numarray import *

n_modes = 48
n_modes_todo = 45
nframes = 1000
sinus_amps = 'sinus_amps.fits'
sinus_freq = 'sinus_freq.fits'
matrix_name = ""
noise = False

ll.ConnectToMsgD("IntMat", "127.0.0.1")

# Parse command line arguments
opts, args = getopt.getopt( sys.argv[1:], "", ['modes=', 'frames=', 'ampfactorfile=', 'freqfile=', 'noise'])
if len(args)>0:
    matrix_name = args[0]

for option,value in opts:
    if option == "--modes":
        n_modes_todo = int(value)
    elif option == "--frames":
        n_frames =  int(value)
    elif option == "--cycles":
        tun_tun_cycles =  int(value)
    elif option == "--ampfactorfile":
        sinus_amps = value
    elif option == "--freqfile":
        sinus_freq = value
    elif option == "--noise":
        noise = True

if matrix_name == "":
    print "Usage: meas_intmat_sinus.py [--modes=<# modes>] [--frames=<# frames>] [--ampfactorfile=<file> [--freqfile=<file>] [--noise] <matrixname>"
    sys.exit(0)

print 'Matrix name: ',matrix_name
print 'Modes      : ',n_modes_todo
print 'Frames     : ',n_frames
print 'Amps       : ',sinus_amps
print 'Freq       : ',sinus_freq
print 'Noise      : '+str(noise)

tmp_modefile = "/tmp/modefile.fits"
tmp_ampfile  = "/tmp/ampfile.fits"
tmp_freqfile = "/tmp/freqfile.fits"

idlroot = os.path.dirname(os.environ['IDL_STARTUP'])
m2c_file    = idlroot+'/adsecP45/data/m2c.fits'
maxamp_file = idlroot+'/adsecP45/data/max_modal_amplitude.fits'
ff_file     = idlroot+'/adsecP45/data/ff_matrix.sav'


amps = fits_lib.readSimpleFits(sinus_amps)
freq = fits_lib.readSimpleFits(sinus_freq)

for m1 in range(n_modes_todo):

    m = m1+6

    # Zero out mirror at start
    modes = zeros( n_modes, Float32)
    p45lib.p45_apply_modes(modes)


    modes[m] = amps[m]
    freqs = zeros( n_modes, Float32)
    freqs[m] = freq[m]

    fits_lib.writeSimpleFits(tmp_ampfile, modes, {}, overwrite = True)
    fits_lib.writeSimpleFits(tmp_freqfile, freqs, {}, overwrite = True)

    noiseflag = ""
    if noise:
        noiseflag = ", /NOISE"

    prefix = os.environ['ADOPT_ROOT']+"/scripts/"
    idl_cmd = ".r "+prefix+"modal_noise"
    idl.ex(idl_cmd)
    idl_cmd = ".r "+prefix+"gen_noise"
    idl.ex(idl_cmd)
    idl_cmd = "modal_noise, '%s', AMPFILE='%s', PERIODSFILE='%s', M2C_FILE='%s', FF_FILE='%s', MAXAMP_FILE='%s' %s" % \
           (tmp_modefile, tmp_ampfile, tmp_freqfile, m2c_file, ff_file, maxamp_file, noiseflag)

    idl.ex(idl_cmd)

    outfile = matrix_name+"_"+str(m)

    cmd = "./sinus_mode.py --modes='%s' --frames='%d' '%s'" % (tmp_modefile, n_frames, outfile)

    print cmd
    os.system(cmd)


# Leave mirror flat at the end
    
modes = zeros( n_modes, Float32)
p45lib.p45_apply_modes(modes)
