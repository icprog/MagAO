#!/usr/bin/python
#@File: select.py
#
# AO Supervisor installation switcher
# 
# L.Fini, Mar 2010.
#
# This procedure will show you AO Supervisor installations available
# and will allow to set one as current.
#
# To set up a new installation you may use prepare.py
#
#@

Version="1.5"

import os,sys

import prepare as prep

vspace = '\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n'


def usage():
  print "\nAO Supervisor installation selector. Version: %s\n" % Version
  print """Usage:

   python select.py [-h] [-s] [-c] [-f VER]

     -h      Print this help page and exit

     -s      Just show currently installed version

     -c      Create environment setup file, do not change
             current installation

     -k      Create installation setup file, do not change
             current installation

     -f VER  Select version VER (non interactive mode)
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


class Installation:
  def __init__(self,path):
    "Make a list of candidate installation directories"
    self._path=os.path.abspath(path)
    dirs=os.listdir(path)
    self._installations=[]
    self._current=''
    fname=os.path.join(path,'current')
    if os.path.exists(fname):
      currpath=os.path.realpath(fname)
      self._current=os.path.basename(currpath)
    for d in dirs:
      if d == 'current':
        pass
      else:
        self._installations.append(d)

  def list(self):
    return self._installations

  def current(self):
    return self._current

class ExeInstallations(Installation):
  def __init__(self,path):
    "Make a list of executables installations"
    self._path=os.path.abspath(path)
    Installation.__init__(self,path)

  def checkInstallation(self,inst):
    path=os.path.join(self._path,inst)
    fname=os.path.join(path,prep.README_NAME)
    has_readme=False
    if os.path.exists(fname): has_readme=True
    has_dirs=True
    for k in ('bin','calib','conf','idl','lib','py'):
      fname=os.path.join(path,k)
      if not os.path.exists(fname): has_dirs=False
    return (has_readme,has_dirs)

  def layout(self):
    dlist={self._current : 'Installation currently active'}
    for d in self._installations:
      if d == self._current:
        stat= '--> Installation currently active'
      else:
        has_readme,has_dirs=self.checkInstallation(d)
        if has_readme and has_dirs:
          stat='    OK'
        elif (not has_readme) and (not has_dirs):
          stat='    This does not seem an AO Supervisor installation'
        else:
          stat='    This seems an incomplete installation'
      dlist[d]=stat
    k=dlist.keys()
    return dlist

  


def main():
  global make_script
  make_script=False
  make_temp_script=False

  if '-h' in sys.argv: usage()

  dryrun=False
  if '-s' in sys.argv: 
    dryrun=True

  if '-c' in sys.argv: 
    dryrun=True
    make_script=True

  if '-k' in sys.argv: 
    dryrun=True
    make_temp_script=True

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
    print "\nAO Supervisor installation selector. Version: %s\n" % Version

# Check current environment
  ADOPT_HOME = required_get("ADOPT_HOME")
  ADOPT_LOGBASE = required_get("ADOPT_LOGBASE")
  ADOPT_ROOT = optional_get("ADOPT_ROOT")
  ADOPT_LOG = optional_get("ADOPT_LOG")
  ADOPT_MEAS = optional_get("ADOPT_MEAS")
  ADOPT_SIDE = optional_get("ADOPT_SIDE")
  ADOPT_SUBSYSTEM = optional_get("ADOPT_SUBSYSTEM")

  inst=ExeInstallations(ADOPT_HOME)

  if interactive:
    print "Installations layout\n"
    dlist=inst.layout()
    keys=dlist.keys()
    keys.sort()
    for k in keys:
      print "%10s %s" %  (k,dlist[k])
    print
  
  if make_script or make_temp_script:
    ADOPT_SOURCE = os.path.dirname(os.path.abspath(sys.argv[0]))

    if not ADOPT_MEAS:
      while 1:
        answ=raw_input("\nDirectory for measures? ")
        if not answ: continue
        measdir=os.path.expanduser(answ)
        measdir=os.path.abspath(measdir)
        ADOPT_MEAS=measdir
        break

    if not ADOPT_SIDE:
      while 1:
        answ=raw_input("\nWhich side? [right/left] ")
        if not answ: continue
        if answ[0].upper() == 'R':
          ADOPT_SIDE='R'
          break;
        if answ[0].upper() == 'L':
          ADOPT_SIDE='L'
          break;

    if not ADOPT_SUBSYSTEM:
      while 1:
        answ=raw_input("\nWhich subsystem? [adsec/wfs] ")
        if not answ: continue
        if answ[0]=='A':
          ADOPT_SUBSYSTEM='ADSEC'
          break
        if answ[0]=='W':
          ADOPT_SUBSYSTEM='WFS'
          break

  if make_script:
    prep.make_script(ADOPT_HOME,ADOPT_LOGBASE,ADOPT_MEAS,ADOPT_SOURCE,ADOPT_SIDE,ADOPT_SUBSYSTEM)
    print "\nCreated file: adopt_environment.sh\n"

  if make_temp_script:
    while 1:
      answ=raw_input("\nTarget installation? ")
      if not answ:
        answ=raw_input("Your selection (press return again to exit)? ")
        if not answ: sys.exit()
  
      if answ not in inst.list():
        print "Installation %s does not exist"%selection
        continue
      break

    prep.make_temp_script(ADOPT_HOME,answ,ADOPT_SOURCE,ADOPT_SIDE,ADOPT_SUBSYSTEM)
    print "\nCreated file: install_environment.sh\n"

  
  if dryrun: sys.exit()

  if interactive:
    while 1:
      selection=raw_input("\nYour selection? ")
      if not selection: 
        selection=raw_input("Your selection (press return again to exit)? ")
        if not selection: sys.exit()
  
      if selection not in inst.list():
        print "Installation %s does not exist"%selection
        continue
      break

  else:
    selection=select_arg
    if selection not in inst.list():
      print "ERROR: selection not available"
      sys.exit(1)

  rootdir=os.path.join(ADOPT_HOME,selection)
  logdir=os.path.join(ADOPT_LOGBASE,selection)

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
    ret=make_links(ADOPT_HOME,rootdir,ADOPT_LOGBASE,logdir,interactive)
  else:
    print "\nNothing done!\n"
    ret=0
  sys.exit(ret)

if __name__ == '__main__': main()
