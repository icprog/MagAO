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
slopes_record = 20

s_date = calib.getDateTime( addTimestamp = False)
s_time = calib.getDateTime( addTimestamp = True, onlyTime = True)
datapath = '/towerdata/adsec_data/%s/Data_%s_%s/' % (s_date, s_date, s_time)
os.makedirs(datapath)

def doit( bin):

	if bin == 1:
		start = 0.2 # LB 20111115 it was 0.1
		end   = 1.0
		step  = 0.1  
		cycles = 5
		ratio = 0.75 

	if bin == 2:
		start = 0.1
		end   = 0.8
		step  = 0.1  
		cycles = 10
		ratio = 0.71 

	if bin == 3:
		start = 0.1
		end   = 2.0
		step  = 0.2  
		cycles = 10
		ratio = 0.61 

	if bin == 4:
		start = 0.1
		end   = 1.5
		step  = 0.2
		cycles = 20
		ratio = 0.61 

        # Repeat threshold in percentage (0..1)
	# If the optimal gain is bigger than this threshold, the cycle is repeated with 50% higher gains
	repeat_th = 0.8

	datastep =0
	tt = 0
	ho = 0

	start_orig = start
	end_orig = end

	# First step: TT and HO together
	while 1:
		datastep +=1
		f, tt, ho = optimize( datapath, 'step%d'%datastep, slopes_skip, slopes_record, tt, ho, start, end, step, cycles, True, ratio, True, set=True)
                repeat_gain = start + (end-start)*repeat_th
		if tt < repeat_gain and ho < repeat_gain:
			break
		range = (end-start)
		start += range/2
		end += range/2

	start = start_orig
	end = end_orig

	# Second step: TT only
	while 1:
		datastep +=1
		f, tt, ho = optimize( datapath, 'step%d'%datastep, slopes_skip, slopes_record, tt, ho, start, end, step, cycles, True, None, False, set=True)
                repeat_gain = start + (end-start)*repeat_th
		if tt < repeat_gain:
			break
		range = (end-start)
		start += range/2
		end += range/2

	start = start_orig
	end = end_orig

	# Third step: HO only
	while 1:
		datastep +=1
		f, tt, ho = optimize( datapath, 'step%d'%datastep, slopes_skip, slopes_record, tt, ho, start, end, step, cycles, False, None, True, set=True)
                repeat_gain = start + (end-start)*repeat_th
		if ho < repeat_gain:
			break
		range = (end-start)
		start += range/2
		end += range/2
	




def optimize( datapath, dataname, slopes_skip, slopes_record, starttt, startho, start, end, step, cycles, use_tt, ratio, use_ho, set=False):

        # Get adsec data
        modalbasis = app.ReadVar(m2cVar).Value().split('/')[-2]
	recpath = 'calib/adsec/current/M2C/%s/RECs/' % modalbasis
        rec = recpath + app.ReadVar(recVar).Value().split('/')[-1]
	print rec

	gainpath = '/towerdata/adsec_calib/M2C/%s/gain/' % modalbasis
        outfile_gains  = datapath + 'gains_'+dataname+'.fits'
        outfile_slopes = datapath + 'slopes_'+dataname+'.fits'

	gain_tt, gain_ho, steps_ramp = define_ramp( starttt, startho, start, end, step, cycles,
                                        use_tt, ratio, use_ho)

	hdr = {}
	hdr['CYCLES'] =cycles
	hdr['STEPS_RAMP'] = steps_ramp
	hdr['M2C'] = modalbasis
	hdr['REC'] = rec[ rec.find('M2C')+4:]

        acquire_sweep( gain_tt, gain_ho, slopes_skip, slopes_record, gainpath, outfile_gains, outfile_slopes, hdr)

	idlcmd = "sweep_gain_plot_modal, GAINSFILE='%s', SLOPESFILE='%s', GAINV=GAINV, GAINTT=GAINTT, GAINHO = GAINHO, /NOSAVE" % (outfile_gains, outfile_slopes)
	print idlcmd
	idl.ex(idlcmd)
	gainv = idl.var.gainv
	gaintt = idl.var.gaintt
	gainho = idl.var.gainho
        gainv2 = array(gainv) # the original gainv is read-only and writeSimpleFits fails
	filename = gainpath+'g_tt_%4.2f_ho_%4.2f.fits' % (gaintt, gainho)
	fits_lib.writeSimpleFits( filename, gainv2, overwrite=True)	

	if set:
		cmd = "print, fsm_set_gain('%s')" % filename
		app.IDLCmd(cmd)
	return (filename, gaintt, gainho)


def define_ramp( starttt, startho, start, end, step, cycles,
                 use_tt = True, ratio=None, use_ho= False):

	steps_ramp = round((end-start)/step)
	steps = steps_ramp * cycles+1

	gain_tt = zeros((steps), dtype='Float32')
	gain_ho = zeros((steps), dtype='Float32')

	for cycles in range(cycles):
		for n in range(steps_ramp):
			if use_tt:
				gain_tt[n+cycles*steps_ramp] = start + n*step
			else:
				gain_tt[n+cycles*steps_ramp] = starttt
			if use_ho:
				if ratio != None:
					gain_ho[n+cycles*steps_ramp] = gain_tt[n+cycles*steps_ramp] * ratio
				else:
					gain_ho[n+cycles*steps_ramp] = start + n*step
			else:
				gain_ho[n+cycles*steps_ramp] = startho
				
	gain_tt[steps-1] = gain_tt[0]
	gain_ho[steps-1] = gain_ho[0]

	return (gain_tt, gain_ho, steps_ramp)

def acquire_sweep( gain_tt,          # numpy array of TT gains to use
                   gain_ho,          # numpy array of HO gains to use
                   slopes_skip,      # How many frames to skip after each gain change
                   slopes_record,    # How many frames to average for each gain step
                   path,             # Gain files path
                   outfile_gains,    # Output filename for applied gains
                   outfile_slopes,   # Output filename for slopes
                   hdr = {}):

	steps = gain_tt.shape[0]
	gains  = zeros((steps,2), dtype='Float32')
	slopes = zeros((steps,slopes_record,1600), dtype='Float32')

	for n in range(steps):

		gain = zeros((672), dtype='Float32')
		gain[0:2] = gain_tt[n]
		gain[2:] = gain_ho[n]
		filename = path+'g_tt_%4.2f_ho_%4.2f.fits' % (gain_tt[n], gain_ho[n])
		fits_lib.writeSimpleFits( filename, gain, overwrite=True)	

		cmd = "print, fsm_set_gain('%s')" % filename
		app.IDLCmd(cmd, idlctrl = 'idlctrl.'+cfg.side+'@M_ADSEC')

		# Record slopes
		s = ccd39.get_frames( slopes_skip + slopes_record, type='slopes', diagInterface = intf)
		slopes[n,:,:] = s[slopes_skip:,:,0]

		gains[n,0] = gain_tt[n]
		gains[n,1] = gain_ho[n]


	fits_lib.writeSimpleFits( outfile_gains, gains, hdr, overwrite=True)
	fits_lib.writeSimpleFits( outfile_slopes, slopes, overwrite=True)



doit( app.ccd39.xbin())
