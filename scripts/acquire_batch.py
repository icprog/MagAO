#!/usr/bin/env python
#
# Batch acquiring procedure
#
#
# 1. List all job files in the job directory
# 2. Take older existing file
# 3. Execute file and catch output
# 4. Mail output to recipient specified in file

#################################
# Configuration starts

jobdir = '/towerdata/jobs'
cmd = '/usr/local/adopt/bin/acquire_batch_single'
#cmd = 'ls -l'

# End configuration
################################


import commands, sys, os

if len(sys.argv)>1:
   jobpath = sys.argv[1]
else: 
   print 'Usage: acquire_batch.py <filename>'
   sys.exit(0)
   #jobfile = commands.getoutput('ls -t1r '+jobdir).splitlines()[0]
   #jobpath = os.path.join(jobdir, jobfile)

par = file(jobpath).readlines()
dest = filter( lambda x: 'MailTo' in x, par)[0].split()[2:]

logfile = 'acquire.log'

cmdline = '%s %s | tee %s' % (cmd, jobpath, logfile)
os.system(cmdline)

# Mail result
mailcmd = '/bin/mail -s "Tower data" '+ (' '.join(dest))
print mailcmd
m = os.popen(mailcmd, 'w')
output = file(logfile)
m.write(output.read())
m.close()


