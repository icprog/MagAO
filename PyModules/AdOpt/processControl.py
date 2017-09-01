#!/usr/bin/env python
#
#+File: processControl.py
#
# Library for process start/stop/check
#
# Can work in both standalone scripts and MsgD-clients. The start/stopWait() functions
# are only available in MsgD clients.


import os, re, time
import daemonize
from AdOpt import AOExcept, AOConfig, AOConstants, cfg


######################################################################
#
# Process start/stop functions


def startProcessByName( process, multi=False, stdoutfile='auto', verbose=True, dryrun=False):
    '''
    Starts a process, given its name in the processList configuration file.

    Spawns a new process. The process log will be directed into the file specified,
    or in a .stdout file if nothing is specified.
    if <multi> is True, will permit opening the same process multiple times, otherwise
    an exception is thrown.
    '''

    p = getProcess(process)
    if p:
       if stdoutfile == 'auto':
         stdoutfile = getStdoutFile(process)

       if not dryrun:
         if not multi:
           if getProcessID(process) >0:
              print process
              raise AOExcept.AdOptError('Duplicate process: %s' % process) 
                
       path = p[3]
       name = p[1]
       options = p[2]

       if process != 'msgdrtdb':
          options = ('-i '+process+' '+options).strip()
        
       if dryrun:
           print "  Would start: %s/%s %s" % (path, name, options)
       else:
           if verbose:
               print "startProcessByName(): starting process %s/%s %s" % (path, name, options)
           else:
               print 'startProcessByName(): starting',process
           startDaemon(path, name, options, stdoutfile)

    else:
        print'startProcessByName(): process %s not found!' % process



def stopProcessByName( process, kill= False):
    '''
    Stops a process, given its name in the processList configuration file,
    using a TERMINATE message or a kill -9.
    Proper terminating requires calling this function from an AOApp.
    '''

    if kill:
        p = getProcess(process)
        if p:
            id = getProcessID(process)
            if id != 0:
		#print 'ID=',id
                command = "/bin/kill -9 "+str(id) # Shouldn be -9 !!!
                os.system( command)
                print 'stopProcessByName(): process %s killed' % process
                return True
            else:
                print 'stopProcessByName(): process %s down' % process
                return False
        else:
            print 'stopProcessByName(): process %s not found in configuration!' % process
            return False

    client = cfg.taskname(process)
    if client == None:
        # if the client is not in the configuration use it as process name
        client = process

    from AdOpt.wrappers import thrdlib
    thrdlib.thSendMsg( code = thrdlib.TERMINATE, dest=client)
    return True


def getStdoutFile( process):
    '''
    Returns the file where the process' stdout is redirected.
    '''
    return '/tmp/%s.stdout' % process


def restartProcessList( list, multi= False):
    '''
    Restarts a whole process list.
    '''

    for process in list:
        stopProcessByName(process)
    for process in list:
        startProcessByName( process, multi)


def cmdLine(process):
    '''
    Returns the commandline used by a process.
    '''
    p = getProcess(process)
    if not p:
        return None

    path = p[3]
    executable = p[1]
    options = p[2]

    if process != 'msgdrtdb':
        options = ('-i '+process+' '+options).strip()

    if p[1][-3:] == '.py':
        executable = 'python '+executable

    if options != '':
        return executable+" "+options
    else:
        return executable
    


# getProcessID      get the internal ID of a process
#
# Uses the "ps" command to see if a process is running or not.
            
def getProcessID( process):
    '''
    Gets the current pid of a process, using the "ps" command.
    '''
    cmd = cmdLine(process)
    if not cmd:
        return 0

    command = "/bin/ps axww | grep \" "+cmd+"\" | grep -v grep"
    pipe = os.popen(command)
    str = pipe.read()
    pipe.close()

    m = re.compile("^\s*(\d+)").search(str)

    #print m
    if m == None:
        return 0
    else:
        return m.groups()[0]

def startDaemon( workingdir, name, options, stdoutfile=None):
    '''
    Starts a separate process like a daemon
    '''

    pid = os.fork()

    if pid==0:
        try:
            try:
                os.unlink(stdoutfile)
            except OSError:
                pass
            daemonize.createDaemon(workingdir, stdoutfile)
            os.execv( name, [name] + options.split())
        except Exception, exc:
            print " *** Exception! *** "
            print exc
            print "Couldn't start program, exiting"
            os._exit(0)



#################
# thAOApp functions

def startAndWait( app, process, timeout = 10):
    '''
    Starts a process and waits for its CLSTAT variable to become ready.
    Needs a thAOApp object already set up and connected.
    Does nothing if the process is already up and running
    '''

    from AdOpt.wrappers import thrdlib

    varname = cfg.clstat(process)
    if varname == None:
        print 'startAndWait(): process %s does not have a CLSTAT variable, skipping' % process
        return

    try:
        if app.ReadVar(varname).Value() != 0:    
            return
    except:
        # Expected when the process has been never started
        pass

    startProcessByName(process)
    waitProcessStart( app, process, timeout = timeout)

def waitProcessStart( app, process, timeout = 10):
    '''
    Waits for a process' CLSTAT variable to become ready.
    '''

    from AdOpt.wrappers import thrdlib
    import time

    now = time.time()
    varname = cfg.clstat(process)
    if varname == None:
        print 'waitProcessStart(): process %s does not have a CLSTAT variable, skipping' % process
        return

    while 1:
        try:
            # app.WaitVarValue(varname, 2, timeout=timeout)
            app.WaitVarFunc(varname, lambda x: x>0, timeout=timeout)
            break
        except AOExcept.AdOptError:
            # Expected when the process is still starting up
            if time.time()-now > timeout:
                raise AOExcept.AdOptError( AOConstants.AOErrCode['TIMEOUT_ERROR'], "Timeout error waiting for process %s start" % process)
        except thrdlib.varNotFound:
            # Expected when the process is still starting up
            if time.time()-now > timeout:
                raise AOExcept.AdOptError( AOConstants.AOErrCode['TIMEOUT_ERROR'], "Timeout error waiting for process %s start" % process)

            time.sleep(0.2)
            continue


def stopAndWait( app, process, timeout = 10):
    '''
    Stops a process and waits for its CLSTAT variable to become zero.
    Needs a thAOApp object already set up and connected.
    '''


    if stopProcessByName( process):
        waitProcessStop( app, process, timeout = timeout)

def waitProcessStop( app, process, timeout = 10):
    '''
    Waits for a process' CLSTAT variable to become zero
    '''

    from AdOpt.wrappers import thrdlib

    varname = cfg.clstat(process)
    if varname == None:
        print 'waitProcessStop(): process %s does not have a CLSTAT variable, skipping' % process
        return

    try:
        app.WaitVarValue( varname, 0, timeout=timeout)
    except AOExcept.AdOptError, e:
        if e.code == AOConstants.AOErrCode['TIMEOUT_ERROR']:
            print 'waitProcessStop(): process %s did not stop after %d seconds, sending kill signal' % (process, timeout)
            stopProcessByName( process, kill = True)
    except:
        # Expected when there was no such process
        pass

def stopProcesses( app, process_list):
    '''
    Kills a list of processes and waits for their CLSTAT
    variables to become zero.
    '''

    # Kill everything
    for p in process_list:
        stopProcessByName(p)

    for p in process_list:
        waitProcessStop( app, p)

def startProcesses( app, process_list):
    '''
    Start a list of processes and wait for their CLSTAT
    variables to signal correct initialization.
    '''

    for p in process_list:
        try:
            app.log('Starting process %s' %p)
            startProcessByName( p, multi=False)
            app.log( "... process %s started !" %p)
        except OSError, e:
            app.log( 'Error: process %s cannot be started' % p)
            print '****** Exception, cannot start',p
            print e

    for p in process_list:
        waitProcessStart( app, p)


def getMsgdProcessList( ):
    from AdOpt.wrappers import thrdlib
    processes = []
    try:
        procs = thrdlib.thClntsList()
    except:
        procs = {}
    for proc in procs:
        if proc['value'].find("Clnt") != -1:
            processes.append(proc['name'])
    return processes


####################################################
# Module internal functions

def getProcess( process):
    if cfg.processes.has_key(process):
        return cfg.processes[process]
    else:
        return None


