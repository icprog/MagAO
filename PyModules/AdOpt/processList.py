#!/usr/bin/env python

import shlex, os, sys
from AdOpt import AOConfig

# Generate a conf. file with a process - taskname dictionary for IDL

def generateConf(outfile):
 
    from AdOpt import cfg 
    lst = cfg.processes.keys()
    f = file(outfile, 'w')
    try:
        for process in lst:
            f.write('%-30s structure\n' % process[:-1])
            f.write(' msgd_name string %s\n' % cfg.taskname(process))
            f.write(' conf      string %s\n' % cfg.conffile(process))
            f.write('end\n')
        f.close()
    except:
        print 'Problem with process %s configuration' % process[:-1]
        sys.exit(1)


   
def parseProcessList(filename):
    processes = {}
    lines = file(filename).readlines()
    for line in lines:
        line = line.strip()
        if line == '':
            continue
        if line[0] == '#':
            continue
        tokens = shlex.split(line)
        if len(tokens) == 4:
            processes[tokens[0]] = tokens[1:]
            processes[tokens[0]].insert(3, os.getenv('ADOPT_ROOT')+'/bin')
        else:
            print 'Warning: invalid line:'
            print line
    return processes 


