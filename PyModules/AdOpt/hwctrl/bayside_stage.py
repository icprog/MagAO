#+Class: bayside_stage
#
# Python generic class for a bayside stage

from AdOpt import AOConstants, cfg
from simplemotor import *
import agw_power
import time

class bayside_stage(SimpleMotor):

    def __init__(self, name, aoapp, timeout_s = 1):

        self._timeout_s = timeout_s
        SimpleMotor.__init__(self, name, aoapp)

        try:
            self.v_switches_req  = self.app.ReadVar( self.varname('LIMIT', 'REQ'))
            self.v_switches_cur  = self.app.ReadVar( self.varname('LIMIT', 'CUR'))
            self.v_enable_req  = self.app.ReadVar( self.varname('ENABLE', 'REQ'))
            self.v_enable_cur  = self.app.ReadVar( self.varname('ENABLE', 'CUR'))
            #self.v_encoder_req   = self.app.ReadVar( self.varname('ENCODER', 'req'))
            #self.v_encoder_cur   = self.app.ReadVar( self.varname('ENCODER', 'cur'))
            self.v_special  = self.app.ReadVar( self.varname('SPECIAL'))

            self._onaftermove = False

        except AOVarNotInRTDB, e:
            print e
            errstr =  "Controller process %s is not working properly" % cfg.taskname(name)
            print errstr
            raise AOMissingProcess, ( errstr, e.message, cfg.taskname(name))

    def enable(self, enable = True, onaftermove = False):
        '''
            Enables/disables the motor drive. Also specifies whether the motor drive
            should remain active after a movement has been completed, or if it should be disabled.
        '''
        if not self.checkPolicy():
            return

        if not self.isOnline():
            raise InvalidStateException()

        if enable: 
            value = AOConstants.AOStatesCode['ENABLE_OFFAFTERMOVE']
            if onaftermove:
                value = AOConstants.AOStatesCode['ENABLE_ALWAYS']
        else:
            value = AOConstants.AOStatesCode['ENABLE_OFF']

        self._onaftermove = onaftermove

        self.app.SetAndWaitCur( self.v_enable_req, self.v_enable_cur, value, timeout= self._timeout_s)

#    Obsoleted, controller already works in millimeters.
#    def m_to_stage(self, value):
#        return float(value) * self.stepsRatio
#
#    def stage_to_m(self, value):
#        return float(value) / self.stepsRatio

    def recover_negative_limit(self):
        '''
            Recovers an iDrive from a negative limit switch error.
        '''
        self.recover_limit( amount = +10)

    def recover_positive_limit(self):
        '''
            Recovers an iDrive from a positive limit switch error.
        '''
        self.recover_limit( amount = -10)

    def activateLimits( self, sw_pos=False, sw_neg=False, hw_pos=False, hw_neg=False):
        '''
        Activates the positioning limits, both software and electrical.
        The first argument may be a dictionary with the arguments as keywords.
        '''

        if not self.checkPolicy():
            return

        # If the first argument is a dictionary, unpack it
        if type(sw_pos) == types.DictionaryType:
            sw_neg = sw_pos['sw_neg']
            hw_pos = sw_pos['hw_pos']
            hw_neg = sw_pos['hw_neg']
            sw_pos = sw_pos['sw_pos']

        value = 0
        if hw_neg:
            value = value | 0x01
        if hw_pos:
            value = value | 0x02
        if sw_neg:
            value = value | 0x04
        if sw_pos:
            value = value | 0x08
        
        self.app.SetAndWaitCur( self.v_switches_req, self.v_switches_cur, value, timeout= self._timeout_s)

    def limitsActive(self):
        '''
        Returns whether the limit switches (both software and electrical) are activated (as in loaded, not the actual status).
        '''

        if not self.isOnline():
            raise InvalidStateException()

        switches = self.app.ReadVar( self.v_switches_cur).Value() 
        s = {}
        s['hw_neg'] = ( switches & 0x01) == 0x01
        s['hw_pos'] = ( switches & 0x02) == 0x02
        s['sw_neg'] = ( switches & 0x04) == 0x04
        s['sw_pos'] = ( switches & 0x08) == 0x08
        return s

    def recover_limit(self, amount):
        '''
        Recovers from a limit switch error: disables all limit switches,
        quits error, moves away and restores the previous limit switch setting.

        This routine may block for a significant amount of time (a dozen seconds).
        '''
        if not self.checkPolicy():
            return

        if not self.isActive():
            raise InvalidStateException()

        # Save old status
        old_switches = self.limitsActive()

        # Disable limit switches
        self.ActivateLimits( False, False, False, False)

        self.quit_error()
       
        self.enable()
        self.moveto( self.pos() + amount)

        self.disable()
        self.activateLimits( old_switches)
   
    def quit_error(self):
        '''
        Quits any error on the iDrive controller.
        '''

        if not self.checkPolicy():
            return

        if not self.isOnline():
            raise InvalidStateException()

        self.enable()
        time.sleep(0.5)
        self.disable()
        time.sleep(0.5)

    def home(self, waitTimeout=None, waitStart=None):
        '''
        Starts homing procedure. waitTimeout is a timeout for the entire movement,
        while waitStart is a timeout only for the start of homing movement.
        '''
        if not self.checkPolicy():
            return

        if not self.isOnline():
            raise InvalidStateException()

        self.app.SetVar( self.v_special, 1);

        if waitStart:
            self.waitStatus( AOConstants.AOStatesCode['STATE_HOMING'], waitTimeout = waitStart)

        if waitStart and waitTimeout:
            self.waitStatus( AOConstants.AOStatesCode['STATE_OFF'], waitTimeout = waitTimeout)

        if waitTimeout and not waitStart:
            self.waitStatus( AOConstants.AOStatesCode['STATE_HOMING'], waitTimeout = waitTimeout)
            self.waitStatus( AOConstants.AOStatesCode['STATE_OFF'], waitTimeout = waitTimeout)


        

    # Changes stage encoder.

    def set_encoder(self, encoder):
        '''
        Sets the iDrive to work with either the rotary or the linear encoder.
        '''

        if not self.checkPolicy():
            return

        # Must be in state OFF
        s = self.getStatus()
        if s != AOStatesCode['STATE_OFF']:
            raise InvalidStateException( s, AOStatesCode['STATE_OFF'])

        # Switch encoder

        if encoder == self.rotaryEncoder():
            agw_power.turnoff(name)
        else:
            agw_power.turnon(name)

        # Ask the controller to reconfigure
        self.app.SetAndWaitCur( self.v_encoder_req, self.v_encoder_cur, encoder, timeout= self._timeout_s)

        # Wait for state OFF
        self.waitStatus( AOStatesCode['STATE_OFF'], waitTimeout = 60)



