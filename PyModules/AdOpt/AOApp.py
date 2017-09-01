#!/usr/bin/env python
#
#@File: AOApp.py
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
#
# Misc:
#
# IDLCmd:            cmd                  send a cmd string to IDL ctrl 
#
#@



#import string, time
import time
from qt import *
from qtnetwork import *
from AdOpt.wrappers import msglib
import sys, signal, thread, threading, Queue, select

from AOVar import *
from AOConstants import *
from AOExcept import *
import AOConfig

class AOApp:
    """

    THIS CLASS IS OBSOLETE.

    All new applications should use the new thAOApp class instead.
    -Alfio Puglisi Jan 2008


    This is a base class implementing a python application for use in the 
    framework of the LBT Adaptive Optic Software System.
    The base class AOApp provides connection to MsgD
    and a listener thread that reacts to socket messages from
    MsgD. It provides methods to synchronize messaging with the MsgD

    INTERNALS
    When created, an AOApp object connects to the MsgD and starts a new thread
    called "listener". In the "listener" thread a QSocketNotifier is created,
    attached to the MsgD and put in listening mode.

    Asynchronous messaging is synchronized through this private "listener" socket:
    accesses to a variable in the RTDB are requested via the msglib in the main thread,
    while answers from MsgD are received by the "listener" thread that routes the messages 
    putting them in queues. 
    The main thread is notified when a message is available and gets it from the queue.
    Each thread has a private queue, thus allowing derived applications to be multi-threaded.

    History: 
    Sept 2005 - created by Lorenzo Busoni <lbusoni@arcetri.astro.it>

    Bugs/TODO:
    - Very slow, probably due to Queue get implemented with timeouts
    """
    
   
    
    
    def __init__(self, myname, debug=False, serverfd = None, cfg = None, argv = None):

        # Parse command line arguments, if any
        self.__cfgfile = None
        self.__myname = myname

        if argv:
            if '-m' in argv:
                self.__logmethod = int(argv[ argv.index('-m')+1])
            if '-d' in argv:
                self.__logfd = int(argv[ argv.index('-d')+1])
            if '-f' in argv:
                self.__cfgfile = argv[ argv.index('-f')+1]

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

        self.__logmethod = 1

        self.__wait_msgid = {}
        self.__notif = {}
        self.__fifo = {}
        self.__events = {}
        self.__fifo_timeout =  10 # [s] timeout Queue
        self.__lock = thread.allocate_lock()
        self.__sharedBufsRead = {}
        self.__sharedBufsWrite = {}

        # connect to MsgD
        if serverfd:
            self.__ServerFD = serverfd
        else:
            if self.__myname:
                self.__ServerFD = msglib.ConnectToServer( self.__myname)
                if self.__ServerFD <0:
                    raise AOConnectionError, "Error connecting to MsgD-RTDB server: " + msglib.GetErrorString( self.__ServerFD)
            else:
                counter=0
                while counter <100:
                    self.__myname = "%s%02d" % (myname,counter)
                    self.__ServerFD = msglib.ConnectToServer( self.__myname)
                    if self.__ServerFD >= 0:
                        break
                    self.log( "Connection as %s refused" % self.__myname)
                    counter += 1

                if counter == 100:
                    errstr = "Error connecting to MsgD-RTDB server: " + msglib.GetErrorString( self.__ServerFD)
                    raise AOConnectionError, errstr

        self.log( 'Connected as '+self.__myname)

        # start thread for socket listening
        listener_id = thread.start_new_thread(self.listen, ())

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
        
    def signal_handler(self, sig, frame):
        if sig == signal.SIGINT or sig == signal.SIGTERM:
            self.log('SIGINT received... cleaning-up application...')
            ret = msglib.CloseServer("%s" %(self.__myname))
            self.log('MsgD communication terminated (code: %d)' %ret)
            sys.exit(0)
        

    def log(self, *args):
        """
        Replacement for all output functions, uses the AO log convention
        """
        for x in args:
            if self.__logmethod == 1:
                print x
                sys.stdout.flush()
            elif self.__logmethod == 2:
                os.write( self.__logfd, str(x))
                
            elif self.__logmethod == 3:
                pass

        if self.__logmethod == 2:
            os.write( self.__logfd, '\n')
            os.fsync( self.__logfd)

    def debug(self, *args):
        log( args)

                
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

    def SetTimeout(self, tmt):
        self.__fifo_timeout=tmt

    def Timeout(self):
        return self.__fifo_timeout

#######################################
# Listening for messages from MsgD
#######################################

    def listen(self):
        # Create a listener on the socket   
        #self.listener = QSocketNotifier( self.__ServerFD, QSocketNotifier.Read)
        #self.listener.setEnabled(1)
        #if QObject.connect( self.listener, SIGNAL("activated(int)"), self.slotListener) != True:
        #    errstr = "listener error: connecting socket signal"
        #    raise AdOptErr, errstr
        # 
        #self.app.exec_loop()

        ###   
        p=select.poll()
        p.register(self.__ServerFD, select.POLLIN)
        while True:
            try:
                ll = p.poll(1000)
                if len(ll) > 0:
                    self.slotListener(self.__ServerFD)
            except SystemExit, e:
                raise
            except Exception, e:
                #print 'AOApp polling exception: ',e
                time.sleep(1)

    def slotListener(self,fd):
        message = msglib.WaitMessage(1000)
        if message['status'] == 'NO_ERROR':
            self.serveMsg(message)
        elif message['errcode'] == AOErrCode['CLOSEDCONN_ERROR']:
            # Disaster - msgd exited
            print 'MsgD communication closed! Don''t know how to proceed.'
            sys.exit()
        else:
            raise Exception(message['status'])


    def serveMsg(self,message):
        #print "DEBUG: Got message: %s" % (message) 

        if message['status'] != "NO_ERROR":
            errstr =  "slotListener error: Status is " + message['status']
            raise AOConnectionError, errstr, message

        self.__lock.acquire()
    
        if (message['sender'] in self.__wait_msgid.keys() and\
        message['code']==AOMsgCode['CLNREADY']):
            # this message notifies that a client is ready
            # used when WaitClientReady is blocking
            toThread = self.__wait_msgid[message['sender']]
            if toThread not in self.__fifo.keys():
                self.__fifo[toThread]=Queue.Queue()
            self.__fifo[toThread].put(message)
            self.__lock.release()
            return
                
        if message['seqnum'] in self.__wait_msgid.keys():
            # someone is waiting for this message: put it into the fifo
            # of the requesting thread 
            toThread = self.__wait_msgid[message['seqnum']]
            if toThread not in self.__fifo.keys():
                self.__fifo[toThread]=Queue.Queue()
            self.__fifo[toThread].put(message)
            self.__lock.release()
            return

        self.__lock.release()

        if message['code'] == AOMsgCode['VARCHANGD']: # This is a response to a SETVAR
            TheVar = Msg2Var(message)
            if TheVar.Name() in self.__notif.keys():
                # the owner in the message of notif is the client that last modified the var.
                owner = self.__notif[TheVar.Name()]['var'].Owner() # save the good owner
                lastnotifier = TheVar.Owner()             
                self.__notif[TheVar.Name()]['var'] = TheVar
                self.__notif[TheVar.Name()]['var'].SetLastNotifier(lastnotifier)
                self.__notif[TheVar.Name()]['var'].SetOwner(owner)
                # self.listener.emit(PYSIGNAL('VARCHANGD_%s'%(TheVar.Name())), (self.__notif[TheVar.Name()],))
                
                # If the VarNotif is just to mirror a variable, do it
                for varb in self.__notif[TheVar.Name()]['copy']:
                    varb.Copy(TheVar)
                # Starts a new thread for each method bound to the variable
                try:
                    for ifunc in self.__notif[TheVar.Name()]['changed']:
                        thread.start_new_thread(ifunc , (self.__notif[TheVar.Name()]['var'],)  )
                except:
                    raise

#        elif message['code'] == AOMsgCode['VARREMOVD']: # This is a response to a DELVAR
#            if message['name'] in self.__notif.keys():
#                try:
#                    for ifunc in self.__notif[message['name']]['deleted']:
#                        thread.start_new_thread(ifunc , (self.__notif[message['name']]['var'],)  )
#                except:
#                    raise
#                del self.__notif[message['name']]
            
        elif message['code'] == AOMsgCode['CLNREADY']:
            if message['sender'] in self.__notif.keys():
                # here if WaitClientReady was non-blocking
                try:
                    for ifunc in self.__notif[message['sender']]['ready']:
                        thread.start_new_thread(ifunc , (message['sender'],)  )
                except:
                    raise
                del self.__notif[message['sender']]
        
        elif message['code'] == AOMsgCode['ECHO']: # Answer to the ECHO coming from elsewhere 
            msglib.SendMessage( message['sender'], AOMsgCode['ECHO'],'',message['seqnum'])


##################################################
# Generic msg interface
#################################################

    def SendMsg(self,to,code,data,timeout=None,seqnum=-1,binary=False, want_answer = True):
        # if this thread has not an own Queue, create it
        mythread = thread.get_ident()
        if mythread not in self.__fifo.keys():
            self.__fifo[mythread]=Queue.Queue()

        self.__lock.acquire()
        if seqnum == -1:
            if binary:
                expected_id = msglib.SendBinaryMessage(to, code, data) 
            else:
                expected_id = msglib.SendMessage(to, code, data) 
        else: 
            expected_id = msglib.SendMessage(to, code, data,seqnum) 
        if expected_id < 0:
            self.__lock.release()
            errstr = "SendMsg Error. Msg %d to %s. error is %s" %\
                      (code,to,msglib.GetErrorString(expected_id)) 
            raise AdOptError, (expected_id, errstr)

        if not want_answer:
            return

        # put the msgid in the list of the current thread
        self.__wait_msgid[expected_id]=mythread
        self.__lock.release()
  
        if timeout:
            my_timeout = timeout
        else:
            my_timeout = self.__fifo_timeout

        try:
            message = self.__fifo[mythread].get(True, my_timeout)
            del self.__wait_msgid[expected_id]
        except Queue.Empty:
            del self.__wait_msgid[expected_id]
            errstr =  "fifo error: fifo_timeout on msg %d to %s" % (code, to)
            raise AdOptError, (AOErrCode['TIMEOUT_ERROR'], errstr)
        except:
            del self.__wait_msgid[expected_id]
            raise 
        return message

###################################################
# Handling of RTDB variables
###################################################

    def WriteVarNO(self,var, timeout=0):
        if isinstance(var,AOVar) == False:
            errstr = "WriteVar Error: arg 1 must be an instance of AOVar class"
            raise AttributeError, errstr
        status = msglib.WriteVar(var.Name(), AOVarCode[ var.Type()],\
                len(var), var.Value(), self, self.serveMsg, timeout)
        if status != AOErrCode['NO_ERROR']:
            errstr = "Error in WriteVar %s: %s " % (var.Name(), AErrType[status])
            raise AOVarError, errstr

    def WriteVar(self,var, nocreate=False):
        if isinstance(var,AOVar) == False:
            errstr = "WriteVar Error: arg 1 must be an instance of AOVar class"
            raise AttributeError, errstr
    
        try:
            self.SetVar(var)
            reply = self.ReadVar(var.Name())

        except AOVarNotInRTDB, e:
            if nocreate:
                raise e
            # var doesnt exist in rtdb: create, set, read and check
            if var.Owner() == 'PUBLIC':
                own = 'g'
            else:
                own = 'p'
            tipo  = AOVarCode[ var.Type()]
            status =  msglib.CreateVar(var.Name(), tipo, own, len(var))
            if status != AOErrCode['NO_ERROR']:
                errstr = "Error in CreateVar %s: %s " % (var.Name(), AOErrType[status])
                raise AOVarError, (errstr, None, var)
            self.SetVar(var)
            reply = self.ReadVar(var.Name())
        if reply != var:
            if reply.owner != var.owner:
                errstr = "WriteVar Error: no right to change var %s. Owner is %s (but I'm %s)" % \
                          (var.Name(), reply.owner, var.owner)
                raise AOVarError, errstr
            errstr =  "WriteVar error on variable %s\nset %s\ngot %s" % (var.Name(),var,reply)
            raise AOVarError, (errstr, None, var)

   
    # Returns a variable object read from MsgD-RTDB.
    # if name is an AOVar object, it will become a copy of the returned object
 
    def ReadVar(self,name):
        
        if isinstance(name, AOVar):
            varname = name.Name()
        else:
            varname = name

        if isinstance(varname,str) == False:
            errstr = "ReadVar Error: arg 1 must be a string or a variable object"
            raise AttributeError, errstr

        # if this thread has not an own Queue, create it
        mythread = thread.get_ident()
        if mythread not in self.__fifo.keys():
                self.__fifo[mythread]=Queue.Queue()

        # call GetVar
        self.__lock.acquire()
        expected_id = msglib.GetVar(varname)
        if expected_id < 0:
            self.__lock.release()
            errstr = "ReadVar Error. var is %s. error is %s" %\
                      (varname,msglib.GetErrorString(expected_id)) 
            raise AdOptError, (expected_id, errstr)

        # put the msgid in the list of the current thread
        self.__wait_msgid[expected_id]=mythread
        self.__lock.release()
   
        try:
            message = self.__fifo[mythread].get(True, self.__fifo_timeout)
            del self.__wait_msgid[message['seqnum']]
            if message['code'] == AOMsgCode['VARERROR']:
                errstr = "ReadVar: variable %s not defined in rtdb" %(varname) 
                raise AOVarNotInRTDB, (errstr, message)
            elif message['code'] == AOMsgCode['VARLOCKD']:
                errstr = "ReadVar: variable %s is locked" %(varname) 
                raise AOVarError, (errstr, message)
            elif message['code'] == AOMsgCode['VARREPLY']:
                var = Msg2Var(message)
            else:
                errstr = "ReadVarList: unexpected message" 
                raise AOConnectionError, (errstr, message)
                
        except Queue.Empty:
            errstr =  "fifo error: fifo_timeout on variable %s" % (varname)
            raise AdOptError, (AOErrCode['TIMEOUT_ERROR'], errstr)
        
        if isinstance(name, AOVar):
            name.Copy(var)
        return var
    

    def CreaVar(self, var):
        if isinstance(var,AOVar) == False:
            errstr = "CreaVar Error: arg 1 must be an instance of AOVar class"
            raise AttributeError, errstr

        if var.Owner() == 'PUBLIC':
            own = 'g'
        else:
            own = 'p'

        tipo  = AOVarCode[ var.Type()]

        status =  msglib.CreateVar(var.Name(), tipo, own, len(var))
        if status != AOErrCode['NO_ERROR']:
            errstr = "Error in CreateVar %s: %s " % (var.Name(), AOErrType[status])
            raise AOVarError, (errstr, None, var)

        # ReadVar back to check name, type, owner are rigth
        # value (hence length) are allowed to be different
        reply = self.ReadVar(var.Name())
        if reply.Name() != var.Name() or reply.Type() != var.Type() \
           or reply.Owner() != var.Owner():
            errstr =  "CreaVar error: duplicated variable %s\nWant %s\nGot  %s" % \
                (var.Name(), var, reply)
            raise AODuplicatedVarError, (errstr, None, var)
        return reply

    def SetVar(self, var, value=None):
        if isinstance(var,AOVar) == False and not(isinstance(var,str) == True and value != None):
            errstr = "CreaVar Error: arg 1 must be an instance of AOVar class or a string"
            raise AttributeError, errstr

        if isinstance(var,str):
            var = self.ReadVar(var)

        if value != None:   
            try:
                var.SetValue(value)
            except ValueError, e:
                raise AOVarError, e

        tipo  = AOVarCode[ var.Type()]
        status = msglib.SetVar( var.Name(), tipo, len(var), var.Value())
        if status < 0:
            errstr = "Error in SetVar %s: %s " % (var, AOErrType[status])
            raise AOVarError, errstr

    def DelVar(self, name, timeout=None):
        '''
        Remove a variable from RTDB
        '''
        if timeout == None:
            status = msglib.DelVar(name, self, self.serveMsg)
        else:
            status = msglib.DelVar(name, self, self.serveMsg, timeout)
        if status != AOErrCode['NO_ERROR']:
            errstr = "Error in DelVar %s: %s " % (name, AOErrType[status])
            raise AOVarError, errstr
        
    def LockVars(self, prefix, timeout=None):
        if timeout == None:
            status = msglib.LockVars(prefix, self, self.serveMsg)
        else:
            status = msglib.LockVars(prefix, self, self.serveMsg, timeout)
        if status != AOErrCode['NO_ERROR']:
            errstr = "Error in LockVars %s: %s " % (prefix, AOErrType[status])
            raise AOVarError, errstr

    def UnlockVars(self, prefix, timeout=None):
        if timeout == None:
            status = msglib.UnlockVars(prefix, self, self.serveMsg)
        else:
            status = msglib.UnlockVars(prefix, self, self.serveMsg, timeout)
        if status != AOErrCode['NO_ERROR']:
            errstr = "Error in UnlockVars %s: %s " % (prefix, AOErrType[status])
            raise AOVarError, errstr

    def VarNotif(self, name, sigChanged=None, sigDeleted=None):
        '''
        Ask to be notified when a variable V is changed or deleted.
        name can be a string or an AOVariable.
        If name is an AOVar, when V is changed it is mirrored in name.
        Otherwise:
        The function sigChanged will be run in a separate thread when the variable is changed.
        The function sigDeleted will be run in a separate thread when the variable is deleted.

        Note that multiple functions can be defined for a single variable: in this case, 
        the functions will be scheduled concurrently.

        NOTE: given 2 different variables A and B modified in this order, since the handling 
        functions are run in seperate threads it can happen that the handler of A terminates 
        after the handler of B. At the moment there is no way to serialize these threads.
        '''

        if sigChanged == None and sigDeleted==None and isinstance(name,AOVar) == False :
            return

        if isinstance(name,AOVar) == True:
            try:
                var = self.ReadVar(name.Name())
            except AOVarNotInRTDB:
                raise
            name.Copy(var)
        else:
            try:
                var = self.ReadVar(name)
            except AOVarNotInRTDB:
                raise

        #if isinstance(sigChanged,AOVar) == True:
        #    sigChanged.Copy(var)

        if var.Name() not in self.__notif.keys():
            self.__notif[var.Name()]={'var':var, 'changed':[], 'deleted':[], 'copy':[]}
        if isinstance(name,AOVar) == True:
            self.__notif[var.Name()]['copy'].append(name)

        if sigChanged != None:
            self.__notif[var.Name()]['changed'].append(sigChanged)
        if sigDeleted != None:
            self.__notif[var.Name()]['deleted'].append(sigDeleted)
            
        status = msglib.VarNotif(var.Name())
        if status != 0:
            errstr = "Error in VarNotif variable %s: %s " % \
                      (var.Name(), msglib.GetErrorString(status))
            raise AOVarError, errstr


    def VarNoNotif(self, name, sigChanged=None, sigDeleted=None):
        '''
        Remove sigChanged and/or sigDeleted from the list of tasks scheduled when the variable 
        is changed.
        If sigChanged and sigDeleted are both not specified, delete all scheduled tasks. 
        '''
        if sigChanged == None and sigDeleted==None:
            del self.__notif[name]
            status = msglib.VarNoNotif(name)
            if status != 0:
                errstr = "Error in VarNoNotif variable %s: %s " % \
                          (name, msglib.GetErrorString(status))
                raise AOVarError, errstr
                
        if sigChanged != None:
            self.__notif[name]['changed'].remove(sigChanged)
        if sigDeleted != None:
            self.__notif[name]['deleted'].remove(sigDeleted)

    def ReadVarList(self,prefix='*'):
        # if this thread has not an own Queue, create it
        mythread = thread.get_ident()
        if mythread not in self.__fifo.keys():
                self.__fifo[mythread]=Queue.Queue()

        # call VarList
        varlist=[]
        self.__lock.acquire()
        expected_id = msglib.VarList(prefix)
        if expected_id < 0:
            self.__lock.release()
            errstr = "ReadVarList Error. prefix was %s. error is %s" %\
                      (prefix,msglib.GetErrorString(expected_id)) 
            raise AdOptError, (expected_id, errstr)

        # put msgid in fifo and release the lock
        self.__wait_msgid[expected_id]=mythread
        self.__lock.release()
        try:
            while(1):
                message = self.__fifo[mythread].get(True, self.__fifo_timeout)
                if message['code'] == AOMsgCode['VARERROR']:
                    errstr = "ReadVarList: variable %s not defined in rtdb" %(prefix) 
                    raise AOVarError, (errstr, message)
                elif message['code'] == AOMsgCode['VARLOCKD']:
                    errstr = "ReadVarList: variable %s is locked" %(message['name']) 
                    raise AOVarError, (errstr, message)
                elif message['code'] == AOMsgCode['VAREND']:
                    del self.__wait_msgid[message['seqnum']]
                    return varlist
                elif message['code'] == AOMsgCode['VARREPLY']:
                    varlist.append(Msg2Var(message))
                else:
                    errstr = "ReadVarList: unexpected message" 
                    raise AOConnectionError, (errstr, message)
                    
        except Queue.Empty:
            errstr =  "fifo error: fifo_timeout on varlist prefix %s" % (prefix)
            raise AdOptError, (AOErrCode['TIMEOUT_ERROR'], errstr)
        except AOVarError, e:
            if e.message['code'] == AOMsgCode['VARERROR']:
                errstr = "ReadVarList Error. prefix was %s" %(prefix) 
                raise AOVarError, (errstr, e.message)


    def SetReady(self):
        status = msglib.SendMessage('', AOMsgCode['SETREADY'], '')
        if status < 0:
            errstr = "SetReady Error. error is %s (%d)" % (msglib.GetErrorString(status), status) 
            raise AdOptError, (status,errstr)

    def WaitClientReady(self, client, timeout=None, sighandler=None):
        ''' 
        Set timeout < 0 if you want to be asynchronously notified when the
        client will become ready. Set also sighandler to a valid function name
        that will be called when the client will become ready
        Set timeout to a positive number for a blocking version of this 
        function.
        Set timeout to None to wait until necessary with no timeout
        '''
        # if this thread has not an own Queue, create it
        mythread = thread.get_ident()
        if mythread not in self.__fifo.keys():
                self.__fifo[mythread]=Queue.Queue()

        self.__lock.acquire()
        err = msglib.SendMessage( "", AOMsgCode['WAITCLRDY'], client)
        if err < 0:
            self.__lock.release()
            errstr = "WaitClientReady Error. client is %s. error is %s" %\
                      (client,msglib.GetErrorString(err)) 
            raise AdOptError, (err,errstr)
        self.__wait_msgid[client]=mythread
        self.__lock.release()

        if timeout == None or timeout >= 0: # Blocking    
            try:
                message = self.__fifo[mythread].get(True, timeout)
            except Queue.Empty:
                del self.__wait_msgid[client]
                raise
            else:
                del self.__wait_msgid[client]
        else: # Non-blocking
            if sighandler != None:
                if client not in self.__notif.keys():
                    self.__notif[client]={'ready':[]}
                self.__notif[client]['ready'].append(sighandler)


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
        #if isinstance(varname, AOVar):
        #    varname = varname.Name()

        if isinstance(var, AOVar):
            varname = var.Name()
        else:
            varname = var


        # Puts in the __events queue an event object plus an empty value for a var object
        # We'll call VarNotif and block waiting for the event to be set. The handler function
        # will unblock us and the actual value will be compared with the expected one

        ev = self.varevent( threading.Event(), None)
        if not self.__events.has_key(varname):
            self.__events[varname] = []
        self.__events[varname].append( ev)

        # Start the notifications BEFORE asking for the current value, otherwise
        # we could get into a race condition and we would miss the change.
        self.VarNotif( varname, self.unblockHandler)

        # See if the right value is already present
        v = self.ReadVar(varname).Value()
        if func(v) == True:
            self.VarNoNotif( varname, self.unblockHandler)
            return 

        start = time.time()
        while 1:
            ev.event.clear()
            ev.event.wait(timeout)
            if ev.var:
                v = ev.var.Value()
                if func(v) == True:
                    break
            if timeout:
                diff = time.time() - start
                if diff > timeout:
                    errstr = "Timeout waiting for %s" % varname
                    raise AdOptError, (AOErrCode['TIMEOUT_ERROR'], errstr)
                timeout = timeout - diff

        self.VarNoNotif( varname, self.unblockHandler)

    def SetAndWaitCur( self, req_var, cur_var, value, timeout=None):
        '''
        Sets a _REQ variable and waits for the corresponding _CUR variable
        to reflect the change. If timeout is None (default) will wait indefinitely.
        Raises an exception if the timeout is exceeded.
        '''

        req_var.SetValue(value)
        self.WriteVar( req_var, nocreate = True)
        self.WaitVarValue( cur_var, value, timeout = timeout)

         
    def unblockHandler(self, var):
        for ev in self.__events[var.Name()]:
            ev.var = var
            ev.event.set()

    def Ping(self,client, timeout=None):
        '''
        Ping a client and wait for the response
        '''
        self.SendMsg(client, AOMsgCode['ECHO'],'', timeout)


#################################
# IDL commands
#################################
    def IDLCmd(self, cmd):
        '''
        Send an IDL string to the IDL controller and return the idl answer dictionary
        Raise an exception in case of error code not null in the answer
        '''
        if isinstance(cmd,str) == False:
            errstr = "IDLCmd Error: arg 1 must be a string"
            raise AttributeError, errstr

        # if this thread has not an own Queue, create it
        mythread = thread.get_ident()
        if mythread not in self.__fifo.keys():
            self.__fifo[mythread]=Queue.Queue()

        self.__lock.acquire()
        expected_id = msglib.SendMessage('IDLCTRL',AOMsgCode['IDLCMD'],cmd)
        if expected_id < 0:
            self.__lock.release()
            errstr = "IDLCmd Error. cmd is %s. error is %s" %\
            (cmd,msglib.GetErrorString(expected_id)) 
            raise AdOptError, (expected_id,errstr)

        # put the msgid in the list of the current thread
        self.__wait_msgid[expected_id]=mythread
        self.__lock.release()
        try:
            message = self.__fifo[mythread].get(True, self.__fifo_timeout)
            del self.__wait_msgid[message['seqnum']]
            if message['code'] == AOMsgCode['IDLREPLY']:
                idlans = {}
                idlans['stdout']=message['stdout'] 
                idlans['stderr']=message['stderr']
                idlans['err_code']=message['err_code']
            else:
                errstr = "IDLCmd: unexpected message" 
                raise AOConnectionError, (errstr, message)
        except Queue.Empty:
            errstr =  "fifo error: fifo_timeout on IDL cmd %s" % (cmd)
            raise AdOptError, (AOErrCode['TIMEOUT_ERROR'], errstr)
        if idlans['err_code'] != AOErrCode['NO_ERROR']:
            errstr = "IDLCmd: error code %d" % (idlans['err_code']) 
            raise AOIDLError, (errstr,idlans)
        return idlans

##################################
# Shared memory functions
##################################

    def BufCreate( self, bufname, bufsize, maxCons=10):
        '''
        Creates a BUFCONT shared buffer.
        '''

        if not self.__sharedBufsWrite.has_key(bufname):
            result = msglib.bufCreate( self.MyName(), bufname, bufsize, maxCons)
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

        errcode = msglib.bufWrite( self.__sharedBufsWrite[bufname]['bufinfo'], data, self.__sharedBufsWrite[bufname]['counter'], timeout)
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
            result = msglib.bufRequest( self.MyName(), bufname)
            if result['errcode'] == AOErrCode['NO_ERROR']:
                self.__sharedBufsRead[bufname] = result['bufinfo']
            else:
                raise AdOptError(code=result['errcode'], errstr='Attaching to shared buffer '+bufname)

        frameValid = False
        while not frameValid:
            start = time.time()
            data = msglib.bufRead( self.__sharedBufsRead[bufname], timeout)
            if data['errcode'] != AOErrCode['NO_ERROR']:
                # Delete local bufinfo, except in case of a timeout
                if data['errcode'] != AOErrCode['TIMEOUT_ERROR']:
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
    

    
##################################   
# End AOApp class
##################################


def Msg2Var(message):
    return AOVar(message['name'],AOVarType[message['type']],\
        message['value'], message['owner'] or 'PUBLIC')



