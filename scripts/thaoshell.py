#!/usr/bin/env python
#@File: adopt.py
#
# Command interface for the Adopt system
#@


import sys, readline, code, time

from AdOpt import thAOApp, setupDevices, cfg, frames_lib, centerPupils, makePupils
from AdOpt.WfsArbScripts import *
from AdOpt.fits_lib import *

#@Function: aohelp
#
# help system
#@


def aohelp():
    print
    print "***************************************************"
    print "            AdOpt interactive console"
    print "***************************************************"
    print "You have the following objects defined:"
    print
    print "power     - to control AGW power"
    print "fw1       - filter wheel #1"
    print "fw2       - filter wheel #2"
    print "adc       - adc (as a single device)"
    print "rerot     - rerotator"
    print "cuberot   - cube rotator"
    print "cubestage - cube stage"
    print "tt  - tip tilt mirror"
    print "stagex    - bayside X stage"
    print "stagey    - bayside Y stage"
    print "stagez    - bayside Z stage"
    print "ccd39     - ccd39"
    print "ccd47     - ccd47"
    print "lamp      - flowerpot lamp"
    print "lens      - camera lens"
    print
    print "To see which methods are available for each object, type \"dir(object)\""


#@Class: Unbuffered
#
# Helper class for unbuffered stdou

class Unbuffered:
    def __init__(self, stream):
        self.stream = stream
    def write(self, data):
        self.stream.write(data)
        self.stream.flush()
    def __getattr__(self, attr):
        return getattr( self.stream, attr)


#@ Main interpreter loop

if __name__ == "__main__":

    # Set unbuffered output
    sys.stdout = Unbuffered(sys.stdout)

    # Default values
    dev_ok = []
    dev_req = ['power','fw1','fw2','adc','ccd39','ccd47','rerotator','mercury','cubestage','stagex','stagey','stagez','tt','lamp','lens','tv']
    file_to_exec = None
    interactive = True
    wait_at_end = False
    cmd_to_exec = None
    myname = 'Shell'

    # Command line parsing

    sys.argv.pop(0)
    while len(sys.argv)>0:
        opt = sys.argv.pop(0)
        if opt == '-dev':
            dev_req = [ x.strip() for x in sys.argv.pop(0).split(',')]
            continue
        if opt == '-wait':
            wait_at_end = True
            continue

        if opt == '-name':
            myname = sys.argv.pop(0)
            continue

        # Skip logging argument
        if opt == '-l':
            sys.argv.pop(0)
            continue

        if opt == '-e':
            cmd_to_exec = sys.argv.pop(0)
            continue


        # An argument means to execute a file
        file_to_exec = opt
        interactive = False

    app = thAOApp(myname)


    #app.mute()
    dev_ok = setupDevices.loadDevices( app)
    #app.unmute()

    # Transfer devices in the main namespace

    for dev in dev_ok:
        cmd = '%s = app.%s' % (dev, dev)
        bytecode = compile(cmd, '<string>', 'single')
        eval(bytecode)

    # Special thorlabs devices

    from AdOpt.hwctrl import thorlabs_ccd
    try:
	thorlabs1 = thorlabs_ccd.thorlabs_ccd(app, 1)
    except Exception, e:
        print e
        print 'No thorlabs1 available'

    try:
	thorlabs2 = thorlabs_ccd.thorlabs_ccd(app, 2)
    except:
        print 'No thorlabs2 available'

    # Exec command if specified

    if cmd_to_exec:
        bytecode = compile(cmd_to_exec, '<string>', 'single')
        try:
            eval(bytecode)
        except Exception, e:
            print e
            if wait_at_end:
                time.sleep(30000)
        sys.exit(0)

    if interactive:
        aohelp()
        print
        print "Devices actually available:"
        print dev_ok



    # Execute

    if interactive:
        import readline
        import rlcompleter
        readline.parse_and_bind("tab: complete")
        readline.parse_and_bind("set show-all-if-ambiguous on")

        shell = code.InteractiveConsole( globals())
        shell.interact("")
    else:
        execfile(file_to_exec)
        if wait_at_end:
            shell = code.InteractiveConsole( globals())
            shell.interact("")
 

