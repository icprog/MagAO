#!/usr/bin/env python

HOST = '193.206.155.78'   # Default host
PORT = 14500

# Saves 16-bit binary files from thorlabs...

import socket
import numpy.numarray as numarray
import numpy
from AdOpt import fits_lib

def connect( host = None, port = None):

    if host == None:
        host = HOST
    if port == None:
        port = PORT

    s = socket.socket( socket.AF_INET, socket.SOCK_STREAM)
    print 'Connecting to %s:%d' % (host, port)
    s.connect((host, PORT))
    if s:
    	print 'Connected'
    return s

def disconnect(s):
    s.close()

def read( s, savefile = None):

    print 'sending command'
    s.send("frame")
    expected_len = 1024*768*2
    str = ''
    print 'receiving frame'
    while len(str) < expected_len:
        str += s.recv(expected_len)



    try:
        r = numpy.reshape( numpy.frombuffer( str, numpy.uint16, 768*1024), (768,1024))
        r = r.byteswap()/64
    except Exception, e:
        print 'Exception: ',e

    print 'done'

    if savefile:        
        fits_lib.writeSimpleFits( savefile, r.astype(numpy.uint32), {}, overwrite=True)

    return r
    



    
