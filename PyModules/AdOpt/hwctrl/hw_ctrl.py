# Python generic class for an hw controller

import time, types

from AdOpt import cfg
from AdOpt.AOConstants import *
from AdOpt.AOExcept import *


class hw_ctrl:
    '''
    Python base class for hardware controllers. Provides generic services common to all hardware

    To talk with only one hardware controller, derive your class from this one.
    To talk with more than one controller, include multiple instances of this class inside yours


    waitStatus() : waits until the controller status assumes the specified value
    getStatus()  : intended to be overriden by derived classes to have the current status

    isOnline()   : checks whether the controller is online (status >= STATE_CONNECTED)
    isActive()   : checks whether the controller is configured and up and running (status >= STATE_OFF)

    setPolicy()  : sets the control policy ('auto' or 'manual')
    policy()     : gets the control policy
    checkPolicy(): checks whether the current policy allows automatic control
    
    '''

    def __init__( self, name, aoapp, ctrl_check = True, power_check = False):

        self.name = name
        self.app = aoapp

        ctrl_check = False
        power_check = False

        self._policy = 'auto'


        # Can raise KeyError exception
        self.side = cfg.get_cfg_value( self.name, "mySide", test=True)
#        self.var_prefix = cfg.get_cfg_value( self.name, 'MyName')
       
        # Always read STATUS variable and keep its value current 
        try:
            self.v_status = self.app.ReadVar(self.varname('STATUS'))
            self.app.VarNotif( self.v_status)
        except AOVarNotInRTDB, e:
            raise HwCtrlCreationException(AOErrCode['VAR_NOT_FOUND_ERROR'], e.errstr) 

        if ctrl_check:
            try:
                self.app.WaitClientReady(cfg.taskname(name), timeout=1)
            except:
                errstr = "Controller process %s was not found" % cfg.taskname(name)
                raise AOMissingProcess, (errstr, cfg.taskname(name))

        if power_check:
            try:
                self.app.WaitClientReady( cfg.taskname('bcurelays'), timeout=1)
            except:
                errstr =  "Controller process %s was not found" % cfg.taskname('bcurelays')
                raise AOMissingProcess, (errstr, cfg.taskname('bcurelays'))
       
    def setPolicy( self, policy):
        '''
        Sets the control policy. Valid values are 'auto' and 'manual'.
        '''

        if (policy != 'auto') and (policy != 'manual'):
            raise AdOptError( errstr = 'Invalid policy: "%s"' % policy)

        self._policy = policy

    def policy(self):
        return self._policy 

    def checkPolicy(self):
        '''
        Returns True if the current policy allows automatic control, False otherwise.
        '''
        return self.policy() == 'auto'


    def varname(self, *varname):
        return cfg.varname( self.name, '.'.join(varname))

    def other_varname(self, device = None, *varname):
        if device == None:
            device = self.device_type
        return cfg.varname( device, '.'.join(varname))

    def get_cfg(self, key):
        return cfg.get_cfg_value( self.name, key)

    def has_cfg(self, key):
        try:
            test = cfg.get_cfg_value( self.name, key, test=True)
            return True
        except:
            return False

    # Can be overridden by derived classes
    def getStatus(self):
        return self.v_status.Value()

    def waitStatus( self, statelist, waitTimeout = 10):
        '''
        Waits until getStatus() returns the wanted state.

        <statelist> may be a single state value or a list of states.
        In the latter case,any of the listed states will satisfy the condition.
        Both state codes and state names are allowed.
        '''
        
        if hasattr( statelist, '__iter__') == False:
            statelist = [statelist]

        for n in range(len(statelist)):
            if type(statelist[n]) == types.StringType:
                statelist[n] = AOStatesCode[statelist[n]]

        self.app.WaitVarFunc( self.v_status, lambda x: x in statelist, timeout = waitTimeout)

    def isOnline( self):
        '''
        Returns True if the controller is online and reachable.
        '''

        state = self.getStatus()
        return state > AOConstants.AOStatesCode['STATE_NOCONNECTION']

    def waitOnline(self, timeout=60):
        return self.app.WaitVarFunc( self.v_status, lambda x: x > AOConstants.AOStatesCode['STATE_NOCONNECTION'], timeout = timeout)

    def isActive(self):
        '''
        Returns True if the controller is correctly configured and operating.
        '''
        state = self.getStatus()
        return (state >= AOConstants.AOStatesCode['STATE_OFF']) and (state != AOConstants.AOStatesCode['STATE_ERROR'])

    def isOperating(self, wait=0):
        '''
        Returns True if the controller is presently operating
        '''
        if wait==0:
            state = self.getStatus()
            return state == AOConstants.AOStatesCode['STATE_OPERATING']
        else:
            try:
                self.waitStatus(['STATE_OPERATING'], waitTimeout=wait)
                return True
            except:
                return false
               

    def isReady(self):
        '''
        Returns True if the controller is presently READY
        '''
        state = self.getStatus()
        return state == AOConstants.AOStatesCode['STATE_READY']



    def waitActive(self, timeout=60):
        return self.app.WaitVarFunc( self.v_status, lambda x: (x >= AOConstants.AOStatesCode['STATE_OFF']) and
                                                             (x != AOConstants.AOStatesCode['STATE_ERROR']), timeout = timeout)


##
# Root for an HwCtrl exception
class HwCtrlException(AdOptError):
    def __init__(self, code, errstr):
        AdOptError.__init__(self, code, errstr)

##
# HwCtrl not created
class HwCtrlCreationException(HwCtrlException):
    """Exception raised when the HwCtrl is not correctly created"""
    def __init__(self, code, errstr):
        HwCtrlException.__init__(self, code, "Fatal HwCtrl error: "+ errstr)

##
# Notify for a missing controller
class AOMissingProcess(HwCtrlException):
    """Exception raised when a controller process is not correctly running"""
    def __init__(self, errstr=None, process=None):
        HwCtrlException.__init__(self,AOErrCode['CTRL_MISSING_ERROR'], "Fatal HwCtrl error: "+errstr)
        self.process = process
    def __str__(self):
        return AdOptError.__str__(self)

# Controller not in right state

class InvalidStateException(HwCtrlException):
    """Exception raised when the controller is in the wrong state for the requested operation"""
    def __init__(self, curstate=None, reqstate=None, ctrl=None):
        if curstate == None and ctrl != None:
            curstate = ctrl.getStatus()

        if ctrl:
            name = ctrl.name
        else:
            name = 'Controller'

        str = '%s in wrong state for requested operation.' % name
        if curstate:
            str += ' Current state: %s.' % AOConstants.AOStatesType[curstate]
        if reqstate:
            str += ' State must be %s.' % AOConstants.AOStatesType[reqstate]

        HwCtrlException.__init__(self,AOErrCode['CTRL_WRONG_STATE_ERROR'],str)



