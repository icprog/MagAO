#!/usr/bin/env python
#@File: ccd.py
#
# Python generic class for a ccd.
#@


from hw_ctrl import *
import thread, threading
import types

import numpy
from AdOpt.wrappers import diagbuf
from AdOpt import frames_lib, fits_lib, calib

#@Class: ccd
#
# Python generic class for a CCD
#
# Wrappers CCD functionality for both the HO and TV arms.
# The API is geared towards SciMeasure-type controllers.
#
# All methods raise exception in case errors are encountered.
#@

class ccd(hw_ctrl):

    #@Method{API}: __init_
    #
    # Constructor of the ccd object
    #
    # Raises an exception in case of error, or in case one of the checks specified in the constructor fails.
    #@

    def __init__(self, name,                 #@P{name}: ccd name (either 'ccd39' or 'ccd47')
                       aoapp,                #@P{aoapp}: calling aoapp object
                       timeout_s = 4,        #@P{timeout_s}: timeout for basic operations, in seconds
                       ctrl_check = True,    #@P{ctrl_check}: checks whether the hardware controller is operating correctly
                        power_check = True): #@P{power_check}: checks whether the ccd is powered on correctly
                                             #@R: None

        self._timeout_s = timeout_s 

        self.ccdGain = {}
        self.ccdGain[0] = 'High'
        self.ccdGain[1] = 'Med. Hi'
        self.ccdGain[2] = 'Low. Hi'
        self.ccdGain[3] = 'Low'

        hw_ctrl.__init__( self, name, aoapp, ctrl_check, power_check)

        # Initialize local variable objects
        try:
            self.ccdnum    = cfg.cfg[name]['ccdNum'].Value()

            self.v_enable  = self.app.ReadVar( self.varname('ENABLE','REQ'))
            self.v_enabled  = self.app.ReadVar( self.varname('ENABLE','CUR'))

            self.v_xbin    = self.app.ReadVar( self.varname('XBIN', 'REQ'))
            self.v_ybin    = self.app.ReadVar( self.varname('YBIN', 'REQ'))
            self.v_speed   = self.app.ReadVar( self.varname('SPEED', 'REQ'))
            self.v_black   = self.app.ReadVar( self.varname('BLACK', 'REQ'))
            self.v_rep     = self.app.ReadVar( self.varname('REP', 'REQ'))
            self.v_gain    = self.app.ReadVar( self.varname('GAIN', 'REQ'))

            self.v_curxbin    = self.app.ReadVar( self.varname('XBIN', 'CUR'))
            self.v_curybin    = self.app.ReadVar( self.varname('YBIN', 'CUR'))
            self.v_curspeed   = self.app.ReadVar( self.varname('SPEED', 'CUR'))
            self.v_curblack   = self.app.ReadVar( self.varname('BLACK', 'CUR'))
            self.v_currep     = self.app.ReadVar( self.varname('REP', 'CUR'))
            self.v_curgain    = self.app.ReadVar( self.varname('GAIN', 'CUR'))
            self.v_curframerate     = self.app.ReadVar( self.varname('FRMRT', 'CUR'))
            self.v_speedlist   = self.app.ReadVar( self.varname('SPEEDS'))

            self.v_dx      = self.app.ReadVar( self.varname('DX'))
            self.v_dy      = self.app.ReadVar( self.varname('DY'))
            
            self.app.VarNotif( self.v_enabled)
            self.app.VarNotif( self.v_curxbin)
            self.app.VarNotif( self.v_curybin)
            self.app.VarNotif( self.v_curspeed)
            self.app.VarNotif( self.v_curblack)
            self.app.VarNotif( self.v_currep)
            self.app.VarNotif( self.v_curframerate)
            self.app.VarNotif( self.v_curgain)
            self.app.VarNotif( self.v_dx)
            self.app.VarNotif( self.v_dy)
            self.app.VarNotif( self.v_speedlist)

            self._lastDarkName = ''
            self._lastDark = None

        except AOVarNotInRTDB, e:
            print e.errstr
            raise
            errstr =  "Controller process %s is not working properly" % cfg.taskname(name)
            raise AOMissingProcess, ( errstr, cfg.taskname(name))

        #@C
        # power check is disabled at the moment.
        #@C

        #try:
        #    self.v_pwr_req  = self.app.ReadVar( self.other_varname( 'PWR', 0, self.name, 'REQ'))
        #    self.v_pwr_cur  = self.app.ReadVar( self.other_varname( 'PWR', 0, self.name, 'CUR'))
        #except AOVarNotInRTDB, e:
        #    print e.errstr
        #    errstr =  "Controller process %s is not working properly" % cfg.taskname('power')
        #    raise AOMissingProcess, ( errstr, cfg.taskname('power'))

        print self.name+" interface loaded"

    #@Method{API}: curDark
    #
    # Returns the current dark file.
    #@

    def curDark(self):     #R: current dark filename (string)
        if self.name == 'ccd39':
            if not hasattr(self.app, 'sc'):
                #setupDevices.loadDevices( app, ['sc'], check = True)
                pass
            return self.app.sc.get_dark()
        elif self.name == 'ccd47':
            if not hasattr(self.app, 'tv'):
                #setupDevices.loadDevices( app, ['tv'], check = True)
                pass
            return self.app.tv.get_dark()

        return self._lastDarkName

    #@Method{API}: enabled
    #
    # Returns whether the ccd is enabled (integrating frames).
    #@

    def enabled(self):     #R: 1 if ccd is enabled, 0 otherwise.
        if not self.isOnline():
            raise InvalidStateException(ctrl=self)

        return self.v_enabled.Value()

    #@Method{API}: xbin
    #
    #       Returns the current ccd X binning.
    #@

    def xbin(self):        #R: current ccd binning (from 1 to 16 depending on ccd type)
        if not self.isOnline():
            raise InvalidStateException(ctrl=self)

        return self.v_curxbin.Value()

    #@Method{API}: ybin
    #
    #       Returns the current ccd Y binning.
    #@

    def ybin(self):        #R: current ccd binning (from 1 to 16 depending on ccd type)
        if not self.isOnline():
            raise InvalidStateException(ctrl=self)

        return self.v_curybin.Value()

    #@Method{API}: dx
    #
    #       Returns the ccd x dimension (not binned).
    #@

    def dx(self):         #R:  ccd dimension in pixels(from 80 to 1024 depending on ccd type)
        return self.v_dx.Value()

    #@Method{API}: dy
    #
    #       Returns the ccd y dimension (not binned).
    #@

    def dy(self):         #R:  ccd dimension in pixels(from 80 to 1024 depending on ccd type)
        return self.v_dy.Value()


    #@Method{API}: speed
    #
    #       Returns the current ccd readout speed
    #@

    def speed(self):         #R:  ccd readout speed in kpixel/sec
        if not self.isActive():
            raise InvalidStateException(ctrl=self)

        return self.v_curspeed.Value()

    #@Method{API}: speedlist
    #
    #       Returns the current list of available speeds
    #@

    def speedlist(self):         #R:  list of ccd readout speeds in kpixel/sec
        if not self.isActive():
            raise InvalidStateException(ctrl=self)

        return filter( None, self.v_speedlist.Value())



    #@Method{API}: blacks
    #
    #       Returns the current ccd black levels
    #@

    def blacks(self):          #R: tuple of 2 or 4 values from 0 to 1023 (depending on ccd type)
        if not self.isActive():
            raise InvalidStateException(ctrl=self)

        return self.v_curblack.Value()

    #@Method{API}: reps
    #
    #       Returns the current ccd repetition delay
    #@

    def reps(self):           #R: current ccd repetition delay from 0 to 65535
        if not self.isActive():
            raise InvalidStateException(ctrl=self)

        return self.v_currep.Value()

#    Not used, see the agw_power class
#
#    def turnon(self, waitready = True, waitready_timeout = 60):
#        if self.app.ReadVar(self.v_pwr_cur).Value() == 1:
#            return
#        self.v_pwr_req.SetValue(1)
#        self.app.WriteVar( self.v_pwr_req, nocreate = True)
#        self.app.WaitVarValue( self.v_pwr_cur, 1, timeout = self._timeout_s)
#
#        if waitready:
#            self.app.WaitVarValue( self.v_status, cfg.state_ready, timeout= waitready_timeout)
#
#    def turnoff(self):
#        if self.app.ReadVar(self.v_pwr_cur).Value() == 0:
#            return
#        self.v_pwr_req.SetValue(0)
#        self.app.WriteVar( self.v_pwr_req, nocreate = True)
#        self.app.WaitVarValue( self.v_pwr_cur, 0, timeout = self._timeout_s)



    #@Method{API}: start
    #
    #       Starts CCD integration.
    #@

    def start(self):
        if not self.checkPolicy():
            return

        if not self.isActive():
            raise InvalidStateException(ctrl=self)

        self.v_enable.SetValue(1)
        self.app.WriteVar( self.v_enable, nocreate = True)
        self.app.WaitVarValue( self.v_enabled, 1, timeout = self._timeout_s)

    #@Method{API}: stop
    #
    #       Stop CCD integration.
    #@

    def stop(self):
        if not self.checkPolicy():
            return

        if not self.isActive():
            raise InvalidStateException(ctrl=self)

        self.v_enable.SetValue(0)
        self.app.WriteVar( self.v_enable, nocreate = True)
        self.app.WaitVarValue( self.v_enabled, 0, timeout = self._timeout_s)

    #@Method{API}: reprogram
    #
    # Repograms the ccd to a different configuration of binning and readout speed.
    # May block for a significant time (worst case about a minute).
    #
    # Integration is automatically restarted if the ccd was integrating before the reprogramming.
    #
    # Invalid values for binning and/or speed will cause the timeout to expire and a timeout exception.
    #@
       
    def reprogram( self, xbin=None,      #@P(xbin): requested x binning
                         ybin=None,      #@P{ybin}: requested y binning
                         speed=None,     #@P{speed}: requested readout speed
                         gain=None,      #@P{gain}: requested gain
                         timeout = 60):  #@P{timeout}: requested y binning
                                         #@R: None

        if not self.checkPolicy():
            return

        if not self.isOnline():
            raise InvalidStateException(ctrl=self)

        state = self.isrunning()
        self.stop()

        if gain != None:
            self.v_gain.SetValue(gain)
            self.app.WriteVar( self.v_gain, True)
        if xbin:
            self.v_xbin.SetValue(ybin)
            self.app.WriteVar( self.v_xbin, True)
        if ybin:
            self.v_ybin.SetValue(ybin)
            self.app.WriteVar( self.v_ybin, True)
        if speed:
            self.v_speed.SetValue(speed)
            self.app.WriteVar( self.v_speed, True)

        try:
            if xbin:
                self.app.WaitVarValue( self.v_curxbin, self.v_xbin.Value(), timeout = timeout)
            if ybin:
                self.app.WaitVarValue( self.v_curybin, self.v_ybin.Value(), timeout = timeout)
            if speed:
                self.app.WaitVarValue( self.v_curspeed, self.v_speed.Value(), timeout = timeout)
            if gain:
                self.app.WaitVarValue( self.v_curgain, self.v_gain.Value(), timeout = timeout)
        except AdOptError, e:
            raise AdOptError(errstr= 'Problem configuring CCD. Check ccd controller log')


        if state:
            self.start()

    #@Method{API}: isrunning
    #
    #        Return True if the ccd is currently integrating.
    #@

    def isrunning(self):
        return self.app.ReadVar( self.v_enabled.Name()).Value()

    def set_default_black(self):
        self.set_black( (500,500,500,500))

    def set_black( self, values):
        '''
            Sets a new value for black levels.
        '''
        if not self.checkPolicy():
            return

        if not self.isOnline():
            raise InvalidStateException(ctrl=self)

        self.v_black.SetValue(values)
        self.app.WriteVar( self.v_black, nocreate = True)
        self.app.WaitVarValue( self.v_curblack, values, timeout = self._timeout_s*2)

    def get_black(self):
        '''
            Returns current black levels.
        '''
        if not self.isOnline():
            raise InvalidStateException(ctrl=self)

        return self.app.ReadVar(self.v_curblack).Value()

    def get_gain(self):
        '''
            Returns current black levels.
        '''
        if not self.isOnline():
            raise InvalidStateException(ctrl=self)

        return self.app.ReadVar(self.v_curgain).Value()

    def set_gain(self, value):
        '''
            Set a new repetition delay
        '''
        if not self.checkPolicy():
            return

        if not self.isActive():
            raise InvalidStateException(ctrl=self)

        int_v = int(value)
        self.v_gain.SetValue(int_v)
        self.app.WriteVar( self.v_gain, nocreate = True)
        self.app.WaitVarValue( self.v_curgain, int_v, timeout = self._timeout_s)

    # Convert a SciMeasure numerical gain setting to a string

    def gain2Str( self, gain):
        try:
            return self.ccdGain[gain]
        except KeyError:
            return 'invalid'

    def set_rep(self, value):
        '''
            Set a new repetition delay
        '''
        if not self.checkPolicy():
            return

        if not self.isActive():
            raise InvalidStateException(ctrl=self)

        int_v = int(value)
        self.v_rep.SetValue(int_v)
        self.app.WriteVar( self.v_rep, nocreate = True)
        self.app.WaitVarValue( self.v_currep, int_v, timeout = self._timeout_s)

    def get_rep( self):
        '''
            Returns current repetition delay
        '''
        if not self.isActive():
            raise InvalidStateException(ctrl=self)

        return self.app.ReadVar(self.v_currep).Value()

    def get_framerate( self):
        '''
            Returns current repetition delay
        '''
        if not self.isActive():
            raise InvalidStateException(ctrl=self)

        return self.app.ReadVar(self.v_curframerate).Value()

    def set_framerate( self, framerate, change_speed=False):
        '''
        Sets the requested framerate without changing the current binning.
        Framerate is in Hz

        If change_speed is True, will select the slowest available
        readout speed which enables the requested framerate. Otherwise, the
        current readout speed is unchanged.
    
        Returns the actual framerate (which could be slightly different due to
        controller limitations).
        '''
        if not self.checkPolicy():
            return

        if not self.isActive():
            raise InvalidStateException(ctrl=self)

        xbin, ybin = self.get_binning()

        framerate = float(framerate)
        min_readout = 1e9
        np = cfg.get_cfg_value( self.name, 'num_programsets')
        for p in range(np):
            np1 = cfg.get_cfg_value( self.name, ['programset%d' % p, 'num_programs'])
            for p1 in range(np1):
                base = 'programset%d.program%d' % (p, p1)
                p_xbin     = cfg.get_cfg_value( self.name, [base, 'binx'])
                p_ybin     = cfg.get_cfg_value( self.name, [base, 'biny'])
                delay_base = cfg.get_cfg_value( self.name, [base, 'delay_base'])
                delay_inc  = cfg.get_cfg_value( self.name, [base, 'delay_inc'])
                speed      = cfg.get_cfg_value( self.name, [base, 'readout_speed'])

                if p_xbin != xbin or p_ybin != ybin:
                    continue

                if (change_speed==False) and (speed != self.speed()):
                    continue

                # See if the requested framerate is achievable
                max_freq = 1.0/float(delay_base) * 1e6
                min_freq = 1.0/(float(delay_base) + float(delay_inc)*65536) * 1e6
                if framerate < min_freq or framerate > max_freq:
                    continue

                # See if the readout speed is good
                if speed > min_readout:
                    continue

                min_readout = speed
                min_delay   = delay_base
                min_inc     = delay_inc

        if min_readout == 1e9:
            raise AdOptError, (AOErrCode['VALUE_OUT_OF_RANGE_ERROR'], \
                'Framerate %d is out of range for binning %dx%d' % (framerate, xbin, ybin))

        rep = round((1.0/framerate * 1e6 - min_delay) / min_inc)

        self.app.debug( "selected program %dx%d, %d kpix/sec" % (xbin, ybin, min_readout))
        self.app.debug( "Delay base %d, delay inc %d, repetitions %d" % (min_delay, min_inc, rep))
        self.app.debug( "Real framerate %f Hz" %  (1/(min_delay + min_inc*rep) * 1e6))

        # Select the chosen program
        self.reprogram( xbin, ybin, min_readout)

        # Set the correct repetition number
        self.set_rep(rep)

        return 1/(min_delay + min_inc*rep) * 1e6

    def get_binning(self):
        '''
        Returns current X by Y binning
        '''
        if not self.isActive():
            raise InvalidStateException(ctrl=self)

        return ( self.app.ReadVar(self.v_curxbin).Value(), self.app.ReadVar(self.v_curybin).Value()) 

    def get_frames(self, num, type = 'pixels', diagInterface = None, callback = None, subtractDark=True, average=False, useFloat=False, timeout=1000):
        '''
        Reads 'num' consecutive frames from ccd, returns a 3D array.
        callback is a function to call at each frame read, it will be called
        with the current frame number
        '''
        if not self.isActive():
            raise InvalidStateException(ctrl=self)

        if num == 1:
            return self.get_frame(type = type, subtractDark = subtractDark, timeout = timeout)

        cleanIntf = False
        if not diagInterface:
            diagInterface = diagbuf.diagbuf( self.app, self.ccdnum)
            cleanIntf = True

        # Read 1 frame to see the format and data type
        f = self.get_frame( diagInterface = diagInterface, type = type, subtractDark=subtractDark, timeout = timeout)
        s = f.shape

        if average:
            t = f.dtype
            out_t = t
            if useFloat:
                t = numpy.float32
                out_t = t

            # Promote Int16 array to Int32 to avoid overflows
            if t == numpy.int16:
                t = numpy.int32

            avg = numpy.zeros((s[0], s[1]), dtype = t)
            avg += f.astype(t)
        else:
            frames = numpy.zeros((num, s[0], s[1]), dtype = f.dtype)
            frames[0,:,:] = f

        for i in range(num-1):
            if average:
                avg += self.get_frame( timeout = timeout, diagInterface = diagInterface, type = type, subtractDark = subtractDark).astype(t)
            else:
                frames[i+1, :, :] = self.get_frame( diagInterface = diagInterface, type = type, subtractDark = subtractDark, timeout = timeout)
            if callback != None:
                callback( self.app, i)

        if cleanIntf:
            diagInterface.release() 


        if average:
            avg /= num
            return avg.astype(out_t)

        return frames

    def get_sharedbuf(self):
        '''
        Override to get the actual framebuffer.
        '''
        return cfg.ccd_framebuffer[self.ccdnum]

    #@Method{API}: get_frame
    #
    # Gets a single frame from the diagnostic stream associated with the CCD. 
    # Data type can be one of several in case of the HO ccd (pixels, slopes, etc).
    # A dark file can be optionally subtracted.
    #@

    def get_frame(self, timeout=1000,          #@P{timeout}: timeout in milliseconds
                        type = 'pixels',       #@P{type}: can be 'pixels', 'slopes', 'slopes_raster' or 'background'. The last case is identical to 'pixels', except that background parameters are ignored.
                        diagInterface = None,  #@P{diagInterface}: diagnostic interface object. If not given, a new one will be setup
                        subtractDark=True,     #@P{subtractDark}: the current slopecomputer background will be be subtracted if <type> if 'pixels', otherwise it has no effects.
                        refreshData=True,      #@P{refreshData}: if true, wait for the next frame to be downloaded, otherwise it will recycle previous data.
                        cachedDark=False):     #@P{cachedDark}: is set, the last background is used, otherwise a new one is reloaded from the current slope computer settings.
                                               #@R: 2D numpy object, of type uint16 (for pixels) or float32 (for slopes)
        if not self.isActive():
            raise InvalidStateException(ctrl=self)

        to_read = []
        read = []

        cleanIntf = False
        if not diagInterface:
            diagInterface = diagbuf.diagbuf( self.app, self.ccdnum)
            cleanIntf = True

        if type == 'pixels' or type=='background':
            f = diagInterface.pixelsRaster( refresh = refreshData, timeout = timeout)
        elif type == 'slopes':
            f = diagInterface.slopesRaw( refresh = refreshData, timeout = timeout)
        elif type == 'slopes_raster':
            f = diagInterface.slopesRaster( refresh = refreshData, timeout = timeout)
        else:
            raise AdOptError( AOConstants.AOErrCode['ERROR_VALUE_OUT_OF_RANGE'], "Unknown data type '%s'", type)

        # Subtract background if requested
        if (type == 'pixels') and (subtractDark == True):
            if cachedDark==False:
		        # Get the right dark!
                curDark = ''
                if self.name == 'ccd39':
                    if hasattr(self.app, 'sc'):
                        curDark = self.app.sc.get_dark()
                elif self.name == 'ccd47':
                    if hasattr(self.app, 'tv'):
                        curDark = self.app.tv.get_dark()

                if curDark != self._lastDarkName and curDark != '':
                    self._lastDarkName = curDark
                    self._lastDark = fits_lib.readSimpleFits( calib.BackgDir(self.name, self.xbin())+'/'+self._lastDarkName)
            if self._lastDarkName != '':
                #print 'ccd.get_frame(): using dark %s' % self._lastDarkName
                f = f - self._lastDark
                #f[ numpy.where(f <0)] = 0
            else:
                print 'ccd.get_frame(): no dark subtraction'

        # Cleanup and return
        if cleanIntf:
            diagInterface.release() 
        return f


    def get_detector_string( self):
        return self.name.upper()

    def split_quadrants( self, frame):
        '''
        Splits the four ccd quadrants.
        '''
        return frames_lib.splitQuadrants(frame)
        

    def equalize_quadrants(self, target=1000, target_threshold=20.0, delay=0.2):
        '''
        Equalize the current luminosity level to the target value.
        '''
        if not self.checkPolicy():
            return

        if not self.isActive():
            raise InvalidStateException(ctrl=self)

        ###################
        # Start config

        #
        # Estimate 1 black level = 20 counts
        ratio = 1.0/20

        min_level = 0
        max_level = 1023

        max_iterations = 200

        #
        # End config
        #####################

        self.app.log("ccd.equalize_quadrants(): Current settings: bin %d, readout speed %d kpix/sec" % (self.xbin(), self.speed()))

#        blacks = list(self.get_black())
#        for i in range(len(blacks)):
#            blacks[i] = 600
#        self.set_black(tuple(blacks))

        self.app.log("ccd.equalize_quadrants(): Starting values: "+str( self.get_black()))

        counter=0
        while 1:

            cur_blacks = self.get_black()

            # Wait for the next frame
            quadrants = self.split_quadrants( self.get_frame(subtractDark=False))

            diff = [ (x.mean()-target)*ratio for x in quadrants]

            absdiff = map( abs, diff)
            if max(absdiff) < target_threshold*ratio:
                self.app.log("ccd.equalize_quadrants(): Final values: "+str(cur_blacks))
                return

            new_blacks = tuple( [ int(cur_blacks[n] + diff[n]) for n in range(len(cur_blacks))] )

            if len(filter( lambda x: x < min_level, new_blacks)) > 0 or \
               len(filter( lambda x: x > max_level, new_blacks)) > 0:
                raise AdOptError( AOConstants.AOErrCode['ERROR_VALUE_OUT_OF_RANGE'], "Black level out of range")

            ntry = 2
            while 1:
                try:
                    self.set_black(new_blacks)
                    break
                except AdOptError:
                    ntry -= 1
                    if ntry >0:
                        self.app.log("ccd.set_black(): Retry")
                        continue
                     

            if delay>0:
                time.sleep(delay)

            counter += 1
            if counter >= max_iterations:
                self.app.log("Equalize quadrants FAILED after %d iterations", max_iterations)
                raise AdOptError( AOConstants.AOErrCode['TIMEOUT_ERROR'], 'Giving up after %d iterations' % max_iterations)



