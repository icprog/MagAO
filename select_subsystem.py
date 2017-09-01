#!/usr/bin/python
#@File: select_subsystem.py
#
# AO Supervisor subsystem selector
# 
# A.Puglisi, Jul 2010.
#
# This procedure will allow to set the current subsystem among the ones available
#
#@

Version="1.0"

import os,sys

vspace = '\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n'


def usage():
  print "\nAO Supervisor subsystem selector. Version: %s\n" % Version
  print """Usage:

   python select_subsystem.py [-h] [-s] [-f SUB]

     -h      Print this help page and exit

     -s      Just show currently installed version

     -f SUB  Select subsystem SUB (non interactive mode)
"""
  sys.exit()


def required_get(vname):
  try:
    var = os.environ[vname]
  except:
    print """
Environment variable %s not defined!
""" % vname
    print """
As a result of previous installations of the AO Supervisor, you should
have a file named: 

       "adopt_environment.sh" 

to define the required environment variables. This file must be executed 
at login to include the definitions in your current environment.  The file 
is generated as a result of running the AO Supervisor installation procedure.

If you have done one or more installations you must verify that the file is
actually executed at login.

If you have not yet done any AO Supervisor installation, do it now using
the procedure:

     prepare.py

"""
    sys.exit()
  return var

def optional_get(vname):
  global make_script
  var = os.environ.get(vname,'')
  if not var:
    print """
Environment variable %s not defined!
""" % vname
    if not make_script:
      print """
It seems that your installation is not complete. Anyway a new environment
setup script will be generated at the end of this procedure.
"""
    make_script=True
  return var

def make_link(dir,link):
  try:
    os.unlink(link)
  except:
    pass
  try:
    os.symlink(dir,link)
  except:
    ret= 1
  else:
    ret= 0
  return ret

  
def make_links(homedir,rootdir,logbasedir,logdir,interactive=True):
  rootlink=os.path.join(homedir,"current")
  ret = make_link(rootdir,rootlink)
  if ret==0:
    loglink=os.path.join(logbasedir,"current")
    ret = make_link(logdir,loglink)

  if interactive:
    if ret==0:
      print "\nThe following links have been created:\n"
      print " ADOPT_ROOT: ",rootlink,"-->",rootdir
      print "  ADOPT_LOG: ",loglink,"-->",logdir
      print
    else:
      print "ERROR: creating symbolic links!!\n"
  else:
    if ret==0:
      print "OK"
    else:
      print "ERROR: creating symbolic links"

  return ret


class Subsystems:
  def __init__(self,path):
    "Make a list of candidate subsystem directories"
    self._path=os.path.abspath(path)
    dirs=os.listdir(path)
    self._subsystems=[]
    self._current=''
    fname=os.path.join(path,'current')
    if os.path.exists(fname):
      currpath=os.path.realpath(fname)
      self._current=os.path.basename(currpath)
    for d in dirs:
      if d == 'current':
        pass
      else:
        self._subsystems.append(d)

  def list(self):
    return self._subsystems

  def current(self):
    return self._current

  def layout(self):
    #dlist={self._current : 'Subsystem currently active'}
    dlist={}
    for d in self._subsystems:
      if d == self._current:
        stat= '--> Subsystem currently active'
      else:
        has_cfg=self.checkSubsystem(d)
        if has_cfg:
          stat='    OK'
        else:
          stat='    This seems an incomplete installation'
      dlist[d]=stat
    k=dlist.keys()
    return dlist

  def checkSubsystem(self,inst):
    return True


class ConfSubsystems(Subsystems):
  def __init__(self,path):
    "Make a list of properly configured subsystems"
    self._path=os.path.abspath(path)
    Subsystems.__init__(self,path)

  def checkSubsystem(self,inst):
    path=os.path.join(self._path,inst)
    has_cfg=True
    for k in ('processConf','processConf/processList.conf','processConf/processList.list'):
      fname=os.path.join(path,k)
      if not os.path.exists(fname): has_cfg=False
    return has_cfg

  


def main():

  if '-h' in sys.argv: usage()

  dryrun=False
  if '-s' in sys.argv: 
    dryrun=True

  interactive=True
  if '-f' in sys.argv:
    try:
      select_arg=sys.argv[2]
    except:
      print "ERROR: wrong argument"
      sys.exit(1)
    else:
      interactive=False

  if interactive:
    print "\nAO Supervisor subsystem selector. Version: %s\n" % Version

# Check current environment
  ADOPT_HOME = required_get("ADOPT_HOME")
  ADOPT_LOGBASE = required_get("ADOPT_LOGBASE")
  ADOPT_ROOT = required_get("ADOPT_ROOT")
  ADOPT_LOG = optional_get("ADOPT_LOG")
  ADOPT_MEAS = optional_get("ADOPT_MEAS")
  ADOPT_SIDE = optional_get("ADOPT_SIDE")
  ADOPT_SUBSYSTEM = required_get("ADOPT_SUBSYSTEM")

  confInst=ConfSubsystems(os.path.join(ADOPT_ROOT,"conf",ADOPT_SUBSYSTEM.lower()) )
  calibInst=Subsystems(os.path.join(ADOPT_ROOT,"calib", ADOPT_SUBSYSTEM.lower()))

  if interactive:
    print "Installations layout\n"
    print "Configuration:\n"
    dlist=confInst.layout()
    keys=dlist.keys()
    keys.sort()
    for k in keys:
      print "%10s %s" %  (k,dlist[k])
    print

    print "Calibration:\n"
    dlist=calibInst.layout()
    keys=dlist.keys()
    keys.sort()
    for k in keys:
      print "%10s %s" %  (k,dlist[k])
    print
  
    while 1:
      selection=raw_input("\nTarget installation? ")
      if not selection:
        selection=raw_input("Your selection (press return again to exit)? ")
        if not selection: sys.exit()
  
      if selection not in confInst.list():
        print "Installation %s does not exist"%selection
        continue
      break

  if dryrun: sys.exit()

  if selection not in confInst.list():
    print "ERROR: selection not available"
    sys.exit(1)

  confdir = os.path.join(ADOPT_ROOT,"conf", ADOPT_SUBSYSTEM.lower())
  conftarget = selection
  conflink = os.path.join(confdir,"current")

  calibdir = os.path.join(ADOPT_ROOT,"calib", ADOPT_SUBSYSTEM.lower())
  calibtarget = selection
  caliblink = os.path.join(calibdir,"current")


  print
  print '##########################################################'
  print '#                                                        #'
  print '#  WARNING  WARNING  WARNING  WARNING  WARNING  WARNING  #'
  print '#                                                        #'
  print '#  If you switch version while the AO Supervisor is      #'
  print '#  running you will mess up it badly.                    #'
  print '#                                                        #'
  print '#  So you must be really sure that everything has been   #'
  print '#  properly shut down.                                   #'
  print '#                                                        #'
  print '##########################################################'
  print

  answ=raw_input("\nOK to proceed selecting %s as current version? " % selection)
  if answ  and answ[0].upper()== 'Y':
    ret=make_link(calibtarget, caliblink)
    if ret==0:
      make_link(conftarget, conflink)
      # Create adsec processes.cfg file
      if ADOPT_SUBSYSTEM.upper() == 'ADSEC':
        outfile = os.path.join(ADOPT_ROOT, "conf", "adsec", "current", "processes.cfg")
        from AdOpt import processList
        processList.generateConf(outfile)

  else:
    print "\nNothing done!\n"
    ret=0
  sys.exit(ret)

if __name__ == '__main__': main()
