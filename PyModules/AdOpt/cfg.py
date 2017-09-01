#!/usr/bin/env pyhton

# Cfg wrapper to load either the left or right hand side

# Load cfg_left or cfg_right according to the following rules:
#
# command line argument: -l <side>: side is 'L', 'R' or 'C'
# environment variable: ADOPT_SIDE
#
# defaults to left

import sys, types, os
from AdOpt import AOExcept, bcolors

######################################################
# Common functions


###########################
# Process lookup functions
#

# The new taskname is just the process name!
def taskname(name):
    return name+'.'+side

# Returns the configuration filename for the specified process

def conffile(name):
    return confdir+'/'+name+'.conf'

# Returns the MsgD CLSTAT variable name for the specified process

def clstat(name):
    return taskname(name)+".CLSTAT"

def varname(process, name=None, msgd=None):
    '''
    Returns a properly formatted variable name. <procside> may be None for the processes that do not have a side.
    '''
    varname = taskname(process)
    if name != None:
        varname += '.'+name
    if msgd != None:
        varname += '@'+msgd

    return varname

def split_varname(varname):
    '''
    Splits a varname into process, side and name components.
    Returns an object with 'process', 'side' and 'name' attributes.
    '''
    class o: pass
    s = varname.split('.',2)
    o.process = s[0]
    o.side = s[1]
    o.name = s[2]
 
    return o


# Returns the default log files for the specified process

def logfile(name):
    return os.path.join( os.getenv('ADOPT_LOG', '/tmp'), taskname(name)+'.log')

def stdoutfile(name):
    return '/tmp/'+name+'.stdout'

##########################    
# Cfg values functions

# Gets a config. value from the array, given the filename
# <name> may be a hw controller name or a hw controller class
# with a name attribute
# If test is True, errors are not fatal and do not print error messages, but raise a KeyError exception

def get_cfg_value( name, keys, test=False):

    # Test if conf file was loaded
    try:
        dummy = cfg[name]
    except KeyError, e:
        if test:
            raise
        print 'No configuration file defined for: ',e
        sys.exit(0)

    try:
        if type(keys) == types.StringType:
            return cfg[name][keys].Value()
        elif type(keys) == types.ListType:
            return cfg[name]['.'.join(keys)].Value()
        else:
            return None
    except KeyError, e:
        if test:
            raise
        print 'No keyword '+str(keys)+' defined in configuration file for '+name
        sys.exit(0)

def side_to_string( s):
    '''
    Converts 'L' or 'R' to a string like 'left' or 'right'.
    'current' is converted to the current side.
    '''
    if s == 'L':
        return 'left'
    if s == 'R':
        return 'right'
    if s == 'C':
        return 'center'
    if s == 'left' or s == 'right' or s == 'center':
        return side
    if s == 'current':
        return side_to_string(side)

    if side == None:
        return 'None'

    raise AOExcept.AdOptError( errstr= 'cfg.side_to_string(): invalid side specified: '+side)



###############################################################################################################
# Read configuration files 
#
# The cfg dictionary has one entry for each configuration file, indexed with the process name.

def loadCfg():
    global cfg
    #if not locals().has_key('processes'):
    #    return
    for key in processes.keys():
        p = processes[key]
        cfg[key] = {}
        if os.path.exists(conffile(key)):
            try:
                cfg[key] = AOConfig.read_config(conffile(key), add_side = True)
            except Exception,e:
                print e
                print bcolors.bcolors.BOLD+'Processing configuration file: '+conffile(key)
                print bcolors.bcolors.BOLD + bcolors.bcolors.FAIL+'[FAILED]'+bcolors.bcolors.ENDC
                print 'Error: problem reading conf file %s' % key
                sys.exit(0)

def getProcess(p):

    if processes.has_key(p):
        return processes[p]
    return None


# End of cfg functions
##############################################




#####################################
#
# Side Left/right/center loader
# Subsystem loader



subsystem = None
side = None

if '-s' in sys.argv:
    side = sys.argv[ sys.argv.index('-s')+1]

if '-subsystem' in  sys.argv:
    subsystem = sys.argv[ sys.argv.index('-subsystem')+1]


if not side:
    import os
    if os.environ.has_key('ADOPT_SIDE'):
        side = os.environ['ADOPT_SIDE']

if not subsystem:
    import os
    if os.environ.has_key('ADOPT_SUBSYSTEM'):
        subsystem = os.environ['ADOPT_SUBSYSTEM']

if not side:
    print 'Telescope side not specified, defaulting to LEFT'
    side = 'L'

if not subsystem:
    print 'AO subsystem not specified, defaulting to WFS'
    subsystem = 'WFS'


from processList import *

# Get subsystem name
cur_dir = os.getenv('ADOPT_ROOT')+'/conf/'+subsystem.lower()+'/current'
subsystem_name = os.readlink( cur_dir)

# Get main config directory for this subsystem
confdir = cur_dir+'/processConf'
if not os.path.exists(confdir):
    print 'Directory '+confdir+' does not exists'
    sys.exit(0)
else:
  print 'Configuration directory: '+confdir

# Read process list 

processes = parseProcessList( confdir + '/processList.list')
process_conf = AOConfig.read_config( confdir + '/processList.conf', useAOVar=False)

# Transfer all process_conf keys in the main namespace
for key in process_conf.keys():
    if key[-3:] == '.sz':       # Skip array size information
        continue
    cmd = '%s = process_conf["%s"]' % (key, key)
    bytecode = compile(cmd, '<string>', 'single')
    eval(bytecode)




############################
# Parts that need the side configuration


cfg = {}
loadCfg()


# Load specific subsystem configuration (cfg_W2.py, cfg_ADSEC.py, etc)
# This should be removed 

exec('from cfg_'+subsystem_name+' import *')


