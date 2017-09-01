#@Class: Adc.py
#
# High-level python class for the ADC
#
#@

from simplemotor import *
from AdOpt import cfg, InterpolatedArray, calib, AOVar


import thread, threading
import time, math

class Adc:
    '''
        Python interface for the ADC controller.
        
        The API allow to:
            - Construct an Adc interface: an Adc is composed of two SimpleMotors,
              moving in opposite directions.
            - home(): move the Adc motos to the homing position
            - abort(): abort the current movement or homing
            - getPos(): return the current motors position (dispersion angle)
            - moveTo(pos, wait): request for a movement (dispersion angle)
            - isMoving(): return true if at least a motor is moving
            - getStatus(): returns the combined status of the adc

        The actual movement is computed as follows:
        wheel1:  + requested movement
        wheel2:  - requested movement

    '''
    
    def __init__(self, aoapp, ctrl_check = True, power_check = False):
        '''
        Constructor
        '''
               
        try:
            self._motor1 = SimpleMotor("adc1", aoapp, ctrl_check,power_check)
            self._motor2 = SimpleMotor("adc2", aoapp, ctrl_check,power_check)
            self._targetReached = True

        except SimpleMotorCreationException, e:
            print e.errstr
            raise AdcCreationException(e.code, "Impossible to construct ADC")

        self.loadCalib( calib.getADCcalibFile())

        self._motor1.app.log('ADC interface loaded')
       
    def waitStatus(self, state, waitTimeout=None):
        self._motor1.waitStatus( state, waitTimeout = waitTimeout) 
        self._motor2.waitStatus( state, waitTimeout = waitTimeout) 
         
    def home(self, waitTimeout=None):
        '''
            Go to homing position. Due to interference between the two magnets,
            first wheel #1 performs homing, then it is moved 180 away, then wheel #2 performs homing.
        '''
        # Precondition: both wheels ready
        self._motor1.waitStatus( AOConstants.AOStatesCode['STATE_READY'], waitTimeout = waitTimeout)
        self._motor2.waitStatus( AOConstants.AOStatesCode['STATE_READY'], waitTimeout = waitTimeout)

        # Do wheel #1 homing and move it away
        self._motor1.home( waitTimeout = 120)
        self._motor1.moveTo( 180, waitTimeout=30)

        # Do wheel #2 homing and wait for completion
        self._motor2.home( waitTimeout = 120)
        
        
    def abort(self):
        '''
            Abort movement
        '''
        self._motor1.abort()
        self._motor2.abort()

    def waitTargetReached(self, timeout = 60, checkStatus = False):
        self._motor1.waitTargetReached( timeout, checkStatus)
        self._motor2.waitTargetReached( timeout, checkStatus)

    
    def getPos(self):
        '''
            Return the current dispersion angle.
        '''

        return self._motor1.getPos() - self.getDerotPos()
    
            
    def moveTo(self, pos, waitTimeout = 0):
        '''
            Move to the specified dispersion angle.

            Move the motor 1 to the given position, the motor 2 to the opposite position. 
            
            Optionally waits for both motors to stop moving. Note that reached position can 
            be different form the requested one: moveTo returns the position
            at return time (the old one if not forced to wait !).
            
            The position can be a value or a string defined in the motor config file.
            
            Raise a SimpleMotorMovingException if the position is invalid (not a value
            nor a valid string)
        '''
        # Precondition: both wheels ready
        self._motor1.waitStatus( AOConstants.AOStatesCode['STATE_READY'])
        self._motor2.waitStatus( AOConstants.AOStatesCode['STATE_READY'])

        disp = self._dispArray[pos]
        pos = 0

        motor1Pos = self._motor1.moveTo( pos + disp);
        motor2Pos = self._motor2.moveTo( pos - disp);

        if waitTimeout:        
            now = time.time()
            while not self.targetReached():
                if time.time() - now >= waitTimeout:
                    raise AdOptError(AOErrCode['TIMEOUT_ERROR'], "Timeout error")
                time.sleep(0.1)

 
    def targetReached(self):
        return self._motor1.targetReached() and self._motor2.targetReached()       
        
    def isMoving(self):
        return self._motor1.isMoving() or self._motor2.isMoving()

    def getStatus(self):
        '''
            Returns the combined status of the adc
        '''

        s1 = self._motor1.getStatus()
        s2 = self._motor2.getStatus()

        if s1 < AOStatesCode['STATE_READY'] or s2 < AOStatesCode['STATE_READY']:
            return min(s1, s2)

        return max(s1,s2)

    def loadCalib( self, filename):
        '''
        Loads an ADC calibration file.
        File is in the format: <elevation angle> <dispersion angle>

        Result:
        self._dispTable is a dictionary where each key is an elevation angle, and the value the corresponding disperion angle
        self._dispArray is the same list as an InterpolatedArray()

        '''

        lines = file(filename).readlines()
        self._dispTable = []
        for line in lines:
            elev, disp = map( float, line.split())
            self._dispTable.append((elev, disp))

        def cmp_volts(a, b):
            '''
            In-place sorting using the first value.
            '''
            return cmp(a[0], b[0])


        self._dispTable.sort(cmp_volts)
        self._dispArray = InterpolatedArray( self._dispTable)

    
    
            
            
##
# ---------- ADC EXCEPTIONS -----------
#

class AdcCreationException(SimpleMotorCreationException):
    def __init__(self, code, errstr):
        SimpleMotorCreationException.__init__(self, code, errstr) 
     
class AdcPositionException(HwCtrlException):
    def __init__(self):
        HwCtrlException.__init__(self, "AdcPositionException", "Invalid ADC position")    

        
