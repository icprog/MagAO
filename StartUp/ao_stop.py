#!/usr/bin/env python
#
# Stops all AdOpt processes

from AdOpt import cfg, processControl, thAOApp
import time
import os, sys, signal

subs = os.getenv('ADOPT_SUBSYSTEM')
if subs==None:
    print 'No ADOPT_SUBSYSTEM environment variable set.'
    print 'Please set the ADOPT_SUBSYSTEM environment variable to either "WFS" or "ADSEC"'
    sys.exit(0)

side = os.getenv('ADOPT_SIDE')


# Check that we are running on the right machine
cmd = os.path.basename(sys.argv[0])

if cmd=='w_stop' and subs!='WFS':
    print 'w_stop can only be run on the WFS computer.'
    sys.exit(0)

if cmd=='adsc_stop' and subs!='ADSEC':
    print 'adsc_stop can only be run on the ADSEC computer.'
    sys.exit(0)


connected = False
if processControl.getProcessID('msgdrtdb') > 0:
    try:
        app = thAOApp('stop')
        connected = True
    except:
        pass
    

lst = cfg.processes.keys()

# Send TERMINATE messages, if possible

if connected:
    for p in lst:
        if p != 'msgdrtdb':
            processControl.stopProcessByName(p)

time.sleep(10)

# Kill everything
for p in lst:
   if p != 'msgdrtdb':
       while processControl.stopProcessByName(p, kill = True) == True:
           pass

# Shutdown VisAO low level stuff
if subs=='WFS':
           os.system("ssh aosup@visaosup.visao visao/bin/visao_stop.py")
           
processControl.stopProcessByName('msgdrtdb', kill = True)

#Additional processes to be killed
adlst = ['InterferometerCtrl','HexapodCtrl','HexapodGui','AdSecMir_gui', 'Housekeeper_gui', 'adsceng', 'wfseng', 'vartool_AO', 'idl', 'tail']
for i in adlst:
    g = os.popen("ps -ef | grep "+i+" | grep -v grep | awk '{ print $2 }'")
    h = g.readlines()
    for j in h:
        os.kill(int(j.rstrip('\n')), signal.SIGKILL)

#check for Zombie processes
flist = lst + adlst

for i in flist:
    g = os.popen("ps -leaf | grep "+i+" | grep -v grep | grep defunct | awk '{ print $5 }'")
    h = g.readlines()
    for j in h:
        if int(j.rstrip('\n'))==1: 
            print "WARNING: some supervisor zombie process ("+i+") is still active... please reboot the PC."
