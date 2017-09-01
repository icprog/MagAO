#+Class: TipTilt
#
# Python interface with the TipTilt controller
#
# At the moment is implemented only the control for high-level TipTilt
# parameters.

from hw_ctrl import *
from AdOpt import InterpolatedArray, AOExcept, calib

import thread, threading
import sys
import time

class TipTilt(hw_ctrl):
    ''' 
        Python interface with the TipTilt controller

        At the moment is implemented only the control for high-level TipTilt
        parameters.
        
        Available methods are:
            - set(amp,freq,offx,offy)    -> Raise TTCreationException()
            - start() or restart()       -> Raise TTStartException()
            - stop()                     -> Raise TTStopException()
            
        All raised exceptions are derived from TTException.
    '''
    
    def __init__(self, name, aoapp, timeout_s = 1, ctrl_check = True, power_check = False):
      
        self._timeout_s = timeout_s
        self._curLock = thread.allocate_lock()
        
        try: 
            hw_ctrl.__init__( self, name, aoapp, ctrl_check, power_check)
            
            # Initialize the REQ and CUR local RTDBVar relatives to high-level parameters
            self._hlAmpReq   = self.app.ReadVar(self.varname('AMP', 'REQ'))
            self._hlFreqReq  = self.app.ReadVar(self.varname('FREQ', 'REQ'))
            self._hlOffxReq  = self.app.ReadVar(self.varname('OFFX', 'REQ'))
            self._hlOffyReq  = self.app.ReadVar(self.varname('OFFY', 'REQ'))
            self._setReq     = self.app.ReadVar(self.varname('MIRROR_SET', 'REQ'))
            self._hlAmpCur   = self.app.ReadVar(self.varname('AMP', 'CUR'))
            self._hlFreqCur  = self.app.ReadVar(self.varname('FREQ', 'CUR'))
            self._hlOffxCur  = self.app.ReadVar(self.varname('OFFX', 'CUR'))
            self._hlOffyCur  = self.app.ReadVar(self.varname('OFFY', 'CUR'))
            self._setCur     = self.app.ReadVar(self.varname('MIRROR_SET', 'CUR'))
            self._go = self.app.ReadVar(self.varname('GO'))
            
            # Ask to be notified on cur change
            self.app.VarNotif(self._hlAmpCur.Name(), self.__curVarChanged)
            self.app.VarNotif(self._hlFreqCur.Name(), self.__curVarChanged)
            self.app.VarNotif(self._hlOffxCur.Name(), self.__curVarChanged)
            self.app.VarNotif(self._hlOffyCur.Name(), self.__curVarChanged)
            self.app.VarNotif(self._setCur.Name(), self.__curVarChanged)
            self._curVarUpdated_event = threading.Event()
            
        except AOMissingProcess, e:
            raise TTCreationException(e.code, e.errstr)
        except KeyError, e:
            raise TTCreationException(AOErrCode['UNEXISTING_KEY_ERROR'], "missing key %s in config file" % e)
        except AOVarNotInRTDB, e:
             raise TTCreationException(AOErrCode['VAR_NOT_FOUND_ERROR'], e.errstr)
        except AOVarError, e:
           raise TTCreationException(None, e.errstr)

        self.loadCalib( calib.getTTcalibFile())

        print self.name+' interface loaded'
        
    
    def set(self, amp = None, freq = None, offx = None, offy = None, timeout = None):
        ''' Set the TipTilt high-level parameters and start or restart the TipTilt.
            Can raise a TTStartException or a WrongXXXException '''

        if not self.checkPolicy():
            return
        
        if timeout != None:
            self._timeout_s = timeout

        # Parameters left as None will take the current setting
        if amp == None:
            amp = self.amp()

        if freq == None:
            freq = self.freq()
           
        if offx == None:
            offx = self.offx()

        if offy == None:
            offy = self.offy()

        
        self.app.SetVar(self._hlAmpReq, float(amp))
        self.app.SetVar(self._hlFreqReq, float(freq))
        self.app.SetVar(self._hlOffxReq, float(offx))
        self.app.SetVar(self._hlOffyReq, float(offy))
        self.app.log("Setting tiptilt to amp=%5.2f, freq=%5.2f, offx=%5.2f, offy=%5.2f" % (float(amp), float(freq), float(offx), float(offy)))
        self.start()
       
    def amp(self):
        return self._hlAmpCur.Value()

    def freq(self):
        return self._hlFreqCur.Value()
     
    def offx(self):
        return self._hlOffxCur.Value()
     
    def offy(self):
        return self._hlOffyCur.Value()
     
     
    def start(self):
        ''' Start or restart the TipTilt.
            Can raise a TTStartException or a WrongXXXException '''
        self.restart()

    def mirrorSet(self, set):
        if set:
            set=1
        else:
            set=0
        self.app.SetVar(self._setReq, set)
        self.app.WaitVarValue( self._setCur, set, timeout = 60)
       
        
        
    def restart(self):
         ''' Start or restart the TipTilt.
            Can raise a TTStartException or a WrongXXXException '''

         if not self.checkPolicy():
             return
        
         # Set all CUR vars to unknown value
         # -- self.__resetAllCur()
         
         # Start te TTCtrl. It should update the CUR vars: I want
         # to wait for this event
         try:
             
             # Start and wait for all CUR vars updated, or timeout
             self.app.SetVar(self._go, 1)  # Can raise an AOVarError
             # -- self.__waitAllCurUpdated()    # Can raise an AdOptError for timeout
             
             # Check req-cur matching
             # -- print "Checking for CURs/REQs matching..."
             
             # -- if self._hlAmpCur.Value() != self._hlAmpReq.Value():
             # --     raise WrongAmpException(self._hlAmpCur.Value(), self._hlAmpReq.Value())
             # -- if self._hlFreqCur.Value() != self._hlFreqReq.Value():
             # --     raise WrongFreqException(self._hlFreqCur.Value(), self._hlFreqReq.Value())
             # -- if self._hlOffxCur.Value() != self._hlOffxReq.Value():
             # --     raise WrongOffxException(self._hlOffxCur.Value(), self._hlOffxReq.Value())
             # -- if self._hlOffyCur.Value() != self._hlOffyReq.Value():
             # --     raise WrongOffyException(self._hlOffyCur.Value(), self._hlOffyReq.Value())
             # -- print "...ok !"
         except AOVarError, e:
             print e
             raise TTStartException
         except CurTimeoutException, e:
             print e
             raise TTStartException
         
         
    def stop(self):
        ''' Stop the TipTilt.
            Can raise a TTStartException or a WrongXXXException '''

        if not self.checkPolicy():
            return
            
        # Set all CUR vars to unknown value
        self.__resetAllCur()
        
        try:
            # Wait for all CUR vars updated, or timeout
            self.app.SetVar(self._go, 0)  # Can raise an AOVarError
            self.__waitAllCurUpdated()    # Can raise an AdOptError for timeout
        except AOVarError, e:
            print e
            raise TTStopException
        except CurTimeoutException, e:
            print e
            raise TTStopException




    def loadCalib(self, filename):
        '''
        Loads a calibration table.
        Calibration must be a rectangular array with regular spacing. Each line of the input file defines a point of the grid:
        <frequency> <modulation> <amp. in microns>

        Result:
        self._freqTable is a dictionary where each key defines a frequency list, each element of which is a (volt,amp) pair, with volts in ascending order.
        self._voltTable is the same with frequency and volt swapped.
        self._freqArray and self._voltArray are the same lists as InterpolatedArray()s
        '''

        lines = file(filename).readlines()
        self._freqTable = {}
        self._voltTable = {}
        self._freqArray = {}
        self._voltArray = {}
        for line in lines:
            freq, volt, amp = map( float, line.split())
            try:
                self._freqTable[freq].append( (volt, amp))
            except KeyError:
                self._freqTable[freq] = [(volt,amp)]
            try:
                self._voltTable[volt].append( (freq, amp))
            except KeyError:
                self._voltTable[volt] = [(freq,amp)]

        def cmp_volts(a, b):
            '''
            In-place sorting using the first value.
            '''
            return cmp(a[0], b[0])

        self._voltTable[0] = []
        for f in self._freqTable.keys():
            self._freqTable[f].append((0,0))
            self._freqTable[f].sort(cmp_volts)
            self._freqArray[f] = InterpolatedArray( self._freqTable[f])
            self._voltTable[0].append( (f,0))
        for v in self._voltTable.keys():
            self._voltTable[v].sort(cmp_volts)
            self._voltArray[v] = InterpolatedArray( self._voltTable[v])

    def micron2psf( self, micron):
        '''
        Microns -> psf conversion.  
        '''
        lambdaD = 750e-9 * 45. * 1e6
        return micron / lambdaD

    def psf2micron( self, psf):
        '''
        Psf -> microns conversion.  
        '''
        lambdaD = 750e-9 * 45. * 1e6
        return psf*lambdaD


    def modulationVF2Psf(self, freq, volt):
        '''
        Returns the modulation in lambda/D at the specified frequency and volt setting.
        '''

        # Square-interpolate the four closest points to freq-volt
        volts = sorted(self._voltTable.keys())

        if volt<volts[0]: 
            v0=0
        else:
            v0 = volts[ map( lambda x: x<=volt, volts).index(False)-1]
        v1 = volts[ map( lambda x: x>volt, volts).index(True)]
        
        # Now we have v0 <= volt < v1 

        amp1 = self._voltArray[v0][freq]        
        amp2 = self._voltArray[v1][freq]        

        vFact = (volt-v0)/(v1-v0)
        amp = (amp2-amp1)*vFact + amp1

        return self.micron2psf(amp)


    def modulationLambdaDF2V(self, lambdaD, freq):
        '''
        Returns the volt setting needed to obtain a modulation of the requested lambda/D at the specified frequency.
        '''

        try:
            # If freq is defined, use that
            if self._freqArray.has_key(freq):
                useArray = self._freqArray[freq]

            # Otherwise, use the two closest frequencies and interpolate a frequency array
            else:
                useTable = []
                for v in self._voltTable.keys():
                    mod = self._voltArray[v][freq]
                    useTable.append( (v, mod))
                useArray = InterpolatedArray(useTable)

            # Step search for the right value
            epsilon = 1e-2
            value = lambdaD+epsilon*2
            volt=1
            step=0.1
            dir =1
            max_iters = 10000

            while (abs(value-lambdaD) > epsilon) and (max_iters>0):
                value = self.micron2psf( useArray[volt])
                if value < lambdaD:
                    if dir != 1:
                        step /=2.
                    volt += step
                    dir=1
                if value > lambdaD:
                    if dir != -1:
                        step /=2.
                    volt -= step
                    dir = -1
                max_iters-=1

            if max_iters==0:
                raise AOExcept.AdOptError(errstr="TT modulation not found")
        
        except (ValueError, KeyError):
            raise AOExcept.AdOptError(errstr="TT modulation not found")
        

        return volt
        





#def modulationLD2V( app, framerate, modulationLD):
#    '''
#    Compute the Volt setting needed to have a modulation of the specified L/D.
#    '''
#    modAmp = ((100,120.0), (200,106.6), (400,76.), (600,56.0), (1000,25.3))
#    t = InterpolatedArray(modAmp)
#    amp = 10 * (modulationLD / t[framerate])
#    if (amp >5) or (amp<0):
#        raise ValueError("TT modulation out of allowed range for frequency %f" % frm)



    ##     
    ## ---------------------- PRIVATE METHODS --------------------------
         
    ##
    # Started in a thread when a cur var is updated on RTDB
    # The lock prevent concurrent access to cur vars with
    # 
    def __curVarChanged(self, var):
        self._curLock.acquire()
        if var.Name() == self._hlAmpCur.Name():
            self._hlAmpCur.SetValue(var.Value())
        elif var.Name() == self._hlFreqCur.Name():
            self._hlFreqCur.SetValue(var.Value())
        elif var.Name() == self._hlOffxCur.Name():
            self._hlOffxCur.SetValue(var.Value())
        elif var.Name() == self._hlOffyCur.Name():
            self._hlOffyCur.SetValue(var.Value())    
        elif var.Name() == self._setCur.Name():
            self._setCur.SetValue(var.Value())    
        self._curLock.release()
        self.__allCurUpdated()
        
    def __resetAllCur(self):
        self._hlAmpCur.SetValue(None)
        self._hlFreqCur.SetValue(None)
        self._hlOffxCur.SetValue(None)
        self._hlOffyCur.SetValue(None)
            
    def __waitAllCurUpdated(self):
        start = time.time()
        self._curVarUpdated_event.wait(self._timeout_s)
        self._curVarUpdated_event.clear()
        if (time.time() - start) > self._timeout_s:
            raise CurTimeoutException(self._timeout_s)
                 
            
    def __allCurUpdated(self):
        self._curLock.acquire()
        if self._hlAmpCur.Value() != None and \
           self._hlFreqCur.Value() != None and \
           self._hlOffxCur.Value() != None and \
           self._hlOffyCur.Value() != None:

            self._curVarUpdated_event.set() 

        self._curLock.release()



##
# ---------- TIP-TILT EXCEPTIONS -----------
#

##
# Root exception for TipTilt
class TTException(AdOptError):
    ''' Generic excpetion, root of all tipttilt excpetions'''
    def __init__(self, code, errstr):
        AdOptError.__init__(self, code, errstr)

class TTCreationException(TTException):
    def __init__(self, code, errstr):
        TTException.__init__(self, code, "> Fatal error: "+errstr)

class TTStartException(TTException):
    def __init__(self):
        TTException.__init__(self, AOErrCode['CTRL_NOT_STARTED_ERROR'], "> Fatal error: TipTilt not started")
        
class TTStopException(TTException):
    def __init__(self):
        TTException.__init__(self, AOErrCode['CTRL_NOT_STOPPED_ERROR'], "> Fatal error: TipTilt not stopped")   

##
# Raised when the notify for CURs variables updated doesn't
# come on time
class CurTimeoutException(TTException):
    ''' CURs update timed-out '''
    def __init__(self, timeout):
        TTException.__init__(self, AOErrCode['TIMEOUT_ERROR'], "Timeout "+str(timeout)+" s expired for CUR variables updating")

##
# Set of exceptions to indicate that a CUR var doesn't match 
# the corresponding REQ

class CurNotMatchReqException(TTException):
    ''' Genaral exception: catch this if you are not interested on wich
        CUR variable doesn't match the corresponding REQ '''
    def __init__(self, curValue, reqValue, errstr, shortVarName):
        print curValue, reqValue
        TTException.__init__(self, AOErrCode['VAR_CUR_MISMATCH_REQ_ERROR'], "> Error: Cur %s value %s doesn't match Req value %s" % (shortVarName, str(curValue), str(reqValue)))

class WrongAmpException(CurNotMatchReqException): 
    def __init__(self, curValue, reqValue):
        CurNotMatchReqException.__init__(self, curValue, reqValue, "Matching error", "AMP")

class WrongFreqException(CurNotMatchReqException): 
    def __init__(self, curValue, reqValue):
        CurNotMatchReqException.__init__(self, curValue, reqValue, "Matching error", "FREQ")
        
class WrongOffxException(CurNotMatchReqException): 
    def __init__(self, curValue, reqValue):
        CurNotMatchReqException.__init__(self, curValue, reqValue, "Matching error", "OFFX")
        
class WrongOffyException(CurNotMatchReqException): 
    def __init__(self, curValue, reqValue):
        CurNotMatchReqException.__init__(self, curValue, reqValue, "Matching error", "OFFY")

## 
# DEMO MAIN
if __name__ == "__main__":
    aoApp = AOApp("TTCTRL", False)
    
    try:
        # Create the tiptilt
        tipTilt = TipTilt("ttctrl", aoApp, 1)
    
        # Do some operations
        tipTilt.set(1, 300, 2, 0.5)
        tipTilt.stop()
        
        tipTilt.start()
        
        tipTilt.set(2, 200, 1, 1)  # ZERO ERROR: try tipTilt.set(0, 200, 0, 6)
        tipTilt.stop()
        print "Test done."
    except TTCreationException, e:
        print e
    except CurNotMatchReqException, te:
        print te
    except TTStartException, te:
        print te
    except TTStopException, te:
        print te
    # To catch everything without difference use this !
    #except TTException, e:
    #    print e
        
    # This prevent the error "Unhandled exception in thread started 
    # by - Error in sys.excepthook:" (google for it!)
    #time.sleep(1)
    
