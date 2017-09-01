#!/usr/bin/env python
#
#+
# 
# when this module is imported, it parses .h files in Supervisor/lib
# and convert list of #define's into dictionaries of constants 
#
# HISTORY: oct 2005. lbusoni@arcetri.astro.it
#
#-


import string,os


def parse_H(filename, stringtofind, hex=False):
    '''
    Parse a .h file and generate the equivalent Python dictionary.
    ---
    Oct 2008 - A. Puglisi - Obsolete - file .py generated at compilation time instead.
    '''

    start = False;
    dritto={}
    rovescio={}

    lines = file(filename).read().splitlines()
    while len(lines) > 0:
        l = lines.pop(0)

        words = l.split()
        if len(words)<1:
            continue
        if words[0] == '//' and words[1] == stringtofind+'_START':
            start = True
            continue
        if words[0] == '//' and words[1] == stringtofind+'_STOP':
            return (dritto,rovescio)
        if start != True:
            continue
        if words[0][0] == '/': # it is a comment line. skip it
            continue

        if (hex):
            dritto[words[1]]=int(words[2],16)
            rovescio[int(words[2],16)]=words[1]
        else:
            dritto[words[1]]=int(words[2])
            rovescio[int(words[2])]=words[1]

# These four files are generated when /lib is compiled

from AOStates import *
from AOVarCode import *
from AOMsgCode import *
from AOSCode  import *
from AOErrCode import *

# print '.h files parsed'

