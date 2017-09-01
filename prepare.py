#!/usr/bin/python
#@File: prepare.py
#
# AO Supervisor installation preparation
# 
# L.Fini, Feb-Apr 2010.
#
# This procedure will prompt from you a few pieces of information and
# set up the minimum amount of environment variables to allow the
# installation of the AO Supervisor software.
#
# After installation the configure.py procedure will finish up
# environment setting.
#@

Version="1.12"

import os,sys,time

README_NAME='README.ver'

def rmtree(path):
  "removes a directory tree. BE CAREFUL!"
  for root, dirs, files in os.walk(path, topdown=False):
    for name in files:
        os.remove(os.path.join(root, name))
    for name in dirs:
        os.rmdir(os.path.join(root, name))

  

def make_temp_script(ADOPT_HOME,version,sourcedir,side,subsys):
  out=open('install_environment.sh','w')
  print >> out, '#!/bin/bash' 
  print >> out, '# AO Supervisor installation temporary environment' 
  print >> out, '#' 
  print >> out, '# Created on:', time.asctime()
  print >> out, '#' 
  print >> out  
  print >> out, '############################################################'
  print >> out, '# AO Supervisor specific variables'
  print >> out, 'export ADOPT_ROOT=%s' % os.path.join(ADOPT_HOME,version)
  print >> out, 'export ADOPT_SOURCE=%s' % sourcedir
  print >> out, 'export ADOPT_SIDE=%s' % side 
  print >> out, 'export ADOPT_SUBSYSTEM=%s' % subsys 
  print >> out, '############################################################'
  print >> out  
  print >> out, '############################################################' 
  print >> out, '# Note: the following values reflects standard installations' 
  print >> out, '#' 
  print >> out, 'export IDLDIR=/usr/local/itt/idl' 
  print >> out  
  print >> out, 'export QWTDIR=/usr/local/qwt-5.1.2' 
  print >> out, '############################################################' 
  print >> out, 'export QWTLIB=$QWTDIR/lib' 
  print >> out  
  print >> out, '############################################################' 
  print >> out, '# NOTE: uncomment the proper definition'
  print >> out, '#'
  print >> out, '# The following values are for Centos 5.4' 
  print >> out, 'export  QTDIR=/usr/lib/qt-3.3' 
  print >> out, 'export  QT3_DIR=/usr/lib/qt-3.3' 
  print >> out, '#'
  print >> out, '# The following values are for Ubuntu' 
  print >> out, '#export  QTDIR=/usr/share/qt3'
  print >> out, '#export  QT3_DIR=/usr/share/qt3'
  print >> out, '############################################################' 
  print >> out  
  print >> out, 'export  QTINC=$QTDIR/include' 
  print >> out, 'export  QTLIB=$QTDIR/lib' 
  print >> out, 'export  QT3_INC=$QT3_DIR/include' 
  print >> out, 'export  QT3_LIB=$QT3_DIR/lib' 
  print >> out  
  print >> out, 'export PYTHONPATH=$ADOPT_ROOT/lib/python' 
  print >> out  
  print >> out, 'echo '
  print >> out, 'echo "##############################################"'
  print >> out, 'echo "# Temporary installation environment created #"'
  print >> out, 'echo "#                                            #"'
  print >> out, 'echo "# Use only for installation. The run-time    #"'
  print >> out, 'echo "# environment is set up by procedure:        #"'
  print >> out, 'echo "#    adopt_environment.sh                    #"'
  print >> out, 'echo "##############################################"'
  print >> out, 'echo '
  print >> out, 'echo "      Source directory: " $ADOPT_SOURCE'
  print >> out, 'echo "Installation directory: " $ADOPT_ROOT'
  print >> out, 'echo '

  
  out.close()

def make_script(ADOPT_HOME,logbasedir,measdir,sourcedir,side,subsys):
  out=open('adopt_environment.sh','w')
  print >> out, "#!/bin/bash" 
  print >> out, "# AO Supervisor startup file" 
  print >> out, "#" 
  print >> out, "# Created on:", time.asctime()
  print >> out, "#" 
  print >> out  
  print >> out, "############################################################"
  print >> out, "# AO Supervisor specific variables"
  print >> out, "export ADOPT_HOME=%s" % ADOPT_HOME
  print >> out, "export ADOPT_LOGBASE=%s" % logbasedir
  print >> out, "export ADOPT_ROOT=%s" % os.path.join(ADOPT_HOME,"current")
  print >> out, "export ADOPT_SOURCE=%s" % sourcedir
  print >> out, "export ADOPT_LOG=%s" % os.path.join(logbasedir,"current")
  print >> out, "export ADOPT_MEAS=%s" % measdir
  print >> out, "export ADOPT_SIDE=%s" % side 
  print >> out, "export ADOPT_SUBSYSTEM=%s" % subsys 
  print >> out, "############################################################"
  print >> out  
  print >> out, "############################################################" 
  print >> out, "# Note: the following values reflects standard installations" 
  print >> out, "#" 
  print >> out, "export IDLDIR=/usr/local/itt/idl" 
  print >> out  
  print >> out, "export QWTDIR=/usr/local/qwt-5.1.2" 
  print >> out, "############################################################" 
  print >> out, "export QWTLIB=$QWTDIR/lib" 
  print >> out  
  print >> out, "############################################################" 
  print >> out, "# NOTE: uncomment the proper definition"
  print >> out, "#"
  print >> out, "# The following values are for Centos 5.4" 
  print >> out, "export  QTDIR=/usr/lib/qt-3.3" 
  print >> out, "export  QT3_DIR=/usr/lib/qt-3.3" 
  print >> out, "#"
  print >> out, "# The following values are for Ubuntu" 
  print >> out, "#export  QTDIR=/usr/share/qt3"
  print >> out, "#export  QT3_DIR=/usr/share/qt3"
  print >> out, "############################################################" 
  print >> out  
  print >> out, "export  QTINC=$QTDIR/include" 
  print >> out, "export  QTLIB=$QTDIR/lib" 
  print >> out, "export  QT3_INC=$QT3_DIR/include" 
  print >> out, "export  QT3_LIB=$QT3_DIR/lib" 
  print >> out  
  print >> out, "export PYTHONPATH=$ADOPT_ROOT/lib/python" 
  print >> out  
  print >> out, "PATH=$ADOPT_ROOT/bin:$PATH" 
  print >> out  
  print >> out, "alias aoidl="+'"'+"/usr/local/itt/idl/bin/idl -IDL_DLM_PATH '$ADOPT_ROOT/idl/dlm:<IDL_DEFAULT>' $ADOPT_ROOT/idl/init_idl_ao.pro"+'"'
  print >> out, "alias aoidlde="+'"'+"/usr/local/itt/idl/bin/idlde -batch $ADOPT_ROOT/idl/init_idl_ao.pro"+'"'
  print >> out  
  
  out.close()

vspace = '\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n'

def main():
  step=1

  print vspace
  print """
AO Supervisor installation preparation. Version: %s

Please provide the following info:
""" % Version

  print "%d) What's your name" %step
  print '''
  Your name will be recorded in the installation identification file.'''
  while 1:
    installer=raw_input("\nName? ")
    if installer: break
                                     # Prompt for telescope side.
  print vspace
  step+=1
  print '%d) Telescope side.' % step
  print '''
  Specify for which side you are making the installation.'''

  while 1:
    side=raw_input("\nSide? [right/left] ")

    if not side: continue

    if side[0].upper()== "R":
      side="R"
      break
    elif side[0].upper()=='L':
      side="L"
      break
    print '\nPlease specify either left or right' 

                                     # Prompt for subsystem
  print vspace
  print '%d) AO Subsystem.' % step
  print '''
  Specify for which subsystem you are making the installation.

  Choices are: wfs (Wavefront Sensor),  adsec (Adaptive Secondary)'''

  while 1:
    subsys=raw_input("\nSubsystem? [wfs/adsec] ")
    if not subsys: continue

    if subsys[0].upper()== "W":
      subsys="WFS"
      break
    elif subsys[0].upper()=='A':
      subsys="ADSEC"
      break
    print '\nPlease specify either wfs or adsec' 


                                     # Prompt for installation home directory
  print vspace
  step+=1
  print '%d) Home for AO Supervisor software.' % step
  print '''
  This is a suitable directory where run time AO Supervisor files will be
  stored.

  The directory may hold different installations which will be selectable
  by means of a configuration procedure.

  The directory will be created, if not existent, and you must have 
  suitable rights to do so.'''

  curhome=os.environ.get('ADOPT_HOME','')
  while 1:
    home = raw_input("\nAdopt home directory? [default:%s] "%curhome)
    if not home: home=curhome
    if home: break
    print "\nPlease specify a directory"

  home=os.path.expanduser(home)

  ADOPT_HOME=os.path.abspath(home)

                                     # Prompt for log files master directory
  print vspace
  step+=1
  print '\n%d) Directory for log files' % step
  print '''
  This is the directory where log files will be stored.

  AO Supervisor may generate quite a lot of log data, depending 
  on configuration parameters, so you should specify a directory
  on a local disk (to avoid high network traffic) and with plenty
  of space. You must have suitable rights to create the directory.'''

  curblog=os.environ.get('ADOPT_LOGBASE','')
  while 1:
    logbasedir=raw_input('\nLog directory? [default:%s] '%curblog)
    if not logbasedir: logbasedir=curblog
    if logbasedir: break

  logbasedir=os.path.expanduser(logbasedir)
  logbasedir=os.path.abspath(logbasedir)

                                     # Prompt for measure files master directory
  print vspace
  step+=1
  print '\n%d) Directory for measures' % step
  print '''
  This is the directory where files resulting from measures will
  be stored.

  AO Supervisor generates measure files when doing calibrations or
  evaluating parameters (such as darks frames, flats and the like)
  These data files, albeith not directly needed for AO operations
  may be useful for off line diagnosis or postprocessing. Files in 
  this directory can be deleted, if needed, with no harms to AO
  functionality, but should be kept if possible because they may
  prove to be useful.

  You should specify a directory on a local disk (to avoid high 
  network traffic) and with plenty of space. A good choice can be
  the same disk where log files are stored. The directory should
  be different from directories used for executables and for logs.

  You must have suitable rights to create the directory.'''

  curbmeas=os.environ.get('ADOPT_MEAS','')
  while 1:
    measdir=raw_input('\nDirectory for measures? [default:%s] '%curbmeas)
    if not measdir: measdir=curbmeas
    if measdir: 
      measdir=os.path.expanduser(measdir)
      measdir=os.path.abspath(measdir)
      if measdir==logbasedir:
        print "\nYou have chosen the same directory specified for log files"
        print "This is not advisable"
        continue
      if measdir==ADOPT_HOME:
        print "\nYou have chosen the same directory specified for executables"
        print "This is not advisable"
        continue
      break


                                     # Prompt for installation identifier
  print vspace
  step+=1
  print '%d) Installation identifier.' % step
  print '''
  This is a short string which identifies this particular 
  installation.

  Many installations can be stored at the same time, and the 
  configuration procedure allows to set any of them as current
  by using this identifier.

  E.g.: if you specify "XXX" all run time files will be stored
  under:

  %s.  ''' % os.path.join(ADOPT_HOME,'XXX')

  print '''
  The string must be different from "current", which is a reserved word and
  should not contain "/".'''

  while 1:
    remove_previous=False
    version=raw_input("\nInstallation identifier? ")
    ok = version != 'current'
    if version.find('/')>=0: ok = False
    if not ok: 
      print '%s: is not a good choice' % version
      continue
    path=os.path.join(ADOPT_HOME,version)
    if os.path.exists(path):
      remove_previous=True
      answer=raw_input('\nInstallation %s already exists. OK to overwrite? [y/n] '%version)
      if not answer: continue
      if answer[0].upper()!='Y': continue
    break

  logdir = os.path.abspath(os.path.join(logbasedir,version))
# measdir=os.path.abspath(os.path.join(measbasedir,version))
  rootdir = os.path.abspath(os.path.join(ADOPT_HOME,version))
  sourcedir = os.path.dirname(os.path.abspath(sys.argv[0]))

  print vspace
  print '\nThe AO Supervisor will be installed as follows:\n'

  print "                         Installer:", installer
  print " Installing sources from directory:", sourcedir
  print "         Installation to directory:", rootdir
  print "                     Log directory:", logdir
  print "                 Measure directory:", measdir
  print "                              Side:", side
  print "                         Subsystem:", subsys
  if remove_previous:
    print
    print "NOTE: the previous installation dir tree:",rootdir
    print "      will be removed!"

  while 1:
    answer = raw_input("\nOk to proceed? [yes/no] ")

    if not answer: continue
    if answer[0].upper()=='Y': break
    if answer[0].upper()=='N': 
      print "\nNothing done"
      sys.exit()

  make_script(ADOPT_HOME,logbasedir,measdir,sourcedir,side,subsys)
  make_temp_script(ADOPT_HOME,version,sourcedir,side,subsys)

  errors=False

  print "\nCreating installation directories"

  if remove_previous: rmtree(rootdir)

  if not os.path.isdir(rootdir):
    try:
      os.makedirs(rootdir)
    except:
      print "\nCannot create directory:",rootdir
      errors=True

  readmepath=os.path.join(rootdir,README_NAME)
  try:
    fd=open(readmepath,'w')
  except:
    print "\nCannot create README file:",readmepath
    errors=True
  else:
    print >> fd,"AO Supervisor installation %s made on: %s by: %s" % (version,time.asctime(),installer)
    fd.close()

  if not os.path.isdir(logdir):
    try:
      os.makedirs(logdir)
    except:
      print "\nCannot create directory:",logdir
      errors=True

  if not os.path.isdir(measdir):
    try:
      os.makedirs(measdir)
    except:
      print "\nCannot create directory:",measdir
      errors=True
  
  print '''



The file "adopt_environment.sh" has been created. You must provide for its
execution at login. Usually this is done by moving it to the home 
directory and adding at the end of file .bashrc the line:

if [ -f adopt_environment.sh ]; then . adopt_environment.sh; fi
'''

  if errors:
    print 'NOTE: Some errors while creating directories have been detected!\n'

  print '''
The file "install_environment.sh" has been created. Prior of going on with installation
you must execute it with:

           source install_environment.sh

to define proper environment for installation. 
'''

if __name__=='__main__': main()
