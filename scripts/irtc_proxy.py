#!/usr/bin/env python
#
#@File: irtc_proxy.py
#
# Proxy for the IRTC. Continously reads the irtc (via the irtc module)
# and puts the frame into shared memory (as a producer).
#
# The shared memory buffer has the following structure:
#
# uint16 data[ 320*256]

import sys, struct, time, thread

from AdOpt import thAOApp, AOVar, cfg
from AdOpt.hwctrl import irtc

live = False
if len(sys.argv)>1:
    if sys.argv[1] == '-live':
        live = True

class storecube:
    def __init__(self):
        self.lastcube = ''
    def store(self, path):
        self.lastcube = path
    def check(self, path):
        return self.lastcube != path

store = storecube()

def slotCubeTrig(message):

    num = int(message['body'])
    print 'Requested cube of %d frames' % num
    lock.acquire()
    try:
    	f = irtc.readCube( num, pathOnly = True)
    except:
	lock.release()
        print 'Error reading from IRTC'
        app.ReplyMsg( message['msgb'], 50000, 'error')

    lock.release()
    if store.check(f):
        app.ReplyMsg( message['msgb'], 50000, f)
        store.store(f)
    else:
        print 'CUBE ERROR: new cube not found!'
        app.ReplyMsg( message['msgb'], 50000, 'error')
     


myname = 'irtcproxy.'+cfg.side

msgHandlers = { 50000: slotCubeTrig }

app = thAOApp(myname, argv = sys.argv, msgHandlers = msgHandlers)

lock = thread.allocate_lock()

# Global setup
sock = None

backg_var =  AOVar.AOVar( name= "%s.IRTC.DARK.REQ" % cfg.side, tipo = 'CHAR_VARIABLE', value='')
app.CreaVar(backg_var)

def readIRTC():

    lock.acquire()
    f = irtc.read()
    lock.release()
    return f

def writeFrame(frame):

    if frame == None:
        return

    frame = frame.byteswap()
    app.BufWrite( 'FRAME', frame.tostring())


app.SetReady()
counter=0
while 1:
    try:
        if live:
            frame = readIRTC()
            if frame != None:
                writeFrame(frame)
                print 'Frame %d written to shared memory' % counter
                counter +=1
                time.sleep(0.1)
            else:
                print 'Frame not got, retrying...'
            time.sleep(1)
        else:
            time.sleep(1)
    except Exception, e:
        print 'Exception:', e
        lock.release()
        time.sleep(1)

