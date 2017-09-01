#!/usr/bin/env python
#
# quick IRC wrapper to send irc commands

from AdOpt import cfg

_host = 'tcstest'    # TCS host
_user = 'tcsnew'     # TCS username
ref_template = 'star_mag%3.1f.dat'  # Star definition filename template
script_file = '/tmp/script_file'     # Filename to use for temporary psf script

import os

def setHost(h):
   _host = h

def setUser(u):
   _user = u

def host():
   return _host

def user():
   return _user

def ex(c):
   cmd = 'ssh %s@%s "%s"' % (_user, _host, c)
   print cmd
   os.system(cmd)

def setReference(mag):
   filename = ref_template % mag
   cmd = 'irc SetReference %s' % filename
   ex(cmd)

def presetAO(mode='ACE-AO'):
   cmd = 'irc PresetAO %s %s' % (mode, cfg.side_to_string(cfg.side))
   ex(cmd)

def acquireRefAO():
   cmd = 'irc AcquireRefAO %s' % cfg.side_to_string(cfg.side)
   ex(cmd)

def runAO():
   cmd = 'irc RunAO false %s' % cfg.side_to_string(cfg.side)
   ex(cmd)

def startAO():
   cmd = 'irc StartAO %s' % cfg.side_to_string(cfg.side)
   ex(cmd)

def pauseAO():
   cmd = 'irc PauseAO %s' % cfg.side_to_string(cfg.side)
   ex(cmd)

def resumeAO():
   cmd = 'irc ResumeAO %s' % cfg.side_to_string(cfg.side)
   ex(cmd)

def stopAO():
   cmd = 'irc StopAO stop %s' % cfg.side_to_string(cfg.side)
   ex(cmd)

def offsetXY(x, y):
   cmd = 'irc OffsetXYAO %f %f %s' % (x,y,cfg.side_to_string(cfg.side))
   ex(cmd)

def offsetZ(z):
   cmd = 'irc OffsetZAO %f %s' % (z,cfg.side_to_string(cfg.side))
   ex(cmd)

def irtcExp(exp, hz=1000000):
   cmd = 'irc SetIRParams %d %d' % (exp*1e6, hz)
   ex(cmd)

def autogain(mode='old'):
   if mode == 'old':
       cmd = 'thaoshell.py -name autogain %s/py/sweep_gain_modal.py' % os.getenv('ADOPT_ROOT')
       os.system(cmd)
   elif mode =='new':
       cmd = 'thaoshell.py -name autogain %s/py/sweep_gain_tt_2ho.py' % os.getenv('ADOPT_ROOT')
       os.system(cmd)
   else:
       print 'Unknown autogain mode: "%s"' % mode
      

def acq(script):
   f = file(script_file, 'w')
   f.write(''.join(script))
   f.close()
   os.system('acq %s' % script_file)

def disturb( filename='', mode=''):
   if filename != '':
      template = """
disturb FILENAME
"""
      script = template.replace('FILENAME', filename)
      acq(script)

   if mode != '':
      template = """
disturb MODE
"""
      script = template.replace('MODE', mode)
      acq(script)

def cameralens( enable):
   if enable:
      onoff = 'on'
   else:
      onoff = 'off'

   template = """
cameralens ONOFF
"""
   script = template.replace('ONOFF', onoff)
   acq(script)

def aodata( nframesao):

   template = """
image 0 0 AO
"""
   script = template.replace('AO', '%d' % nframesao)
   acq(script)


def single_psf( exptime, nframes, nframesao=10):

   template = """
irtc EXPTIME
image NFRAMES 0 AO
"""
   script = template.replace('NFRAMES', '%d' % nframes).replace('EXPTIME', '%5.3f' % exptime).replace('AO', '%d' % nframesao)
   acq(script)


def pisces_psf( nframes, nframesao=1000):

   template = """
pisces NFRAMES 0 AO
"""
   script = template.replace('NFRAMES', '%d' % nframes).replace('AO', '%s'% nframesao)
   acq(script)


def psf( exptime, nframes):

   template = """
irtc EXPTIME
image NFRAMES
image NFRAMES
image NFRAMES

cameralens off
savegain
gainzero
lamp off
wait 5

dark NFRAMES
lamp on
wait 5
restoregain
wait 5
cameralens on

image NFRAMES
image NFRAMES
image NFRAMES

"""

   script = template.replace('NFRAMES', '%d' % nframes).replace('EXPTIME', '%5.3f' % exptime)
   acq(script)


