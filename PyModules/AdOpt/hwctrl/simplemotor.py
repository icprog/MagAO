#@File: simplemotor.py
#
# Python interface with the SimpleMotor controller
#
#@

from hw_ctrl import *
from AdOpt.AOExcept import *


import thread, threading
import sys
import time


class SimpleMotor(hw_ctrl):
    '''
        Python interface for the SimpleMotor controller.
        
        The API allow to:
            - Construct a SimpleMotor interface, given a 'name' and an 'AOApp' 
              instance.
              ***Note*** that the name is used to retrieve the motor configuration
              so it must match a 'name' defined in cfg.py. 
              A SimpleMotor object can only control a single motor, i.e.
              a filterwheel: to control an ADC (pair of motors) use class
              "Adc".

            - home(): move the motor to the homing position
            - abort(): abort the current movement or homing
            - getPos(): return the current motor position
            - moveTo(pos, wait): request for a movement (optionally waiting for 
              position reached)
            - isMoving(): return true if the motor is moving in the specified time interval
            - targetReached(): return true if the last movement was correctly executed.

            - saveState(): save current position and do not perform homing on next power cycle.

            All APIs can throw exceptions if the requested operation could not be completed or if the
            information is not available (e.g. because the movement is turned off).
        
    '''
    def __init__(self, name, aoapp, ctrl_check = True, power_check = False):
        
        self._curLock = thread.allocate_lock()
        #self._swStatusLock = thread.allocate_lock()
        try: 
            hw_ctrl.__init__( self, name, aoapp, ctrl_check, power_check)

            
             # Initialize the REQ and CUR local RTDBVar 
            self._posReq = self.app.ReadVar(self.varname('POS', 'REQ'))
            self._posCur = self.app.ReadVar(self.varname('POS', 'CUR'))
            self._posName = self.app.ReadVar(self.varname('POSNAME', 'CUR'))
            self._lowEnd = self.app.ReadVar(self.varname('LOEND'))
            self._hiEnd  = self.app.ReadVar(self.varname('HIEND'))
            #self._swStatus = self.app.ReadVar(self.varname('SW_STATUS'))
            
            # Ask to be notified on cur change
            self.app.VarNotif(self._posCur.Name(), self.__curPosChanged)
            self.app.VarNotif(self._posName.Name(), self.__curPosChanged)
            #self.app.VarNotif(self._swStatus.Name(), self.__swStatusChanged)
             
            # Init some constants
            self._HOMING_POS = cfg.get_cfg_value(self.name, "HomingPosition")
            self._ABORT_POS = cfg.get_cfg_value(self.name, "AbortPosition")   
            self._goodWindow  = cfg.get_cfg_value(self.name, "GoodWindow")
            self._targetReached = True
            self._targetPos = self._ABORT_POS
            self._accuracy = None
            
        except AOMissingProcess, e:
            raise SimpleMotorCreationException(e.code, e.errstr)
        except KeyError, e:
            raise SimpleMotorCreationException(AOErrCode['UNEXISTING_KEY_ERROR'], "missing key %s in config file" % e)
        except AOVarNotInRTDB, e:
             raise SimpleMotorCreationException(AOErrCode['VAR_NOT_FOUND_ERROR'], e.errstr)
        except AOVarError, e:
           raise SimpleMotorCreationException(None, e.errstr)
       
        self.app.log(self.name+" interface loaded")


    def home(self, waitStart = None, waitTimeout = None):
        '''
            Go to homing position
        '''
        if not self.checkPolicy():
            return

        if not self.isOnline():
            raise InvalidStateException()

        self.moveTo(self._HOMING_POS)

        if waitStart:
            self.waitStatus( AOConstants.AOStatesCode['STATE_HOMING'], waitTimeout = waitStart)

        if waitStart and waitTimeout:
            self.waitStatus( AOConstants.AOStatesCode['STATE_READY'], waitTimeout = waitTimeout)

        if waitTimeout and not waitStart:
            self.waitStatus( AOConstants.AOStatesCode['STATE_HOMING'], waitTimeout = waitTimeout)
            self.waitStatus( AOConstants.AOStatesCode['STATE_READY'], waitTimeout = waitTimeout)


        
        
    def abort(self):
        '''
            Abort movement
        '''
        if not self.checkPolicy():
            return

        if not self.isActive():
            raise InvalidStateException()

        self.moveTo(self._ABORT_POS)
        
    
    def getPosName(self):
        '''
            Get the current motor position name (if defined in cfg file).
            Returns an empty string if the motor is not in any predefined position,
            or if no positions are defined in the cfg file.
        '''

        self._curLock.acquire()
        curPosName = self._posName.Value()
        self._curLock.release()
        return curPosName

    def getPos(self):
        '''
            Get the current motor position
        '''
        if not self.isActive():
            raise InvalidStateException( ctrl = self)

        self._curLock.acquire()
        curPos = self._posCur.Value()
        self._curLock.release()
        return curPos
        
    def getSwStatus(self):
        '''
            Get the current switch status
        '''
        if not self.isOnline():
            raise InvalidStateException()

        self._swStatusLock.acquire()
        curSwStatus = self._swStatus.Value()
        self._swStatusLock.release()
        return curSwStatus
   
    def moveBy(self, pos, accuracy = None, waitTimeout = 0):
        ''' Move the motor to the given position (relative to current position)'''
        return self.moveTo( self.getPos() + pos, accuracy, waitTimeout)
        
    def moveTo(self, pos, accuracy = None, waitTimeout = 0, force=False):
        '''
            Move the motor to the given position (absolute)
            
            Optionally waits for motor to stop moving. Note that reached position can 
            be different form the requested one: moveTo returns the position
            at return time (the old one if not forced to wait !)

            <accuracy> is an optional movement accuracy in mm: if not None, will wait
            until the positioning error is less than the specified value. If None, it
            will use the default configuration value (usually 0.010 mm).
            
            The position can be a value or a string defined in the motor config file.
            
            Raise a SimpleMotorMovingException if the position is invalid (not a value
            nor a valid string)

            One can also check for the end of movement using the _targetReached flag,
            which is guaranteed to be false after the call until the target has been reached.
        '''
        if not self.checkPolicy():
            return

        if not self.isActive():
            raise InvalidStateException()

        # Check if pos is a value or a string
        try:
            floatPos = float(pos)
        except ValueError, e:
            floatPos = None
            try:
	        num = cfg.get_cfg_value(self.name, 'customPositionNum')
                if num>0:
                    for i in range(num):
                        key = 'pos%d_name' % i
                        posName = cfg.get_cfg_value(self.name, key);
                        if posName == pos:
                            key = 'pos%d_pos' % i
                            floatPos = cfg.get_cfg_value(self.name, key);
                            break
            except KeyError, e:
                raise SimpleMotorMovingException
        if floatPos == None:
            raise SimpleMotorMovingException

        self._targetPos = floatPos
        
        # No need for movement?
        window = self._goodWindow
        if accuracy != None:
            window = accuracy
        if (abs(self.getPos() - floatPos) < window) and not force:
            return self.getPos()

        # Interrupt previous movement if needed
        if floatPos != self._ABORT_POS and self.getStatus() == AOStatesCode['STATE_OPERATING']:
            self.abort()

        # Reset target flag
        self._targetReached = False

        # Accuracy
        self._accuracy = accuracy
    
        # Set the req RTDB var with the pos
        self.app.SetVar(self._posReq, floatPos)     
        
        # Wait until both state and window position are OK
        # Target flag is updated in the curPosChanged handler
        if waitTimeout:
            now = time.time()
            while 1:
                if self.getStatus() == AOStatesCode['STATE_READY'] or \
                   self.getStatus() == AOStatesCode['STATE_OFF']:
                    if self._targetReached:
                        break
                if time.time() - now > waitTimeout:
                    raise AdOptError( AOErrCode['TIMEOUT_ERROR'], "Timeout error")
                time.sleep(0.1)
      
        return self.getPos()
            
        
    def isMoving(self):
        '''
            Return True if the motor is moving.
        '''
        if not self.isOnline():
            raise InvalidStateException()

        currentStatus = self.getStatus()
        if currentStatus == AOStatesCode['STATE_HOMING'] or currentStatus == AOStatesCode['STATE_OPERATING']:
            return True
        else:
            return False

    def saveState(self):

        if not self.isOnline():
            raise InvalidStateException()

        self.app.SendMsg( cfg.taskname(self.name), AOConstants.AOMsgCode['MOTOR_SAVESTATE'], '', timeout=1000)


    def targetReached(self):
        '''
            Returns True if the last movement was correctly executed.
        '''

        if not self.isOnline():
            raise InvalidStateException()

        return self._targetReached

    def waitTargetReached(self, timeout = 60, checkStatus = True):
        '''
            Waits until the targetReached flag becomes true.
            If the 'checkStatus' flag is True, it will also wait
            for the movement state to drop from the OPERATING state
            (thus allowing another command immediately upon return)
        '''

        if not self.checkPolicy():
            return

        start = time.time()
        while 1:
            if self._targetReached:
                if checkStatus == False:
                    return
                elif self.getStatus() != AOStatesCode['STATE_OPERATING']:
                    return

            time.sleep(0.1)

            if timeout:
                if time.time() - start > timeout:
                    raise AdOptError( AOErrCode['TIMEOUT_ERROR'], "Timeout error on %s" % self.name)
        
    def __curPosChanged(self, var):
        '''
            Automatically notified when curPos RTDB variable changes.
            Remembers current position and checks whether the target position has been reached.
        '''
        self._curLock.acquire()
        if var.Name() == self._posCur.Name():
            self._posCur.SetValue(var.Value())

            # Use whether the accuracy value or the configuration window value
            window = self._goodWindow
            if self._accuracy != None:
                window = self._accuracy

            if abs(var.Value() - self._targetPos) < window: 
                self._targetReached = True

        if var.Name() == self._posName.Name():
            self._posName.SetValue(var.Value())

        self._curLock.release()
        
        
    #def __swStatusChanged(self, var):
    #    '''
    #        Automatically notified when curPos RTDB variable changes.
    #    '''
    #    self._swStatusLock.acquire()
    #    if var.Name() == self._swStatus.Name():
    #        self._swStatus.SetValue(var.Value())
    #    self._swStatusLock.release()
        
        
##
# ---------- SIMPLE-MOTOR EXCEPTIONS -----------
#

class SimpleMotorCreationException(HwCtrlCreationException):
    def __init__(self, code, errstr):
        HwCtrlCreationException.__init__(self, code, errstr) 
        
class SimpleMotorMovingException(HwCtrlException):
    def __init__(self):
        HwCtrlException.__init__(self, "SimpleMotorMovingException", "Invalid movement") 
        
        
