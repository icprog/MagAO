#!/usr/bin/env python
#
# Python script to ping multiple hosts at once and send status information
# to MsgD-RTDB

import os, re, time, sys
from AdOpt import *
import thread, threading


# Stdout report
report = ("No response",  "Alive")

class pinged:
    pass

outfd = None
sys.argv.pop(0)
while len(sys.argv)>0:
    opt = sys.argv.pop(0)
    if opt == '-h':
        print """
pinger.py   Feb 2006 A.Puglisi

This process monitors multiple IP hosts and updated MsgD-RTDB with status information

Usage: pinger.py
        """
        sys.exit(-1)
    elif opt == '-f':
        filename = sys.argv.pop(0)
        cfg = read_config( filename, add_side = True)
    elif opt == '-l':
        outfile = sys.argv.pop(0)
        outfd = file(outfile,'w')

myName        = cfg['MyName'].Value()
mySide        = cfg['mySide'].Value()
ping_interval = cfg['PollingPeriod'].Value()


# Connect to MsgD-RTDB
self = AOApp( myName)

self._commLock = thread.allocate_lock()


# A checking thread instance

class pingthread(threading.Thread):
       def __init__ (self,app,k):
          threading.Thread.__init__(self)

          self.app = app
          self.host = cfg[k].Value()
          self.name = k[7:]
          self.var = AOVar.AOVar( name = "%s.%s.%s" % (mySide, app.MyName(), self.name), tipo = 'REAL_VARIABLE', value = -1.0, owner = app.MyName())
          self.dieNow = False
          # Variables are created only when the actual checking is done

          # Regexp to interpret ping's output
          self.times    = re.compile(r"mdev = .*?/([\d\.]+)/")

       def run(self):
          while 1:
            self.ping()
            time.sleep(ping_interval)
            if self.dieNow:
                break

       def ping(self):
          var_set = False
          pingaling = os.popen("ping -w1 -q -c1 "+self.host,"r")
          s= ''
          while 1:
             line = pingaling.readline()
             if not line: break

             t = re.findall(self.times, line)
             if t:
                self.var.SetValue(float(t[0]))
                var_set = True
                s = '%s: %f ms' % (self.host, self.var.Value())

          if not var_set:
             self.var.SetValue(-1.0)
             s = '%s: No response' % self.host

          self.app._commLock.acquire()
          self.app.WriteVar( self.var)
          self.app._commLock.release()

          if outfd:
            outfd.write(s+'\n')
            outfd.flush()
          else:
            print s


# Create one thread for each device
thread_list = []
for k in cfg.keys():
    if k[0:7] == "device_":
        thread_list.append( pingthread( self, k))

# Start ping threads

for pinger in thread_list:
    pinger.start()


self.SetReady()


while 1:
    try:
        time.sleep(10)
    except Exception:
        print 'Got exception. Waiting for clean thread finish...'
        for pinger in thread_list:
            pinger.dieNow = True
        sys.exit(0)

