#@File: WfsArbScripts.py
#
# Non-interactive scripts for the WFS arbitrator
#
#@

import time, os, math, types, string
import threading
import traceback

import numpy.numarray as numarray
from AdOpt import cfg, calib, setupDevices, processControl, fits_lib, AOConstants, frames_lib, AOVar
from AdOpt import boardSetup, centerPupils, source_acq
from AdOpt.InterpolatedArray import *
from AdOpt.AOExcept import *
from AdOpt.AOGlobals import *

def labmode(app):
    try:
       labmode = app.ReadVar(cfg.side+'.LAB.MODE').Value()
    except:
       labmode =0
    return labmode != 0



#@Procedure: setHObinning
#
# Sets the ccd39 binning, reconfiguring both the CCD and the slope computer
#
# Dump of the comment in the previous version:
#
# - quando viene selezionato un binning:
#  stop DSP, stop CCD
# - reconfig CCD nuovo binning (la GUI se ne accorge e resiza il display)
# - si prende una LUT (scelta utente o ultima)
# - l'ultima word e' l'effettivo n. di sottoapert. (formato Mario), dovrebbe essere multiplo di 8 o 16
#      - scrivere il valore nell'MsgD-RTDB, le operazioni da ora in poi potrebbero andare in automatico sull'onda del notify
#  - si scrive lo stesso numero nell'sc come NUMSLOPES
#  - upload LUT allo sc
#   - rilettura delle 3 LUT da parte della GUI
#    - comando al P45 lbt\_set\_rtr(n\_slopes, sl\_nomefile)
#    - rileggere sl\_dsp\_map.txt (sl\_nomefile) e uploadarlo allo sc
#    - ricaricare parametri opzionali sc: dark, slopenull
#    - start CCD
#    - start DSP
# 
# - quando si generano matrici REC e slopenull si mette nell'header il nome/timestamp della LUT usata
# - quando vengono caricate, il programma deve lamentarsi se la LUT corrente e quella salvata non coincidono
#
#@

def setHObinning( aoapp,             #@P{aoapp} Calling AOApp object
                  binning,           #@P{binning} New binning (integer, 1-5 inclusive)
                  pupilReg = None,   #@P{pupilReg} Pupil registration name (string, None means the last available)
                  HOgain = None,     #@P{HOgain} SciMeasure gain setting (integer 0-3, inclusive)
                  force=True):       #@P{force}: if False, avoid reconfiguring CCD and BCU if things are already at the right binning.
    '''
    Sets the HO ccd binning.
    '''

    #############################
    # Configuration area
    # 
    # Default readout speeds to use

    speeds = { 1:2500, 2:800, 3:380, 4:335, 5:380 }

    # End of configuration area
    ##############################

    # Initialize objects
    setupDevices.loadDevices( aoapp, ['ccd39', 'sc', 'tt'], check=True)

    if not force:
        if aoapp.ccd39.xbin() == binning:
            return


    # Stop pupil check activity
    processControl.stopAndWait( aoapp, 'pupilcheck39', timeout = 10)

    # Wait for slope computer init (we may be called just after the turnon)
    aoapp.sc.waitActive( timeout=10)

    # Stop everything just to be sure
    antiDriftStatus = antiDrift(aoapp)
    setAntiDrift(aoapp, 0)
    time.sleep(0.5)

    aoapp.sc.fl_stop()
    aoapp.sc.stop()
    aoapp.ccd39.stop()
    aoapp.tt.set(amp=0)


    #@C
    # For security reason, the tip-tilt modulation amplitude is set to zero: the ccd39
    # is physically capable of running much faster of 1Khz at higher binnings, and the
    # tip-tilt would try to follow it if the sync fiber is attached, which would carry
    # the risk of breaking the tip-tilt. Therefore, the program changing the binning
    # must take care of re-activating the tip-tilt modulation when the ccd39 is running
    # at a safe frame frequency.
    #@
    if hasattr(aoapp,'tt'):
        if aoapp.tt.isOnline():
	        aoapp.tt.set( amp = 0) 

    # Get a new LUT
    dir = calib.LUTsDir( 'ccd39', binning) + '/'
    if not pupilReg:
	dd = os.listdir(dir)
	dd.sort()
        pupilReg = dd[-1]

    # Link the new GUI tables
    cur_link = calib.CurLUTlink( 'ccd39')
    try:
        os.unlink(cur_link)
    except:
        pass
    os.symlink( dir + pupilReg, cur_link)


    # Get the real number of subapertures - as a workaround, read the cur_indpup file
    try:
    	real_subaps = len(file( calib.CurIndpup('ccd39')).readlines())/ 4
        #if real_subaps % 2:
        #    real_subaps -=1
    except IOError, e:
        real_subaps = 1

    # Always 800 total subaps..
    n_subaps=800

    # Reset WFS
    aoapp.sc.set_nsubap( n_subaps)
    aoapp.sc.set_pixellut( dir + pupilReg + '/bcuLUT')


    # Reconfig CCD with new binning - also causes the GUI to reload the tables

    print 'Configuring CCD, please wait...'
    aoapp.ccd39.reprogram( binning, binning, speeds[binning], HOgain)

    # --------------- Detailed slopecomp configuration starts
    aoapp.sc.setfluxgain( real_subaps)

    gains = numarray.zeros(6400, numarray.Float32)
    gains[0:real_subaps*4] = 1.0
    gainfile = 'all_one.fits'
    fits_lib.writeSimpleFits( calib.gainsDir()+'/'+gainfile, gains, {}, overwrite=True)
    aoapp.sc.setpixgain(gainfile)


    # Time history offsets
    #aoapp.sc.set_thoffsets('../config/slopes/modo_tilt48.fits')

    # Restart slope computer
    aoapp.sc.start()

    # Restart ccd at 200Hz
    aoapp.ccd39.set_framerate(197)
    aoapp.ccd39.start()

    time.sleep(0.1)
    aoapp.tt.set(amp=0, freq=100)

    # Restart pupil check activity
    processControl.startProcessByName( 'pupilcheck39')

    # Restart antidrift if it was on
    setAntiDrift(aoapp, antiDriftStatus)

    
    aoapp.log('ccd39 change binning done', aoapp.LOG_LEV_INFO)

def updateHOpupil( aoapp,             #@P{aoapp} Calling AOApp object
                  binning,           #@P{binning} binning (integer, 1-5 inclusive)
                  pupilReg = None):   #@P{pupilReg} Pupil registration name (string, None means the last available)
    '''
    Quick update of HO pupil
    '''

    if not pupilReg:
        return

    # Initialize objects
    setupDevices.loadDevices( aoapp, ['sc', 'ccd39', 'tt'], check=True)

    # Wait for slope computer init (we may be called just after the turnon)
    aoapp.sc.waitActive( timeout=10)

    # Stop everything just to be sure
    aoapp.sc.stop()
    aoapp.ccd39.stop()

    print "Setting zero modulation, please wait..."
    aoapp.tt.set(amp=0)
    time.sleep(10)

    # Get a new LUT
    dir = calib.LUTsDir( 'ccd39', binning) + '/'

    # Link the new GUI tables
    cur_link = calib.CurLUTlink( 'ccd39')
    try:
        os.unlink(cur_link)
    except:
        pass
    os.symlink( dir + pupilReg, cur_link)


    # Get the real number of subapertures - as a workaround, read the cur_indpup file
    try:
    	real_subaps = len(file( calib.CurIndpup('ccd39')).readlines())/ 4
        #if real_subaps % 2:
        #    real_subaps -=1
    except IOError, e:
        real_subaps = 1

    # Always 800 total subaps..
    n_subaps=800

    # Reset WFS
    aoapp.sc.set_nsubap( n_subaps)
    aoapp.sc.set_pixellut( dir + pupilReg + '/bcuLUT')

    # --------------- Detailed slopecomp configuration starts
    aoapp.sc.setfluxgain( real_subaps)

    gains = numarray.zeros(6400, numarray.Float32)
    gains[0:real_subaps*4] = 1.0
    gainfile = 'all_one.fits'
    fits_lib.writeSimpleFits( calib.gainsDir()+'/'+gainfile, gains, {}, overwrite=True)
    aoapp.sc.setpixgain(gainfile)

    # Restart slope computer
    aoapp.sc.start()
    aoapp.ccd39.start()

    aoapp.log('SC pupil update done', aoapp.LOG_LEV_INFO)

#@Procedure: setTVbinning
#
# Sets the TV binning to a new value.
# Since the BCU47 is resetted during the binning change, tip-tilt and cameralens positions
# are saved, if possible, and re-set again when the BCU is up.
#@

def setTVbinning( aoapp,            #@P{aoapp}: calling AOApp object
                  binning,          #@P{binning}: New binning (integer, 1,2,4 or 16)
                  force=False,      #@P{force}: if False, avoid reconfiguring CCD and BCU if things are already at the right binning.
                  speed=None):      #@P{speed}: if not None, use this readout speed instead of the default one 
    '''
    Sets the Technical viewer binning.
    '''
    print "********************************************"
    print "****        MagAO setTVbinning          ****"
    print "*                                          *"
    print "********************************************"

    #############################
    # Configuration area

    # Subframe
    #speeds = { 1:2500, 2:2500, 3:2500, 4:2500, 16:2500 }

    # Binned
    speeds = { 1:2500, 2:80, 16:80 }

    ## End of configuration area
    ###############################


    ## Override readout speed with argument
    #if speed:
	#for k in speeds.keys():
            #speeds[k]=speed

    #setupDevices.loadDevices( aoapp, ['ccd47', 'tv'], check=True)

    #if not force:
        #if (aoapp.ccd47.xbin() == binning) and (aoapp.ccd47.speed() == speeds[binning]):
            #return

   ##We have not BCU 47
   
    ## Save tt and cameralens status, if available
    #doTT = False
    ##if hasattr(aoapp,'tt'):
        ##if aoapp.tt.isOnline():
            ##doTT = True
            ##tt_offx = aoapp.tt.offx()
            ##tt_offy = aoapp.tt.offy()
            ##tt_amp = aoapp.tt.amp()
            ##tt_freq = aoapp.tt.freq()
    #doCameralens = False
    ##if hasattr(aoapp,'lens'):
        ##if aoapp.lens.isOnline():
            ##doCameralens = True
            ##cameralens_posx = aoapp.lens.posx()
            ##cameralens_posy = aoapp.lens.posy()


    ## Stop everything
    ##this is handled by VisAO
    ##aoapp.ccd47.stop()
    #aoapp.tv.stop()

   ## Reset BCU47
    #if 1:
        ##***** No BCU 47 to reset in MagAO system ********#
        ##aoapp.power.turnon('prog47')
        ##aoapp.power.turnon('reset47')
        ##time.sleep(0.5) # Probably not needed
        ##aoapp.power.turnoff('reset47')

        #print "Waiting for restart"

        ##aoapp.tv.waitStatus( AOConstants.AOStatesCode['STATE_NOCONNECTION'], waitTimeout=20)
        #aoapp.tv.waitStatus( AOConstants.AOStatesCode['STATE_READY'], waitTimeout=20)

        #print "BCU47 OK"


    ## Get a new LUT
    #dir = calib.LUTsDir( 'ccd47', binning) 

    ## Link the new GUI tables
    #cur_link = calib.CurLUTlink( 'ccd47')
    #if os.access(cur_link, os.F_OK):
        #os.unlink(cur_link)
    #os.symlink( dir, cur_link)

    #n_pixels = 1024*1024 / (binning*binning)

    ## Reset WFS
    #aoapp.tv.set_npixels( n_pixels)
    #aoapp.tv.set_pixellut( calib.CurBcuLUT( 'ccd47'))


    ## Reconfig CCD with new binning - also causes the GUI to reload the tables
    ##if aoapp.ccd47.getStatus() == AOConstants.AOStatesCode['STATE_READY']:
    #print "Attempting reprogram\n"
    aoapp.ccd47.reprogram( binning, binning, speeds[binning])
    ##else:
    ##    raise AdOptError( errstr = "CCD47 NOT READY")

    ## Restart frame grabber and ccd
    #print "Attempting tv restart\n"
    #aoapp.tv.start()
    #print "Attempting ccd47 restart\n"
    ##aoapp.ccd47.start()

    ## Restore TT and Cameralens
    ##if doTT:
	##aoapp.tt.set( amp = tt_amp, freq = tt_freq, offx = tt_offx, offy = tt_offy) 
   ## if doCameralens:
	##aoapp.lens.moveTo( cameralens_posx, cameralens_posy)
	

    #aoapp.log('ccd47 change binning done', aoapp.LOG_LEV_INFO)

#@Procedure: setTVbinning
#
# Synchronizes the TV binning to a new value set by another system (VisAO).
#@

def synchTVbinning( aoapp):            

    '''
    Synchronizes the Master Diagnostic with the Technical viewer binning.
    '''
    print "********************************************"
    print "**** On MagAO This Script Does Nothing ****"
    print "*                                         *"
    print "*      It is all done by techviewctrl     *"
    print "********************************************"    
    
    #binning = aoapp.ccd47.xbin()
    #speed = aoapp.ccd47.speed()
    #window = aoapp.ccd47.dx()
        
    #setupDevices.loadDevices( aoapp, ['ccd47', 'tv'], check=True)

    ## Get a new LUT

    #if window != 1024 and binning ==1:
      #binning = 1024/window
      #print "new binning" 
      #print binning
 
      
    #dir = calib.LUTsDir( 'ccd47', binning) 

    ## Link the new GUI tables
    #cur_link = calib.CurLUTlink( 'ccd47')
    #if os.access(cur_link, os.F_OK):
        #os.unlink(cur_link)
    #os.symlink( dir, cur_link)

    #n_pixels = 1024*1024 / (binning*binning)

    ## Reset WFS - these are the clue.
    ##aoapp.tv.set_npixels( n_pixels)
    ##aoapp.tv.set_pixellut( calib.CurBcuLUT( 'ccd47'))

    #aoapp.log('ccd47 synch binning done', aoapp.LOG_LEV_INFO)
    
def test(app):
    print 'ciao'
    return ["RETRY", "Test OK"]


def getSetupFits(app, movements = None, devices = None):
    '''
    Returns a tuple with:
    - a dictionary containing the current system setup,
    - an error string with any serious error.
    Warnings about data that cannot be saved are logged but not returned into the warning string.

    The dictionary is suitable to be saved into a FITS file.

    <movements> and <devices> are lists of devices to be saved. If not specified,
    a default list (the complete system) will be used.
    '''

    setupDevices.loadDevices( app, ['power','fw1','fw2','adc','ccd39','ccd47','rerot','cuberot','cubestage','stagex','stagey','stagez','tt','lamp','lens','temps'], check=False)

    if movements == None:
        movements = ['fw1', 'fw2', 'rerot', 'cuberot', 'cubestage', 'stagex', 'stagey', 'stagez']

    if devices == None:
        devices = ['lamp', 'ccd39', 'tt', 'cameralens', 'sc', 'ccd47', 'irtc', 'pisces', 'pupils', 'temps']


    hdr = {}
    errstr = ''
    invalid = '-9999'
    status_invalid = 'UNKNOWN'

    hdr['W_UNIT'] = cfg.subsystem_name

    for movement in movements:
        pos = invalid
        status = status_invalid
        try:
            status = AOConstants.AOStatesType[ app.__dict__[movement].getStatus()]
            pos = '%5.3f' % app.__dict__[movement].getPos()
        except AdOptError, e:
            errstr += ('Could not save %s status. Reason:\n' % movement) +str(e) + '\n'
        except (KeyError, AttributeError), e:
            errstr += 'Could not save %s status. Reason: software not started\n' % movement

        hdr[movement+'.STATUS'] = status
        hdr[movement+'.POSITION'] = pos

    if 'lamp' in devices:
        status = status_invalid
        intensity=invalid
        try:
            status = AOConstants.AOStatesType[ app.lamp.getStatus() ]
            intensity = app.lamp.getIntensity()
        except AdOptError, e:
            errstr += 'Could not save lamp status. Reason:\n' +str(e) + '\n'
        except AttributeError, e:
            errstr += 'Could not save lamp status. Reason: software not started\n'
        hdr['lamp.STATUS'] = status
        hdr['lamp.INTENSITY'] = intensity

    if 'ccd39' in devices:
        status = status_invalid
        framerate = invalid
        speed = invalid
        binning = invalid
        dark = 'UNKNOWN'
        try:
            status = AOConstants.AOStatesType[ app.ccd39.getStatus() ]
            framerate = '%5.2f' % app.ccd39.get_framerate()
            speed = app.ccd39.speed()
            binning = app.ccd39.xbin()
            dark = app.sc.get_dark()

        except AdOptError, e:
            errstr += 'Could not save ccd39 status. Reason:\n' +str(e) + '\n'
        except AttributeError, e:
            errstr += 'Could not save ccd39 status. Reason: software not started\n'

        hdr['ccd39.STATUS'] = status
        hdr['ccd39.FRAMERATE'] = framerate
        hdr['ccd39.READOUT_SPEED'] = speed
        hdr['ccd39.BINNING'] = binning
        hdr['ccd39.DARK_FILENAME'] = dark

    if 'ccd47' in devices:
        status = status_invalid
        framerate = invalid
        speed = invalid
        binning = invalid
        dark = 'UNKNOWN'
        try:
            status = AOConstants.AOStatesType[ app.ccd47.getStatus() ]
            framerate = '%5.2f' % app.ccd47.get_framerate()
            speed = app.ccd47.speed()
            binning = app.ccd47.xbin()
            dark = app.ccd47.curDark()

        except AdOptError, e:
            errstr += 'Could not save ccd47 status. Reason:\n' +str(e) + '\n'
        except AttributeError, e:
            errstr += 'Could not save ccd47 status. Reason: software not started\n'

        hdr['ccd47.STATUS'] = status
        hdr['ccd47.FRAMERATE'] = framerate
        hdr['ccd47.READOUT_SPEED'] = speed
        hdr['ccd47.BINNING'] = binning
        hdr['ccd47.DARK_FILENAME'] = dark

    if 'irtc' in devices:
        dark = 'UNKNOWN'
        try:
            dark = app.irtc.curDark()

        except AdOptError, e:
            errstr += 'Could not save IRTC status. Reason:\n' +str(e) + '\n'
        except AttributeError, e:
            errstr += 'Could not save IRTC status. Reason: software not started\n'

        hdr['IRTC.DARK_FILENAME'] = os.path.basename(dark)

    if 'pisces' in devices:
        dark = 'UNKNOWN'
        try:
            dark = app.pisces.curDark()

        except AdOptError, e:
            errstr += 'Could not save PISCES status. Reason:\n' +str(e) + '\n'
        except AttributeError, e:
            errstr += 'Could not save PISCES status. Reason: software not started\n'

        hdr['PISCES.DARK_FILENAME'] = os.path.basename(dark)

    if 'tt' in devices:
        status = status_invalid
        amp = invalid
        freq = invalid
        offx = invalid
        offy = invalid
        amp_lambdaD = invalid

        try:
            status = AOConstants.AOStatesType[ app.tt.getStatus() ]
            amp = app.tt.amp()
            freq = app.tt.freq()
            offx = app.tt.offx()
            offy = app.tt.offy()
        except AdOptError, e:
            errstr += 'Could not save tt status. Reason:\n' +str(e) + '\n'
        except AttributeError, e:
            errstr += 'Could not save tt status. Reason: software not started\n'

        try:
            frm = int(app.ccd39.get_framerate())    # To avoid out-of-range errors in case of 1000.5 Hz and similar
            amp_lambdaD = app.tt.modulationVF2Psf( frm, app.tt.amp())
        except AdOptError, e:
            errstr += 'Could not save lambda/D modulation. Reason:\n' +str(e) + '\n'
        except AttributeError, e:
            errstr += 'Could not save lambda/D modulation. Reason: software not started\n'
        except ValueError:
            pass
        
        hdr['tt.STATUS'] = status
        hdr['tt.AMPLITUDE'] = amp
        hdr['tt.FREQUENCY'] = freq
        hdr['tt.OFFSET_X'] = offx
        hdr['tt.OFFSET_Y'] = offy
        hdr['tt.LAMBDA_D'] = amp_lambdaD

    if 'adc' in devices:
        status = status_invalid
        pos1 = invalid
        pos2 = invalid
        axis = invalid
        angle = invalid
        try: 
            status = AOConstants.AOStatesType[ app.adc.getStatus() ]
            pos1 = app.adc._motor1.getPos()
            pos2 = app.adc._motor2.getPos()
            axis = (pos1+pos2)/2.0
            angle = pos1 / axis

            pos1 = float('%5.2f' % pos1)
            pos2 = float('%5.2f' % pos2)
            axis = float('%5.2f' % axis)
            angle = float('%5.2f' % angle)
        except AdOptError, e:
            errstr += 'Could not save adc status. Reason:\n' +str(e) + '\n'
        except AttributeError, e:
            errstr += 'Could not save adc status. Reason: software not started\n'

        hdr['adc.STATUS'] = status
        hdr['adc.POS_1'] = pos1
        hdr['adc.POS_2'] = pos2
        hdr['adc.DISP_AXIS'] = axis
        hdr['adc.DISP_ANGLE'] = angle


    if 'cameralens' in devices:
        status = status_invalid
        posx = invalid
        posy = invalid
        try:
            status = AOConstants.AOStatesType[ app.lens.getStatus() ]
            posx = float('%5.2f' % (app.lens.posx()*1e6))
            posy = float('%5.2f' % (app.lens.posy()*1e6))
        except AdOptError, e:
            errstr += 'Could not save camera lens status. Reason:\n' +str(e) + '\n'
        except AttributeError, e:
            errstr += 'Could not save camera lens status. Reason: software not started\n'

        hdr['lens.STATUS'] = status
        hdr['lens.POSITION_X'] = posx
        hdr['lens.POSITION_Y'] = posy

    if 'sc' in devices:
        status = status_invalid
        pupils = ''
        slopenull = ''
        decimation= invalid
        loopclosed = invalid
        disturbance = invalid
        try:
            status = AOConstants.AOStatesType[ app.sc.getStatus() ]
            pupils = removePrefix( app.sc.get_pixellut(), calib.LUTsDir('ccd39'))[:-7]
            slopenull = app.sc.get_slopenull()
            decimation = app.sc.get_masterd_decimation()
            loopclosed = app.sc.get_fl_status()
            disturbance = app.sc.get_disturb_status()
        except AdOptError, e:
            errstr += 'Could not save slope computer status. Reason:\n' +str(e) + '\n'
        except (AttributeError, TypeError), e:
            errstr += 'Could not save slope computer information. Reason: software not started\n'

        hdr['sc.STATUS'] = status
        hdr['sc.PUPILS'] = pupils
        hdr['sc.SLOPENULL'] = slopenull
        hdr['sc.DECIMATION'] = decimation
        hdr['sc.LOOPCLOSED'] = loopclosed
        hdr['sc.DISTURBANCE'] = disturbance

    if 'pupils' in devices:
        pupils = []
        for i in range(4):
            p = frames_lib.pupil()
            p.diameter = invalid
            p.cx = invalid
            p.cy = invalid
            p.side = invalid
            p.diffx = invalid
            p.diffy = invalid
        try:
           pupils = frames_lib.getCurPupilsFromRTDB(app)
        except AdOptError, e:
            errstr += 'Could not save pupils information. Reason:\n' +str(e) + '\n'

        i=0
        for p in pupils:
            hdr['pup%d.DIAMETER' %i] = '%5.2f' % p.diameter
            hdr['pup%d.CX' %i]       = '%5.2f' % p.cx
            hdr['pup%d.CY' %i]       = '%5.2f' % p.cy
            hdr['pup%d.SIDE' %i]     = '%5.2f' % p.side
            hdr['pup%d.DIFFX' %i]    = '%5.2f' % p.diffx
            hdr['pup%d.DIFFY' %i]    = '%5.2f' % p.diffy
            i+=1

    if 'temps' in devices:
        try:
            t = app.temps.temps()
            u = app.temps.units()
            for k in u.keys():
                u[k] = filter(lambda x: x in string.printable, u[k])
            i=0
            for k in t.keys():
                hdr['TEMP%d.NAME'%i] = '%s' % k
                hdr['TEMP%d.VALUE'%i] = '%5.1f' % t[k]
                hdr['TEMP%d.UNIT'%i] = '%s' % u[k]
                i += 1
        except:
            pass


    # Get information from telescope, if any available
    try:
        exclude = ['ERRMSG', 'STATUS', 'CLSTAT'] 
        telescope = app.ReadVarList('AOS*@M_ADSEC')   # We don't use cfg.varname(), because it adds the side
        now = time.time()
        timeout = 120 # Max accepted age in seconds
        for var in telescope:
            name = var.Name().replace('AOS.'+cfg.side+'.', '').replace('AOS.', '').replace('@M_ADSEC', '')
            if not name in exclude:
                if type(var.Value()) != types.TupleType:
                    if now-var.MTime() <= timeout:
                        hdr['tel.'+name] = str(var.Value())
                    else:
                        hdr['tel.'+name] = invalid
                else:
                    for n in range(len(var.Value())):
                        if now-var.MTime() <= timeout:
                            hdr['tel.'+name+('%d'%n)] = str(var.Value()[n])
                        else:
                            hdr['tel.'+name+('%d'%n)] = invalid

    except AdOptError, e:
        errstr += 'Error reading telescope status: '+str(e)+'\n'
    except ValueError, e:
        errstr += 'Error reading telescope status: '+str(e)+'\n'
    if len(telescope)==0:
        errstr += 'No telescope information available\n'


    app.log('getSetupFits(): fits header created', app.LOG_LEV_DEBUG)
    if errstr != '':
        pass
        #app.log(errstr, app.LOG_LEV_DEBUG)

    return (hdr, '')

def removePrefix( str, prefix = ''):
    if str[0:len(prefix)] == prefix:
        return str[len(prefix):]

#@Procedure: TurnSystemOff
#
# Turns the WFS system safely off.
#@

def TurnSystemOff( app, sensor, instr):

    app.log('Turning WFS off', app.LOG_LEV_INFO)

    powerOffStages=True

    setupDevices.loadDevices( app, ['stagex', 'stagey', 'stagez','tt'], check = True)

    # Brake stages before removing power
    if powerOffStages:
        try:
            app.stagex.enable(False)
            app.stagey.enable(False)
            app.stagez.enable(False)
            time.sleep(4)		# Brake has a delay of 2 seconds.
        except:
        # Errors may happen if the stages weren't already on. We ignore these kind of errors.
            pass


    # -- LBTI stops here, FLAO goes on
    if sensor == AO_WFS_LBTI:
        return

    # Shut down tip-tilt
    app.tt.mirrorSet(False)

    setupDevices.loadDevices( app, ['power'], check = True)

    exclude = ['fans_LJ', 'boxfans', 'main', 'power', 'flowerpot', 'ps1', 'lamp', 'cuberot', 'cubestage','visaoswitch','visaofan1','visaofan2','visaocomp','visaots8','cubestage','cuberot']
    if not powerOffStages:
        exclude.append('xstage')
        exclude.append('ystage')
        exclude.append('zstage')

    for device in app.power.list():
        if not device in exclude:
            try:
                app.power.turnoff(device, force = True)
            except:
                # Exceptions here are not fatal since we can turn off the main power afterwards
                pass

    if 'fans_LJ' in app.power.list():
        app.power.turnoff('fans_LJ', force = True)
    if 'flowerpot' in app.power.list():
        app.power.turnoff('flowerpot', force = True)

    # W#1
    if 'main' in app.power.list():
    	app.power.turnoff('main', force = True)

    # W2
    if 'ps1' in app.power.list():
    	app.power.turnoff('ps1', force = True)

    if 'boxfans' in app.power.list():
        app.power.turnoff('boxfans', force = True)
    app.log('All devices turned off !', app.LOG_LEV_INFO)



#@Procedure: TurnSystemOn
#
# Turns the system on from a complete shutdown or cold start.
#
# If the system was already on, most of the instructions
# (not all, especially the CCDs and BCU47) are actually NOPs.
#@

def TurnSystemOn(app,     #@P{app}: calling aoapp object
                 sensor,  #@P{sensor}: sensor name (string)
                 instr,   #@P{instr}: instrument name (string)
                 config,  #@P{config}: requested Operate configuration (string)
                 boardSetup, #P{boardSetup}: requested board setup (string, optional, use empty string if none needed)
                 opticalSetup): #P{opticalSetup}: perform optical setup (bool)

    startTime = time.time()

    app.log('Setting configuration '+config+' for sensor '+sensor, app.LOG_LEV_INFO)

    # -- FLAO needs the poweron sequence, LBTI does not

    if sensor == AO_WFS_FLAO or sensor == AO_WFS_MAG:
        try:
            devices = cfg.wfs_configs[ config]
        except KeyError, e:
            raise AdOptError( errstr = 'Unknown configuration string: "%s"' % config)

        app.log('Turning WFS on', app.LOG_LEV_INFO)
        app.log("Devices: "+ str(devices), app.LOG_LEV_DEBUG)

        app.arbMsg('WFS: starting power on sequence')

        try:

            # Setup needed modules
            app.log('Initializing Python modules...', app.LOG_LEV_DEBUG)

            app.log(str(devices))

            setupDevices.loadDevices(app)

            app.log('Waiting for network connection', app.LOG_LEV_INFO)


            setup = setupDevices.get()
            faults, errstr = setup.setup( app, devices)

            if 'ccd47' in devices:
                # Start regular ccd47 image
                app.log('Starting ccd47 image thread')
                app.ccd47Timer = threading.Timer( 1, ccd47TimerHandler, args=[app, sensor, instr])
                app.ccd47Timer.start()

        except AdOptError, e:
            print 'Exception: '+e.errstr
            app.log( "Exception: " + e.errstr, app.LOG_LEV_ERROR)
            raise

        # Done
        if len(faults) > 0:
            s = 'Errors for these devices: '+str(faults)
            s += '\n\n'+errstr
            app.log(s, app.LOG_LEV_ERROR)
            ret = s
        else:
            app.log('Ready', app.LOG_LEV_INFO)
            ret = None


        endTime = time.time()
        app.log('Elapsed time: %5.2f seconds' % (endTime - startTime), app.LOG_LEV_INFO)

    # -- This part is common to all sensors

    if boardSetup != "":
        try:
            from AdOpt import boardSetup as boardSetupModule
            boardSetupFile = calib.setupsDir() + boardSetup
            boardSetupModule.load( app, boardSetupFile)
        except Exception, e:
            app.log('Error applying board setup', app.LOG_LEV_ERROR)
            app.log(str(e), app.LOG_LEV_ERROR)

    if opticalSetup:
        try:
            towerOpticalSetup(app)
        except Exception, e:
            app.log('Error applying optical setup', app.LOG_LEV_ERROR)
            app.log(str(e), app.LOG_LEV_ERROR)

    # Remember stage position for absolute offsets   
    initStagePos(app)

    return ret

#@Function: ccd47TimerHandler
#
# Handles the regular ccd47 image download and set into the RTDB
#@

def ccd47TimerHandler(app, sensor, instr):

    try:
        doGetTVSnap(app, sensor, instr, saveInRTDB=True)
        time.sleep(1)
    except:
        pass

    # Star the timer again
    app.ccd47Timer = threading.Timer( 1, ccd47TimerHandler, args=[app, sensor, instr])
    app.ccd47Timer.start()


#@Function: packCcd47Image
#
# Packs a ccd47 image into a string, suitable to be saved into the RTDB.
#@

def packCcd47Image(image):

    # Pack the TV image in a string, rescaled to 0-255 and 256x256.
    import struct
    fmt = 'ii65536s'
    
    str = abs((frames_lib.rebin(image, 256, 256) /64)).astype('UInt8').tostring()
    return struct.pack(fmt, 256, 256, str)


#@Procedure: doPrepareAcquireRef
#
#  Prepare for reference start acquisition
#
# In preparation for star acquisition, stages are moved
# to the expected star position in the FoV, filters and other
# movements are set to be ready when telescope resumes tracking,
# and CCDs are setup at the right exposure time and binning
# to perform the star acquisition.
#
# Depending on the parameters, this function may block for a significant
# amount of time (up to 60 sec).
#
#@


def doPrepareAcquireRef(app, sensor='',        #@P{sensor}: sensor name, for example AO_WFS_FLAO or AO_WFS_LBTI
                             instr='',         #@P{instr}: instrument, for example 'IRTC' or 'LUCIFER')
                             mode='',          #@P{mode}: Clio observing mode (narrow or wide field)
                             starxpos=0.0,     #@P{starxpos}: reference star position X pos in focal plane mm from center of field
                             starypos=0.0,     #@P{starypos}: reference star position Y pos in focal plane mm from center of field
                             telElevation=0.0, #@P{telElevation}: predicted telescope elevation in degrees
                             derotAngle=0.0,   #@P{derotAngle}: predicted derotator angle in degrees
                             starMag=0.0,      #@P{starMag}: expected star magnitude
                             starColor=0.0,    #@P{starColor}: expected star B-V color.
                             r0= 0.0,          #@P{r0}: r0 value in mm
                             v0= 0.0):         #@P{v0}: wind speed in m/s

    # Some arguments may be None
    if telElevation == None:
        telElevation = 0.0
    if derotAngle == None:
        derotAngle = 0.0
    if starColor == None:
        starColor = 0.0

    # Magnitude is mandatory
    if starMag == None:
        raise AORetryException( code = AOConstants.AOErrCode['WFSARB_ARG_ERROR'], errstr = 'Star magnitude is required.')

    # 20140403 (AP) Removed ccd47 from PresetAO sequence
    setupDevices.loadDevices( app, ['stagex', 'stagey', 'stagez', 'fw1', 'fw2', 'ccd39', 'lens', 'power', 'gimbal'], check=True)

    baseinstr, bs = instr.split('_')

    if bs=='bs5050':
        fw1pos = 0
    elif bs=='bsWindow':
        fw1pos = 1
    elif bs=='bs800':
        fw1pos = 4
    elif bs=='bs950': 
        fw1pos = 5

    app._acquireRefSettings = computeAOsettings( app, baseinstr, starMag, fw1pos, starColor, r0, v0)
    freq = app._acquireRefSettings.CLfreq   # Set immediately the pupil centering framerate

    # Fix also FW1 position for observation
    app._acquireRefSettings.fw1_obs = fw1pos

    fov = calib.readFov( allowFail = False)
    try:
       fov_xc = fov['xc_%s' % mode]
       fov_yc = fov['yc_%s' % mode]
    except KeyError:
        str = 'FoV calibration file does not contain position for Clio mode %s' % mode
        raise AORetryException( code = AOConstants.AOErrCode['VALUE_OUT_OF_RANGE_ERROR'], errstr = str)

    if labmode(app):
        app.log('LABMODE: star position ignored (was %3.1f,%3.1f)' % (starxpos, starypos))
        starxpos =0
        starypos =0

    # Check that star position is inside FoV
    xpos = fov_xc + starxpos
    ypos = fov_yc + starypos

    if (xpos < fov['xmin']) or (xpos > fov['xmax']) or (ypos < fov['ymin']) or (ypos > fov['ymax']):
        str = 'Star position outside limits: position %3.1f,%3.1f  Limits: from %3.1f,%3.1f to %3.1f,%3.1f' % (starxpos, starypos, fov['xmin']-fov_xc,  fov['ymin']-fov_yc, fov['xmax']-fov_xc, fov['ymax']-fov_yc)
        raise AORetryException( code = AOConstants.AOErrCode['VALUE_OUT_OF_RANGE_ERROR'], errstr = str)


    if 0:
        return '%7.3f %d %7.3f %d %d %7.3f %s %s %s %s %d' % (app._acquireRefSettings.HOfreq,
                                     app._acquireRefSettings.HObin,
                                     app._acquireRefSettings.TTamp,
                                     app._acquireRefSettings.fw1_obs,
                                     app._acquireRefSettings.fw2_obs,
                                     app._acquireRefSettings.gain,
                                     app._acquireRefSettings.pupils,
                                     app._acquireRefSettings.base,
                                     app._acquireRefSettings.rec,
                                     app._acquireRefSettings.filtering,
                                     app._acquireRefSettings.nModes)

    # Enable the stages in case the following board setup has a stage movement command
    # (it would fail otherwise)
    app.stagex.enable( True, onaftermove=False)#True)
    app.stagey.enable( True, onaftermove=False)#True)
    app.stagez.enable( True, onaftermove=True)#True)

    app.arbMsg('WFS: loading setup for '+instr)

    # Load the board setup for this instrument
    try:
        setupFile = calib.setupsDir() + instr
        boardSetup.load( app, setupFile)
    except IOError, e:
       raise  AORetryException( code = AOConstants.AOErrCode['WFSARB_ARG_ERROR'], errstr = 'No setup for instrument %s: %s' % (instr, e.__str__()))

    # Move stages and FW#2 to target position
    moveXYZ( app, xpos, ypos, None, zCompensation = True, enableAfter = True, block=False)

    app.arbMsg('WFS: configuring CCDs')

    # Configure CCDs and acquire darks in parallel

    decimation= computeDecimation( freq)
    doSetLoopParams( app, sensor, instr, freq, decimation,  app._acquireRefSettings.HObin,  app._acquireRefSettings.TTamp, takeDark=False, pupilReg=app._acquireRefSettings.pupils, HOgain=app._acquireRefSettings.HOgain)

    if sensor == AO_WFS_FLAO:
        app.fw1.waitTargetReached()

    app.arbMsg('WFS: acquiring CCD darks')

    calibHODarkInternal(app, sensor, instr, num=1000, resumeFW=True)
    selectSlopenull(app, app._acquireRefSettings.slopenull, matchLUT=False)

    # Set FW1 to acquire position
    app.fw1.moveTo( app._acquireRefSettings.fw1_acq)
    app.fw1.waitTargetReached()

    # Wait for movement completion - blocks until everything is ready
    app.stagex.waitTargetReached()
    app.stagey.waitTargetReached()

    app.arbMsg('WFS: preset done')

    # Return an ASCII string with output parameters.
    return '%7.3f %d %7.3f %d %d %7.3f %s %s %s %s %d %7.3f %7.3f' % (app._acquireRefSettings.HOfreq,
                                     app._acquireRefSettings.HObin,
                                     app._acquireRefSettings.TTamp,
                                     app._acquireRefSettings.fw1_obs,
                                     app._acquireRefSettings.fw2_obs,
                                     app._acquireRefSettings.gain,
                                     app._acquireRefSettings.pupils,
                                     app._acquireRefSettings.base,
                                     app._acquireRefSettings.rec,
                                     app._acquireRefSettings.filtering,
                                     app._acquireRefSettings.nModes,
                                     app._acquireRefSettings.rangemin,
                                     app._acquireRefSettings.rangemax)
    

#@Function: computeDecimation
#
# Computes the needed decimation for the given CCD frequency.
#
# This computation is repeated in WfsArbitrator/WfsInterface.cpp
#@

def computeDecimation(freq):
    try:
        master_speed = cfg.cfg['wfsarb']['MaxMasterSpeed'].Value()
    except:
        master_speed = 100
    return int(freq/master_speed)

#@Function: computeAOsettings
#
# Computes the AO parameters based on the reference star characteristics.
#
# Returns an object with the following attributes:
#    .fw1_acq : position of fw1 (acquisition)
#    .fw2_acq : position of fw2 (acquisition)
#    .fw1_obs : position of fw1 (observation)
#    .fw2_obs : position of fw2 (observation)
#    .TVbin    : required TV binning (acquisition)
#    .TVfreq   : required TV frequency (acquisition)
#    .TVavg    : no. of TV frames to average (acquisition)
#    .HObin    : required HO binning (observation)
#    .HOfreq   : required HO frequency (observation)
#    .TTamp    : required tip-tilt modulation amplitude (observation)
#    .nModes   : no. of modes that will be corrected (observation)
#    .HOflux   : expected flux on ccd39 after acquisition
#    .base     : modal basis
#    .rec      : reconstructor to use
#    .filtering: temporal filtering to use
#    .pupils   : pupil registration to use
#    .slopenull: slopenull file to use
#    .gain     : loop gain
#    .seeing   : seeing estimated from r0
#
#    plus the five input parameters of this method, as additional members (settings.mode, etc)
#
#@

def computeAOsettings( app,
                       instr,      #@P{instr}: Instrument ('IRTC', 'LUCIFER')
                       mag,        #@P{flux}:  star nagnitude (given or measured)
                       fw1pos,     #@P{fw1pos}: beamsplitter position
                       starColor,  #@P{starColor}: reference star B-V color
                       r0= 0.0,    #@P{r0}: r0 value in mm
                       v0= 0.0):   #@P{v0}: wind speed in m/s
                                   #@R: settings object (see function header)


    instr = str(instr)

    setupDevices.loadDevices( app, ['ccd39'], check=True)

    flux = mag2Flux(mag, fw1correction( app, fw1pos))

    tablefile = calib.getAOParamsFile( instr)
    if not os.path.exists(tablefile):
       raise  AORetryException( code = AOConstants.AOErrCode['WFSARB_ARG_ERROR'], errstr = 'No settings for %s' % instr)

    app.log('computeAOsettings(): using table: %s' % tablefile, app.LOG_LEV_DEBUG)
    table = file( tablefile).readlines()

    # Load table lines and skip invalid lines
    tablelines = []
    for l in table:
        params = l.split()
        if len(params)<1:
            continue
        try:
            mag = float(params[0])
            tablelines.append(params)
        except ValueError:
            # Skip invalid lines
            continue

    # Sort tablelines by magnitude
    tablelines = sorted(tablelines, lambda a,b:cmp(float(a[0]), float(b[0])))

    # Get highest magnitude and see if we are in the range
    maxFlux = float(tablelines[0][1])
    maxMag  = float(tablelines[0][0])
    if flux > maxFlux:
	mag = flux2Mag(flux, fw1correction( app, fw1pos))
        raise AORetryException( code = AOConstants.AOErrCode['VALUE_OUT_OF_RANGE_ERROR'], errstr = 'Star too bright (estimated mag: %5.1f). Minimum magnitude is %5.1f' % (mag, maxMag))

    # Table has the following features:
    # - flux is always decreasing
    # - frequency is always decreasing within a binning
    # - lines that change the binning are duplicated with the same magnitude, and specify
    #   two sets of parameters, one for the lower binning and one for the higher binning

    for line in range(len(tablelines)-1):

	# Search for a couple of lines where we fall in between
	params1 = tablelines[line]
	params2 = tablelines[line+1]

	flux1 = float(params1[1])
	flux2 = float(params2[1])

	print 'Flux: ', flux, flux1, flux2
	# Skip lines with the same magnitude - it's a binning change
	if flux1 == flux2:
		continue

	# Skip couples where we don't fall in between
	if flux>=flux1 or flux<flux2:
		continue

        print 'selected'

	# Interplate frequency
 	freq1 = float(params1[3])
 	freq2 = float(params2[3])

	freq = float(flux-flux2) / (flux1-flux2) * (freq1-freq2) + freq2

        # Fix freq to be one of the accepted ones, allowing for only 10% upward.
        allowed_freq = [100,200,312,400,625,800,1000,1100,1200,1300,1400,1500,1600,1700,1800,1900,2000]
        if freq not in allowed_freq:
            ff = 1e9
            idx = -1
            for i in range(len(allowed_freq)):
                diff_freq = freq - allowed_freq[i] 
                if diff_freq >= (-allowed_freq[i]/10) and diff_freq < ff:
                   ff = diff_freq
                   idx = i
            freq = allowed_freq[idx]


        # Get the rest of the data from one of the lines - they will be equal

        class obj: pass

        app.log(params, app.LOG_LEV_DEBUG)
        pupils= {}

        settings = obj()
	settings.flux   = flux
        settings.HObin  = int(params2[2])
        settings.HOfreq = freq
        settings.HOgain = int(params2[4])
        settings.HOflux = float(params2[5])
        verygoodseeing_base   = params2[6]
        verygoodseeing_rec    = params2[7]
        goodseeing_base   = params2[8]
        goodseeing_rec    = params2[9]
        badseeing_base = params2[10]
        badseeing_rec  = params2[11]
        verybadseeing_base = params2[12]
        verybadseeing_rec  = params2[13]
        settings.filtering = params2[14]
        pupils[0]         = params2[15]
        pupils[1]         = params2[16]
        pupils[2]         = params2[17]
        pupils[3]         = params2[18]
        settings.slopenull = params2[19]
        settings.TTamp = float(params2[20])
        settings.gain = float(params2[21])
        settings.fw1_acq = int(params2[22])
        settings.fw2_acq = int(params2[23])
        settings.fw2_obs = int(params2[24])
        settings.TVbin = int(params2[25])
        settings.TVfreq = float(params2[26])
        settings.TVavg = int(params2[27])
        settings.TVflux = int(params2[28])
        settings.CLbase = params2[29]
        settings.CLrec  = params2[30]
        settings.CLfreq  = float(params2[31])

        # Get telescope rotator value
        try:
            rotator = app.ReadVar( cfg.cfg['wfsarb']['TelRotatorVar'].Value().replace('<side>',cfg.side)).Value()
        except:
            raise AORetryException( code = AOConstants.AOErrCode['VALUE_OUT_OF_RANGE_ERROR'], errstr = 'Cannot read telescope rotator position')

        # Scan the known pupil values until we found a match
        fov = calib.readFov( allowFail = False)
        idx=0
        while 1:
            try:
                app.log('pupil%dbin%d_min'%(idx, settings.HObin))
                app.log('pupil%dbin%d_max'%(idx, settings.HObin))
                rangemin = fov['pupil%dbin%d_min'%(idx, settings.HObin)]
                rangemax = fov['pupil%dbin%d_max'%(idx, settings.HObin)]
                app.log('%f %f'%(rangemin, rangemax))
            except KeyError:
                raise AORetryException( code = AOConstants.AOErrCode['VALUE_OUT_OF_RANGE_ERROR'], errstr = 'No pupil set for rototator position %3.1f degrees'%rotator)
	    app.log('Pupil idx: %d (%s) (min: %f - max: %f - rotator: %f' % (idx, pupils[idx], rangemin, rangemax, rotator), app.LOG_LEV_DEBUG)
            if (rotator >= rangemin) and (rotator <= rangemax):
                break
            idx+=1 

        try:
            settings.pupils = pupils[idx]
            settings.rangemin = rangemin
            settings.rangemax = rangemax

        except KeyError:
                raise AORetryException( code = AOConstants.AOErrCode['VALUE_OUT_OF_RANGE_ERROR'], errstr = 'Pupils no. %d defined in fov.txt but not found in AO table' %idx)

	app.log('Selected parameters:', app.LOG_LEV_DEBUG)

        if r0==0:
            settings.base = badseeing_base
            settings.rec  = badseeing_rec
	    app.log('Mode: bad seeing', app.LOG_LEV_DEBUG)
        elif r0==1:
            settings.base = goodseeing_base
            settings.rec  = goodseeing_rec
	    app.log('Mode: good seeing', app.LOG_LEV_DEBUG)
        elif r0==2:
            settings.base = verygoodseeing_base
            settings.rec  = verygoodseeing_rec
	    app.log('Mode: damn good seeing', app.LOG_LEV_DEBUG)
        elif r0==-1:
            settings.base = verybadseeing_base
            settings.rec  = verybadseeing_rec
	    app.log('Mode: very bad seeing', app.LOG_LEV_DEBUG)
        else:
            raise AORetryException( code = AOConstants.AOErrCode['VALUE_OUT_OF_RANGE_ERROR'], errstr = 'Bad seeing value %d (accepted range is 0-2)' % r0)

        settings.nModes = calib.nModes( calib.recFile( settings.base, calib.ensureExtension( settings.rec, '.fits')))

	app.log('ccd39 binning : %d' % settings.HObin, app.LOG_LEV_DEBUG)
	app.log('ccd39 speed   : %d Hz' % int(settings.HOfreq), app.LOG_LEV_DEBUG)
	app.log('ccd39 gain    : %s' %  app.ccd39.gain2Str( int(settings.HOgain)), app.LOG_LEV_DEBUG)
	app.log('No. of modes  : %d' % settings.nModes, app.LOG_LEV_DEBUG)
	app.log('Modal basis   : %s' % settings.base, app.LOG_LEV_DEBUG)
	app.log('REC matrix    : %s' % settings.rec, app.LOG_LEV_DEBUG)
	app.log('Pupils        : %s (rotator at %3.1f)' % (settings.pupils, rotator), app.LOG_LEV_DEBUG)
	app.log('TT amplitude: %3.2f mRad' % settings.TTamp, app.LOG_LEV_DEBUG)
	app.log('ccd47 binning : %d' % settings.TVbin, app.LOG_LEV_DEBUG)
	app.log('ccd47 speed   : %3.1f Hz' % settings.TVfreq, app.LOG_LEV_DEBUG)
	app.log('Selected mag: %3.1f' % flux2Mag(flux, fw1correction( app, fw1pos)), app.LOG_LEV_DEBUG)
	app.log('Flux        : %d' % flux, app.LOG_LEV_DEBUG)

        # Duplicate original parameters
        settings.instr    = instr
        settings.mode     = ''
        settings.starMag  = flux2Mag(flux, fw1correction( app, fw1pos))
        settings.starColor  = starColor
        settings.r0  = r0
        settings.v0  = v0

        if (r0 == 0) or (r0 == None):
            settings.seeing = 0.6
        else:
            settings.seeing = 251 * 0.8 / r0

        return settings

    # If we arrive here, not line was found for our magnitude!

    raise AORetryException( code = AOConstants.AOErrCode['VALUE_OUT_OF_RANGE_ERROR'], errstr = 'Star too faint. Limiting magnitude is %3.1f' % float(params2[0]))


#@Procedure: doCheckRef
#
# Checks the reference star positions, as it would be done by AcquireRef.
#
#@

def doCheckRef(app, sensor, instr):

    setupDevices.loadDevices( app, ['ccd47'], check=True)

    if 0:
        return '%7.3f %7.3f %7.3f' % (1, 2, 10)

    # Check star on ccd47
    acqRet = source_acq.acquire(app, TVavg = app._acquireRefSettings.TVavg, TVflux = app._acquireRefSettings.TVflux, getError = True)

    return '%7.3f %7.3f %7.3f' % (acqRet.deltaX, acqRet.deltaY, acqRet.mag)

#@Function: ccd39Flux
#
# Return the current ccd39 flux
#@

def ccd39Flux( app, frame=None, nframes=None):

    setupDevices.loadDevices( app, ['ccd39', 'fw1'], check=True)

    if not frame:
        if not nframes:
            nframes=500
        frame = app.ccd39.get_frames( nframes, average=True, useFloat=True)

    indpup = file( calib.CurIndpup('ccd39')).readlines()
    counts=0
    f= frame.ravel()
    for ind in indpup:
	counts += f[ int(ind)]
    subaps = len(indpup)/ 4

    # Check that CCD is not saturated
    acqData = calib.loadCalibData( calib.sourceAcqCalibFile(), check=['maxCounts'])
    if counts > len(indpup)*acqData['maxCounts']:
        raise AORetryException(errstr = 'Star too bright: ccd39 is saturated, AO loop cannot work.')

    # Total counts over the frame (assume background is good....)
    bin = app.ccd39.xbin()
    ccd_gain = 2.0    # counts/ photoelectron ratio
    flux = (counts/subaps) * app.ccd39.get_framerate() / (bin*bin) / ccd_gain

    # Correct for filterwheel #1 position
    flux *= fw1correction( app, round(app.fw1.getPos()))

    # Correct for gain setting
    gain = app.ccd39.get_gain()
    flux *= acqData['gain_%d'%gain]

    app.log('Total counts: %d - subaps: %d - framerate: %3.1f - Flux: %d' % (int(counts), int(subaps), app.ccd39.get_framerate(), int(flux))) 

    if flux <=0:
        raise AORetryException(errstr = 'Magnitude calculation cannot take place with negative counts')

    return flux

def fw1correction(app, fw1pos):
    acqData = calib.loadCalibData( calib.sourceAcqCalibFile(), check=['maxCounts'])
    return acqData['fw1_%d'%fw1pos]


def flux2Mag(flux, fw1correction=1.0):
    if fw1correction ==0:
        fw1correction =1e-6

    flux *= fw1correction
    acqData = calib.loadCalibData( calib.sourceAcqCalibFile(), check=['refMagFlux', 'refMag'])
    return 2.5*math.log10( float(acqData['refMagFlux']) / flux)+ acqData['refMag']

def mag2Flux(mag, fw1correction=1.0):
    acqData = calib.loadCalibData( calib.sourceAcqCalibFile(), check=['refMagFlux', 'refMag'])
    return math.pow(10, (acqData['refMag']-mag)/2.5) * float(acqData['refMagFlux']) / fw1correction

#@Procedure: setAntiDrift
#
# enable/disable antidrift. Set to 1 to enable and 0 to disable.
#@

def setAntiDrift(app, enable):
    varname = cfg.varname('optloopdiag', 'DRIFT.ENABLE.REQ')
    var = AOVar.AOVar( varname, tipo='INT_VARIABLE', value=enable)
    app.WriteVar(var)
   
#@Procedure: antiDrift
#
# Get the current antidrift status: 1=enabled, 0=disabled
#@

def antiDrift(app):
    varname = cfg.varname('optloopdiag', 'DRIFT.ENABLE.CUR')
    return app.ReadVar(varname).Value()


#@Procedure: doAcquireRef
#
#    Acquire reference source and prepare for closed loop operation.
#    No input parametrs. Assumes that the star is already present on ccd47
#    thanks to a previous prepareAcquireRef() command.
#@


def doAcquireRef(app, sensor, instr):

    # Return a string with output parameters plus the TV image - not a C string, the image may contain zero bytes
    framestr = '0' * (256*256)
    if 0:
        return '%7.3f %7.3f %7.3f %d %d %7.3f %d %d %7.3f %s %s %s %s %7.3f %s %s %d %d IMAGE%s' % ( 0.0, 0.0,
                                              app._acquireRefSettings.HOfreq,
                                              app._acquireRefSettings.nModes,
                                              app._acquireRefSettings.HObin,
                                              app._acquireRefSettings.TTamp,
                                              app._acquireRefSettings.fw1_obs,
                                              app._acquireRefSettings.fw2_obs,
                                              app._acquireRefSettings.starMag,
                                              app._acquireRefSettings.pupils,
                                              app._acquireRefSettings.base,
                                              app._acquireRefSettings.rec,
                                              app._acquireRefSettings.filtering,
                                              app._acquireRefSettings.gain,
                                              app._acquireRefSettings.CLbase,
                                              app._acquireRefSettings.CLrec,
                                              app._acquireRefSettings.CLfreq,
                                              len(framestr),
                                              framestr)

    setupDevices.loadDevices( app, ['fw1', 'fw2', 'ccd39', 'stagex', 'stagey', 'stagez', 'lens'], check=True)

    acqData = calib.loadCalibData( calib.sourceAcqCalibFile(), check=['magError'])

    # Check BCU47 temperature. If it is too low, the camera lens will not work!
    try:
        check = cfg.cfg['wfsarb']['cameralensTempCheck'].Value()
        th = cfg.cfg['wfsarb']['cameralensTempMin'].Value()
        n  = cfg.cfg['wfsarb']['cameralensTempNumber'].Value()
    except:
        app.log('Warning: no configuration on BCU47 temperature: no camera lens temperature check will be done.')
    else:
        if check:
            t  = app.ReadVar('powerboard.'+cfg.side+'.TEMPS').Value()[n]
            if t < th:
                raise AORetryException(0, "Camera lens temperature too low. T=%5.2f, minimum allowed is %5.2f" % (t, th))

    # Re-set wheels (in case this is a repeated AcquireRef)
    app.fw1.moveTo( app._acquireRefSettings.fw1_acq)
    app.fw1.waitTargetReached(checkStatus=True)

    app.arbMsg('WFS: starting source acquisition procedure')

    # Center star on ccd47
    # This code has been removed from the Magellan system, where centering is done manually
    if sensor != AO_WFS_MAG: 
        acqRet = source_acq.acquire(app, TVavg = app._acquireRefSettings.TVavg, TVflux = app._acquireRefSettings.TVflux, fwhm = app._acquireRefSettings.seeing)
    else:
        class obj: pass
        acqRet = obj()
        acqRet.deltaX=0
        acqRet.deltaY=0
        acqRet.mag=0
        acqRet.TVframe = numarray.zeros((1024,1024), dtype='int16')

    # Reset stage position so that offsets work from here
    setStagePos( app, app.stagex.getPos(), app.stagey.getPos())

    # Apply focus correction
    offsetXYZ( app, 0, 0, 0)

    # Turn off stages
    # Magellan: leave stages on
    app.stagex.enable(True, onaftermove=False)#True)
    app.stagey.enable(True, onaftermove=False)#True)
    app.stagez.enable(True, onaftermove=True)#True)

    # Set filterwheels to observing positions
    app.fw1.moveTo( app._acquireRefSettings.fw1_obs)
    app.fw1.waitTargetReached(checkStatus=True)
    mag=0

#
# This part has been commented out since the new AcquireRef command
# may stop before the star is correctly centered on the ccd39
#
#    # Calculate actual magnitude from ccd39 counts
#    flux = ccd39Flux(app, nframes=500)
#    expected_flux = app._acquireRefSettings.flux
#
#    mag = flux2Mag(flux)
#
#    app.arbMsg('WFS: star found, estimated magnitude: %5.2f' % mag)
#    app.log('Estimated magnitude from ccd39: %5.2f' % mag, app.LOG_LEV_DEBUG)
#
#    # If new magnitude too different from the expected one, give an error
#    if abs(mag - app._acquireRefSettings.starMag) > acqData['magError']:
#        raise AORetryException(0, "Magnitude is %3.1f, expected %3.1f" % (mag, app._acquireRefSettings.starMag))
#
#    app.log('Actual magnitude %3.1f - Recomputing AO params' % mag, app.LOG_LEV_INFO)
#
#    app._acquireRefSettings = computeAOsettings( app, 
#                                                 app._acquireRefSettings.instr,
#                                                 flux,
#                                                 app._acquireRefSettings.starColor,
#                                                 app._acquireRefSettings.r0,
#                                                 app._acquireRefSettings.v0)

    selectSlopenull(app, app._acquireRefSettings.slopenull, matchLUT=False)
    #app.tt.set( amp = app._acquireRefSettings.TTamp, offx=0, offy=0, freq=0)

    framestr = packCcd47Image( acqRet.TVframe)

    app.arbMsg('WFS: star acquired')

    # Return a string with output parameters plus the TV image - not a C string, the image may contain zero bytes
    return '%7.3f %7.3f %7.3f %d %d %7.3f %d %d %7.3f %s %s %s %s %7.3f %s %s %d %d IMAGE%s' % (acqRet.deltaX, acqRet.deltaY,
                                              app._acquireRefSettings.HOfreq,
                                              app._acquireRefSettings.nModes,
                                              app._acquireRefSettings.HObin,
                                              app._acquireRefSettings.TTamp,
                                              app._acquireRefSettings.fw1_obs,
                                              app._acquireRefSettings.fw2_obs,
                                              mag,
                                              app._acquireRefSettings.pupils,
                                              app._acquireRefSettings.base,
                                              app._acquireRefSettings.rec,
                                              app._acquireRefSettings.filtering,
                                              app._acquireRefSettings.gain,
                                              app._acquireRefSettings.CLbase,
                                              app._acquireRefSettings.CLrec,
                                              app._acquireRefSettings.CLfreq,
                                              len(framestr),
                                              framestr)

def doGetTVSnap(app, sensor, instr, saveInRTDB=True, useImage=None):

    # Try to read an image (any image) from the ccd47 
    try:
        setupDevices.loadDevices( app, ['ccd47'], check=True)

        if useImage != None:
            f = useImage
        else:
            f = app.ccd47.get_frame()

        # Pack the TV image in a string, rescaled to 0-255 and 256x256.
        framestr = packCcd47Image(f)

        if saveInRTDB:
            varname = cfg.varname('wfsarb', 'TV_IMAGE')
            var = AOVar.AOVar( varname, tipo='CHAR_VARIABLE', value='')
            var.SetValue(framestr)
            app.WriteVar(var)
        else: 
            # Return a string with the TV image - not a C string, the image may contain zero bytes
            return '1 %d IMAGE%s' % (len(framestr), framestr)

    except:
        pass

        

def doSaveStatus( app, sensor, instr, outputFile):
    '''
    Saves the WFS status into a FITS file with dummy data.
    '''

    hdr, errstr = getSetupFits(app)
    hdr['SENSOR'] = sensor
    hdr['INSTR'] = instr
    dummy = numarray.zeros(())
    fits_lib.writeSimpleFits( outputFile, dummy, hdr, overwrite=True)

def doEnableDisturb( app, sensor, instr, wfs=False, ovs=False):
    '''
    Enables/disables the AdSec disturbance on wfs and ovs frames.
    '''
    if sensor == AO_WFS_FLAO or sensor == AO_WFS_LBTI or sensor == AO_WFS_MAG:
        setupDevices.loadDevices( app, ['sc'], check=True)
        app.sc.enable_disturb( wfs, ovs)
    else:
        app.log('Warning: doEnableDisturb() called with sensor="%s", nothing to do' % sensor)

def testDark(app):
    '''
    Test whether the current dark frame is still valid.
    Returns True if the dark is judged to be correct.
    '''

    f = app.ccd39.get_frames(10, average=True)
    bin = app.ccd39.xbin()
    if (bin==1) or (bin==2):
        dim=5 / bin
    elif (bin==3) or (bin==4):
        dim =8 / bin
    else:
        raise AORetryException( 0, "Incorrect binning %d" % bin)

    dx = app.ccd39.dx() / bin;
    dy = app.ccd39.dy() / bin;

    if f[0:dim,0:dim].sum() == 0 or \
       f[0:dim,dy-dim:dy].sum() == 0 or \
       f[dx-dim:dx,0:dim].sum() == 0 or \
       f[dx-dim:dx,dy-dim:dy].sum() == 0:
        return False

    return True



   
def doCloseLoop( app, sensor, instr):
    '''
    Closes the fastlink fiber.
    '''
    if sensor == AO_WFS_FLAO or sensor == AO_WFS_LBTI or sensor == AO_WFS_MAG:
        setupDevices.loadDevices( app, ['ccd39','sc'], check=True)
        app.sc.set_fl_cmd( os.path.join( calib.WfsCalibDir(), 'dsp_programs', 'sl_fl_cmd_switch.txt'))
        app.sc.fl_start()
        initStagePos(app)
    else:
        app.log('Warning: doCloseLoop() called with sensor="%s", nothing to do' % sensor)


def setLoopFreq( app, freq, decimation):
    '''
    Changes the loop speed.
    '''
    setupDevices.loadDevices( app, ['ccd39', 'sc'], check=True)

    app.ccd39.stop()
    app.sc.set_masterd_decimation( decimation)
    app.ccd39.set_framerate(freq, change_speed=True)
    app.ccd39.start() 

def modifyAOInternal( app, sensor, instr, freq, binning, TTmod, Fw1Pos, Fw2Pos, decimation, checkCameralens):
    '''
    Changes the AO loop parameters just before closing the loop.
    For all parameters, -1 means unchanged from current value.
    '''

    # Test modulation range first

    setupDevices.loadDevices( app, ['ccd39', 'tt'], check=True)
    if freq == -1:
        freq = app.ccd39.get_framerate()
    if TTmod == -1:
        TTmod = app.tt.amp()

    if Fw1Pos>=0:
        setupDevices.loadDevices( app, ['fw1'], check=True)
        app.fw1.moveTo(Fw1Pos, waitTimeout=20)
    #if Fw2Pos>=0:
    #    setupDevices.loadDevices( app, ['fw2'], check=True)
    #    app.fw2.moveTo(Fw2Pos, waitTimeout=20)

    if checkCameralens:
	pass

    doSetLoopParams( app, sensor, instr, freq, decimation, binning, TTmod)

def zFocalPlane( app, x, y):
    '''
    Returns the amount of Z compensation to apply for the given x,y (absolute) position in the focal plane.
    ''' 
    fov = calib.readFov( allowFail = False)

    # Transform coordinates to be relative from the FoV center
    x -= fov['xc']
    y -= fov['yc']
    return 0;

def offsetXYZ(app, x, y, z, zCompensation = True, enableAfter=False, block=True):
    '''
    Offset the XYZ stages in parallel.
    Compensates for Z focal plane distortion if not otherwise specified.
    '''

    xtarget = app._stagexpos + x
    ytarget = app._stageypos + y
    ztarget = app._stagezpos + z
    app._zCompensation = zFocalPlane( app, xtarget, ytarget)

    if zCompensation:
        z = ztarget + app._zCompensation
        app.log('offsetXYZ(): Focus correction (absolute): %5.3f mm' % app._zCompensation, app.LOG_LEV_INFO)
    else:
        z = ztarget

    # Check that movement is inside stage limits.
    fov = calib.readFov( allowFail = False)
    if xtarget<fov['xmin'] or xtarget>fov['xmax'] or ytarget<fov['ymin'] or ytarget>fov['ymax'] or z<fov['zmin'] or z>fov['zmax']:
        str = 'Offset outside FoV limits.'
        raise AORetryException( code = AOConstants.AOErrCode['VALUE_OUT_OF_RANGE_ERROR'], errstr = str)

    # Magellan: always keep control engaged and brake off.
    enableAfter = False#True

    if abs(x)> 1e-3:
        app.stagex.enable(True, onaftermove = enableAfter)
        app.stagex.moveTo(xtarget, accuracy = 0.08)
    if abs(y)> 0.050:
        app.stagey.enable(True, onaftermove = enableAfter)
        app.stagey.moveTo(ytarget, accuracy = 0.08)
    if z or zCompensation:
        app.stagez.enable(True, onaftermove = True)
        app.stagez.moveTo(z, accuracy = 0.1)

    if block:
        app.stagex.waitTargetReached( timeout=60, checkStatus=True)
        app.stagey.waitTargetReached( timeout=60, checkStatus=True)
        app.stagez.waitTargetReached( timeout=60, checkStatus=True)

    setStagePos( app, xtarget, ytarget, ztarget)

def initStagePos(app):
    '''
    Initialization of stage position.
    '''
    setupDevices.loadDevices( app, ['stagex', 'stagey', 'stagez'], check=True)
    setStagePos( app, app.stagex.getPos(), app.stagey.getPos(), app.stagez.getPos())

def setStagePos( app, x=None, y=None, z=None):
    '''
    Set the specified positions as the current stage positions.
    '''
    if x != None:
        app._stagexpos = x
    if y != None:
        app._stageypos = y
    if z != None:
        app._stagezpos = z


def moveXYZ( app, x, y, z, zCompensation = True, enableAfter=False, block=True):
    '''
    Move the XYZ stages in parallel to an absolute position.
    Compensates for Z focal plane distortion if not otherwise specified.
    '''

    # Magellan: always keep motor engaged and brake off
    #Not anymore
    enableAfter = False
    #True
    
    app.stagex.enable(True, onaftermove = enableAfter)
    app.stagey.enable(True, onaftermove = enableAfter)
    app.stagez.enable(True, onaftermove = True)

    app._zCompensation = zFocalPlane( app, x,y)

    if zCompensation:
        if z == None:
            z = app.stagez.getPos()
        ztarget = z + app._zCompensation
        app.log('moveXYZ(): Focus correction (absolute): %5.3f mm' % app._zCompensation, app.LOG_LEV_INFO)

    if x != None:
        app.stagex.moveTo(x, accuracy = 0.10)
    if y != None:
        app.stagey.moveTo(y, accuracy = 0.10)
    if z != None or x != None or y != None:
        app.stagez.moveTo(ztarget, accuracy = 0.03)

    if block:
        app.stagex.waitTargetReached( timeout=60, checkStatus=True)
        app.stagey.waitTargetReached( timeout=60, checkStatus=True)
        app.stagez.waitTargetReached( timeout=60, checkStatus=True)

    setStagePos( app, x, y, z)


def offsetZinternal( app, sensor, instr, offsetz):
    '''
    Offset the Z stage.
    '''

    # Check that movement is inside stage limits.

    setupDevices.loadDevices( app, ['stagez'], check=True)

    app.log('offsetZ(): starting offset to %5.3f mm' % (offsetz), app.LOG_LEV_INFO)
    try:
        offsetXYZ( app, 0, 0, offsetz - app._stagezpos)  ## offset Z absolute
        #offsetXYZ( app, 0, 0, offsetz)                  ## offset Z relative
    except AdOptError, e:
        if e.code == AOConstants.AOErrCode['TIMEOUT_ERROR']:
            errstr = 'Stages position not reached'
        else:
            errstr = e.errstr 
        raise AORetryException( e.code, errstr)

    app.log('offsetZ(): done', app.LOG_LEV_INFO)

def offsetXYinternal( app, sensor, instr, offsetx, offsety, brake=True):
    '''
    Offset the XY stages. 
    '''

    setupDevices.loadDevices( app, ['stagex', 'stagey'], check=True)

    app.log('offsetXY(): starting offset of %5.3f, %5.3f mm' % (offsetx, offsety), app.LOG_LEV_INFO)

    try:
        offsetXYZ( app, offsetx, offsety, 0, zCompensation =True, enableAfter = not brake)

    except AdOptError, e:
        if e.code == AOConstants.AOErrCode['TIMEOUT_ERROR']:
            errstr = 'Stages position not reached'
        else:
            errstr = e.errstr 
        raise AORetryException( e.code, errstr)

    app.log('offsetXY(): done', app.LOG_LEV_INFO)


def optimizeGainInternal( app, sensor, instr):
    '''
    Run the gain optimization procedure.
    '''
    setupDevices.loadDevices( app, ['ccd39'], check = True)
    app.log('Starting gain optimization procedure')
    from AdOpt import autogain
    try:
        a = autogain.autogain(app)
        a.doit( app.ccd39.xbin())
    except AdOptError, e:
        if e.code == AOConstants.AOErrCode['WFSARB_AUTOGAIN_USER_STOP']:
            return
        raise

def correctModesInternal( app, sensor, instr, modes):
    '''
    Correct higher-order modes.
    '''

    app.log('correctModes(): not implemented', app.LOG_LEV_INFO)
    pass


def emergencyOffInternal(app, sensor, instr):
    '''
    Immediate power off.
    '''

    setupDevices.loadDevices( app, ['power'], check = True)

    # W#1
    if 'main' in app.power.list():
        app.power.turnoff('main', force = True)
    # W#2
    if 'ps1' in app.power.list():
        app.power.turnoff('ps1', force = True)





def doSetLoopParams( app, sensor, instr, freq, decimation, binning, modulation, forceBinning=False, takeDark=True, pupilReg=None, HOgain=None):
    '''
    Changes the AO loop parameters.
    '''

    setupDevices.loadDevices( app, ['ccd39', 'tt'], check=True)

    curBin = app.ccd39.xbin()
    curGain = app.ccd39.get_gain()
    curFramerate = app.ccd39.get_framerate()
    curModulation = app.tt.amp()

    # At binning 1, switch between 2500 and 5000 kpix/sec based on the requested framerate
    #
    # Commented out because this seems to be already implemented in ccd.change_framerate() in ccd.py
    # If that one works, there is no need for this code.

#    curSpeed = app.ccd39.speed()
#    speed=None
#    force = False
#    if binning=1:
#        if freq>1000:
#            speed=5000
#        else:
#            speed=2500
#        if speed != curSpeed:
#            force = True
#    if (curBin != binning) or (forceBinning == True) or (pupilReg != None) or (HOgain != None) or force:
#        app.tt.set( amp=0)
#        setHObinning( app, binning, pupilReg = pupilReg, HOgain= HOgain, speed=speed)

    # Set modulation to zero before changing binning and/or framerate
    setTTmodulation( app, 0)

    if (curBin != binning) or (forceBinning == True) or (pupilReg != None) or (HOgain != None):
        setHObinning( app, binning, pupilReg = pupilReg, HOgain= HOgain)

    # Always do these because the change binning may have changed everything
    setLoopFreq( app, freq, decimation)
    setTTmodulation( app, modulation)

    newBin = app.ccd39.xbin()
    newFramerate = app.ccd39.get_framerate()
    newGain = app.ccd39.get_gain()

    # Re-take dark if anything regarding the ccd has changed.
    if (newBin != curBin) or (newFramerate != curFramerate) or (newGain != curGain):
        if takeDark:
            calibHODarkInternal(app, sensor, instr)
   
def setTTmodulation( app, modulation):
    '''
    Sets the TT modulation in lambda/D
    '''  

    setupDevices.loadDevices( app, ['ccd39', 'tt'], check=True)

    time.sleep(3)
  
    # Divide by 1e3 the modulation because the tip-tilt wants it in radians
    # and the table specifies everything in milliradians
    try:
        #app.tt.set( amp = modulation / 1e3)
        frm = round(app.ccd39.get_framerate())
        app.tt.set( amp = modulation / 1e3, freq =frm) 
        time.sleep(10)
        app._lastTTmodulation = modulation
    except Exception, e:
        raise AORetryException(AOConstants.AOErrCode['VALUE_OUT_OF_RANGE_ERROR'], "TT modulation out of allowed range for frequency %3.1f" % frm)


def doStopLoop( app, sensor, instr):
    '''
    Closes the fastlink fiber.
    '''
    if sensor == AO_WFS_FLAO or sensor == AO_WFS_LBTI or sensor == AO_WFS_MAG:
        setupDevices.loadDevices( app, ['sc'], check=True)
        app.sc.fl_stop()
    else:
        app.log('Warning: doStopLoop() called with sensor="%s", nothing to do' % sensor)

def doPauseLoop(app, sensor, instr):
    '''
    Pauses the AO loop. Disables the disturbance, if any, and re-enables so that
    it will restart as soon as the loop is resumed.
    '''

    if sensor != AO_WFS_FLAO and sensor != AO_WFS_LBTI and sensor != AO_WFS_MAG:
        app.log('Warning: doPauseLoop() called with sensor="%s", nothing to do' % sensor)
        return

    setupDevices.loadDevices( app, ['sc'], check=True)

    # Save current light level
    app._pauseMag = flux2Mag(ccd39Flux( app, nframes=20))

    # Disable disturbance, remembering previous status
    wfs,ovs = app.sc.get_disturb()
    app.sc.enable_disturb( False, False)
    time.sleep(0.02)

    # Pause loop
    app.sc.fl_stop()
    time.sleep(0.02)

    # Re-enable disturbance
    app.sc.enable_disturb( wfs, ovs)


def doResumeLoop(app, sensor, instr):
    '''
    Resumes a paused loop.
    '''

    if sensor != AO_WFS_FLAO and sensor != AO_WFS_LBTI and sensor != AO_WFS_MAG:
        app.log('Warning: doResumeLoop() called with sensor="%s", nothing to do' % sensor)
        return

    # Check light level
    mag = flux2Mag(ccd39Flux( app, nframes=20))
    if abs(app._pauseMag - mag) >2:
        app.log('Star magnitude on WFS changed from %3.1f to %3.1f' % (app._pauseMag, mag))
        app.log('resumeLoop not executed - loop still paused')
        raise AORetryException( code= AOConstants.AOErrCode['WFSARB_STAR_WRONGFLUX'], errstr='Star not found - loop still paused')

    doCloseLoop(app, sensor, instr)


    


def setHOarmPolicyInternal( app, ccd39=None, adc=None, tt=None, fw1=None, rerot=None):
    '''
    Sets the HO arm auto/manual policy.
    '''

    if ccd39 != None:
        app.ccd39.setPolicy( ccd39)
    if adc != None:
        app.adc.setPolicy(adc)
    if tt != None:
        app.tt.setPolicy(tt)
    if fw1 != None:
        app.fw1.setPolicy(fw1)
    if rerot != None:
        app.rerot.setPolicy(rerot)


def setTVarmPolicyInternal( app, ccd47=None, fw2=None):
    '''
    Sets the TV arm auto/manual policy.
    '''

    if ccd47 != None:
        app.ccd47.setPolicy( ccd47)
    if fw2 != None:
        app.fw2.setPolicy( fw2)

def setPointingPolicyInternal( app, stages=None, cube=None, lamp=None):
    '''
    Sets the Pointing auto/manual policy.
    '''
    app.log('setPointingPolicy: setting policy', app.LOG_LEV_DEBUG)
    setupDevices.loadDevices( app, ['stagex', 'stagey', 'stagez', 'cuberot', 'cubestage', 'lamp'], check=True)

    if stages != None:
        app.stagex.setPolicy( stages)
        app.stagey.setPolicy( stages)
        app.stagez.setPolicy( stages)
    if cube != None:
        app.cuberot.setPolicy( cube)
        app.cubestage.setPolicy( cube)
    if lamp != None:
        app.lamp.setPolicy( lamp)
    app.log('setPointingPolicy: done', app.LOG_LEV_DEBUG)


def calibHODarkInternal(app, sensor, instr, name=None, num=100, apply=True, resumeFW=True):
    '''
    Acquires a dark frame using the current binning, exposure time etc.
    Results are saved in $ADOPT_ROOT/calib/WFS/backgrounds with a timestamp.
    If a name is provided, a link with the specified name will be created too.

    Timestamps are always unique, so the file will be always created. If
    the name provided already exists, the link is not created and a warning
    is generated.
    '''

    app.log('calibHODarkInternal(): starting measure, %d frames' % num, app.LOG_LEV_INFO)
    errstr = ''

    if sensor==AO_WFS_FLAO:
        setupDevices.loadDevices( app, ['ccd39', 'sc', 'power', 'flowerpot', 'lamp', 'fw1'], check=True)
    if sensor==AO_WFS_LBTI:
        setupDevices.loadDevices( app, ['ccd39', 'sc'], check=True)
    if sensor==AO_WFS_MAG:
        setupDevices.loadDevices( app, ['ccd39', 'sc', 'fw1'], check=True)

    if not app.ccd39.isOperating(wait=1):
        raise AORetryException( 0, 'Ccd39 is not ready')

    antiDriftStatus = antiDrift(app)
    setAntiDrift(app, 0)

    ## -- FLAO only: filterwheel & calib unit

    if sensor==AO_WFS_FLAO:
        if not app.fw1.isReady():
            raise AORetryException( 0, 'Filterwheel #1 is not ready')

        # Move FW1 and set lamp to zero in parallel
        pos = app.fw1.getPos()
        app.fw1.moveTo(4)

        app.fw1.waitTargetReached()

    if sensor == AO_WFS_MAG:
        if not app.fw1.isReady():
            raise AORetryException( 0, 'Filterwheel #1 is not ready')
        pos = app.fw1.getPos()
        app.fw1.moveTo(2)
        app.fw1.waitTargetReached()

    # Equalize ccd39 and read dark frame
    app.ccd39.equalize_quadrants( target=500)

    time.sleep(2)   # Allow ccd39 stabilization?

    frame = frames_lib.averageFrames(app.ccd39.get_frames(num, type='pixels', subtractDark = False))
    hdr, errstr = getSetupFits(app)
    hdr.update( {'AVERAGED FRAMES': num})
    hdr.update( {'DETECTOR': 'CCD39'} )

    filename = saveCalibFile( app, calib.BackgDir('ccd39', app.ccd39.xbin()), frame, hdr, calib.ensureExtension( name, '.fits'), getFullPath=True)

    # -- FLAO only: restore FW1 and lamp position
    if sensor==AO_WFS_FLAO:

        if resumeFW:
            app.fw1.moveTo(pos)
            app.fw1.waitTargetReached()

    if sensor == AO_WFS_MAG:
        if resumeFW:
    	    app.fw1.moveTo(pos)
            app.fw1.waitTargetReached()

    if apply:
        selectHODark(app, name=os.path.basename(filename))

    setAntiDrift(app, antiDriftStatus)

    app.log('calibHODarkInternal(): done', app.LOG_LEV_INFO)

    return errstr

def calibTVDarkInternal(app, sensor, instr, name=None, num=10, apply=True, resumeFW=True, moveGimbal=False):
    '''
    Acquires a dark frame using the current binning, exposure time etc.
    Results are saved in $ADOPT_ROOT/calib/WFS/ccd47/backgrounds with a timestamp.
    If a name is provided, a link with the specified name will be created too.

    Timestamps are always unique, so the file will be always created. If
    the name provided already exists, the link is not created and a warning
    is generated.

    For the Magellan system, the shutter is closed to take darks
    '''
    app.log('calibTVDarkInternal(): starting measure, %d frames' % num, app.LOG_LEV_INFO)
    errstr = ''
    if sensor==AO_WFS_FLAO:
        setupDevices.loadDevices( app, ['ccd47', 'fw1'], check=True)
    if sensor==AO_WFS_MAG:
        setupDevices.loadDevices( app, ['ccd47', 'gimbal', 'shutter'], check=True)

    if not app.ccd47.isOperating():
        raise AORetryException( 0, 'Ccd47 is not ready')


    if sensor == AO_WFS_FLAO:
        # Turn off lamp and move fw1 to empty
        pos = app.fw1.getPos()
        app.fw1.moveTo(2)
        calibUnitStat = app.flowerpot.getOperatingState()
        if calibUnitStat == app.flowerpot.OPSTATE_CALIBRATION or calibUnitStat == app.flowerpot.OPSTATE_INTERNALSOURCE:
            if not app.lamp.isOperating():
                raise AORetryException( 0, 'Lamp is not ready')

            lampInt = app.lamp.getIntensity()
            app.lamp.setIntensity(0, waitTimeout=100)

            time.sleep(3)       # Wait for lamp cooling

        app.fw1.waitTargetReached()

    if sensor== AO_WFS_MAG:
        app.shutter.close()
        if moveGimbal:
            app.gimbal.dark()

    #app.ccd47.equalize_quadrants( target=500)

    frame = frames_lib.averageFrames( app.ccd47.get_frames(num, type='pixels', subtractDark = False, nextFrame = True))
    hdr, errstr = getSetupFits(app)
    hdr.update( {'AVERAGED FRAMES': num})
    hdr.update( {'DETECTOR': 'CCD47'} )

    errstr = saveCalibFile( app, calib.BackgDir('ccd47', app.ccd47.xbin()), frame, hdr, calib.ensureExtension( name, '.fits'))


    if sensor == AO_WFS_FLAO:
        # Restore lamp and fw1
        if resumeFW:
            app.fw1.moveTo(pos)
        if calibUnitStat == app.flowerpot.OPSTATE_CALIBRATION or calibUnitStat == app.flowerpot.OPSTATE_INTERNALSOURCE:
            app.lamp.setIntensity(lampInt, waitTimeout=100)

        if resumeFW:
            app.fw1.waitTargetReached()

    if sensor == AO_WFS_FLAO:
        if resumeFW:
            app.fw1.moveTo(pos)
            app.fw1.waitTargetReached()

    if sensor == AO_WFS_MAG:
        app.shutter.open()
        if moveGimbal:
            app.gimbal.center()

    if apply:
        selectTVDark(app)

    app.log('calibTVDarkInternal(): done', app.LOG_LEV_INFO)
    return errstr


def calibIRTCDarkInternal(app, sensor, instr, name=None, num=10, apply=True, ccd='irtc'):
    '''
    Acquires an IRTC dark frame
    Results are saved in $ADOPT_ROOT/calib/WFS/irtc/backgrounds with a timestamp.
    If a name is provided, a link with the specified name will be created too.

    Timestamps are always unique, so the file will be always created. If
    the name provided already exists, the link is not created and a warning
    is generated.
    Can operate on 'irtc' or 'pisces'.
    '''
    app.log('calibIRTCDarkInternal(): starting measure, %d frames' % num, app.LOG_LEV_INFO)
    errstr = ''
    setupDevices.loadDevices( app, [ccd, 'power', 'flowerpot', 'lamp'], check=True)

    # Turn off lamp
    if sensor == AO_WFS_FLAO:
        calibUnitStat = app.flowerpot.getOperatingState()
        if calibUnitStat == app.flowerpot.OPSTATE_CALIBRATION or calibUnitStat == app.flowerpot.OPSTATE_INTERNALSOURCE:
            if not app.lamp.isOperating():
                raise AORetryException( 0, 'Lamp is not ready')

            lampInt = app.lamp.getIntensity()
            app.lamp.setIntensity(0, waitTimeout=60)
            time.sleep(3)       # Wait for lamp cooling

    # Read IRTC
    try:
        if ccd == 'irtc':
            data, IrtcHdr = app.irtc.get_frames(num, type='pixels', subtractDark = False, getHeader = True)
        elif ccd == 'pisces':
            data, IrtcHdr = app.pisces.get_frames(num, type='pixels', getHeader = True)
    except:
        raise AORetryException( 0, 'Error reading '+ccd.upper())

    IrtcHdr = fits_lib.cleanHdr(IrtcHdr)
    frame = frames_lib.averageFrames( data, useFloat=True).astype(numarray.Int16)
    hdr, errstr = getSetupFits(app)

    IrtcHdr.update( hdr)
    IrtcHdr.update( {'AVERAGED FRAMES': num})
    IrtcHdr.update( {'DETECTOR': ccd.upper()} )

    filename = saveCalibFile( app, calib.BackgDir(ccd, 1), frame, IrtcHdr, calib.ensureExtension( name, '.fits'), getFullPath = True)
    fits_lib.writeSimpleFits(filename+'_cube.fits', data, IrtcHdr)

    # Restore lamp
    if sensor == AO_WFS_FLAO:
        if calibUnitStat == app.flowerpot.OPSTATE_CALIBRATION or calibUnitStat == app.flowerpot.OPSTATE_INTERNALSOURCE:
            app.lamp.setIntensity( lampInt, waitTimeout=60)

    if apply:
        v = app.ReadVar( cfg.ccd_darkvariable[ccd]+'.REQ')
        app.SetVar( v, os.path.basename(filename))

    app.log('calibIRTCDarkInternal(): done', app.LOG_LEV_INFO)

    return errstr


#@Procedure: calibMovementsInternal
#
# Calibrates all the mechanical movements of the WFS
#
# Calibrates stages and tip-tilt movements using optical feedback.
# Also registers the reference PSF position on the TV camera
#@

def calibMovementsInternal(app, sensor, instr):

    centerPupils.calibCentering( useTT=True, useStages = True)
    source_acq.calibMovement(app)
    source_acq.calibReference(app)


def saveCalibFile( app, calibDir, data, hdr, name=None, getFullPath = False):
    '''
    Saves calibration data in the specified directory as a FITS file with header.
    If <name> is provided, a link with that name will be created too.
    Returns an error string, or the filename if requested (full path)
    '''

    datetime = calib.getDateTime()
    filename = os.path.join( calibDir, datetime)+'.fits'
    fits_lib.writeSimpleFits( filename, data, hdr)
    logstr = 'saved as %s' % datetime

    errstr = ''

    if name:
        linkname = os.path.join( calibDir, name)
        if not os.path.exists(linkname):
            os.symlink( filename, linkname)
            logstr += ' and as %s' % name
        else:
            errstr = 'Cannot create link %s: file exists' % name

    app.log('saveCalibFile(): %s' % logstr, app.LOG_LEV_DEBUG)
    if (errstr != ''):
        app.log('saveCalibFile(): %s' % errstr, app.LOG_LEV_ERROR)

    if getFullPath:
        return filename
    return errstr


def savePsfCallback( app, num):

    percent = float(num)/ app._savePsfNum * 100;
    diff = percent - app._savePsfLastPercent
    if diff >=10:
        app.log('savePsf(): %d percent done' % percent, app.LOG_LEV_INFO)
        app._savePsfLastPercent = percent
    
def savePsfInternal( app, sensor, instr, filename, num=50):
    '''
    Saves an averaged TV frame.
    File will be a float FITS.
    '''
    try:
        app.log('savePsfInternal(): starting measure, %d frames' % num, app.LOG_LEV_INFO)
        setupDevices.loadDevices( app, ['ccd47'], check=True)

        app._savePsfNum = num
        app._savePsfLastPercent=-1

        frames = app.ccd47.get_frames( num, type = 'pixels', callback = savePsfCallback, average=True, useFloat=True)
        hdr, errstr = getSetupFits(app)
        hdr.update( {'DETECTOR': 'CCD47'} )
        fits_lib.writeSimpleFits( filename, frames, hdr)
    except AdOptError, e:
        raise AORetryException( code = e.code, errstr = "Cannot save PSF")
    except Exception, e:
        print e
        raise AORetryException( code = 0, errstr = "Cannot save PSF")

    app.log('savePsf(): 100 percent done', app.LOG_LEV_INFO)

def saveIrtcInternal( app, sensor, instr, filename, num=50, ccd='irtc'):
    '''
    Saves an IRTC cube.
    File will be an integer FITS.
    Can operate on 'irtc' or 'pisces'
    '''

    try:
        app.log('saveIrtcInternal(): starting measure, %d frames' % num, app.LOG_LEV_INFO)
        setupDevices.loadDevices( app, [ccd], check=True)

        if ccd=='irtc':
            frames, IrtcHdr = app.irtc.get_frames( num, type='pixels', getHeader = True)
        elif ccd=='pisces':
            frames, IrtcHdr = app.pisces.get_frames( num, type='pixels', getHeader = True)
        IrtcHdr = fits_lib.cleanHdr(IrtcHdr)
        hdr, errstr = getSetupFits(app)
        IrtcHdr.update(hdr)

        IrtcHdr.update( {'DETECTOR': ccd.upper()} )

        fits_lib.writeSimpleFits( filename, frames, IrtcHdr)
    except AdOptError, e:
        raise AORetryException( code = e.code, errstr = "Cannot save PSF")
    except Exception, e:
        print e
        raise AORetryException( code = 0, errstr = "Cannot save PSF")

    app.log('saveIrtc(): done', app.LOG_LEV_INFO)



def setSourceInternal( app, sensor, instr, config, magnitude=None):
    '''
    Sets the calibration unit to either "CALIBRATION" or "TELESCOPE" state.
    In calibration state, an equivalent magnitude value can be specified for the calibration lamp.
    '''

    if sensor != AO_WFS_FLAO:
        return 

    setupDevices.loadDevices( app, ['power', 'flowerpot'], check=True)
   
    if config == "CALIBRATION": 
        app.flowerpot.setOperatingState( app.flowerpot.OPSTATE_CALIBRATION)

    if config == "TELESCOPE":
        app.flowerpot.setOperatingState( app.flowerpot.OPSTATE_OBSERVATION)

    if config == "INTERNAL SOURCE":
        app.flowerpot.setOperatingState( app.flowerpot.OPSTATE_INTERNALSOURCE)

    if magnitude != None:
        app.log('setSourceInternal(): magnitude setting not implemented.', app.LOG_LEV_INFO)

def calibSlopeNullInternal(app, sensor, instr, name=None, num=100, apply=True):
    '''
    Acquires a slope null using the current pupils, binning etc.
    Results are saved in $ADOPT_ROOT/calib/WFS/slopenulls with a timestamp.
    If a name is provided, a link with the specified name will be created too.

    Timestamps are always unique, so the file will be always created. If
    the name provided already exists, the link is not created and a warning
    is generated.
    '''

    selectHODark(app)  # Load appropriate dark frame
    selectSlopenull(app, 'zero1600.fits', matchLUT=False)  # Load zero slopenull

    app.log('calibSlopeNullInternal(): starting measure, %d frames' % num, app.LOG_LEV_INFO)
    setupDevices.loadDevices( app, ['ccd39', 'sc'], check=True)

    slopes = frames_lib.averageFrames( app.ccd39.get_frames(num, type='slopes'))
    hdr, errstr = getSetupFits(app)
    hdr.update( {'AVERAGED FRAMES': num})

    errstr = saveCalibFile( app, calib.slopenullDir(app.ccd39.xbin()), slopes, hdr, calib.ensureExtension( name, '.fits'))

    if apply:
        selectSlopenull(app)

    app.log('calibSlopeNullInternal(): done', app.LOG_LEV_INFO)

    return errstr


def getCalibFile( dir, hdrmatch={}, name = None):
    '''
    Gets the calibration file in the specified directory that matches the specified header.
    The most recent file is used. In case a name is provided, that file will be used instead,
    provided that the header matches. <name> can be both a proper name or the timestamp.

    Returns the filename, or throws an exception if no match is found. If a name is specified
    and the header doesn't match, an exception is thrown
    '''
    if name == None:

        # No name specified: scan the existing files (excluding symlinks) starting from the most recent,
        # and see if one matches the header.
        dd = filter( lambda x: not os.path.islink(x), os.listdir( dir))
        dd.sort()
        for n in range(len(dd)-1, -1, -1):
            filename = os.path.join( dir, dd[n])
            print filename
            hdr = fits_lib.readFitsHdr(filename)
            match = True
            for k in hdrmatch.keys():
                try:
                    if hdr[k] != hdrmatch[k]:
                        match = False
                        break
                except KeyError:
                    match = False
                    break

            if match:
                return dd[n]

        errstr = 'No matching file found' 
        raise AORetryException( code = AOConstants.AOErrCode['FILE_ERROR'], errstr = errstr)

    # A name is specified. Check the header

    if not os.path.exists( os.path.join( dir, name)):
        name += '.fits'
        if not os.path.exists( os.path.join( dir, name)):
            errstr = 'File %s does not exists' % name
            raise AORetryException( code = AOConstants.AOErrCode['FILE_ERROR'], errstr = errstr)

    hdr = fits_lib.readFitsHdr( os.path.join( dir, name))
    for k in hdrmatch.keys():
        try:
            if hdr[k] != hdrmatch[k]:
                errstr = 'File %s header error: %s is %s instead of %s' % (name, k, str(hdr[k]), str(hdrmatch[k]))
                raise AORetryException( code = AOConstants.AOErrCode['FILE_ERROR'], errstr = errstr)
        except KeyError, e:
            raise AORetryException( code = AOConstants.AOErrCode['FILE_ERROR'], errstr="File %s has incomplete FITS header: key %s missing!" % ( name, e))

    return name

    


def selectHODark( app, name=None):
    '''
    Selects the dark frame to use. It will select the last dark frame
    taken which matches the current binning and exposure time.
    Specifying <name> will override the search and select that dark frame instead,
    with a check on matching binning. <name> can be both a proper name or the timestamp.
    '''
    app.log('selectHODark(): starting', app.LOG_LEV_DEBUG)
    setupDevices.loadDevices( app, ['ccd39', 'sc'], check=True)
    hdr = {}
    hdr['HIERARCH ccd39.BINNING'] = app.ccd39.xbin()
    hdr['HIERARCH ccd39.FRAMERATE'] = '%5.2f' % app.ccd39.get_framerate()

    name = getCalibFile( calib.BackgDir('ccd39', app.ccd39.xbin()), hdrmatch = hdr, name = name)

    # When we are here, the file exists and matches current parameters! Go on

    app.sc.set_dark(name)
    app.log('selectHODark(): selected dark file %s' % name, app.LOG_LEV_INFO)

def selectTVDark( app, name=None):
    '''
    Selects the dark frame to use. It will select the last dark frame
    taken which matches the current binning and exposure time.
    Specifying <name> will override the search and select that dark frame instead,
    with a check on matching binning. <name> can be both a proper name or the timestamp.
    '''
    app.log('selectTVDark(): starting', app.LOG_LEV_DEBUG)
    setupDevices.loadDevices( app, ['ccd47', 'tv'], check=True)
    hdr = {}
    hdr['HIERARCH ccd47.BINNING'] = app.ccd47.xbin()
    hdr['HIERARCH ccd47.FRAMERATE'] = '%5.2f' % app.ccd47.get_framerate()

    name = getCalibFile( calib.BackgDir('ccd47', app.ccd47.xbin()), hdrmatch = hdr, name = name)

    # When we are here, the file exists and matches current parameters! Go on

    app.tv.set_dark(name)
    app.log('selectTVDark(): selected dark file %s' % name, app.LOG_LEV_INFO)


def selectSlopenull( app, name=None, matchLUT=True):
    '''
    Selects the slope null to use. It will select the last slopenull
    taken which matches the current binning and pupils.
    Specifying <name> will override the search and select that slopenull,
    with a check on matching binning and pupils. <name> can be both a proper name or the timestamp.

    If the optional parameter matchLUT is False, the procedure will not check that
    the LUT saved into the slopenull file matches the current one. This is useful for example for zero slopes.
    '''


    app.log('selectSlopenull(): starting', app.LOG_LEV_DEBUG)
    setupDevices.loadDevices( app, ['ccd39', 'sc'], check=True)
    hdr = {}   
    #hdr['HIERARCH ccd39.BINNING'] = app.ccd39.xbin()
    if matchLUT:
        hdr['HIERARCH sc.PUPILS'] = removePrefix( app.sc.get_pixellut(), calib.LUTsDir('ccd39'))[0:-7]

    name = getCalibFile( calib.slopenullDir(app.ccd39.xbin()), name = name)

    app.sc.set_slopenull(name)
    app.log('selectSlopenull(): selected slopenull %s' % name, app.LOG_LEV_INFO)



def towerOpticalSetup(app, setup=None):
    '''
    Optical setup of W#1 in test tower.
    Sequence of operations:

    1) Load the specified board setup (optional)
    2) Rotate filterwheels to "dark" positions
    3) Perform a ccd39 RON measurement
    4) Acquire a ccd39 dark
    5) Rotate filterwheels back to the previous position
    6) Autocenter and then autofocus stages
    7) Correct for cube rotator vignetting (todo)
    8) Autocenter again

    '''

    # DEPRECATED! DO not use anymore
    return

    setupDevices.loadDevices( app, ['fw1', 'fw2','ccd39'], check=True)

    if setup:
        boardSetup.load( app, setup)

    app.ccd39.set_framerate(197)
    calibHODarkInternal(app)

    centerPupils.centerStages( app)
    centerPupils.centerFocus( app)
    fixCubeVignetting( app)
    centerPupils.centerStages( app)


def fixCubeVignetting( app):

    # Cube original: 181.784

    movrange = 1.0  # Range to explore (centered on current position)
    step = 0.1      # Movement step
    numframes = 10  # Frames to average for each measure

    setupDevices.loadDevices( app, ['cuberot', 'ccd39'], check=True)

    start = app.cuberot.getPos()-movrange/2
    npos = int(movrange/step)

    d = [0]*npos
    p = [0]*npos
    for n in range(npos):
        pos = start+ n*step

        print 'Moving to ',pos
        app.cuberot.moveTo( pos, waitTimeout=10, accuracy = 0.1)
        time.sleep(1)   # Cube problem

        frame = frames_lib.averageFrames( app.ccd39.get_frames(numframes))
        curPupils = frames_lib.findPupils( frame, 0.20, 0.4, fourPupils=True, amoeba=True, binning = app.ccd39.xbin())
        app.log('fixCubeVignetting(): pos %5.3f diameter %5.3f' % (pos, curPupils[0].dy), app.LOG_LEV_DEBUG)
        d[n] = curPupils[0].dy     # Amoeba algorithm has separate x and y diameters
        p[n] = pos

    pos = p[ d.index(max(d))] 
    app.log('fixCubeVignetting(): selected cube position %5.3f' % pos, app.LOG_LEV_INFO)
    app.cuberot.moveTo(pos, waitTimeout=10)


######################################################
#
# arbFunc
#
# Wrapper for arbitrator-called functions with exception handlers and error returns.


def arbFunc( app, func, args):
    '''
    Wrapper for arbitrator-called functions with exception handlers and error returns.
    '''

    app.log('arbFunc %s %s' % (func.__name__, str(args)), app.LOG_LEV_DEBUG)

    try:
        errstr = func(*args)
        if errstr == None:
            errstr = ''

        if errstr != '':
            app.log('arbFunc %s done.' % func.__name__, app.LOG_LEV_DEBUG)
        else:
            app.log('arbFunc %s done.' % func.__name__, app.LOG_LEV_DEBUG)

        return [ "OK", errstr]

    except AORetryException, e:
        aoretry = "RETRY: %s (%d) %s" % (e.errstr, e.code, AOConstants.AOErrType[e.code])
        errstr = aoretry + "\n" + traceback.format_exc()
        app.log( errstr, app.LOG_LEV_ERROR)
        return [ "RETRY", aoretry]

    except AdOptError, e:
        aoretry = "%s (%d) %s" % (e.errstr, e.code, AOConstants.AOErrType[e.code])
        errstr = aoretry + "\n" + traceback.format_exc()
        app.log(errstr, app.LOG_LEV_ERROR)
        return [ "ERROR", aoretry]

    except Exception, e:
        errstr = traceback.format_exc()
        app.log(errstr, app.LOG_LEV_ERROR)
        return [ "ERROR", str(e)]

    except BaseException, e:
        errstr = traceback.format_exc()
        app.log(errstr, app.LOG_LEV_ERROR)
        return [ "ERROR", str(e)]

    app.log('Serious error: out of valid code path!!', app.LOG_LEV_ERROR)
    return [ "ERROR", "Unknown error"]


################################################################
# State change functions called directly from the WFS arbitrator


def initArb(app):
    '''
    Arbitrator initialization
    '''
    initStagePos(app)

def setPowerOff(app, sensor, instr):
    '''
    Goes into PowerOff state, safely turning off the system.
    '''

    return arbFunc( app, TurnSystemOff, (app, sensor, instr))


def setOperating( app, sensor, instr, config="", boardSetup="", opticalSetup=False):
    '''
    Goes into Operating state, 
    '''

    try:
        return arbFunc( app, TurnSystemOn, (app, sensor, instr, config, boardSetup, opticalSetup))
    except Exception, e:
        app.log('Serious error: non-trapped exception in arbFunc()', app.LOG_LEV_ERROR)
        app.log(str(e), app.LOG_LEV_ERROR)
        print e

def prepareAcquireRef( app, sensor, instr, mode, starxpos, starypos, telElevation, derotAngle, starMag, starColor, r0, v0):
    '''
    Prepare for reference start acquisition: move stages and setup ccd.
    '''
    return arbFunc( app, doPrepareAcquireRef, (app, sensor, instr, mode, starxpos, starypos, telElevation, derotAngle, starMag, starColor, r0, v0))

def acquireRef( app, sensor, instr):
    '''
    Acquire reference source and prepare for closed loop operation.
    '''
    return arbFunc( app, doAcquireRef, (app, sensor, instr))

def enableDisturb( app, sensor, instr, wfs=False, ovs=False):
    '''
    Enable/disable command disturbance on AdSec.
    '''
    return arbFunc( app, doEnableDisturb, (app, sensor, instr, wfs, ovs))

def saveStatus( app, sensor, instr, outputFile):
    '''
    Acquire reference source and prepare for closed loop operation.
    '''
    return arbFunc( app, doSaveStatus, (app, sensor, instr, outputFile))

def closeLoop(app, sensor, instr):
    '''
    Closes the AO loop.
    '''
    return arbFunc( app, doCloseLoop, (app, sensor, instr))

def getTVSnap(app, sensor, instr):
    '''
    Gets an image from the Technical viewer.
    '''
    return arbFunc( app, doGetTVSnap, (app, sensor, instr))

def checkRef(app, sensor, instr):
    '''
    Checks the reference star position
    '''
    return arbFunc( app, doCheckRef, (app, sensor, instr))

def offsetXY(app, sensor, instr, offsetx, offsety, brake):
    '''
    Offsets the XY stages
    '''
    return arbFunc( app, offsetXYinternal, (app, sensor, instr, offsetx, offsety, brake))

def offsetZ(app, sensor, instr, offsetz):
    '''
    Offsets the Z stage
    '''
    return arbFunc( app, offsetZinternal, (app, sensor, instr, offsetz))

def optimizeGain(app, sensor, instr):
    '''
    Run the gain optimization procedure
    '''
    return arbFunc( app, optimizeGainInternal, (app, sensor, instr))

def correctModes(app, sensor, instr, modes):
    '''
    Correct higher-order modes
    '''
    return arbFunc( app, correctModesInternal, (app, sensor, instr, modes))

def emergencyOff(app, sensor, instr):
    '''
    Immediate power off
    '''
    return arbFunc( app, emergencyOffInternal, (app, sensor, instr))

def pauseLoop(app, sensor, instr):
    '''
    Pauses the AO loop
    '''
    return arbFunc( app, doPauseLoop, (app, sensor, instr))


def resumeLoop(app, sensor, instr):
    '''
    Resumes a paused AO loop.
    '''
    return arbFunc( app, doResumeLoop, (app, sensor, instr))

def stopLoop(app, sensor, instr):
    '''
    Stops an AO loop.
    '''
    return arbFunc( app, doStopLoop, (app, sensor, instr))


#####################################################
# Non-state change functions

###################################
# Policy functions

def setHOarmPolicy( app, ccd39=None, adc=None, tt=None, fw1=None, rerot=None):
    '''
    Sets the HO arm auto/manual policy.
    '''
    return arbFunc( app, setHOarmPolicyInternal, (app, ccd39, adc, tt, fw1, rerot))


def setTVarmPolicy( app, ccd47=None, fw2=None):
    '''
    Sets the TV arm auto/manual policy.
    '''
    return arbFunc( app, setTVarmPolicyInternal, (app, ccd47, fw))

def setPointingPolicy( app, stages=None, cube=None, lamp=None):
    '''
    Sets the Pointing auto/manual policy.
    '''
    return arbFunc( app, setPointingPolicyInternal, (app, stages, cube, lamp))

##############################
# Calibration functions

def setSource(app, sensor, instr, config, magnitude):
    '''
    Sets the source to use (reference or telescope).
    '''
    return arbFunc( app, setSourceInternal, (app, sensor, instr, config, magnitude))

def calibSlopenull(app, sensor, instr, name=None, num=1000, apply=True):
    '''
    Acquires a slope null using the current pupils, binning etc. averaging <num> frames.
    Results are saved in $ADOPT_ROOT/calib/WFS/slopenulls with a timestamp.
    If a name is provided, a link with the specified name will be created too.
    If apply is True, the new slope null will be immediately applied to the system.
    '''
    return arbFunc(app, calibSlopeNullInternal, (app, sensor, instr, name, num, apply))

def calibHODark(app, sensor, instr, name=None, num=100, apply=True):
    '''
    Acquires a dark frame using the current binning, exposure time etc. averaging <num> frames.
    Results are saved in $ADOPT_ROOT/calib/WFS/backgrounds with a timestamp.
    If a name is provided, a link with the specified name will be created too.
    If apply is True, the new dark will be immediately applied to the system.
    '''
    return arbFunc(app, calibHODarkInternal, (app, sensor, instr, name, num, apply))

def calibTVDark(app, sensor, instr, name=None, num=10, apply=True):
    return arbFunc(app, calibTVDarkInternal, (app, sensor, instr, name, num, apply))

def calibIRTCDark(app, sensor, instr, name=None, num=10, apply=True, ccd='irtc'):
    return arbFunc(app, calibIRTCDarkInternal, (app, sensor, instr, name, num, apply, ccd))

def savePsf( app, sensor, instr, filename, num = 50):
    '''
    Save a ccd47 averaged frame into the specified filename.
    Format will be a float FITS file.
    '''
    return arbFunc( app, savePsfInternal, (app, sensor, instr, filename, num))

def saveIrtc( app, sensor, instr, filename, num = 50, ccd='irtc'):
    '''
    Save an IRTC cube into the specified filename.
    Format will be an integer FITS file.
    '''
    return arbFunc( app, saveIrtcInternal, (app, sensor, instr, filename, num, ccd))


def savePisces( app, sensor, instr, filename, num = 50, ccd='pisces'):
    '''
    Save an PISCES cube into the specified filename.
    Format will be an integer FITS file.
    '''
    return arbFunc( app, saveIrtcInternal, (app, sensor, instr, filename, num, ccd))



#@Procedure: calibMovements
#
# Calibrates all mechanical movements in the WFS.
#@

def calibMovements(app, sensor, instr):

    return arbFunc(app, calibMovementsInternal, (app, sensor, instr))



def modifyAO(app, sensor, instr, freq = None, binning = None, TTmod = None, Fw1Pos = None, Fw2Pos = None, decimation=0, checkCameralens = False):
    '''
    Changes AO loop parameters after a star has been acquired but before closing the loop.
    For all values, -1 means unchanged from current value.
    If <checkCameralens> is True, the camera lens position is checked and corrected before applying the settings.
    '''
    return arbFunc( app, modifyAOInternal, (app, sensor, instr, freq, binning, TTmod, Fw1Pos, Fw2Pos, decimation, checkCameralens ))

def refineLoop( app, sensor, instr, params):
    '''
    Changes loop parameters while the loop is closed
    '''
    pass

def emergencyStop( app, sensor, instr):
    '''
    Call to quickly interrupt whatever operations was in progress.
    '''
    pass



