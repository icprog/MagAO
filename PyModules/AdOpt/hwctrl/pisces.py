#@File: pisces.py
#
# Python module to read from PISCES at the telescope.
#
# Assumes that the archive:/newdata TCS directory is mounted locally on /mnt/newdata.
# PISCES acquisition is triggered by an irc command running on obs4, and then read from the newdata directory.

import os
from AdOpt import fits_lib

# Arcetri
#addr = "193.206.155.87" 
#user = 'adopt'
#path = '/Repository/'

addr = "obs4" 
user = 'LBTO'
path = '/mnt/newdata/'



def read( pathOnly = False):

    cmd = 'irc GetImage'
    cmdline = 'ssh %s@%s "%s"' % (user, addr, cmd)

    #print cmdline
    f = os.popen(cmdline, 'r')
    lines = f.read().splitlines()
    filename = lines[0]    

    if pathOnly:
        return path + filename

    return fits_lib.readSimpleFits(path + filename)


def readCube( num, pathOnly = False):

    cmd_cube = 'irc GetSeqImages %d' % num
    cmdline = 'ssh %s@%s "%s"' % (user, addr, cmd_cube)

    f = os.popen(cmdline, 'r')
    lines = f.read().splitlines()
    tokens = lines[2].split('/')
    filename = tokens[-1]

    if pathOnly:
        return path + '/' + filename

    return fits_lib.readSimpleFits(path +'/'+ filename)




