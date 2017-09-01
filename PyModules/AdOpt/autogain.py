#@File: gain_optimize.py
#
# Sweep the loop gain and record slopes in the meantime

# First and last gain value, and step, and how many times to repeat the ramp-up

from AdOpt import fits_lib, cfg, setupDevices, thAOApp, calib, AOVar
from AdOpt.wrappers import idl, diagbuf
from AdOpt.AOExcept import *
from numpy import *
import os, time

class autogain:

    def __init__(self, app):
        self.intf = diagbuf.diagbuf( app, 39)
        self.m2cVar = 'ADSEC.'+cfg.side+'.M2C@M_ADSEC'
        self.recVar = 'ADSEC.'+cfg.side+'.B0_A@M_ADSEC'
        self.skipVar = 'adsecarb.'+cfg.side+'.SAFESKIP_PERCENT@M_ADSEC'
        self.modalbasis = app.ReadVar(self.m2cVar).Value().split('/')[-2]
        self.basepath = calib.modalBasisDir(self.modalbasis)
        self.gainpath = self.basepath + 'gain/'
        self.app = app

        self.stopCmd = 'print, fsm_set_gain("gain0.1_10modi.fits")'
        self.stopVarName = 'autogain.'+cfg.side+'.STOP'
        self.stopVar = AOVar.AOVar( self.stopVarName, tipo='INT_VARIABLE', value=0)
        app.WriteVar(self.stopVar)

        self.recFilename = calib.recFile( self.modalbasis, self.app.ReadVar(self.recVar).Value().split('/')[-1])

        self.nmodes = calib.nModes( self.recFilename)

        if self.nmodes == 5:
            self.TTM = True
        else:
            self.TTM = False

        # Default configuration (can be overridden in autogain.conf)
        # How many slope frames to record at each step, and how many to skip after each eain application
        self.slopes_skip   = 10
        self.slopes_record = 28  # This to be in sync with the 13Hz vibration

        # Remove high-force modes from gain vectors
        self.removeBadModes = 1

        # Interpolate mid- and high-order gains
        self.interpolateGains = 1

        # Use retro-reflector: divide all gains and ranges by 2
        self.RR = 0

        # Use sinusoidal IM: divide all gains and ranges by 2
        self.sinusIM = 0

        # Max no. of iterations before giving up
        self.max_iterations = 5

        # Detect safe skip
        # Percentage of sake skip over which the autogain is stopped
        # Values 0-1. If 0, no check is done. If 1, waits for 100% safe skip before stopping
        self.safe_skip = 0.9

        # Repeat threshold in percentage (0..1)
	# If the optimal gain is bigger than this threshold, the cycle is repeated with 50% higher gains
	self.repeat_th = 0.8

	# Reduction factor to apply at the end. 1.0 = no change
	reduction_factor = 0.9





        # Override attributes using autogain.conf file
        conf = cfg.cfg['autogain']
        for k in conf.keys():
            print k, conf[k].Value()
            setattr( self, k, conf[k].Value())
        


        s_date = calib.getDateTime( addTimestamp = False)
        s_time = calib.getDateTime( addTimestamp = True, onlyTime = True)
        self._tn = '%s_%s' % (s_date, s_time)
        self.datapath = '/towerdata/adsec_data/%s/Data_%s/' % (s_date, self._tn)
        os.makedirs(self.datapath)

        self.badmodes = False
        badmodesfile = self.basepath + 'high_force_modes.fits'
        if os.path.exists(badmodesfile):
            self.badmodes = fits_lib.readSimpleFits(badmodesfile)
        else:
            self.removeBadModes=0


        setupDevices.loadDevices( self.app, ['ccd39'], check=True)

    def checkStop(self):
        return self.app.ReadVar( self.stopVarName).Value() != 0

    def tracknum(self):
        return self._tn

    # Zero out high-force modes

    def removeHighForceModes(self, gainv):
        if self.removeBadModes:
            gainv[self.badmodes] = 0
        return gainv

    def define_ramp( self, starttt, startho1, startho2, start, end, step, cycles, ratio=None, target = 'all'):

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

			if gain_ho1[n+cycles*steps_ramp] < 0.2:
				gain_ho1[n+cycles*steps_ramp] = 0.2
			if gain_ho2[n+cycles*steps_ramp] < 0.2:
				gain_ho2[n+cycles*steps_ramp] = 0.2

	gain_tt[steps-1] = gain_tt[0]
	gain_ho1[steps-1] = gain_ho1[0]
	gain_ho2[steps-1] = gain_ho2[0]

	return (gain_tt, gain_ho1, gain_ho2, steps_ramp)

    def acquire_sweep( self, gain_tt,          # numpy array of TT gains to use
                             gain_ho1,          # numpy array of HO gains to use
                             gain_ho2,          # numpy array of HO gains to use
                             middle_ho,
                             path,             # Gain files path
                             outfile_gains,    # Output filename for applied gains
                             outfile_slopes,   # Output filename for slopes
                             hdr = {}):

	steps = gain_tt.shape[0]
	gains  = zeros((steps,3), dtype='Float32')
	slopes = zeros((steps,self.slopes_record,1600), dtype='Float32')

	for n in range(steps):

		gain = zeros((672), dtype='Float32')
		gain[0:2] = gain_tt[n]
		gain[2:middle_ho] = gain_ho1[n]
                gain[middle_ho:] = gain_ho2[n]
                gain = self.removeHighForceModes(gain)
		filename = path+'g_tt_%4.2f_ho1_%4.2f_ho2_%4.2f.fits' % (gain_tt[n], gain_ho1[n], gain_ho2[n])
                #if not os.path.exists(filename):
		fits_lib.writeSimpleFits( filename, gain, overwrite=True)	

		cmd = "print, fsm_set_gain('%s')" % filename
                try:
		    self.app.IDLCmd(cmd, idlctrl = 'idlctrl.'+cfg.side+'@M_ADSEC')
                except:
                    # Try again because every now and then a message is lost
                    self.app.log("Timeout applying gain, retrying...")
		    self.app.IDLCmd(cmd, idlctrl = 'idlctrl.'+cfg.side+'@M_ADSEC')

		# Record slopes
		s = self.app.ccd39.get_frames( self.slopes_skip + self.slopes_record, type='slopes', diagInterface = self.intf)
		slopes[n,:,:] = s[self.slopes_skip:,:,0]

		gains[n,0] = gain_tt[n]
		gains[n,1] = gain_ho1[n]
		gains[n,2] = gain_ho2[n]

		if self.safe_skip>0:
	                skip = self.app.ReadVar( self.skipVar)
			if skip > self.safe_skip:
				raise AdOptError( code = AOConstants.AOErrCode['IDL_SAFE_SKIP_ACTIVE'], errstr = 'Safe skip active - cannot proceed')


		if self.checkStop():
			self.app.IDLCmd(cmd, idlctrl = 'idlctrl.'+cfg.side+'@M_ADSEC')
		        raise AdOptError( code = AOConstants.AOErrCode['WFSARB_AUTOGAIN_USER_STOP'], errstr = 'User stop')


	fits_lib.writeSimpleFits( outfile_gains, gains, hdr, overwrite=True)
	fits_lib.writeSimpleFits( outfile_slopes, slopes, overwrite=True)





    def optimize( self, dataname, starttt, startho1, startho2, start, end, step, cycles, ratio, target='all', set=False, middle_ho=672):

        # Get adsec data

        outfile_gains  = self.datapath + 'gains_'+dataname+'.fits'
        outfile_slopes = self.datapath + 'slopes_'+dataname+'.fits'

        gain_tt, gain_ho1, gain_ho2, steps_ramp = self.define_ramp( starttt, startho1, startho2, start, end, step, cycles, ratio, target)
        hdr = {}
        hdr['CYCLES'] =cycles
        hdr['STEPS_RAMP'] = steps_ramp
        hdr['M2C'] = self.modalbasis
        hdr['REC'] = self.recFilename[ self.recFilename.find('M2C')+4:]

        self.acquire_sweep( gain_tt, gain_ho1, gain_ho2, middle_ho, self.gainpath, outfile_gains, outfile_slopes, hdr)

        idlcmd = "sweep_gain_plot_tt_2ho, GAINSFILE='%s', SLOPESFILE='%s', GAINV=GAINV, GAINTT=GAINTT, GAINHO1=GAINHO1, GAINHO2=GAINHO2, TARGET = '%s',  MIDDLE_HO = %d, /NOSAVE" % (outfile_gains, outfile_slopes, target, middle_ho)
        print idlcmd
        idl.ex(idlcmd)
        gainv = idl.var.gainv
        gaintt = idl.var.gaintt
        gainho1 = idl.var.gainho1
        gainho2 = idl.var.gainho2
        gainv2 = array(gainv) # the original gainv is read-only and writeSimpleFits fails
        filename = self.gainpath+'g_tt_%4.2f_ho1_%4.2f_ho2_%4.2f.fits' % (gaintt, gainho1, gainho2)
        fits_lib.writeSimpleFits( filename, gainv2, overwrite=True)	

        if set:
            cmd = "print, fsm_set_gain('%s')" % filename
            self.app.IDLCmd(cmd)
        return (filename, float(gaintt), float(gainho1), float(gainho2))



    def doit( self, bin):

        start    = getattr(self, 'bin%d_start' % bin)
        end      = getattr(self, 'bin%d_end' % bin)
        step     = getattr(self, 'bin%d_step' % bin)
        cycles   = getattr(self, 'bin%d_cycles' % bin)
        ratio    = getattr(self, 'bin%d_ratio' % bin)
        max_tt   = getattr(self, 'bin%d_max_tt' % bin)
        max_ho1  = getattr(self, 'bin%d_max_ho1' % bin)
        max_ho2  = getattr(self, 'bin%d_max_ho2' % bin)
        ho_middle= getattr(self, 'bin%d_ho_middle' % bin)

	# Use half values for RR since we are in double pass
	if self.RR:
		start /=2
		end   /=2
		step  /=2

	# Use half values for sinusoidal IM since the normalization is different
	if self.sinusIM:
		start /=2
		end   /=2
		step  /=2

        # Higher values for TTM at bin1 or 2
	#if self.TTM:
        if 0:
		start *=2
		end   *=2
		step  *=2
		max_tt *=2
		max_ho1 *=2

	datastep =0
	tt = 0
	ho1 = 0.2
	ho2 = 0.2

	start_orig = start
	end_orig = end

	# First step: TT only with 0.2 on the rest
	iterations=0
	while 1:
		datastep +=1
		f, tt, tmp1, tmp2 = self.optimize( 'step%d'%datastep, tt, ho1, ho2, start, end, step, cycles, None, 'tt', set=True, middle_ho=ho_middle)
                repeat_gain = start + (end-start)*self.repeat_th
		if tt < repeat_gain:
			break
		if tt > max_tt:
			tt = max_tt
			break
		iterations+=1
		if iterations > self.max_iterations:
			break
		myrange = (end-start)
		start += myrange/2
		end += myrange/2

	start = start_orig
	end = end_orig

	# Second step: HO1 only
	iterations=0
	while 1:
		datastep +=1
		f, tmp1, ho1, tmp2 = self.optimize( 'step%d'%datastep, tt, ho1, ho2, start, end, step, cycles, None, 'ho1', set=True, middle_ho=ho_middle)
                repeat_gain = start + (end-start)*self.repeat_th
		if ho1 < repeat_gain:
			break
		if ho1 > max_ho1:
			ho1 = max_ho1
			break
		iterations+=1
		if iterations > self.max_iterations:
			break
		myrange = (end-start)
		start += myrange/2
		end += myrange/2

	start = start_orig
	end = end_orig

	# Third step: HO2 only
        if not self.TTM:
		iterations=0
		while 1:
			datastep +=1
			f, tmp1, tmp2, ho2 = self.optimize( 'step%d'%datastep, tt, ho1, ho2, start, end, step, cycles, None, 'ho2', set=True, middle_ho=ho_middle)
			repeat_gain = start + (end-start)*self.repeat_th
			if ho2 < repeat_gain:
				break
			if ho2 > max_ho2:
				ho2 = max_ho2
				break
			iterations+=1
			if iterations > self.max_iterations:
				break
			myrange = (end-start)
			start += myrange/2
			end += myrange/2

        # Reduce everything by the reduction factor
        tt *= self.reduction_factor
        #ho1 *= self.reduction_factor
        #ho2 *= self.reduction_factor

	# Generate gain vector

        if (not self.interpolateGains) or (self.TTM):
            gainv = array([0.0]*672, dtype='Float32')
            gainv[0:2] = tt
            if not self.TTM:
                gainv[2:ho_middle] = ho1
                gainv[ho_middle:] = ho2
            else:
                gainv[2:] = ho1
        else:
            gainv = array([0.0]*672, dtype='Float32')
            gainv[0:2] = tt
            xg1 = (2+ho_middle)/2
            xg2 = (ho_middle+self.nmodes)/2
            alpha  = (ho2-ho1) / (xg2-xg1)
            for g in range(self.nmodes-2):
                gainv[g+2] = ho1+ alpha*(g-xg1)

        gainv = self.removeHighForceModes(gainv)

        filename = self.gainpath+'g_tt_%4.2f_ho1_%4.2f_ho2_%4.2f.fits' % (tt, ho1, ho2)
        fits_lib.writeSimpleFits( filename, gainv, overwrite=True)	
        cmd = "print, fsm_set_gain('%s')" % filename
        self.app.IDLCmd(cmd)


if __name__ == "__main__":
    pass
#    app = thAOApp('acq')
#    a = autogain(app)
#    a.doit( app.ccd39.xbin())
