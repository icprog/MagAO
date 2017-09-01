#!/usr/bin/env python

import getopt, sys

from AdOpt import thAOApp, AOExcept
from AdOpt.hwctrl import ccd


# Parse command line arguments
opts, args = getopt.getopt( sys.argv[1:], "i:", ['quiet', 'target='])

quiet = 0
target = 200
for option, value in opts:
    if option == "--quiet":
        quiet = 1
    elif option == "--target":
        target = int(value)

if not quiet:
    print "Start quadrant equalization?"
    answ = sys.stdin.readline().strip()
    if answ != "y" and answ != "yes":
        sys.exit(0)

self = thAOApp('blackeq')

ccd39 = ccd.ccd('ccd39', self)

try:
    ccd39.equalize_quadrants( target = target)
except AOExcept.AdOptError, e:
    self.log(str(e))
    print e 

