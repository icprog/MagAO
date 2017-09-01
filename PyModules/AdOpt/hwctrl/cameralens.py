#+File: cameralens.py
#
# Python interface for the camera lens XY stage
#

from hw_ctrl import *
from AdOpt import AOVar, calib, fits_lib

import thread


class CameraLens(hw_ctrl):
    '''
        Python interface for the camera lens XY stage.
        
        The API allow to:
            - Construct a CameraLens interface, given a 'name' and an 'AOApp' 
              instance.
              ***Note*** that the name is used to retrieve the camera lens configuration
              so it must match a 'name' defined in cfg.py. 

            - moveTo(): move camera lens to the given position (with optional wait)
            - moveBy(): offset camera lens by the specified amount (with optional wait)
            - pos(): return current camera lens position as an (x,y) tuple.
            - setTracking(): enables automatic tracking of pupil rerotator position
            - isTracking(): returns True if tracking is enabled

            Unit is always millimeters.

    '''
    def __init__(self, name, aoapp, ctrl_check = True, power_check = False):
        
        self._curLock = thread.allocate_lock()

        try: 
            hw_ctrl.__init__( self, name, aoapp, ctrl_check, power_check)
            
             # Initialize the REQ and CUR local RTDBVar 
            self._posxReq = self.app.ReadVar(self.varname('POSX', 'REQ'))
            self._posyReq = self.app.ReadVar(self.varname('POSY', 'REQ'))
            self._posxCur = self.app.ReadVar(self.varname('POSX', 'CUR'))
            self._posyCur = self.app.ReadVar(self.varname('POSY', 'CUR'))

            # Ask to be notified on cur change
            self.app.VarNotif(self._posxCur.Name(), self.__curPosChanged)
            self.app.VarNotif(self._posyCur.Name(), self.__curPosChanged)


             
        except AOMissingProcess, e:
            raise HwCtrlCreationException(e.code, e.errstr)
        except KeyError, e:
            raise HwCtrlCreationException(AOErrCode['UNEXISTING_KEY_ERROR'], "missing key %s in config file" % e)
        except AOVarNotInRTDB, e:
             raise SimpleMotorCreationException(AOErrCode['VAR_NOT_FOUND_ERROR'], e.errstr)
        except AOVarError, e:
           raise SimpleMotorCreationException(None, e.errstr)

        self._rerotPos = 0
        self._rerotTracking = False

        # Create tracking status variable
        self.trackVar = AOVar.AOVar(name= self.varname('TRACKING', 'CUR'), tipo="INT_VARIABLE", value=0)
        self.app.CreaVar( self.trackVar)

        # Pupil rerotator-related setup. Errors here are not fatal
        try:
            self._rerotLock = thread.allocate_lock()
            self._rerotVarName = cfg.side+".Rerotator00.POS.CUR"
            self.v_rerotPosCur = aoapp.ReadVar( self._rerotVarName)

            # Ask to be notified on derotator position change
            aoapp.VarNotif(self._rerotPosCur.Name(), self.__rerotPosChanged)

            self._rerotValid = True
        except:
            self.app.log('Warning: interface with derotator not working. Camera lens will not follow pupil rerotator position')

        
        print "Camera lens interface loaded"

    def moveTo(self, x, y, waitTimeout = None):
        '''
            Set camera lens position. Position is in mm, timeout in seconds.
        '''

        self.app.SetVar( self._posxReq, float(x))
        self.app.SetVar( self._posyReq, float(y))

        if waitTimeout:
            self.app.WaitVarValue( self._posxCur, x, timeout = waitTimeout)
            self.app.WaitVarValue( self._posyCur, y, timeout = waitTimeout)

    def moveBy(self, x, y, waitTimeout = None):
        '''
            Offset camera lens position. Unit is mm, timeout in seconds.
        '''

        self.moveTo( self.posx() + x, self.posy() + y, waitTimeout = waitTimeout)

    def posx(self):
        '''
            Returns the current camera lens X position.
        '''

        self._curLock.acquire()
        posx = self._posxCur.Value()
        self._curLock.release()
        return posx


    def posy(self):
        '''
            Returns the current camera lens Y position.
        '''

        self._curLock.acquire()
        posy = self._posyCur.Value()
        self._curLock.release()
        return posy

    def pos(self):
        '''
            Returns the currenct camera lens (X,Y) position
        '''

        return (self.posx(), self.posy())

    def __curPosChanged(self, var):
        '''
            Automatically notified when curIntensity RTDB variable changes.
        '''
        self._curLock.acquire()
        if var.Name() == self._posxCur.Name():
            self._posxCur.SetValue(var.Value())
        if var.Name() == self._posyCur.Name():
            self._posyCur.SetValue(var.Value())
        self._curLock.release()

    def __rerotPosChanged( self, var):
        '''
            Automatically notified when pupil rerotator RTDB variable changes.
            In case tracking is on, the current position is updated to follow the pupil rerotator.
        '''
        if not self._rerotValid:
            return

        if self._rerotTracking:
            self._rerotLock.acquire()
            if var.Name() == self.v_rerotPosCur.Name():
                self.track( var.Value())
            self._rerotLock.release()

       
    def setTracking(self, tracking):
        '''
        Enables or disables the automatic pupil rerotator position tracking.
        '''
        self._rerotTracking = tracking
        if tracking:
          v=1
        else:
          v=0

        self.trackVar.SetValue(v)
        self.app.WriteVar(self.trackVar)


    def isTracking(self):
	return self._rerotTracking

    def track(self, value):
        '''
        Track the pupil rerotator movement.
        '''

        self.app.log('Camera lens tracking not implemented')
        return


    def guessPos( self, derotPos, bin, refx=0, refy=0, refrot=0, apply=False):
        '''
        Guess the correct cameralens position given the telescope derotator position,
        current binning and a reference position at bin1 (at a given derotation)
        '''

	filename = os.path.join(calib.cameraLensCalibDir(), 'LC_guess_bin1.fits')
	tablebin1 = fits_lib.readSimpleFits(filename)
        refpos = int((refrot%360)*10)
        offsetx = refx - tablebin1[1,refpos]
        offsety = refy - tablebin1[2,refpos]

	filename = os.path.join(calib.cameraLensCalibDir(), 'LC_guess_bin%d.fits' % bin)
	table = fits_lib.readSimpleFits(filename)
	pos = int((derotPos%360)*10)

	x = table[1, pos]+offsetx
        y = table[2, pos]+offsety
        self.app.log('Camera lens guessed position: X=%3.1f Y=%3.1f' % (x, y))
        if apply:
		self.moveTo(x,y)

	return (x,y)

 
