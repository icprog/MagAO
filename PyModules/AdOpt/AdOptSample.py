#!/usr/bin/env python
#

#
# This is an example of an application for use in the 
# framework of the LBT Adaptive Optic Software System.
# The base class AOApp provides connection to MsgD
# and a listener thread that reacts to socket messages from
# MsgD.
# Base services like SetReady, WaitClientReady, CreateVar,
# WriteVar, ReadVar, VarNotif, VarNoNotif, VarList and possibly
# more are implemented
#
# The class AOVar implements the AdOpt Var object.
#
# This example shows hot to derive a class from AOApp
# and how to create, modify, write and read back variables
# from RTDB.
# This code is intended to be read more than to be run, and its
# main purpose is to be a memo of the capabilities of AOApp.
#
# PS: AOApp internals "should" allow the derived class to be 
#     thread-safe...
#
#
#


from AdOpt import *

class AOSample (AOApp):

    # if you need to add properties to the application
    # do it in __init__
    def __init__(self,myname):
        AOApp.__init__(self,myname)
        self.miaroba=12

    # this is an handler for a notif msg
    def slotModVarp(self,var):
        print 'HANDLER 1> Var ->%s<- notified by %s. New value is %s' % \
            (var.Name(),var.LastNotifier(), var.Value())

    # this is another handler
    def slotModAnother(self,var):
        print 'HANDLER 2> Var ->%s<- was notified by %s. Value is %s' % \
            (var.Name(),var.LastNotifier(),var.Value())

    # this is a function called in the new thread
    def thread1(self,v1):
        v2 = AOVar.AOVar('thr1_var2', 'int', 1000)
        self.WriteVar(v1)
        self.WriteVar(v2)
        idx=0
        while idx<10:
            v=self.ReadVar(v1.Name())
            print 'Thread1 read var %s value %s' % (v.Name(), v.Value()) 
            v=self.ReadVar(v2.Name())
            print 'Thread1 read var %s value %s' % (v.Name(), v.Value()) 
            idx += 1

# Obviously you can define functions
# external to your class
def ramo2(app,v1):
    import random
    idx=0
    while idx<10:
        v=app.ReadVar(v1.Name())
        print 'Ramo2 read var %s value %s' % (v.Name(), v.Value()) 
        v1.SetValue(random.random())
        app.WriteVar(v1)
        idx += 1

#
# This is the main script
#   
if __name__ == "__main__":
    import sys
    import time
    import thread
    
    main = AOSample('AOSAMPLE')

    # Who am I?
    print "Registered with name %s" % (main.MyName())


    # Print AOMsg and AOVar dicts
    print "Code 5010 is MsgType ->%s<-" % (AOConstants.AOMsgType[5010])
    print "Type VARNOTIF is MsgCode->%s<-" % (AOConstants.AOMsgCode['VARNOTIF'])
    print "Allowed RTDBVar types are->%s<-" % (AOConstants.AOVarCode.keys())


    # Create private and global variable 
    print 'CREATE PRIVATE AND GLOBAL VARIABLES' 
    varp = AOVar.AOint('VarPrivate', 12, main.MyName())
    varg = AOVar.AOreal('VarGlobal', 1.23)

    # Write them to rtdb
    print  'WRITE THEM TO RTDB'
    main.WriteVar(varp)
    main.WriteVar(varg)

    # Ask to be notified
    print 'ASK TO BE NOTIFIED'
    main.VarNotif(varp.Name(), main.slotModVarp)
    main.VarNotif(varg.Name(), main.slotModAnother)

    # change something and write. 
    # slotModVarp and slotModAnother are triggered
    print 'CHANGE VALUES AND WRITE.' 
    varp.SetValue(144)
    varg.SetValue(0.567)
    main.WriteVar(varp)
    main.WriteVar(varg)

    # Ask to be no more notified
    print 'ASK TO BE NO MORE NOTIFIED ON VARP'
    main.VarNoNotif(varp.Name(), main.slotModVarp)
    
    # Trigger again: handler of varp is no more triggered
    print 'TRIGGER AGAIN: HANDLER OF VARP IS NO MORE TRIGGERED'
    main.WriteVar(varp)
    main.WriteVar(varg)
    
    # Say I'm ready
    print "SAY I'M READY"
    main.SetReady()
    
    # Wait for someone to be ready
    print  'WAIT 3s FOR SOMEONE TO BE READY: ' 
    print  '%s' % (main.WaitClientReady('cliente', 3))
    
    # ReadVarList NOT WORKING YET
    # varlist = main.ReadVarList('.*')
    # print 'rtdb vars: %s' % (varlist)
   
    # should be thread safe... 
#print  'START NEW THREAD'
#   ramo1 = thread.start_new_thread(main.thread1, (varp,))

    # do something here
#   ramo2(main,varg)



 
