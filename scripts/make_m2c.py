#!/usr/bin/env python
#
# Script to create an M2C directory tree based on the example 'KL' tree

import sys, os, shutil
from AdOpt import calib

prefix = calib.m2cDir()
basem2c = 'KL'

if len(sys.argv) != 2:
   print 'Usage: make_m2c.py <m2c name>'
   sys.exit(0)

m2c = sys.argv[1]

# Create copy the example directory

print
print 'Copying data...'

destdir = os.path.join(prefix,m2c)
srcdir = os.path.join(prefix,basem2c)

shutil.copytree( srcdir, destdir)

print
print 'Created directory: '+destdir
print
print 'M2C file should be copied manually with this command:'
print
print 'cp m2c.fits '+destdir
print
