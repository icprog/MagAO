#!/usr/bin/env python

# Record the change in time of a variable

from AdOpt import * 
import sys, time

def handler(var):
    s = "%f: %s %s" % (time.time()-now, varname, str(var.Value()))
    f.write(s+"\n")
    f.flush()
    print s

if len(sys.argv)<3:
    print "Usage: record.py <variable name> <outfile>"
    sys.exit()

varname = sys.argv[1]
outfile = sys.argv[2]
f = file(outfile, "w")

app = AOApp("record")
now = time.time()

v = app.ReadVar(varname)

app.VarNotif(varname, handler)

while 1:
    time.sleep(1)
    handler( app.ReadVar(varname))
    
