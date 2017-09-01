#!/usr/bin/env python
#
# Log archiving procedure
#
# Script to scan the log directory and move all archived log files into long-term storage.
# Intented to be called periodically from cron.
#
# History
# Jan 2009  A. Puglisi - first version

import os

###########################
# Configuration starts. Command line options can override these values.

# Detailed output
verbose = False

# Test run - print on stdout which actions would be taken, but don't rename or remove any files
test = False

# Where to find log files
logdir = os.getenv('ADOPT_LOG') or '/tmp'

# Where to archive them
archivedir = '/backup/logs/'

# List of clients to exclude from logging - see also the following remove flag
exclude = ['MIRRORCTRL', 'msgdrtdb']

# Remove excluded files? - if yes, logs archived but excluded from storage will be removed.
rm_exclude = True

# Configuration ends
###########################

import os, re, sys, shutil
from optparse import OptionParser

parser = OptionParser()
parser.add_option("-v", "--verbose", action="store_true", dest="verbose", default=verbose, help="detailed output (defaults to silent)")
parser.add_option("-t", "--test", action="store_true", dest="test", default=test, help="test run (do not modify or remove files")
parser.add_option("--noremove", action="store_false", dest="rm_exclude", default=rm_exclude, help="do not remove files excluded from archiving")

(options, args) = parser.parse_args()

lst = os.listdir(logdir)

p = re.compile(r'\.\d{14}\.')

for f in lst:
    if p.search(f):
        # Looks like an archived log

        # Check exclusion
        ex = filter( lambda x: re.search ( x+ r'\d\d\.', f), exclude)
        if len(ex)>0:

            # See if we have to remove it
            if options.rm_exclude:
                if options.verbose:
                   print 'Removing %s' % os.path.join(logdir, f)
                if not options.test:
                   try:
                      os.unlink( os.path.join( logdir, f))
                   except OSError, e:
                      print 'Cannot move %s: %s' % (f, str(e))
            continue


        # Get prefix
        try:
            pos = f.index('.')
            dir = f[0:pos]
        except ValueError:
            dir = "."
    
        # Move away
        if options.verbose:
            print '%s -> %s' % ( os.path.join(logdir,f), os.path.join(archivedir, dir, f))
        if not options.test:
            try:
                try:
                    os.makedirs( os.path.join(archivedir, dir))
                except OSError, e:
                    pass
                shutil.move( os.path.join(logdir,f), os.path.join(archivedir, dir, f))
            except OSError, e:
                print 'Cannot move %s: %s' % (f, str(e))


