#!/usr/bin/env python
#
# Checks which AdOpt processes are running

from AdOpt import cfg, processControl, thAOApp
import time
import os, sys
from AdOpt.wrappers import thrdlib


subs = os.getenv('ADOPT_SUBSYSTEM')
if subs==None:
    print 'No ADOPT_SUBSYSTEM environment variable set.'
    print 'Please set the ADOPT_SUBSYSTEM environment variable to either "WFS" or "ADSEC"'
    sys.exit(0)

side = os.getenv('ADOPT_SIDE')


# Check that we are running on the right machine
cmd = os.path.basename(sys.argv[0])

if cmd=='w_check' and subs!='WFS':
    print 'w_check can only be run on the WFS computer.'
    sys.exit(0)

if cmd=='adsc_check' and subs!='ADSEC':
    print 'adsc_check can only be run on the ADSEC computer.'
    sys.exit(0)


if subs=='WFS':
    lst = (cfg.sys_processes+' '+cfg.wfs_processes).split()
elif subs=='ADSEC':
    lst = (cfg.sys_processes+' '+cfg.adsec_processes).split()
else:
    print 'Unknown subsystem '+subs
    sys.exit(0)

lst.insert(0,'msgdrtdb')

print

#init thrdlib for checking the msgd connection
try:
    str = 'adsccheck'+"%d" % time.time()
    thrdlib.thInit( str)
    #thrdlib.thInit( 'adsccheck')
    thrdlib.thStart( IP='127.0.0.1')
except:
    dictMgsd={}

count=0
for p in lst:
    id = processControl.getProcessID(p)
    dictMgsd = processControl.getMsgdProcessList()
    if id != 0:
        try: 
            index = dictMgsd.index(p+'.'+os.environ['ADOPT_SIDE'])
            ext="and connected"
        except:
            ext="but NOT connected"
        if (p == "msgdrtdb"): 
            print '%-20s running' % p
        elif (p == "idlctrl"):
            print '%-20s running %s to Msgd' % (p, ext)
            #check if child idl process is up and connected to Msgd
            g = os.popen("ps -leaf | grep idl | grep "+id+" | grep -v grep | grep -v "+p+" |  awk '{ print $2 }'")
            h = g.readlines()
            cc =0
            for j in h:
                if j == 'Z\n':
                    cc +=1
            if (cc==len(h)):
                print '%-20s Zombie process' % 'IDL process'
            else:
                try: 
                    index = dictMgsd.index('adsec'+'.'+os.environ['ADOPT_SIDE'])
                    ext="and connected"
                except:        
                    ext="but NOT connected"
                    print '%-20s running %s to Msgd' %  ('IDL process', ext)
        else:
            print '%-20s running %s to Msgd' % (p, ext)
        count += 1
    else:
        print '%-20s NOT running' % p

#childs = {"idlctrl":"idl"}
#for k, v in childs.iteritems():
#    id = processControl.getProcessID(k)


if count==0:
   print 'No AdOpt processes are running'
