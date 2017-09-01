#!/usr/bin/env python
#
#@File: thorlabs_proxy.py
#
# Proxy for the thorlabs CCD. Continously reads the CCD (via the thorlabs module)
# and puts the frame into shared memory (as a producer).
#
# The shared memory buffer has the following structure:
#
# uint16 dx;
# uint16 dy;
# uint16 data[]

import sys, struct, time

from AdOpt import thAOApp
from AdOpt.hwctrl import thorlabs

if len(sys.argv)>1:
    mynum = int(sys.argv[1])
else:
    mynum = 0 

myname = 'ThorlabsProxy'

app = thAOApp(myname, id = mynum, argv = sys.argv)

addrs = {}
addrs[1] = '193.206.155.40'
addrs[2] = '192.168.0.201'
addrs[0] = None  # Use default


# Global setup
sock = None

def readThorlabs():
    global sock

    try:
        if not sock:
            sock = thorlabs.connect( host = addrs[mynum])
        return thorlabs.read(sock)
    except Exception, e:
	if sock:
	    sock.close()

    return None

def writeFrame(frame):

    if frame == None:
        return

    app.BufWrite( 'FRAME', frame.tostring())


app.SetReady()
counter=0
while 1:
    frame = readThorlabs()
    if frame != None:
        writeFrame(frame)
        print 'Frame %d written to shared memory' % counter
        counter +=1
	time.sleep(0.1)
    else:
        print 'Frame not got, retrying...'
        time.sleep(1)

