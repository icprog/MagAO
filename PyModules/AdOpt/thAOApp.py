#!/usr/bin/env python
#
#+
#
# Methods implemented are:
#
#    NAME                IN      RET      DESCR 
#
# MsgD client:
#
# MyName:                         s       return the application name 
# SetReady:                               notify the MsgD your app is ready
# WaitClientReady:   clientname           wait for a client to become ready (block)
#
# Variables:
#
# CreaVar:           var                  create a rtdb variable. Check by reading back
# WriteVar:          var                  write  a rtdb variable. Create it if not present. 
# SetVar:            var                  set an rtdb variable. Doesn't check.
# ReadVar:           varname     var      read   a rtdb variable. 
# VarList:           prefix      varlist  read a list of rtdb variables.
# VarNotif:          varname,handler      call handler when varname is changed
# VarNoNotif:        varname,handler      don't call handler anymore when varname is changed
#
# Shared memory:
#
# BufRead:           bufname     data buffer  reads a shared memory buffer (only BUFCONT implemented)


VERSION="2.1"

import time, sys, signal, threading, atexit
from qt import *
from qtnetwork import *

from AOVar import *
from AOConstants import *
from AOExcept import *
import AOConfig

from AdOpt.wrappers import thrdlib, logger


class thAOApp:
    """
    This is a base class implementing a python application for use in the 
    framework of the LBT Adaptive Optic Software System.
    The base class AOApp provides connection to MsgD
    and a listener thread that reacts to socket messages from
    MsgD. It provides methods to synchronize messaging with the MsgD

    INTERNALS
    When created, an AOApp object connects to the MsgD and starts the
    thrdlib handling mechanism. All thrdlib features are wrapped in equivalent
    Python functions. Additionally, RTDB variables can be mirrored in local
    AOVar objects without using a callbac, and message handlers can be installed
    directly for a variable change notification.


    History: 
    Sept 2005 - created by Lorenzo Busoni <lbusoni@arcetri.astro.it>
    Apr  2008 - Alfio Puglisi converted to thrdlib

    """
    
   
    
    def __init__(self, myname, multi=False, debug=False, serverfd = None, cfg = None, argv = None, side = None, msgHandlers= None, ip=None):
        '''
thAOApp constructor.
<myname>:     MsgD client name prefix.
              If <multi> is True, client name is make with the prefix plus a counter.
<multi>       if True, turn on auto-number of client name. If False, connection may be
              rejected if another client with the same name is present.
<msgHandler>: if present, is a dictionary of handlers {msgcode: handlerFunc} that will 
              be installed in addition to the standard thAOApp ones.
<argv>:       Dictionary of additional arguments. 
              -f <cfg file>   specifies config file
<cfg>:        Array of configuration items. If not specified config items are read from
              configuration file
<ip>:         MsgD IP:port (e.g.: 192.168.5.5:9752). If not specified it is taken
              from environment variable ADOPT_SERVER, if avialable. Else dafaults
              to: 127.0.0.1:9752

'''

        # Parse command line arguments, if any
        self.__cfgfile = None
        self.__logToStdout = False
        self.__logToFd = None
        self.Version=VERSION
        self._identity = ''

        if argv:
            if '-stdout' in argv:
                self.__logToStdout = True
            if '-d' in argv:
                self.__logToFd = int(argv[ argv.index('-d')+1])
            if '-f' in argv:
                self.__cfgfile = argv[ argv.index('-f')+1]

	# Remove -i from sys.argv
        if '-i' in sys.argv:
            idx = sys.argv.index('-i')
            del sys.argv[idx]  # -i
            self._identity = sys.argv[idx]
            myname = self._identity+'.'+os.environ['ADOPT_SIDE']
            del sys.argv[idx] 
	  

        # Use the cfg array specified in the constructor
        # - OR -
        # read the configuration file in the command line
        # - OR -
        # leave the cfg array uninitialized: the myname argument becomes mandatory and part of the setup will be skipped

        if cfg != None:
            self._cfg = cfg
        elif self.__cfgfile != None:
            self._cfg = AOConfig.read_config(self.__cfgfile, add_side = True)
        else:
            self._cfg = None

        # Can raise KeyError exception
        if not self._cfg:
             if side != None:
                self._cfg = dict()
                self._cfg['mySide'] = AOVar(name='mySide', tipo='CHAR_VARIABLE', value =side)

        # Transfer logger level here - other modules should not include the logger module
        self.LOG_LEV_TRACE    = logger.LOG_LEV_TRACE
        self.LOG_LEV_DEBUG    = logger.LOG_LEV_DEBUG
        self.LOG_LEV_INFO     = logger.LOG_LEV_INFO
        self.LOG_LEV_WARNING  = logger.LOG_LEV_WARNING
        self.LOG_LEV_ERROR    = logger.LOG_LEV_ERROR
        self.LOG_LEV_FATAL    = logger.LOG_LEV_FATAL
# L.F. removed: l'asterisco serve per non essere trovato da grep
#        self.LOG_LEV_*DISABLED = logger.LOG_LEV_*DISABLED
#        self.LOG_LEV_*ALWAYS   = logger.LOG_LEV_*ALWAYS



        self.__notif = {}
        self.__sharedBufsRead = {}
        self.__sharedBufsWrite = {}
        self.__nextID = 0
        self.__timeout = 1000   # Generic timeout for messages (ms)
        self.__timeToDie = False
        self.__varToRetry = []
        self.__retryTimer = None

        # connect to MsgD
        if not ip:
            try:
                addr = os.environ['ADOPT_SERVER']
            except KeyError:
                addr = '127.0.0.1'
        else:
            addr=ip
        self.log('Using server: %s' % addr, self.LOG_LEV_DEBUG)

        if not multi:
            self.__myname = myname
            thrdlib.thInit( self.__myname)

            # Install our default variable handler
            thrdlib.thHandler( handler = self.varHandler, code = thrdlib.VARREPLY)
            thrdlib.thHandler( handler = self.varHandler, code = thrdlib.VARCHANGD)
            thrdlib.thHandler( handler = self.termHandler, code = thrdlib.TERMINATE)

            # Install optional handlers
            if msgHandlers:
                for key in msgHandlers.keys():
                    thrdlib.thHandler( handler = msgHandlers[key], code = key)

            thrdlib.thStart( IP=addr)

        else:
            counter=0
            while counter <100:
                self.__myname = "%s%02d" % (myname,counter)
                try:
                    thrdlib.thInit( self.__myname)
                    # Install our default variable handler
                    thrdlib.thHandler( handler = self.varHandler, code = thrdlib.VARREPLY)
                    thrdlib.thHandler( handler = self.varHandler, code = thrdlib.VARCHANGD)
                    thrdlib.thHandler( handler = self.termHandler, code = thrdlib.TERMINATE)

                    # Install optional handlers
                    if msgHandlers:
                        for key in msgHandlers.keys():
                            thrdlib.thHandler( handler = msgHandlers[key], code = key)

                    thrdlib.thStart(IP=addr)
                    break
                except:
                    thrdlib.thCleanup()
                    self.log( "Connection as %s refused" % self.__myname, self.LOG_LEV_ERROR)
                    counter += 1

                    if counter == 100:
                        raise AOConnectionError

        logger.setName( self.__myname)
        logger.setLevel( self.LOG_LEV_DEBUG)
        self.log( 'Connected as '+self.__myname, self.LOG_LEV_DEBUG)
        atexit.register( logger.archive)

        # Create a string variable in RTDB. This can be used to eval commands.
        # Mainly for debugging purpose.
        if debug==True:
            prefix = ''
            cmdVar=AOchar(self.__myname+'.CMDLINE') 
            self.WriteVar(cmdVar)
            self.VarNotif(cmdVar.Name(), self.evalCmd)
            
        # Trap the Ctrl-C signal and kill (used from GUI stop button)
        signal.signal(signal.SIGINT, self.signal_handler)
        signal.signal(signal.SIGTERM, self.signal_handler)

    def identity(self):
	return self._identity

    def clientname(self):
        return self.__myname
        
    def signal_handler(self, sig, frame):
        if sig == signal.SIGINT or sig == signal.SIGTERM:
            self.log('SIGINT received... cleaning-up application...', self.LOG_LEV_DEBUG)
            thrdlib.thClose()
            thrdlib.thCleanup()
            self.log('MsgD communication terminated', self.LOG_LEV_INFO)
            sys.exit(0)


    def log(self, args, level=None):
        """
        Replacement for all output functions, uses the AO log convention
        """

        lines = str(args).splitlines()
        for line in lines:
           self._log(line, level)

    def _log(self, args, level=None):
        if self.__logToStdout == 1:
            print args
            sys.stdout.flush()

        elif self.__logToFd != None:
            os.write( self.__logfd, str(args))
            os.write( self.__logfd, '\n')
            os.fsync( self.__logfd)

        else:
            if level != None:
                logger.log( str(args), level)
            else:
                logger.log( str(args))

    def debug(self, *args):
        self.log( args, self.LOG_LEV_DEBUG)

    def evalCmd(self,var):
        """
        Used to eval commands given through the CMDLINE var 
        """
        try:
            print eval(var.Value())
        except:
            raise

    def MyName(self):
        """
        Return the name used for the connection with MsgD
        """
        return self.__myname

    def TimeToDie(self):
        return self.__timeToDie

    def termHandler( self, message):
        self.log('TERMINATE received. It''s TimeToDie.', self.LOG_LEV_INFO)
        self.die()

    def die(self):
        if self.__retryTimer:
            self.__retryTimer.cancel()
        self.__timeToDie = True

    def varHandler( self, message):

        if not message['name'] in self.__notif.keys():
            return

        var = Msg2Var(message)

        # Mirror variables and call variable handlers
        for handler in self.__notif[var.Name()]:
            if handler['var']:
                handler['var'].Copy(var)
            if handler['func']:
                if handler['args']:
                    args = (var,) + handler['args']
                else:
                    args = var
                handler['func'](args)


##################################################
# Generic msg interface
#################################################

    def SendMsg(self,to,code,data,timeout=None,binary=False, payload=0, ignf=0, ttl=0, want_answer = True):

        if type(data) is not str: raise AOTypeError
           
        seqn = thrdlib.thSendMessage( code, to, data, payload, ignf, ttl);

        if not want_answer:
            return

        if not timeout:
            timeout = 60000

        return thrdlib.thWaitMsg( seqn = seqn, timeout = timeout)

    def ReplyMsg(self,msgb,code, data):

        return thrdlib.thReplyMsg( msgb, code, data)


###################################################
# Handling of RTDB variables
###################################################

    def WriteVar(self,var, nocreate=False):
        if isinstance(var,AOVar) == False:
            errstr = "WriteVar Error: arg 1 must be an instance of AOVar class"
            raise AttributeError, errstr

        if nocreate:
            thrdlib.thWriteVar( var.Name(), AOVarCode[ var.Type()], var.Value(), timeout=-1)    
        else:
            thrdlib.thWriteVar( var.Name(), AOVarCode[ var.Type()], var.Value(), timeout = self.__timeout)
            if var.owner == 'PUBLIC':
                thrdlib.thVarOp( var.Name(), thrdlib.GIVEVAR)

    # Returns one or more variable objects read from MsgD-RTDB.
    # Name can include wildcards ('*')
    #
    # If only one variable is needed, ReadVar() is more efficient
 
    def ReadVarList(self, prefix="*"):
        
        if isinstance(prefix, str) == False:
            errstr = "ReadVarList Error: arg 1 must be a string"
            raise AttributeError, errstr

        try:
            lst = thrdlib.thReadVar( prefix)
        except thrdlib.error:
            self.log('thrdlib: error reading variable list %s' % prefix)
            return []

        vars = [Msg2Var(x) for x in lst]
        return vars

    # Returns one variable object read from MsgD-RTDB.

    def ReadVar( self, name):
        
        if isinstance(name, AOVar):
            varname = name.Name()
        else:
            varname = name

        if isinstance(varname,str) == False:
            errstr = "ReadVar Error: arg 1 must be a string or a variable object"
            raise AttributeError, errstr

        msg = thrdlib.thGetVar( varname)
        if len(msg)<1:
            raise AOVarNotInRTDB, 'ReadVar error: variable %s not found' % varname

        var = Msg2Var( msg)

        if isinstance(name, AOVar):
            name.Copy(var)
        return var


    

    def CreaVar(self, var):
        return self.WriteVar(var)

    def SetVar(self, var, value=None):

        var.SetValue(value)
        return self.WriteVar(var, nocreate=True)

    def DelVar(self, name, timeout=None):
        '''
        Remove a variable from RTDB
        '''
        thrdlib.thVarOp( name, thrdlib.DELVAR)

    def LockVars(self, prefix, timeout=None):
        thrdlib.thVarOp( prefix, thrdlib.LOCKVAR, timeout)

    def UnlockVars(self, prefix, timeout=None):
        thrdlib.thVarOp( prefix, thrdlib.UNLOCKVAR, timeout)

    def VarNotif(self, name, sigChanged=None, args = None, retry = False):
        '''
        Ask to be notified when a variable V is changed.
        name can be a string or an AOVariable.
        If sigChanged is a valid function, it will be run in a separate thread when the variable is changed.
        If sigChanged is None and name is an AOVar, when V is changed it is mirrored in name.

        The <retry> parameter is obsolete and should not be used

        Arguments for sigChanged will be the variable object and, if any the additional arguments <args>

        Note that multiple functions can be defined for a single variable: in this case, 
        the functions will be scheduled sequentially.

        NOTE: given 2 different variables A and B modified in this order, since the handling 
        functions are run in separate threads it can happen that the handler of A terminates 
        after the handler of B. At the moment there is no way to serialize these threads.
        '''


        if sigChanged == None and isinstance(name,AOVar) == False :
            return

        if isinstance(name,AOVar) == True:
            var = name
            name = var.Name()
        else:
            try:
                var = self.ReadVar(name)
            except (thrdlib.varNotFound, AOVarNotInRTDB):
                if retry and not self.TimeToDie():
                    self.__varToRetry.append((name, sigChanged, args))
                    if self.__retryTimer == None:
                        self.__retryTimer = threading.Timer( 10, self.retryNotif)
                        self.__retryTimer.start()
                    return
                raise

        if var.Name() not in self.__notif.keys():
            self.__notif[var.Name()] = []

        id = self.nextID()

        if sigChanged:
            handler = {'id': id, 'var': None, 'func': sigChanged, 'args': args}
            self.__notif[var.Name()].append(handler)
        else:
            handler = {'id': id, 'var': var,  'func': None, 'args': None}
            self.__notif[var.Name()].append(handler)

        thrdlib.thVarOp( var.Name(), thrdlib.VARNOTIF, 1)

        return id

    def retryNotif( self):

        self.__retryTimer = None

        n = len(self.__varToRetry)
        for i in range(n):
            vv = self.__varToRetry.pop(0)
            self.VarNotif(vv[0], sigChanged = vv[1], args = vv[2], retry = True)
            

    def VarNoNotif(self, name, handler_id):
        '''
        Remove variable notification request and handlers
        '''

        lst = self.__notif[name]
        self.__notif[name] = filter( lambda x: x['id'] != handler_id, lst)

        # Remove variable notification if no handlers are left
        if len( self.__notif[name]) == 0:
            thrdlib.thVarOp( name, thrdlib.VARNOTIF, 0)

    def SetReady(self):
        thrdlib.thSendMsg( AOMsgCode['SETREADY'])

    def WaitClientReady(self, client, timeout=None, sighandler=None):
        ''' 
        Set timeout < 0 if you want to be asynchronously notified when the
        client will become ready. Set also sighandler to a valid function name
        that will be called when the client will become ready
        Set timeout to a positive number for a blocking version of this 
        function.
        Set timeout to None to wait until necessary with no timeout
        '''

        if timeout >= 0:
            thrdlib.thWaitReady(client, timeout)
            return

        thrdlib.thSendMsg(AOMsgCode['WAITCLRDY'], client)
        thrdlib.thHandler(sighandler, thrdlib.WAITCLRDY) 

    # Small helper class for WaitVarValue
    class varevent:
        def __init__(self, event, var):
            self.event = event
            self.var = var

        
    def WaitVarValue(self, var, value, timeout=None):
        ''' 
        Waits for the specified variable to assume the specified value.
        Set timeout to None (default) to wait as long as necessary
        Raises an exception if the timeout is exceeded.
        '''

        self.WaitVarFunc(var, lambda x: x==value, timeout=timeout)

    def WaitVarFunc(self, var, func, timeout=None):
        ''' 
        Waits for the specified variable to satisfy the callback function (which must return True)
        Set timeout to None (default) to wait as long as necessary
        Raises an exception if the timeout is exceeded.
        '''

        # I only want the name of the var, not the object

        if isinstance(var, AOVar):
            varname = var.Name()
        else:
            varname = var

        # Setup a threading event.
        # We'll call VarNotif and block waiting for the event to be set. The handler function
        # will unblock us when the variable value satisfy the function.

        ev = self.varevent( threading.Event(), None)

        # Start the notifications BEFORE asking for the current value, otherwise
        # we could get into a race condition and we would miss the change.
        id = self.VarNotif( varname, sigChanged = self.unblockHandler, args = (ev, func) )

        # See if the right value is already present
        v = self.ReadVar(varname).Value()
        if func(v) == True:
            self.VarNoNotif( varname, handler_id = id)
            return

        start = time.time()
        ev.event.wait(timeout)  # Wait to be unlocked by event handler

        # always unregister
        self.VarNoNotif( varname, handler_id = id)

        if not ev.event.isSet():
            errstr = "Timeout waiting for %s" % varname
            raise AdOptError, (AOErrCode['TIMEOUT_ERROR'], errstr)


    def unblockHandler(self, args):
        var, ev, func = args
        if func(var.Value()) == True:
            ev.event.set()

    def SetAndWaitCur( self, req_var, cur_var, value, timeout=None):
        '''
        Sets a _REQ variable and waits for the corresponding _CUR variable
        to reflect the change. If timeout is None (default) will wait indefinitely.
        Raises an exception if the timeout is exceeded.
        '''

        req_var.SetValue(value)
        self.WriteVar( req_var, nocreate = True)
        self.WaitVarValue( cur_var, value, timeout = timeout)

         
    def Ping(self,client, timeout=None):
        '''
        Ping a client and wait for the response
        '''
        seqn = thrdlib.SendMsg(AOMsgCode['ECHO'], client)
        thrdlib.WaitMsg(seqn = seqn, timeout = timeout)


#################################
# IDL commands
#################################
    def IDLCmd(self, cmd, idlctrl=None):
        '''
        Send an IDL string to the IDL controller and return the idl answer dictionary
        Raise an exception in case of error code not null in the answer
        '''
        if isinstance(cmd,str) == False:
            errstr = "IDLCmd Error: arg 1 must be a string"
            raise AttributeError, errstr

        if not idlctrl:
            from AdOpt import cfg
            idlctrl = 'idlctrl.'+cfg.side+'@M_ADSEC'

        answer = self.SendMsg(idlctrl, AOMsgCode['IDLCMD'], cmd, timeout=2000, want_answer = True)

	return answer

##################################
# Shared memory functions
##################################

    def BufCreate( self, bufname, bufsize, maxCons=10):
        '''
        Creates a BUFCONT shared buffer.
        '''

        if not self.__sharedBufsWrite.has_key(bufname):
            result = thrdlib.bufCreate( self.MyName(), bufname, bufsize, maxCons)
            if result['errcode'] == AOErrCode['NO_ERROR']:
                self.__sharedBufsWrite[bufname] = {'bufinfo': result['bufinfo'], 'counter': 0}
            else:
                raise AdOptError(code = result['errcode'], errstr='Creating shared buffer '+bufname)

        return AOErrCode['NO_ERROR']

    def BufWrite( self, bufname, data, counter=None, timeout=100):
        '''
        Writes into a shared memory buffer. The timeout value is only used for BUFSNAP buffers.
        If the buffer does not exists yet, it will be created with default parameters (BUFCONT and maxCons=10)
        Buffer counter is normally automatically incremented. If <counter> has a valid value, the counter
        will be reset to that value.
        '''

        if not self.__sharedBufsWrite.has_key(bufname):
            self.BufCreate( bufname, len(data))

        if counter != None:
            self.__sharedBufsWrite[bufname]['counter'] = counter

        errcode = thrdlib.bufWrite( self.__sharedBufsWrite[bufname]['bufinfo'], data, self.__sharedBufsWrite[bufname]['counter'], timeout)
        if errcode != AOErrCode['NO_ERROR']:
            raise AdOptError(code = errcode, errstr='Writing into shared buffer '+bufname)
       
        self.__sharedBufsWrite[bufname]['counter'] += 1 
        return AOErrCode['NO_ERROR']


        
    def BufRead(self, bufname, timeout = 100, nextCounter=None):
        '''
        Reads a BUFCONT shared buffer. It will attach to the buffer the first time it is requested.
        Errors result in AdOptError exceptions. Errors different from timeouts will result in another
        attach the next time the buffer is read.
        Timeout is in milliseconds (default: 100)
        If nextCounter is not None, the function will wait for a frame counter greater than the one specified,
        up to the specified timeout.

        Returns: dictionary with 'counter' and 'data' keys. Data is a binary string.
        '''

        if not self.__sharedBufsRead.has_key(bufname):
            print 'Attaching to buffer '+bufname 
            result = thrdlib.bufRequest( self.MyName(), bufname)
            if result['errcode'] == AOErrCode['NO_ERROR']:
                self.__sharedBufsRead[bufname] = result['bufinfo']
            else:
                raise AdOptError(code=result['errcode'], errstr='Attaching to shared buffer '+bufname)

        frameValid = False
        ritenta=10
        while not frameValid:
            start = time.time()
            data = thrdlib.bufRead( self.__sharedBufsRead[bufname], timeout)
            if data['errcode'] != AOErrCode['NO_ERROR']:
                if data['errcode'] == -69042:
                    ritenta -= 1
                    if ritenta<0:
                    	del self.__sharedBufsRead[bufname]
                        raise AdOptError( code = data['errcode'], errstr='Reading shared buffer '+bufname)
                    continue 
                # Delete local bufinfo, except in case of a timeout
                if data['errcode'] != AOErrCode['TIMEOUT_ERROR']:
                    print 'Error %d, detaching from buffer %s' % (data['errcode'], bufname)
                    del self.__sharedBufsRead[bufname]
                raise AdOptError( code=data['errcode'], errstr='Reading shared buffer '+bufname)

            frameValid = True

            # Check whether the counter has advanced enough
            if nextCounter != None:
                if data['counter'] <= nextCounter:
                    frameValid = False
                    now = time.time()
                    timeout -= (now-start)*1000
                    if timeout<0:
                        raise AdOptError( code=AOErrCode['TIMEOUT_ERROR'], errstr='Reading shared buffer '+bufname)

        return {'counter': data['counter'], 'data': data['data']}

    def nextID( self):
        self.__nextID += 1
        return self.__nextID


    def mute(self):
        '''
        Blocks stdout and stderr.
        '''

        self.stdout = sys.stdout
        self.stderr = sys.stderr

        class mutefile:
            def write(self,s):
                pass
            def flush(self):
                pass

        sys.stdout = mutefile()
        sys.stderr = mutefile()


    def unmute(self):
        '''
        Restores stdout and stderr.
        '''

        sys.stdout = self.stdout
        sys.stderr = self.stderr

    def execfile(self, filename, verbose=True):
        '''
        Execs a command file with verbose output
        '''

        for line in file(filename).readlines():
            self.log("Executing: "+line.strip(), self.LOG_LEV_DEBUG)
            if verbose:
                print line.strip()+'\n'
            bytecode = compile(line, '<string>', 'single')
            eval(bytecode)

### Arbitrator messages

    def arbMsg( self, msg, level=3, arb='wfsarb'):
        '''
        Writes a message in the specified arbitrator MSG variable
        '''
	from AdOpt import cfg, AOVar
        varname = cfg.varname(arb, 'MSG')
        var = AOVar.AOVar( varname, tipo='CHAR_VARIABLE', value='')
        var.SetValue('%d:%s' % (level, msg))
        self.WriteVar(var)






    
##################################   
# End AOApp class
##################################


def Msg2Var(message):
    return AOVar(message['name'],AOVarType[message['type']],\
        message['value'], message['owner'] or 'PUBLIC', message['mtime'])




