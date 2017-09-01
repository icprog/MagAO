#!/usr/bin/env python
#@File: p45lib.py
#
# Function to communicate with the P45 mirror
#@

import fits_lib, os, time
from numarray import *
from AdOpt import cfg
from AdOpt.wrappers import msglib

# Configuration

temp_file = cfg.tmpdir+"temp_in"                       # temporary file
apply_modes_file = cfg.tmpdir+"apply_mode_vector.fits"      # file prefix for set_p45_modal
apply_rtr_file = cfg.tmpdir+"apply_rtr_matrix.fits"         # file prefix for set_p45_modal

msg_timeout = 100      # Communication timeout in ms


def p45_set():
    msglib.SendMessage( cfg.taskname_IDLCtrl, cfg.msg_idlsetmir, "")
    msg = msglib.WaitMessage( msg_timeout )
    if msg['status'] != 'NO_ERROR':
        print "Error setting mirror"

def p45_init_ol():
    msglib.SendMessage( cfg.taskname_IDLCtrl, cfg.msg_idlinitol, "")
    msg = msglib.WaitMessage( msg_timeout )
    if msg['status'] != 'NO_ERROR':
        print "Error initialiting optical loop"

def p45_rip():
    msglib.SendMessage( cfg.taskname_IDLCtrl, cfg.msg_idlripmir, "")
    msg = msglib.WaitMessage( msg_timeout )
    if msg['status'] != 'NO_ERROR':
        print "Error RIPping the mirror"

def p45_flat():
#    msglib.SendMessage( cfg.taskname_IDLCtrl, cfg.msg_idlflatmir, "")
    pass

def p45_set_nslopes(app, n):
    msg = app.SendMsg( cfg.taskname_IDLCtrl, cfg.msg_idlsetrtr, str(n), timeout=60) 
    if msg['status'] != 'NO_ERROR':
        print "Error setting subap number"

    
def p45_apply_modes( modes):

    if len(modes) != 48:
        print "Wrong array length: "+str(len(modes))+" instead of 48"
        return

    m = zeros(48, Float32)
    for i in range(len(m)):
        m[i] = modes[i]
   
    # Write command to a temporary file and then move it to the
    # right name (to avoid reads on a partial file)
    hdr = {}
    hdr['type'] = 'modes'

    counter =0
    while (os.access( apply_modes_file, os.F_OK)) and (counter <100):
        time.sleep(0.01)
        counter = counter+1
    if counter == 100:
        print 'Mirror handshake failed. Force it?'
        answer = sys.stdin.readline().strip()
        if answer == "y" or answer == "yes":
            os.unlink( apply_modes_file)
        else:
            sys.exit(0)

    fits_lib.writeSimpleFits( apply_modes_file, m, hdr)

    # Send a command to IDL now...

    print "Sending command to IDL"
    msglib.SendMessage( cfg.taskname_IDLCtrl, cfg.msg_idlapplymodes, apply_modes_file)
    msg = msglib.WaitMessage(msg_timeout)
    print msg['status']
    if msg['status'] != 'NO_ERROR':
        print "Error applying modes vector"


def p45_set_gain( value):

    print "Sending gain command to IDL"
    idl_cmd = "print,set_g_gain(-1,%5.2f)" % value 
    print idl_cmd
    msglib.SendMessage( cfg.taskname_IDLCtrl, cfg.msg_idlcmd, idl_cmd)
    msg = msglib.WaitMessage(msg_timeout)
    if msg['status'] != 'NO_ERROR':
        print "Error setting gain"



def p45_apply_rtr( filename):

    print "Sending command to IDL"
    msglib.SendMessage( cfg.taskname_IDLCtrl, cfg.msg_idlapplyrtr, filename)
    msg = msglib.WaitMessage(msg_timeout)
    if msg['status'] != 'NO_ERROR':
        print "Error applying rtr matrix"


def lbt672_set_gain( app, filename):
    idl_cmd = "print, fsm_set_gain('%s')" % filename
    print idl_cmd

    app.SendMsg( app, "idlctrl@M_ADSEC", cfg.msg_idlcmd, idl_cmd, timeout = 1000)







##############################
# Old file-based communication
#############################


# Do not modify this
f = "apply_modes_file"
cmd_file = cfg.tmpdir+f+"_in.fits"
res_file = cfg.tmpdir+f+"_out.fits"
tmp      = cfg.tmpdir+temp_file
def set_p45_modal_old( modes):

    if len(modes) != 48:
        print "Wrong array length: "+str(len(modes))+" instead of 48"
        return

    # Ensure a minimum time between commands
    time.sleep(0.01)

    m = zeros(48, Float32)
    for i in range(len(m)):
        m[i] = modes[i]
   
    # Write command to a temporary file and then move it to the
    # right name (to avoid reads on a partial file)
    hdr = {}
    hdr['type'] = 'modes'
    fits_lib.writeSimpleFits( tmp, m, hdr)
    os.rename( tmp, cmd_file)

    # Wait for result
    while 1:
        try:
            res = fits_lib.readSimpleFits( res_file )
            os.remove( res_file)
            print "Apply mode result: ", res
            break
        except:
            pass

    # Wait for mirror settling time + 10%
    time.sleep(0.011)


def zero_p45():
    print "Flattening p45..."
    p45_apply_modes( [0.0] * 48)
