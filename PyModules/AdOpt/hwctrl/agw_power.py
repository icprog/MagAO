#+Class: AGW_Power
#
# Python interface to control the AGW power distribution
#

from hw_ctrl import *
from AdOpt import AOErrCode
from AdOpt.AOExcept import AdOptError

import thread, threading
import sys
import time

tbd_error = AOErrCode['TBD_ERROR']

class device:
    '''
    Class for a single device
    '''

    def __init__( self, parent, name, vCur, vReq, hwctrl, timeout = 30):
        self._name = name
        self._vCur = vCur
        self._vReq = vReq
        self._timeout = timeout
        self._deps_on = []
        self._deps_off = []
        self._parent = parent
        self._hwctrl = hwctrl


    def turnon(self, force = False):

#        if not self._hwctrl.isOnline():
#            raise InvalidStateException( ctrl = self._hwctrl)

        if self.is_on() and not force:
            return

        for device in self._deps_on:
            if not self._parent._devices[device].is_on():
                raise AgwPowerException( tbd_error, "Cannot turn on %s: required %s is off" % (self._name, device))

        for device in self._deps_off:
            if self._parent._devices[device].is_on():
                raise AgwPowerException( tbd_error, "Cannot turn on %s: blocker %s is on" % (self._name, device))

        try:
            self._parent.app.SetVar(self._vReq, 1)
            self._parent.app.WaitVarValue( self._vCur, 1, self._timeout)
        except Exception, e:
            raise AdOptError(errstr = 'Cannot turn on %s. Reason: %s' % (self._name, e.__str__()))

    def turnoff(self, force = False):

#        if not self._hwctrl.isOnline():
#            raise InvalidStateException( ctrl = self._hwctrl)

        if self.is_on() or force:
            try:
                self._parent.app.SetVar(self._vReq, 0)
                self._parent.app.WaitVarValue( self._vCur, 0, self._timeout)
            except Exception, e:
                raise AdOptError(errstr = 'Cannot turn off %s. Reason: %s' % (self._name, e.__str__()))

    def is_on(self):
        v = self._parent.app.ReadVar( self._vCur.Name()).Value()
        if v == 1:
            return True
        return False



class AGW_Power:
    ''' 
        Python interface to control the AGW power distribution

        Available methods are:
            - turnOn(device)    -> turns on the specified device
            - turnOff(device)   -> turns off the specified device
            - isOn(device)      -> returns True if the specified device is on
            
        All raised exceptions are derived from AdOptError.
    '''

    def __init__(self, aoapp, timeout_s = 20):

        self._timeout_s = timeout_s
        self._devices = {}
        self._ctrl = {}
        self.app = aoapp

        # Items are separated into three different hardware controllers:
        # - main power board
        # - bcu 39
        # - flowerpot pic
        # After initialization, we don't care where a device is, provided
        # that the dependency tree has been set up.

        try:
            for hw in ['moxadioccd','moxadiodrv','visaopwrctrl','picctrl']:
                try:
                    ctrl = hw_ctrl( hw, aoapp)
                except KeyError:
                    continue

                nItems = ctrl.get_cfg('num_items')
                for i in range(nItems):
                    code =  ctrl.get_cfg('item%d.code' % i)
                    vReq = self.app.ReadVar( ctrl.varname(code, 'REQ'))
                    vCur = self.app.ReadVar( ctrl.varname(code, 'CUR'))
                    self._devices[code] = device( self, ctrl.get_cfg('item%d.name' % i), vCur, vReq, ctrl)
                    if ctrl.has_cfg('item%d.depends' % i):
                        self._devices[code]._deps_on = [ x.strip() for x in ctrl.get_cfg('item%d.depends' % i).split(',')]
                    if ctrl.has_cfg('item%d.blockedby' % i):
                        self._devices[code]._deps_off = [ x.strip() for x in ctrl.get_cfg('item%d.blockedby' % i).split(',')]

                self._ctrl[hw] = ctrl

            print 'AGW power interface loaded'

        except AOMissingProcess, e:
            raise AgwPowerException(e.code, e.errstr)
        except KeyError, e:
            raise AgwPowerException(AOErrCode['UNEXISTING_KEY_ERROR'], "missing key %s in config file" % e)
        except AOVarNotInRTDB, e:
            raise AgwPowerException(AOErrCode['VAR_NOT_FOUND_ERROR'], e.errstr)
        except AOVarError, e:
            raise AgwPowerException(None, e.errstr)

    def turnon( self, device, force = False):
        self.app.log('Turning on %s' % device)
        return self._devices[device].turnon(force = force)

    def turnoff( self, device, force = False):
        self.app.log('Turning off %s' % device)
        return self._devices[device].turnoff(force = force)

    def is_on( self, device):
        return self._devices[device].is_on()

    def list( self):
        return self._devices.keys()

    def depends( self, device):
        return self._devices[device]._deps_on

    def blockedby( self, device):
        return self._devices[device]._deps_off

        
    
class AgwPowerException(AdOptError):
    ''' Generic AGW power exception'''
    def __init__(self, code, errstr):
        AdOptError.__init__(self, code, errstr)


## 
# DEMO MAIN
if __name__ == "__main__":
    aoApp = AOApp("pyAGW", False)
    
    try:
        power = AGW_Power( aoApp, 1)

        try:
            power.turnon('BCU39 reset signal')   
        except:
            pass
        power.turnon('adc')
        power.turnon('fans_LJ')
        
        print "Test done."
    except AgwPowerException, e:
        print "Fatal AGW Power exception: ", e

    # To catch everything without difference use this !
    except AdOptError, e:
        print "Fatal exception: ", e
        
    
