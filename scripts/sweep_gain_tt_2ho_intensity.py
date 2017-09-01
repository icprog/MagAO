#@File: sweep_gain.py
#
# Sweep the loop gain and record slopes in the meantime
#
# Run this file from thaoshell: execfile('sweep_gain.py')

# First and last gain value, and step, and how many times to repeat the ramp-up

from AdOpt import fits_lib, cfg
from AdOpt.wrappers import idl, diagbuf
from numpy import *
import os

intf = diagbuf.diagbuf( app, 39)

m2cVar = 'ADSEC.'+cfg.side+'.M2C@M_ADSEC'
recVar = 'ADSEC.'+cfg.side+'.B0_A@M_ADSEC'

# How many slope frames to record at each step, and how many to skip after each gain application
slopes_skip   = 10
slopes_record = 38

s_date = calib.getDateTime( addTimestamp = False)
s_time = calib.getDateTime( addTimestamp = True, onlyTime = True)
datapath = '/towerdata/adsec_data/%s/Data_%s_%s/' % (s_date, s_date, s_time)
os.makedirs(datapath)

def doit( bin):

	if bin == 1:
		start = 0.2 # LB 20111115 it was 0.1
		end   = 1.5
		step  = 0.1
		cycles = 10
		ratio = 0.75 
                ho_middle = 100

	if bin == 2:
		start = 0.2
		end   = 2.0
		step  = 0.3  
		cycles = 10
		ratio = 0.71
                ho_middle = 33

	if bin == 3:
		start = 0.1
		end   = 2.0
		step  = 0.30  
		cycles = 10
		ratio = 0.61 
                ho_middle = 22

	if bin == 4:
		start = 0.1
		end   = 2.0
		step  = 0.4
		cycles = 10
		ratio = 0.61 
                ho_middle = 6

        # Repeat threshold in percentage (0..1)
	# If the optimal gain is bigger than this threshold, the cycle is repeated with 50% higher gains
	repeat_th = 0.8

	datastep =0
	tt = 0
	ho1 = 0
	ho2 = 0

	start_orig = start
	end_orig = end

	# First step: TT and HO together
	rms_min = 9e99
	while 1:
		datastep +=1
		f, tt, ho1, ho2, rms = optimize( datapath, 'step%d'%datastep, slopes_skip, slopes_record, tt, ho1, ho2, start, end, step, cycles, ratio, target='all', set=True, middle_ho=ho_middle)
 		if rms > rms_min:
			# should use previous minimum
			tt = tt_min
			ho1 = ho1_min
			ho2 = ho2_min
			break
		repeat_gain = start + (end-start)*repeat_th
		if tt < repeat_gain and ho1 < repeat_gain and ho2 < repeat_gain:
			break
		range = (end-start)
		start += range/2
		end += range/2
		if rms < rms_min:
			rms_min = rms
			tt_min = tt
			ho1_min = ho1
			ho2_min = ho2

	start = start_orig
	end = end_orig

	# Second step: TT only
	rms_min = 9e99
	while 1:
		datastep +=1
		f, tt, tmp1, tmp2, rms = optimize( datapath, 'step%d'%datastep, slopes_skip, slopes_record, tt, ho1, ho2, start, end, step, cycles, None, 'tt', set=True, middle_ho=ho_middle)
 		if rms > rms_min:
			# should use previous minimum
			tt = tt_min
			break
                repeat_gain = start + (end-start)*repeat_th
		if tt < repeat_gain:
			break
		range = (end-start)
		start += range/2
		end += range/2
		if rms < rms_min:
			rms_min = rms
			tt_min = tt

	start = start_orig
	end = end_orig

	# Third step: HO1 only
	rms_min = 9e99
	while 1:
		datastep +=1
		f, tmp1, ho1, tmp2, rms = optimize( datapath, 'step%d'%datastep, slopes_skip, slopes_record, tt, ho1, ho2, start, end, step, cycles, None, 'ho1', set=True, middle_ho=ho_middle)
  		if rms > rms_min:
			# should use previous minimum
			ho1 = ho1_min
			break
		repeat_gain = start + (end-start)*repeat_th
		if ho1 < repeat_gain:
			break
		range = (end-start)
		start += range/2
		end += range/2
		if rms < rms_min:
			rms_min = rms
			ho1_min = ho1

	start = start_orig
	end = end_orig

	# Third step: HO2 only
	rms_min = 9e99
	while 1:
		datastep +=1
		f, tmp1, tmp2, ho2, rms = optimize( datapath, 'step%d'%datastep, slopes_skip, slopes_record, tt, ho1, ho2, start, end, step, cycles, None, 'ho2', set=True, middle_ho=ho_middle)
  		if rms > rms_min:
			# should use previous minimum
			ho2 = ho2_min
			break
                repeat_gain = start + (end-start)*repeat_th
		if ho2 < repeat_gain:
			break
		range = (end-start)
		start += range/2
		end += range/2
		if rms < rms_min:
			rms_min = rms
			ho2_min = ho2

def optimize( datapath, dataname, slopes_skip, slopes_record, starttt, startho1, startho2, start, end, step, cycles, ratio, target='all', set=False, middle_ho=672):

    # Get adsec data
    modalbasis = app.ReadVar(m2cVar).Value().split('/')[-2]
    recpath = 'calib/adsec/current/M2C/%s/RECs/' % modalbasis
    rec = recpath + app.ReadVar(recVar).Value().split('/')[-1]
    print rec

    gainpath = '/towerdata/adsec_calib/M2C/%s/gain/' % modalbasis
    outfile_gains  = datapath + 'gains_'+dataname+'.fits'
    outfile_slopes = datapath + 'slopes_'+dataname+'.fits'
    outfile_frames = datapath + 'frames_'+dataname+'.fits'

    gain_tt, gain_ho1, gain_ho2, steps_ramp = define_ramp( starttt, startho1, startho2, start, end, step, cycles, ratio, target)
    hdr = {}
    hdr['CYCLES'] =cycles
    hdr['STEPS_RAMP'] = steps_ramp
    hdr['M2C'] = modalbasis
    hdr['REC'] = rec[ rec.find('M2C')+4:]

    acquire_sweep( gain_tt, gain_ho1, gain_ho2, middle_ho, slopes_skip, slopes_record, gainpath, outfile_gains, outfile_slopes, outfile_frames, hdr)

    idlcmd = "sweep_gain_plot_tt_2ho, GAINSFILE='%s', SLOPESFILE='%s', FRAMESFILE='%s', GAINV=GAINV, GAINTT=GAINTT, GAINHO1 = GAINHO1,GAINHO2 = GAINHO2, RMSMIN = RMSMIN, TARGET = '%s',  MIDDLE_HO = %d, /NOSAVE" % (outfile_gains, outfile_slopes, outfile_frames, target, middle_ho)
    print idlcmd
    idl.ex(idlcmd)
    gainv = idl.var.gainv
    gaintt = idl.var.gaintt
    gainho1 = idl.var.gainho1
    gainho2 = idl.var.gainho2
    rms = idl.var.rmsmin
    gainv2 = array(gainv) # the original gainv is read-only and writeSimpleFits fails
    filename = gainpath+'g_tt_%4.2f_ho1_%4.2f_ho2_%4.2f.fits' % (gaintt, gainho1, gainho2)
    fits_lib.writeSimpleFits( filename, gainv2, overwrite=True)	

    if set:
        cmd = "print, fsm_set_gain('%s')" % filename
        app.IDLCmd(cmd)
    return (filename, gaintt, gainho1, gainho2, rms)


def define_ramp( starttt, startho1, startho2, start, end, step, cycles, ratio=None, target = 'all'):

	steps_ramp = round((end-start)/step)
	steps = steps_ramp * cycles+1
    
	gain_tt = zeros((steps), dtype='Float32')
	gain_ho1 = zeros((steps), dtype='Float32')
	gain_ho2 = zeros((steps), dtype='Float32')
    
	for cycles in range(cycles):
		for n in range(steps_ramp):
                        
                        gain_tt[n+cycles*steps_ramp] = starttt
                        gain_ho1[n+cycles*steps_ramp] = startho1
                        gain_ho2[n+cycles*steps_ramp] = startho2
                        
                        if target == 'tt' or target == 'all':
				gain_tt[n+cycles*steps_ramp] = start + n*step
                
                        if target == 'ho1' or target == 'both' or target == 'all':
				if ratio != None:
					gain_ho1[n+cycles*steps_ramp] = gain_tt[n+cycles*steps_ramp] * ratio
				else:
					gain_ho1[n+cycles*steps_ramp] = start + n*step

                        if target == 'ho2' or target == 'both' or target == 'all':
				if ratio != None:
					gain_ho2[n+cycles*steps_ramp] = gain_tt[n+cycles*steps_ramp] * ratio
				else:
					gain_ho2[n+cycles*steps_ramp] = start + n*step

	gain_tt[steps-1] = gain_tt[0]
	gain_ho1[steps-1] = gain_ho1[0]
	gain_ho2[steps-1] = gain_ho2[0]

	return (gain_tt, gain_ho1, gain_ho2, steps_ramp)

def acquire_sweep( gain_tt,          # numpy array of TT gains to use
                   gain_ho1,          # numpy array of HO gains to use
                   gain_ho2,          # numpy array of HO gains to use
                   middle_ho,
                   slopes_skip,      # How many frames to skip after each gain change
                   slopes_record,    # How many frames to average for each gain step
                   path,             # Gain files path
                   outfile_gains,    # Output filename for applied gains
                   outfile_slopes,   # Output filename for slopes
                   outfile_frames,   # Output filename for frames
                   hdr = {}):

	steps = gain_tt.shape[0]
	gains  = zeros((steps,3), dtype='Float32')
	slopes = zeros((steps,slopes_record,1600), dtype='Float32')

        ff = ccd39.get_frame( type='pixels', diagInterface = intf)
	frames = zeros((steps,slopes_record,ff.shape[0],ff.shape[1]), dtype='Int32')

	for n in range(steps):

		gain = zeros((672), dtype='Float32')
		gain[0:2] = gain_tt[n]
		gain[2:middle_ho] = gain_ho1[n]
                gain[middle_ho:] = gain_ho2[n]
		filename = path+'g_tt_%4.2f_ho1_%4.2f_ho2_%4.2f.fits' % (gain_tt[n], gain_ho1[n], gain_ho2[n])
		fits_lib.writeSimpleFits( filename, gain, overwrite=True)	

		cmd = "print, fsm_set_gain('%s')" % filename
		app.IDLCmd(cmd, idlctrl = 'idlctrl.'+cfg.side+'@M_ADSEC')

		# Record slopes
		s = ccd39.get_frames( slopes_skip + slopes_record, type='slopes', diagInterface = intf)
		f = ccd39.get_frames( slopes_skip + slopes_record, type='pixels', diagInterface = intf)
		slopes[n,:,:] = s[slopes_skip:,:,0]
                frames[n,:,:,:] = f[slopes_skip:,:,:]

		gains[n,0] = gain_tt[n]
		gains[n,1] = gain_ho1[n]
		gains[n,2] = gain_ho2[n]


	fits_lib.writeSimpleFits( outfile_gains, gains, hdr, overwrite=True)
	fits_lib.writeSimpleFits( outfile_slopes, slopes, overwrite=True)
	fits_lib.writeSimpleFits( outfile_frames, frames, overwrite=True)



doit( app.ccd39.xbin())

import time
time.sleep(10)
