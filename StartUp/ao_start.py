#!/usr/bin/env python
#
#@File: ao_start.py
#
# Main startup script. Starts all AO-related processes


from AdOpt import processControl, thAOApp, cfg
import sys, os, time
import getopt

def usage():
  print """
ao_start.py

Orderly start up the AO processes.

Usage:
       ao_start.py [-h]
       ao_start.py [-d] [-q] [side]

where:

       -d:    dry run (just show processes which would start)

       -t:    Terse output

       -h:    print usage page and exit

       side:  specify side
"""


def main():
  verbose=True

# Check that we are running on the right machine
  cmd = os.path.basename(sys.argv[0])

  try:
    opts,args = getopt.getopt(sys.argv[1:], "hdq")
  except getopt.GetoptError, err:

    print str(err)
    usage()
    sys.exit(2)
    
  dryrun=False

  for o,a in opts:
    if o=='-d':
      dryrun=True
    elif o=='-q':
      verbose=False
    elif o=='-h':
      usage()
      sys.exit(2)
    else:
      assert False, "unhandled option"

# Check side and subsystem

  if len(args) >0:
    side = sys.argv[0]
  else:
    side = os.getenv('ADOPT_SIDE')

  subs = os.getenv('ADOPT_SUBSYSTEM')

  goon_ok=True

  if side==None:
    print 'No side specified, and no ADOPT_SIDE environment variable set.'
    print 'Please set the ADOPT_SIDE environment variable, or specify a side (L or R) on the command line.'
    goon_ok=False
  else:
    print 'Side: '+side

  if subs==None:
    print 'No ADOPT_SUBSYSTEM environment variable set.'
    print 'Please set the ADOPT_SUBSYSTEM environment variable to either "WFS" or "ADSEC"'
    goon_ok=False
  else:
    print 'Subsystem: '+subs

  if cmd=='w_start' and subs!='WFS':
    print 'w_start can only be run on the WFS computer.'
    goon_ok=False

  if cmd=='adsc_start' and subs!='ADSEC':
    print 'adsc_start can only be run on the ADSEC computer.'
    goon_ok=False

  goon_ok = goon_ok or dryrun

  if not goon_ok: sys.exit(0)

# Check whether the system is already running
  if not dryrun:
    if processControl.getProcessID('msgdrtdb') > 0:
      print 'MsgD-RTDB is active. Please execute a stop command before.'
      sys.exit(0)

# New-style staged boot
  if hasattr(cfg, 'staged_boot'):
    if dryrun:
      print "\nWould start subsystem",subs," on side", side,"  Mode staged boot"

   # Very first boot the low level VisAO system
    if subs=='WFS':
           os.system("ssh aosup@visaosup.visao visao/bin/visao_start.py")
           
    # First the msgd-rtdb
    processControl.startProcessByName('msgdrtdb',verbose=verbose,dryrun=dryrun)

    # Check that msgb started correctly
    if not dryrun:
      time.sleep(1)
      if processControl.getProcessID('msgdrtdb') <= 0:
        print 'MsgD-RTDB did not start correctly'

      try:
        app = thAOApp('start')
        connected = True
      except:
        print 'Cannot connect to MsgD-RTDB'
        sys.exit(0)


    # Start processes in stages and wait for each stage to be ready before going on
    for stage in cfg.staged_boot:
        if verbose: print "-- Stage",stage
        procList = stage.split()

        for process in procList:
          processControl.startProcessByName(process,verbose=verbose,dryrun=dryrun)
          if not dryrun: time.sleep(0.5)
        if not dryrun:
          for process in procList:
            processControl.waitProcessStart( app, process)
 
  else:
#    sys.exit(0)
    if dryrun:
      print "\nWould start subsystem",subs," on side", side,"  Mode boot start"

    # First the msgd-rtdb
    processControl.startProcessByName('msgdrtdb',verbose=verbose,dryrun=dryrun)

    # Start the rest
    for process in cfg.boot_start:
        if process != 'msgdrtdb':
            processControl.startProcessByName(process,verbose=verbose,dryrun=dryrun)
            if not dryrun: time.sleep(0.5)

  # Automatic start of AdOptControl
  #print "\nSpawning AdOptControl GUI ...", 
  #os.spawnlp(os.P_NOWAIT, 'AdOptControl')
 # print "Done"

if __name__ == '__main__': main()
