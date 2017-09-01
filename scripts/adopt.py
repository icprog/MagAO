#!/usr/bin/env python2
#
#+File: adopt.py
#
# Command interface for the Adopt system
#-

import sys, dl
sys.setdlopenflags(dl.RTLD_NOW|dl.RTLD_GLOBAL)

import ll
import readline, code
from BCUfunctions import *
from fits_lib import *
from p45lib import *

#+Function: help
#
# help system
#-

def adopthelp():
	print globals() 

#+ Main interpreter loop
#-

counter=1
while 1:
    res = ll.ConnectToMsgD("Python%d" % counter, "127.0.0.1")
    if res>0:
        break
    counter = counter+1
    print "Retry #%d, connect result: %d" % (counter,res)

print "Connected"
shell = code.InteractiveConsole( globals())

shell.interact("AdOpt command interface")


