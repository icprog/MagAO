#!/usr/bin/env python

# - quando viene selezionato un binning:
#  stop DSP, stop CCD
# - reconfig CCD nuovo binning (la GUI se ne accorge e resiza il display)
# - si prende una LUT (scelta utente o ultima)
# - l'ultima word e' l'effettivo n. di sottoapert. (formato Mario), dovrebbe essere multiplo di 8 o 16
#      - scrivere il valore nell'MsgD-RTDB, le operazioni da ora in poi potrebbero andare in automatico sull'onda del notify
#  - si scrive lo stesso numero nell'sc come NUMSLOPES
#  - upload LUT allo sc
#   - rilettura delle 3 LUT da parte della GUI
#    - comando al P45 lbt_set_rtr(n_slopes, sl_nomefile)
#    - rileggere sl_dsp_map.txt (sl_nomefile) e uploadarlo allo sc
#    - ricaricare parametri opzionali sc: dark, slopenull
#    - start CCD
#    - start DSP
# 
# - quando si generano matrici REC e slopenull si mette nell'header il nome/timestamp della LUT usata
# - quando vengono caricate, il programma deve lamentarsi se la LUT corrente e quella salvata non coincidono
#
# - la meas_intmat_p45 legge n_slopes dalla variabile MSGDRTDB

import sys, os, time
from AdOpt import thAOApp, setupDevices, cfg, calib, WfsArbScripts, AOExcept

app = thAOApp('chngbin__', side = cfg.side)

if len(sys.argv)<2:
    print "Please specify the new binning"
    sys.exit(-1)
else:
    binning = int(sys.argv[1])
    print "Setting binning: %d" % binning


useLast = False
useNum = False
showError = False
if '-showError' in sys.argv:
    showError = True

if len(sys.argv)>=3:
    if sys.argv[2] == 'last':
        useLast = True
    else:
        useNum = True

try:

    # Get a new LUT
    dir = calib.LUTsDir( 'ccd39', binning) + '/'
    dirlist = os.listdir(dir)
    dirlist.sort()

    if not useLast and not useNum:
        print "Please select a LUT:"
        for n in range(len(dirlist)):
            print "%d: %s" % (n, dirlist[n])

        print "Enter selection:"
        num = int(sys.stdin.readline())

    if useLast:
        num = len(dirlist)-1

    if useNum:
        num = int(sys.argv[2])

    app.mute()
    setupDevices.loadDevices(app)
    app.unmute()

    # Zero out modulation
    print "Setting zero modulation, please wait..."
    app.tt.set(amp=0)
    time.sleep(10)
    WfsArbScripts.setHObinning( app, binning, dirlist[num])

except AOExcept.AdOptError, e:
    print 'Error:',str(e)
    if showError:
        import time
        time.sleep(100)

except Exception, e:
   print 'Error:',e
   if showError:
       import time
       time.sleep(100)



