#+File: lamp.py
#
# Python interface for the flowerpot lamp
#

from hw_ctrl import *

import thread, threading


class Lamp(hw_ctrl):
    '''
        Python interface for the flowerpot Lampr.
        
        The API allow to:
            - Construct a Lamp interface, given a 'name' and an 'AOApp' 
              instance.
              ***Note*** that the name is used to retrieve the lamp configuration
              so it must match a 'name' defined in cfg.py. 

            - setIntensity(): set lamp to the given intensity (with optional wait)
            - getIntensity(): get current lamp intensity
        
    '''
    def __init__(self, name, aoapp, ctrl_check = True, power_check = False):
        
        self._curLock = thread.allocate_lock()
        try: 
            hw_ctrl.__init__( self, name, aoapp, ctrl_check, power_check)
            
             # Initialize the REQ and CUR local RTDBVar 
            self._intensityReq = self.app.ReadVar(self.varname('LampIntensity', 'REQ'))
            self._intensityCur = self.app.ReadVar(self.varname('LampIntensity', 'CUR'))
           
            # Ask to be notified on cur change
            self.app.VarNotif( self._intensityCur)
             
        except AOMissingProcess, e:
            raise HwCtrlCreationException(e.code, e.errstr)
        except KeyError, e:
            raise HwCtrlCreationException(AOErrCode['UNEXISTING_KEY_ERROR'], "missing key %s in config file" % e)
        except AOVarNotInRTDB, e:
             raise SimpleMotorCreationException(AOErrCode['VAR_NOT_FOUND_ERROR'], e.errstr)
        except AOVarError, e:
           raise SimpleMotorCreationException(None, e.errstr)
        
        print "Lamp interface loaded"


    def setIntensity( self, intensity, waitTimeout = None):
        '''
            Set lamp intensity.
        '''

        if not self.checkPolicy():
            return

        if intensity == self.getIntensity():
            return

        self.app.SetVar( self._intensityReq, intensity)

        if waitTimeout != None:
            self.app.WaitVarValue( self._intensityCur, intensity, timeout = waitTimeout)

        
    def getIntensity(self):
        '''
            Get current lamp intensity
        '''
        return self._intensityCur.Value()