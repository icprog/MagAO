#!/usr/bin/env python
#
#+File: adopt.py
#
# Command interface for the Adopt system
#-


import sys

import readline, code
from AdOpt import *
import agw_power
import simplemotor
import adc
import ccd
import ccd47 as _ccd47
import tt
import bayside_stage
import lamp as _lamp
import cameralens
import thorlabs_ccd

#+Function: help
#
# help system
#-

def adopthelp():
	print globals() 

def help():
    print 'OBSOLETE, please use thaoshell instead.'
    return

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
    print "ttmirror  - tip tilt mirror"
    print "stagex    - bayside X stage"
    print "stagey    - bayside Y stage"
    print "stagez    - bayside Z stage"
    print "ccd39     - ccd39"
    print "ccd47     - ccd47"
    print "lamp      - flowerpot lamp"
    print "lens      - camera lens"
    print
    print "To see which methods are available for each object, type \"dir(object)\""



#+ Main interpreter loop
#-

if __name__ == "__main__":

    app = AOApp("Shell")

    # Default values
    dev_ok = []
    dev_req = ['power','fw1','fw2','adc','ccd39','ccd47','rerotator','mercury','cubestage','stagex','stagey','stagez','ttmirror','thorlabs','lamp','lens']
    file_to_exec = None
    interactive = True
    wait_at_end = False

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

        # Skip logging argument
        if opt == '-l':
            sys.argv.pop(0)
            continue

        # An argument means to execute a file
        file_to_exec = opt
        interactive = False



    # Load requested devices

    if 'power' in dev_req:
        try:
            power = agw_power.AGW_Power(app)
            dev_ok.append('power')
        except Exception, e:
            print e

    if 'fw1' in dev_req:
        try:
            fw1   = simplemotor.SimpleMotor('filterwheel1', app)
            dev_ok.append('fw1')
        except Exception, e:
            print e

    if 'fw2' in dev_req:
        try:
            fw2   = simplemotor.SimpleMotor('filterwheel2', app)
            dev_ok.append('fw2')
        except Exception, e:
            print e

    if 'adc' in dev_req:
        try:
            adc   = adc.Adc( app)
            dev_ok.append('adc')
        except Exception, e:
            print e

    if 'ccd39' in dev_req:
        try:
            ccd39 = ccd.ccd('ccd39', app)
            dev_ok.append('ccd39')
        except Exception, e:
            print e

    if 'ccd47' in dev_req:
        try:
            ccd47 = _ccd47.ccd47(app)
            dev_ok.append('ccd47')
        except Exception, e:
            print e

    if 'rerotator' in dev_req:
        try:
            rerot = simplemotor.SimpleMotor('rerotator', app)
            dev_ok.append('rerot')
        except Exception, e:
            print e

    if 'mercury' in dev_req:
        try:
            cuberot = simplemotor.SimpleMotor('mercury', app)
            dev_ok.append('cuberot')
        except Exception, e:
            print e

    if 'cubestage' in dev_req:
        try:
            cubestage = simplemotor.SimpleMotor('cubestage', app)
            dev_ok.append('cubestage')
        except Exception, e:
            print e

    if 'stagex' in dev_req:
        try:
            stagex = bayside_stage.bayside_stage('baysidex', app)
            dev_ok.append('stagex')
        except Exception, e:
            print e

    if 'stagey' in dev_req:
        try:
            stagey = bayside_stage.bayside_stage('baysidey', app)
            dev_ok.append('stagey')
        except Exception, e:
            print e

    if 'stagez' in dev_req:
        try:
            stagez = bayside_stage.bayside_stage('baysidez', app)
            dev_ok.append('stagez')
        except Exception, e:
            print e

    if 'ttmirror' in dev_req:
        try:
            ttmirror = tt.TipTilt('ttctrl', app)
            dev_ok.append('ttmirror')
        except Exception, e:
            print e

    if 'thorlabs' in dev_req:
        try:
            thorlabs = thorlabs_ccd.thorlabs_ccd(app)
            dev_ok.append('thorlabs')
        except Exception, e:
            print e

    if 'lamp' in dev_req:
        try:
            lamp = _lamp.Lamp('picctrl', app)
            dev_ok.append('lamp')
        except Exception,e:
            print e

    if 'lens' in dev_req:
        try:
            lens = cameralens.CameraLens('cameralensctrl', app)
            dev_ok.append('lens')
        except Exception,e:
            print e

    if interactive:
        help()
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
 

