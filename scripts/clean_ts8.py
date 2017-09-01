#!/usr/bin/env python
#@File: clean_ts8.py
#
# Script to clean ts8 connections
#@

import sys, getopt, telnetlib

def usage():
    print 'Usage: clean_ts8.py [--host=<host>] [--user=<username>] [--password=<password]'
    sys.exit(0)

host = 'ts839dx'
user = 'root'
password = 'dbps'

# Parse command line arguments

try:
    opts, args = getopt.getopt( sys.argv[1:], "h", ['host=', 'user=', 'password='])

    for option,value in opts:
        if option == '-h':
            usage()
        if option == '--host':
            host = value
        if option == '--user':
            user = value
        if option == '--password':
            password = value
except:
	usage()



c = telnetlib.Telnet(host)
c.read_until('login: ')
c.write(user+'\n')
c.read_until('password: ')
c.write(password+'\n')
c.read_until('>')
for i in range(1,9):
    c.write('kill %d\n' % i)
    output = c.read_until('>')
    if 'Warning' in output:
        print 'Port %d: no connections' % i
    elif 'logged out' in output:
        print 'Port %d: cleaned connection' % i
    else:
        print 'Port %d: not cleaned'

