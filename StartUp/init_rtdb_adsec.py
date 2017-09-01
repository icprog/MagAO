#!/usr/bin/env python
#
from AdOpt import *
import sys, os, string

# defaults values, can be overwritten in config file
conffile = 'init_rtdb_adsec.conf'
myself   = 'ADSECRTDB'
basedir = '.'
files = []

if len(sys.argv)>1: 
    if sys.argv[1] == '-h':
        print """
init_rtdb_adsec.py   L.Busoni, Sep 2005

This process reads adsec configuration files and create the 
corresponding variables in the rtdb.

Usage: init_rtdb_adsec.py  [-f config file ] [adsec_conf_files]
-f    specify config file (def init_rtdb_adsec.conf)

input files must be conforming to the rules described in
read_ascii_array/structure.pro in the idl tree of CVS.
        """
        sys.exit(-1)
    elif sys.argv[1] == '-f':
        conffile = sys.argv[2].strip()
    else:
        files = sys.argv[1:]

# read config file        
try:
    params=AOConfig.read_ascii_structure(conffile)
    myself=params['myself']
    if not files:
        basedir=params['basedir']
        datadir=params['datadir']
        confdir=params['confdir']
        progdir=params['progdir']
        tempdir=params['tempdir']
        measdir=params['measdir']
        files=params['files'][1:]
        # internal links in conf files are referred to basedir/conf
        os.chdir(os.path.join(basedir))
except:
    print 'Init_rtdb_adsec failed. Error in config file: %s' % (conffile)
    raise 

# start AdOpt application
app=AOApp(myself)


# this is a dirty trick to discover where DSP programs are
try:
    sysfile  = os.path.join(basedir,confdir,'sys_const.txt')
    templist = AOConfig.read_ascii_structure( sysfile )
    progvers = templist['adsec_prog_ver']
except:
    print 'Init_rtdb_adsec failed. Error in parsing %s' % (sysfile)
    raise 
 


for filename in files:
    try:
        filename = string.replace(filename, '$datadir', datadir) 
        filename = string.replace(filename, '$confdir', confdir) 
        filename = string.replace(filename, '$progdir', os.path.join(progdir,progvers))
        filename = string.replace(filename, '$tempdir', tempdir) 
        filename = string.replace(filename, '$measdir', measdir) 
        lista = AOConfig.read_config(filename)
    except:
        raise
    for item in lista:
        try:
            v = AOConfig.Conv2AOVar(item)
        except:
            print 'WARNING: Conv2AOVar ERROR %s' % (item) #TODO    
        try:
            print v
            v.SetOwner('global')
            app.CreaVar(v)
            app.WriteVar(v)
        except AOVarError, e :
            print e, item
            print 'WARNING: VARIABLE %s WAS NOT WRITTEN IN RTDB----' % (item) 
        except AdOptError, e:
            #TODO lascio correre per ora, ma poi devo bloccare
            print 'Generic AdOpt Error %s on var %s' % e, v
            print 'WARNING: VARIABLE %s HAS UNSUPPORTED TYPE OR NAME TOO LONG.\n ---- NOT WRITTEN IN RTDB----' % (item) 
    

