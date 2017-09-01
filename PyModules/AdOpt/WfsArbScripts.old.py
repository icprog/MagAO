#@File: WfsArbScripts.py
#
# Non-interactive scripts for the WFS arbitrator
#
#@

import time, os, math, types
import threading
import traceback

import numpy.numarray as numarray
from AdOpt import cfg, calib, setupDevices, processControl, fits_lib, AOConstants, frames_lib, AOVar
from AdOpt import boardSetup, centerPupils, source_acq
from AdOpt.InterpolatedArray import *
from AdOpt.AOExcept import *


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
    aoapp.log('Linking %s to %s' % (cur_link, dir + pupilReg), aoapp.LOG_LEV_DEBUG)
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

    if aoapp.ccd39.getStatus() == AOConstants.AOStatesCode['STATE_READY']:
        print 'Configuring CCD, please wait...'
        aoapp.ccd39.reprogram( binning, binning, speeds[binning])
        #time.sleep(1)
        #aoapp.ccd39.set_default_black()
    else:
        raise AdOptError( errstr = "CCD39 NOT READY")

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

    # Restart ccd at 1Khz
    aoapp.ccd39.set_framerate(197)
    aoapp.ccd39.start()

    time.sleep(0.1)
    aoapp.tt.set(amp=0, freq=0)

    # Restart pupil check activity
    processControl.startProcessByName( 'pupilcheck39')

    
    aoapp.log('ccd39 change binning done', aoapp.LOG_LEV_INFO)

#@Procedure: setTVbinning
#
# Sets the TV binning to a new value.
# Since the BCU47 is resetted during the binning change, tip-tilt and cameralens positions
# are saved, if possible, and re-set again when the BCU is up.
#@

def setTVbinning( aoapp,            #@P{aoapp}: calling AOApp object
                  binning,          #@P{binning}: New binning (integer, 1,2,4 or 16)
                  force=False):      #@P{force}: if False, avoid reconfiguring CCD and BCU if things are already at the right binning.
    '''
    Sets the Technical viewer binning.
    '''


    #############################
    # Configuration area

    # Subframe
    #speeds = { 1:2500, 2:2500, 3:2500, 4:2500, 16:2500 }

    # Binned
    speeds = { 1:250, 2:250, 3:250, 4:250, 16:250 }

    # End of configuration area
    ##############################

    setupDevices.loadDevices( aoapp, ['ccd47', 'tv'], check=True)

    if not force:
        if aoapp.ccd47.xbin() == binning:
            return

    # Save tt and cameralens status, if available
    doTT = False
    if hasattr(aoapp,'tt'):
        if aoapp.tt.isOnline():
            doTT = True
            tt_offx = aoapp.tt.offx()
            tt_offy = aoapp.tt.offy()
            tt_amp = aoapp.tt.amp()
            tt_freq = aoapp.tt.freq()
    doCameralens = False
    if hasattr(aoapp,'lens'):
        if aoapp.lens.isOnline():
            doCameralens = True
            cameralens_posx = aoapp.lens.posx()
            cameralens_posy = aoapp.lens.posy()


    # Stop everything
    aoapp.ccd47.stop()
    aoapp.tv.stop()

   # Reset BCU47
    if 1:
        aoapp.power.turnon('prog47')
        aoapp.power.turnon('reset47')
        time.sleep(0.5) # Probably not needed
        aoapp.power.turnoff('reset47')

        print "Waiting for restart"

        aoapp.tv.waitStatus( AOConstants.AOStatesCode['STATE_NOCONNECTION'], waitTimeout=20)
        aoapp.tv.waitStatus( AOConstants.AOStatesCode['STATE_READY'], waitTimeout=20)

        print "BCU47 OK"


    # Get a new LUT
    dir = calib.LUTsDir( 'ccd47', binning) 

    # Link the new GUI tables
    cur_link = calib.CurLUTlink( 'ccd47')
    if os.access(cur_link, os.F_OK):
        os.unlink(cur_link)
    os.symlink( dir, cur_link)

    n_pixels = 1024*1024 / (binning*binning)

    # Reset WFS
    aoapp.tv.set_npixels( n_pixels)
    aoapp.tv.set_pixellut( calib.CurBcuLUT( 'ccd47'))


    # Reconfig CCD with new binning - also causes the GUI to reload the tables
    if aoapp.ccd47.getStatus() == AOConstants.AOStatesCode['STATE_READY']:
        aoapp.ccd47.reprogram( binning, binning, speeds[binning])
    else:
        raise AdOptError( errstr = "CCD47 NOT READY")

    # Restart frame grabber and ccd
    aoapp.tv.start()
    aoapp.ccd47.start()

    # Restore TT and Cameralens
    if doTT:
	aoapp.tt.set( amp = tt_amp, freq = tt_freq, offx = tt_offx, offy = tt_offy) 
    if doCameralens:
	aoapp.lens.moveTo( cameralens_posx, cameralens_posy)
	

    aoapp.log('ccd47 change binning done', aoapp.LOG_LEV_INFO)


#@Procedure: equalizeCcd39
#
#   Equalize the luminosity level of the four ccd39 quadrants.
#   Takes care of turning off the lamp if needed.
#@

def equalizeCcd39( app):
    '''
    Equalize the luminosity level of the four ccd39 quadrants.
    Takes care of turning off the lamp if needed.
    '''

    app.log("Starting ccd39 equalization.")

    setupDevices.loadDevices( app, ['power', 'ccd39'], check = True)

    if app.power.is_on('lamp'):
        setupDevices.loadDevices( app, ['lamp'], check = True)
        app.lamp.setIntensity(0, waitTimeout=50)
    app.ccd39.start()

    try:
        app.ccd39.equalize_quadrants()
    except AdOptError, e:
        app.log(e.errstr, app.LOG_LEV_ERROR)
    except Exception, e:
        app.log(str(e), app.LOG_LEV_ERROR)


    app.log('ccd39 equalization done', app.LOG_LEV_INFO)


#@Procedure: equalizeCcd47
#
#   Equalize the luminosity level of the two ccd47 quadrants.
#
#   Takes care of turning off the lamp if needed. Fw number 2
#   is set to a beam blocking position and moved back to the
#   original position when equalizing is done.
#@

def equalizeCcd47( app):
    '''
    Equalize the luminosity level of the two ccd47 quadrants.
    Uses the filter wheel #2 to mask out any light from the system
    '''

    app.log("Starting ccd47 equalization.", app.LOG_LEV_DEBUG)

    setupDevices.loadDevices( app, ['fw2', 'ccd47'], check = True)

    fw2pos = app.fw2.getPos()
    app.fw2.moveTo(0.5, waitTimeout=10)

    app.ccd47.start()

    try:
        app.ccd47.equalize_quadrants()
    except AdOptError, e:
        app.log(e.errstr, app.LOG_LEV_ERROR)
    except Exception, e:
        app.log(str(e), app.LOG_LEV_ERROR)

    app.fw2.moveTo( fw2pos, waitTimeout=10)

    app.log('ccd47 equalization done', app.LOG_LEV_INFO)




def test(app):
    print 'ciao'
    return ["RETRY", "Test OK"]


def getSetupFits(app, movements = None, devices = None):
    '''
    Returns a tuple with:
    - a dictionary containing the current system setup,
    - an error string about the items that could not be queries.

    The dictionary is suitable to be saved into a FITS file.

    <movements> and <devices> are lists of devices to be saved. If not specified,
    a default list (the complete system) will be used.
    '''

    setupDevices.loadDevices( app, ['power','fw1','fw2','adc','ccd39','ccd47','rerot','cuberot','cubestage','stagex','stagey','stagez','tt','lamp','lens'], check=False)

    if movements == None:
        movements = ['fw1', 'fw2', 'rerot', 'cuberot', 'cubestage', 'stagex', 'stagey', 'stagez']

    if devices == None:
        devices = ['lamp', 'ccd39', 'tt', 'cameralens', 'sc', 'ccd47', 'irtc']


    hdr = {}
    errstr = ''
    invalid = '-9999'
    status_invalid = 'UNKNOWN'

    hdr['W_UNIT'] = cfg.wfs_name

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

    if 'cameralens' in devices:
        status = status_invalid
        posx = invalid
        posy = invalid
        try:
            status = AOConstants.AOStatesType[ app.lens.getStatus() ]
            posx = float('%5.2f' % app.lens.posx())
            posy = float('%5.2f' % app.lens.posy())
        except AdOptError, e:
            errstr += 'Could not save lens status. Reason:\n' +str(e) + '\n'
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

    # Get information from telescope, if any available
    try:
        exclude = ['ERRMSG', 'STATUS'] 
        telescope = app.ReadVarList("R.AOS*")
        now = time.time()
        timeout = 120 # Max accepted age in seconds
        for var in telescope:
            name = var.Name()[6:]
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
        app.log(errstr, app.LOG_LEV_DEBUG)
        app.log('getSetupFits(): end of error string.', app.LOG_LEV_DEBUG)

    return (hdr, errstr)

def removePrefix( str, prefix = ''):
    if str[0:len(prefix)] == prefix:
        return str[len(prefix):]

#@Procedure: TurnSystemOff
#
# Turns the WFS system safely off.
#@

def TurnSystemOff( app):

    app.log('Turning WFS off', app.LOG_LEV_INFO)

    setupDevices.loadDevices( app, ['power', 'stagex', 'stagey', 'stagez'], check = True)

    # Brake stages before removing power
    try:
        app.stagex.enable(False)
        app.stagey.enable(False)
        app.stagez.enable(False)
        time.sleep(3)		# Brake has a delay of 2 seconds.
    except:
        # Errors may happen if the stages weren't alrady on. We ignore these kind of errors.
        pass

    exclude = ['fans_LJ', 'boxfans', 'main', 'power', 'flowerpot', 'ps1', 'lamp', 'cuberot', 'cubestage']
    for device in app.power.list():
        if not device in exclude:
            try:
                app.power.turnoff(device, force = True)
            except:
                # Exceptions here are not fatal since we can turn off the main power afterwards
                pass

    app.power.turnoff('fans_LJ', force = True)
    app.power.turnoff('flowerpot', force = True)

    # W#1
    if 'main' in app.power.list():
    	app.power.turnoff('main', force = True)

    # W2
    if 'ps1' in app.power.list():
    	app.power.turnoff('ps1', force = True)

    app.power.turnoff('boxfans', force = True)

    app.log('All devices turned off !', app.LOG_LEV_INFO)

    # W#1 can wait for bcu39 shutdown, W#2 cannot.
    if cfg.wfs_name == 'W1':
        app.log('Waiting for network shutdown...', app.LOG_LEV_INFO)
        setupDevices.waitIPhost( app, 'bcu39', waitTimeout = 60, offline = True)


#@Procedure: TurnSystemOn
#
# Turns the system on from a complete shutdown or cold start.
#
# If the system was already on, most of the instructions
# (not all, especially the CCDs and BCU47) are actually NOPs.
#@

def TurnSystemOn(app,     #@P{app}: calling aoapp object
                 config,  #@P{config}: requested Operate configuration (string)
                 boardSetup, #P{boardSetup}: requested board setup (string, optional, use empty string if none needed)
                 opticalSetup): #P{opticalSetup}: perform optical setup (bool)

    startTime = time.time()

    app.log('Setting configuration '+config, app.LOG_LEV_INFO)

    try:
        devices = cfg.wfs_configs[ config]
    except KeyError, e:
        raise AdOptError( errstr = 'Unknown configuration string: "%s"' % config)

    app.log('Turning WFS on', app.LOG_LEV_INFO)
    app.log("Devices: "+ str(devices), app.LOG_LEV_DEBUG)

    try:

        # Setup needed modules
        app.log('Initializing Python modules...', app.LOG_LEV_DEBUG)

        app.log(str(devices))

        setupDevices.loadDevices(app)

        app.log('Waiting for network connection', app.LOG_LEV_INFO)


        setup = setupDevices.get()
        faults, errstr = setup.setup( app, devices)

        if 'ccd39' in devices:
            app.log('Equalizing ccd39', app.LOG_LEV_INFO)
            try:
                equalizeCcd39(app)
            except AdOptError, e:
                app.log( e.errstr, app.LOG_LEV_ERROR)
                faults.append('CCD39 equalization')
                errstr += '\nError during ccd39 equalization: %s' % e.errstr

        if 'ccd47' in devices:
            app.log('Equalizing ccd47', app.LOG_LEV_INFO)
            try:
                equalizeCcd47(app)
            except AdOptError, e:
                app.log( e.errstr)
                faults.append('CCD47 equalization')
                errstr += '\nError during ccd47 equalization: %s' % e.errstr

            # Start regular ccd47 image
            app.ccd47Timer = threading.Timer( 1, ccd47TimerHandler, args=[app])
            app.ccd47Timer.start()

        # Raise lamp intensity
        if 'lamp' in devices:
            try:
                app.log('Raising lamp', app.LOG_LEV_INFO)
                app.lamp.setIntensity(35, waitTimeout=50)
            except AdOptError, e:
                app.log( e.errstr)
                faults.append('Lamp raising')
                errstr += '\nError while raising lamp: %s' % e.errstr

        app.log('done lamp', app.LOG_LEV_DEBUG)

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


    return ret

#@Function: ccd47TimerHandler
#
# Handles the regular ccd47 image download and set into the RTDB
#@

def ccd47TimerHandler(app):

    try:
        #doGetTVSnap(app, saveInRTDB=True)
        time.sleep(1)
    except:
        pass

    # Star the timer again
    app.ccd47Timer = threading.Timer( 1, ccd47TimerHandler, args=[app])
    app.ccd47Timer.start()


#@Function: packCcd47Image
#
# Packs a ccd47 image into a string, suitable to be saved into the RTDB.
#@

def packCcd47Image(image):

    # Pack the TV image in a string, rescaled to 0-255 and 256x256.
    import struct
    fmt = 'ii65536s'
    
    str = (frames_lib.rebin(image, 256, 256) /64).astype('UInt8').tostring()
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


def doPrepareAcquireRef(app, instr='',         #@P{instr}: instrument, for example 'IRTC' or 'LUCIFER')
                             mode='',          #@P{mode}: either 'TTM-AO' or 'ACE-AO'
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

    app._acquireRefSettings = computeAOsettings( app, instr, mode, starMag, starColor, r0, v0)

    setupDevices.loadDevices( app, ['stagex', 'stagey', 'stagez', 'adc', 'rerot', 'fw1', 'fw2', 'ccd39', 'ccd47', 'lens', 'power'], check=True)

    fov = calib.readFov( allowFail = False)
    fov_xc = fov['xc']
    fov_yc = fov['yc']

    # Check that star position is inside FoV
    xpos = fov_xc + starxpos
    ypos = fov_yc + starypos

    if (xpos < fov['xmin']) or (xpos > fov['xmax']) or (ypos < fov['ymin']) or (ypos > fov['ymax']):
        str = 'Star position outside limits: position %f,%f  Limits: from %f,%f to %f,%f' % (starxpos, starypos, fov['xmin']-fov_xc,  fov['ymin']-fov_yc, fov['xmax']-fov_xc, fov['ymax']-fov_yc)
        raise AORetryException( code = AOConstants.AOErrCode['VALUE_OUT_OF_RANGE_ERROR'], errstr = str)


    if 0:
        return '%f %d %f %d %d %f %s %s %s %s %d' % (app._acquireRefSettings.HOfreq,
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

    app.stagex.enable( True, onaftermove=True)
    app.stagey.enable( True, onaftermove=True)
    app.stagez.enable( True, onaftermove=True)

    # Center camera lens (will move cube and all three stages)
#    centerCameralens(app)

    # Load the board setup for this instrument
    setupFile = calib.setupsDir() + instr
    boardSetup.load( app, setupFile)

    # Move stages to target position
    app.stagex.moveTo( xpos)
    app.stagey.moveTo( ypos)

    # Move ADC and rerotator to predicted elevation/derotator angle
    # [TODO] Move all this to Wfs Arb
    #app.adc.moveTo( telElevation)
    #app.adc.setTracking( False)
    #app.adc.setDerotPos( derotAngle)
    #app.rerot.setTracking( False)
    #app.rerot.setDerotPos( derotAngle)
    #app.lens.setTracking( False)

    # Prepare fw1 and fw2
    app.fw1.moveTo( app._acquireRefSettings.fw1_acq)
    app.fw2.moveTo( app._acquireRefSettings.fw2_acq)

    # Set CCDs - blocks here until they are ready
    
    setTVbinning( app, app._acquireRefSettings.TVbin)
    app.ccd47.set_framerate( app._acquireRefSettings.TVfreq)

    decimation= computeDecimation( app._acquireRefSettings.HOfreq)
    doSetLoopParams( app, app._acquireRefSettings.HOfreq, decimation,  app._acquireRefSettings.HObin,  app._acquireRefSettings.TTamp, takeDark=False)

    selectSlopenull(app, app._acquireRefSettings.slopenull, matchLUT=False)

    # Wait for movement completion - blocks until everything is ready
    app.stagex.waitTargetReached()
    app.stagey.waitTargetReached()
    app.fw1.waitTargetReached()
    app.fw2.waitTargetReached()
    app.adc.waitTargetReached()
    app.rerot.waitTargetReached()

    # Return an ASCII string with output parameters.
    return '%f %d %f %d %d %f %s %s %s %s %d' % (app._acquireRefSettings.HOfreq,
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
    

#@Function: computeDecimation
#
# Computes the needed decimation for the given CCD frequency.
#
# This computation is repeated in WfsArbitrator/WfsInterface.cpp
#@

def computeDecimation(freq):
    master_speed = 400.1
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
                       mode,       #@P{mode}: AO mode (either 'TTM-AO' or 'ACE-AO')
                       starMag,    #@P{starMag}: reference star magnitude
                       starColor,  #@P{starColor}: reference star B-V color
                       r0= 0.0,    #@P{r0}: r0 value in mm
                       v0= 0.0):   #@P{v0}: wind speed in m/s
                                   #@R: settings object (see function header)

    instr = str(instr)
    mode = str(mode)
    starMag = float(starMag)

    tablefile = calib.getAOParamsFile( instr, mode)
    if not os.path.exists(tablefile):
       raise  AORetryException( code = AOConstants.AOErrCode['WFSARB_ARG_ERROR'], errstr = 'No settings for %s - %s' % (instr, mode))

    app.log('computeAOsettings(): using table: %s' % tablefile, app.LOG_LEV_DEBUG)
    app.log('computeAOsettings(): requested magnitude is %f' % starMag, app.LOG_LEV_DEBUG)
    table = file( tablefile).readlines()

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
    minimumMag = float(tablelines[0][0])
    if starMag < minimumMag:
        raise AORetryException( code = AOConstants.AOErrCode['VALUE_OUT_OF_RANGE_ERROR'], errstr = 'Star too bright (estimated mag: %5.1f). Minimum magnitude is %5.1f' % (starMag, minimumMag))

    for params in tablelines:

        mag = float(params[0])

        # Find the first line with a magnitude > star mag
        if mag < starMag:
            continue

        # Line OK. Extract data

        class obj: pass

        app.log(params, app.LOG_LEV_DEBUG)

        settings = obj()
        settings.fw1_acq = int(params[12])
        settings.fw2_acq = int(params[13])
        settings.fw1_obs = int(params[14])
        settings.fw2_obs = int(params[15])
        settings.TVbin = int(params[16])
        settings.TVfreq = float(params[17])
        settings.TVavg = int(params[18])
        settings.TVflux = int(params[19])
        settings.HObin = int(params[1])
        settings.HOfreq = float(params[2])
        settings.nModes = int(params[4])
        settings.HOflux = float(params[3])
        settings.base   = params[5]
        settings.rec = params[6]
        settings.filtering = params[7]
        settings.pupils = params[8]
        settings.slopenull = params[9]
        settings.TTamp = float(params[10])
        settings.gain = float(params[11])

        # Duplicate original parameters
        settings.instr    = instr
        settings.mode     = mode
        settings.starMag  = starMag
        settings.starColor  = starColor
        settings.r0  = r0
        settings.v0  = v0

        if (r0 == 0) or (r0 == None):
            settings.seeing = 0.6
        else:
            settings.seeing = 251 * 0.8 / r0

        return settings

    # If we arrive here, not line was found for our magnitude!

    raise AORetryException( code = AOConstants.AOErrCode['VALUE_OUT_OF_RANGE_ERROR'], errstr = 'Star too faint for mode %s. Maximum magnitude is %f' % (mode, mag))


#@Procedure: doCheckRef
#
# Checks the reference star positions, as it would be done by AcquireRef.
#
#@

def doCheckRef(app):

    setupDevices.loadDevices( app, ['fw1', 'fw2', 'ccd47'], check=True)

    # Resume automatic movements
    # [TODO] Move to Wfs ARB
    #setAdcTracking( app, True)
    #setRerotTracking( app, True)

    if 0:
	    return '%f %f %f' % (1, 2, 10)


    # Acquire ccd47 dark
    calibTVDarkInternal(app)

    # Check star on ccd47
    acqRet = source_acq.acquire(app, TVavg = app._acquireRefSettings.TVavg, TVflux = app._acquireRefSettings.TVflux, getError = True)

    return '%f %f %f' % (acqRet.deltaX, acqRet.deltaY, acqRet.mag)

#@Function: ccd39Mag
#
# Estimate magnitude acquiring frames on ccd39.
# If the <frame> parameter is valid, it is assumed to be a frame sequence just acquired.
# Current information about ccd39 and other devices is used when calculating magnitude, so 
# the device state must not be changed in between.
# If the <frame> parameter is None, then a new series of frames will be acquired (optionally, <nframes> can specify the exact length).
#@

def ccd39Mag( app, frame=None, nframes=None):

    setupDevices.loadDevices( app, ['ccd39', 'fw1'], check=True)
    acqData = calib.loadCalibData( calib.sourceAcqCalibFile(), check=['counts_mag10_ccd39'])

    if not frame:
        if not nframes:
            nframes=50
        frame = app.ccd39.get_frames( nframes, average=True, useFloat=True)

    filter = round(app.fw1.getPos())

    # Total counts over the frame (assume background is good....)
    counts = frame.sum()

    # Correct for filter position
    if filter == 2:
        counts *=2
    if filter == 4:
        counts *= 1.04
    
    c = counts * app.ccd39.get_framerate()
    if c <=0:
        raise AORetryException(errstr = 'Magnitude calculation cannot take place with negative counts')

    correction = 0.5 ### Because we always estimate 0.5 magnitudes brighter!!!!

    return -2.5 * math.log10( c / acqData['counts_mag10_ccd39']) + 10.5 + correction



#@Procedure: doAcquireRef
#
#    Acquire reference source and prepare for closed loop operation.
#    No input parametrs. Assumes that the star is already present on ccd47
#    thanks to a previous prepareAcquireRef() command.
#@


def doAcquireRef(app):

    # Return a string with output parameters plus the TV image - not a C string, the image may contain zero bytes
    framestr = '0' * (256*256)
    if 0:
        return '%f %f %f %d %d %f %d %d %f %s %s %s %s %f %d IMAGE%s' % ( 0.0, 0.0,
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
                                              len(framestr),
                                              framestr)

    setupDevices.loadDevices( app, ['fw1', 'fw2', 'ccd39', 'stagex', 'stagey', 'stagez'], check=True)

    acqData = calib.loadCalibData( calib.sourceAcqCalibFile(), check=['counts_mag10_ccd39', 'magError'])

    # Resume automatic movements
    # [TODO] Move to Wfs Arb
    #setAdcTracking( app, True)
    #setRerotTracking( app, True)

    # Turn off stages
    app.stagex.enable(False)
    app.stagey.enable(False)
    app.stagez.enable(False)
    time.sleep(3)   # Brake delay time

    # Acquire darks
    calibTVDarkInternal(app)

    # New code with one thread per dark
    t = threading.Thread( target = calibTVDarkInternal, args=(app,))
    t.start()
    calibHODarkInternal(app, num=1000)
    t.join()

    # Re-set wheels (in case this is a repeated AcquireRef)
    app.fw1.moveTo( app._acquireRefSettings.fw1_acq)
    app.fw2.moveTo( app._acquireRefSettings.fw2_acq)
    app.fw1.waitTargetReached(checkStatus=True)
    app.fw2.waitTargetReached(checkStatus=True)


    # Center star on ccd47
    acqRet = source_acq.acquire(app, TVavg = app._acquireRefSettings.TVavg, TVflux = app._acquireRefSettings.TVflux, fwhm = app._acquireRefSettings.seeing)

    # Turn off stages
    app.stagex.enable(False)
    app.stagey.enable(False)
    app.stagez.enable(False)

    # Set filterwheels to observing positions
    app.fw1.moveTo( app._acquireRefSettings.fw1_obs)
    app.fw2.moveTo( app._acquireRefSettings.fw2_obs)
    app.fw1.waitTargetReached(checkStatus=True)
    app.fw2.waitTargetReached(checkStatus=True)

    # Calculate actual magnitude from ccd39 counts
    mag = ccd39Mag( app, nframes=10)
    app.log('Estimated magnitude from ccd39: %f' % mag, app.LOG_LEV_DEBUG)

    # If new magnitude too different from the expected one, reconfigure things
    if abs(mag- app._acquireRefSettings.starMag) > acqData['magError']:
        app.log('Expected magnitude %f - actual %f - Recomputing AO params' % (app._acquireRefSettings.starMag, mag), app.LOG_LEV_INFO)
        app._acquireRefSettings = computeAOsettings( app, app._acquireRefSettings.instr, app._acquireRefSettings.mode, mag, app._acquireRefSettings.starColor, app._acquireRefSettings.r0, app._acquireRefSettings.v0)
        setHObinning( app, app._acquireRefSettings.HObin, pupilReg=app._acquireRefSettings.pupils)
        app.ccd39.set_framerate( app._acquireRefSettings.HOfreq)
        calibHODarkInternal(app)
        selectSlopenull(app, app._acquireRefSettings.slopenull, matchLUT=False)
        app.tt.set( amp = app.tt.modulationLambdaDF2V( app._acquireRefSettings.TTamp, app._acquireRefSettings.HOfreq), offx=0, offy=0, freq=0)


    framestr = packCcd47Image( acqRet.TVframe)

    # Return a string with output parameters plus the TV image - not a C string, the image may contain zero bytes
    return '%f %f %f %d %d %f %d %d %f %s %s %s %s %f %d IMAGE%s' % (acqRet.deltaX, acqRet.deltaY,
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
                                              len(framestr),
                                              framestr)

def doGetTVSnap(app, saveInRTDB=True, useImage=None):

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
            varname = cfg.side+'.WFS.TV_IMAGE'
            var = AOVar.AOVar( varname, tipo='CHAR_VARIABLE', value='')
            var.SetValue(framestr)
            app.WriteVar(var)
        else: 
            # Return a string with the TV image - not a C string, the image may contain zero bytes
            return '1 %d IMAGE%s' % (len(framestr), framestr)

    except:
        pass

        

def doSaveStatus( app, outputFile):
    '''
    Saves the WFS status into a FITS file with dummy data.
    '''

    hdr, errstr = getSetupFits(app)
    dummy = numarray.zeros(())
    fits_lib.writeSimpleFits( outputFile, dummy, hdr, overwrite=True)

def doEnableDisturb( app, wfs=False, ovs=False):
    '''
    Enables/disables the AdSec disturbance on wfs and ovs frames.
    '''
    setupDevices.loadDevices( app, ['sc'], check=True)
    app.sc.enable_disturb( wfs, ovs)

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



   
def doCloseLoop( app, nocheck = False):
    '''
    Closes the fastlink fiber.
    Also checks whether the ccd39 dark frame seems OK. Will retake the dark frame if not valid.
    '''
    setupDevices.loadDevices( app, ['ccd39','sc'], check=True)

    # Check ccd39 dark

    if not nocheck:
        if not testDark(app):
            app.log('Dark subtraction error. Retaking dark...', app.LOG_LEV_INFO)
            calibHODarkInternal(app)
            if not testDark(app):
                raise AORetryException(0, "Dark subtraction error")

    app.sc.fl_start()


def doSetLoopFreq( app, freq, decimation):
    '''
    Changes the loop speed.
    '''
    setupDevices.loadDevices( app, ['ccd39', 'sc'], check=True)

    app.ccd39.stop()
    app.sc.set_masterd_decimation( decimation)
    app.ccd39.set_framerate(freq)
    app.ccd39.start() 

def refineAOInternal( app, freq, binning, TTmod, Fw1Pos, Fw2Pos, decimation, checkCameralens):
    '''
    Changes the AO loop parameters just before closing the loop.
    For all parameters, -1 means unchanged from current value.
    '''

    if Fw1Pos>=0:
        setupDevices.loadDevices( app, ['fw1'], check=True)
        app.fw1.moveTo(Fw1Pos, waitTimeout=20)
    if Fw2Pos>=0:
        setupDevices.loadDevices( app, ['fw2'], check=True)
        app.fw2.moveTo(Fw2Pos, waitTimeout=20)

    if checkCameralens:
        centerCameralens(app)

    doSetLoopParams( app, freq, decimation, binning, TTmod)

def offsetZinternal( app, offsetz):
    '''
    Offset the Z stage. Only useful in closed loop.
    '''

    # Check that movement is inside stage limits.
    
    fov = calib.readFov( allowFail = False)
    z = app.stagez.getPos() + offsetz

    if (z<fov['zmin']) or (z>fov['zmax']):
        str = 'Offset outside FoV limits.'
        raise AORetryException( code = AOConstants.AOErrCode['VALUE_OUT_OF_RANGE_ERROR'], errstr = str)

    setupDevices.loadDevices( app, ['stagez'], check=True)

    try:
        app.log('offsetZ(): starting offset of %5.3f mm' % offsetz, app.LOG_LEV_INFO)
        app.stagez.enable()
        app.stagez.moveBy( offsetz, accuracy = 0.020, waitTimeout=60)
        app.stagez.enable(False)
        app.log('offsetZ(): done.')
    except AdOptError, e:
        if e.code == AOConstants.AOErrCode['TIMEOUT_ERROR']:
            errstr = 'Stages position not reached'
        else:
            errstr = e.errstr 
        raise AORetryException( e.code, errstr)

    app.log('offsetZ(): done', app.LOG_LEV_INFO)

def offsetXYinternal( app, offsetx, offsety):
    '''
    Offset the XY stages. Only useful in closed loop.
    '''

    setupDevices.loadDevices( app, ['stagex', 'stagey'], check=True)

    # Check that movement is inside stage limits.
    
    fov = calib.readFov( allowFail = False)
    x = app.stagex.getPos() + offsetx
    y = app.stagey.getPos() + offsety

    if (x<fov['xmin']) or (x>fov['xmax']) or (y<fov['ymin']) or (y>fov['ymax']):
        str = 'Offset outside FoV limits.'
        raise AORetryException( code = AOConstants.AOErrCode['VALUE_OUT_OF_RANGE_ERROR'], errstr = str)


    app.log('offsetXY(): starting offset of %5.3f, %5.3f mm' % (offsetx, offsety), app.LOG_LEV_INFO)

    try:
        if offsetx !=0:
            app.stagex.enable()
            app.stagex.moveBy( offsetx, accuracy = 0.020)
        if offsety !=0:
            app.stagey.enable()
            app.stagey.moveBy( offsety, accuracy = 0.020)
        if offsetx !=0:
            app.stagex.waitTargetReached( timeout=60, checkStatus=True)
        if offsety !=0:
            app.stagey.waitTargetReached( timeout=60, checkStatus=True)

        app.stagex.enable(False)
        app.stagey.enable(False)

    except AdOptError, e:
        if e.code == AOConstants.AOErrCode['TIMEOUT_ERROR']:
            errstr = 'Stages position not reached'
        else:
            errstr = e.errstr 
        raise AORetryException( e.code, errstr)

    app.log('offsetXY(): done', app.LOG_LEV_INFO)

def correctModesInternal( app, modes):
    '''
    Correct higher-order modes.
    '''

    app.log('correctModes(): not implemented', app.LOG_LEV_INFO)
    pass


def emergencyOffInternal(app):
    '''
    Immediate power off.
    '''

    setupDevices.loadDevices( app, ['power'], check = True)

    # W#1
    if 'main' in app.power.list():
        app.power.turnoff('main', force = True)





def doSetLoopParams( app, freq, decimation, binning, modulation, forceBinning=False, takeDark=True):
    '''
    Changes the AO loop parameters.
    '''

    setupDevices.loadDevices( app, ['ccd39', 'tt'], check=True)

    curBin = app.ccd39.xbin()
    curFramerate = app.ccd39.get_framerate()
    curModulation = app.tt.amp()

    if (curBin != binning) or (forceBinning == True):
        app.tt.set( amp=0)
        setHObinning( app, binning)

    # Always do these because the change binning may have changed everything
    doSetLoopFreq( app, freq, decimation)
    setTTmodulation( app, modulation)

    newBin = app.ccd39.xbin()
    newFramerate = app.ccd39.get_framerate()

    # Re-take dark if anything regarding the ccd has changed.
    if (newBin != curBin) or (newFramerate != curFramerate):
        if takeDark:
            calibHODarkInternal(app)
   
def setTTmodulation( app, modulation):
    '''
    Sets the TT modulation in lambda/D
    '''  

    setupDevices.loadDevices( app, ['ccd39', 'tt'], check=True)

    frm = round(app.ccd39.get_framerate())
   
    try:
        app.tt.set( amp = app.tt.modulationLambdaDF2V( modulation, frm))
        app._lastTTmodulation = modulation
    except Exception, e:
        raise AORetryException(AOConstants.AOErrCode['VALUE_OUT_OF_RANGE_ERROR'], "TT modulation out of allowed range for frequency %f" % frm)


def doStopLoop( app):
    '''
    Closes the fastlink fiber.
    '''
    setupDevices.loadDevices( app, ['sc'], check=True)

    app.sc.fl_stop()

def doPauseLoop(app):
    '''
    Pauses the AO loop. Disables the disturbance, if any, and re-enables so that
    it will restart as soon as the loop is resumed.
    '''

    setupDevices.loadDevices( app, ['sc'], check=True)

    # Save current light level
    app._pauseMag = ccd39Mag( app, nframes=10)

    # Disable disturbance, remembering previous status
    wfs,ovs = app.sc.get_disturb()
    app.sc.enable_disturb( False, False)
    time.sleep(0.02)

    # Pause loop
    app.sc.fl_stop()
    time.sleep(0.02)

    # Re-enable disturbance
    app.sc.enable_disturb( wfs, ovs)


def doResumeLoop(app):
    '''
    Resumes a paused loop.
    '''

    # Check light level
    mag = ccd39Mag( app, nframes=10)
    if abs(app._pauseMag - mag) >2:
        raise AORetryException( code= AOConstants.AOErrCode['WFSARB_STAR_WRONGFLUX'], errstr='Star magnitude on WFS changed from %5.1f to %f5.1f' % (app._pauseMag, mag))

    doCloseLoop(app)


    


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


def calibHODarkInternal(app, name=None, num=100, apply=True):
    '''
    Acquires a dark frame using the current binning, exposure time etc.
    Results are saved in $ADOPT_ROOT/calib/WFS/backgrounds with a timestamp.
    If a name is provided, a link with the specified name will be created too.

    Timestamps are always unique, so the file will be always created. If
    the name provided already exists, the link is not created and a warning
    is generated.

    To have a good dark, the filterwheel #1 is rotated to the mirror position.
    After the dark measurement, the filterwheel is rotated back to the position
    it had before the measurement.
    '''

    app.log('calibHODarkInternal(): starting measure, %d frames' % num, app.LOG_LEV_INFO)
    errstr = ''
    setupDevices.loadDevices( app, ['ccd39', 'sc', 'fw1', 'power', 'flowerpot', 'lamp'], check=True)

    calibUnitStat = app.flowerpot.getOperatingState()
    if not app.ccd39.isOperating():
        raise AORetryException( 0, 'Ccd39 is not ready')

    if not app.fw1.isReady():
        raise AORetryException( 0, 'Filterwheel #1 is not ready')

    if calibUnitStat == app.flowerpot.OPSTATE_CALIBRATION or calibUnitStat == app.flowerpot.OPSTATE_INTERNALSOURCE:
        if not app.lamp.isOperating():
            raise AORetryException( 0, 'Lamp is not ready')

    # Move FW1 and set lamp to zero in parallel
    pos = app.fw1.getPos()
    app.fw1.moveTo(4)

    if calibUnitStat == app.flowerpot.OPSTATE_CALIBRATION or calibUnitStat == app.flowerpot.OPSTATE_INTERNALSOURCE:
        lampInt = app.lamp.getIntensity()
        app.lamp.setIntensity(0, waitTimeout=100)

    app.fw1.waitTargetReached()

    # Equalize ccd39 and read dark frame
    app.ccd39.equalize_quadrants( target=500)

    time.sleep(2)   # Allow ccd39 stabilization?

    frame = frames_lib.averageFrames(app.ccd39.get_frames(num, type='pixels', subtractDark = False))
    hdr, errstr = getSetupFits(app)
    hdr.update( {'AVERAGED FRAMES': num})
    hdr.update( {'DETECTOR': 'CCD39'} )

    errstr = saveCalibFile( app, calib.BackgDir('ccd39', app.ccd39.xbin()), frame, hdr, ensureExtension( app, name, '.fits'))

    # Restore FW1 and lamp position
    app.fw1.moveTo(pos)
    if calibUnitStat == app.flowerpot.OPSTATE_CALIBRATION or calibUnitStat == app.flowerpot.OPSTATE_INTERNALSOURCE:
        app.lamp.setIntensity(lampInt, waitTimeout=100)
    app.fw1.waitTargetReached()

    if apply:
        selectHODark(app)

    app.log('calibHODarkInternal(): done', app.LOG_LEV_INFO)

    return errstr

def calibTVDarkInternal(app, name=None, num=10, apply=True):
    '''
    Acquires a dark frame using the current binning, exposure time etc.
    Results are saved in $ADOPT_ROOT/calib/WFS/ccd47/backgrounds with a timestamp.
    If a name is provided, a link with the specified name will be created too.

    Timestamps are always unique, so the file will be always created. If
    the name provided already exists, the link is not created and a warning
    is generated.
    '''
    app.log('calibTVDarkInternal(): starting measure, %d frames' % num, app.LOG_LEV_INFO)
    errstr = ''
    setupDevices.loadDevices( app, ['ccd47', 'fw2', 'tv', 'power', 'flowerpot', 'lamp'], check=True)

    calibUnitStat = app.flowerpot.getOperatingState()
    if not app.ccd47.isOperating():
        raise AORetryException( 0, 'Ccd47 is not ready')

    if not app.fw2.isReady():
        raise AORetryException( 0, 'Filterwheel #2 is not ready')

    # Turn off lamp and move fw2
    if calibUnitStat == app.flowerpot.OPSTATE_CALIBRATION or calibUnitStat == app.flowerpot.OPSTATE_INTERNALSOURCE:
        if not app.lamp.isOperating():
            raise AORetryException( 0, 'Lamp is not ready')

        lampInt = app.lamp.getIntensity()
        app.lamp.setIntensity(0, waitTimeout=100)

        time.sleep(3)       # Wait for lamp cooling

    pos = app.fw2.getPos()
    app.fw2.moveTo(0.5, waitTimeout=30)   # Blocking position

    app.ccd47.equalize_quadrants( target=500)

    frame = frames_lib.averageFrames( app.ccd47.get_frames(num, type='pixels', subtractDark = False, nextFrame = True))
    hdr, errstr = getSetupFits(app)
    hdr.update( {'AVERAGED FRAMES': num})
    hdr.update( {'DETECTOR': 'CCD47'} )

    errstr = saveCalibFile( app, calib.BackgDir('ccd47', app.ccd47.xbin()), frame, hdr, ensureExtension( app, name, '.fits'))

    # Restore lamp and fw2
    if calibUnitStat == app.flowerpot.OPSTATE_CALIBRATION or calibUnitStat == app.flowerpot.OPSTATE_INTERNALSOURCE:
        app.lamp.setIntensity(lampInt, waitTimeout=100)

    app.fw2.moveTo(pos, waitTimeout=30)   # Blocking position

    if apply:
        selectTVDark(app)

    app.log('calibTVDarkInternal(): done', app.LOG_LEV_INFO)
    return errstr


def calibIRTCDarkInternal(app, name=None, num=10, apply=True):
    '''
    Acquires an IRTC dark frame
    Results are saved in $ADOPT_ROOT/calib/WFS/irtc/backgrounds with a timestamp.
    If a name is provided, a link with the specified name will be created too.

    Timestamps are always unique, so the file will be always created. If
    the name provided already exists, the link is not created and a warning
    is generated.
    '''
    app.log('calibIRTCDarkInternal(): starting measure, %d frames' % num, app.LOG_LEV_INFO)
    errstr = ''
    setupDevices.loadDevices( app, ['irtc', 'power', 'flowerpot', 'lamp'], check=True)

    # Turn off lamp
    calibUnitStat = app.flowerpot.getOperatingState()
    if calibUnitStat == app.flowerpot.OPSTATE_CALIBRATION or calibUnitStat == app.flowerpot.OPSTATE_INTERNALSOURCE:
        if not app.lamp.isOperating():
            raise AORetryException( 0, 'Lamp is not ready')

        lampInt = app.lamp.getIntensity()
        app.lamp.setIntensity(0, waitTimeout=60)
        time.sleep(3)       # Wait for lamp cooling

    # Read IRTC
    try:
        data, IrtcHdr = app.irtc.get_frames(num, type='pixels', subtractDark = False, getHeader = True)
    except:
        raise AORetryException( 0, 'Error reading IRTC')

    IrtcHdr = fits_lib.cleanHdr(IrtcHdr)
    frame = frames_lib.averageFrames( data, useFloat=True).astype(numarray.Int16)
    hdr, errstr = getSetupFits(app)

    IrtcHdr.update( hdr)
    IrtcHdr.update( {'AVERAGED FRAMES': num})
    IrtcHdr.update( {'DETECTOR': 'IRTC'} )

    filename = saveCalibFile( app, calib.BackgDir('irtc', 1), frame, IrtcHdr, ensureExtension( app, name, '.fits'), getFullPath = True)
    fits_lib.writeSimpleFits(filename+'_cube.fits', data, IrtcHdr)

    # Restore lamp
    if calibUnitStat == app.flowerpot.OPSTATE_CALIBRATION or calibUnitStat == app.flowerpot.OPSTATE_INTERNALSOURCE:
        app.lamp.setIntensity( lampInt, waitTimeout=60)

    if apply:
        v = app.ReadVar( cfg.ccd_darkvariable['irtc']+'.REQ')
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

def calibMovementsInternal(app):

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


def ensureExtension( app, filename, extension):
    '''
    Add the extension if the filename does not have it.
    ''' 

    if filename == None:
        return None

    if extension[0] != '.':
        extension = '.'+extension

    if len(filename) < len(extension):
        return filename + extension
    if filename[-len(extension):] != extension:
        return filename + extension
    else:
        return filename

def savePsfCallback( app, num):

    percent = float(num)/ app._savePsfNum * 100;
    diff = percent - app._savePsfLastPercent
    if diff >=10:
        app.log('savePsf(): %d percent done' % percent, app.LOG_LEV_INFO)
        app._savePsfLastPercent = percent
    
def savePsfInternal( app, filename, num=50):
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

def saveIrtcInternal( app, filename, num=50):
    '''
    Saves an IRTC cube.
    File will be an integer FITS.
    '''

    try:
        app.log('saveIrtcInternal(): starting measure, %d frames' % num, app.LOG_LEV_INFO)
        setupDevices.loadDevices( app, ['irtc'], check=True)

        frames, IrtcHdr = app.irtc.get_frames( num, type='pixels', getHeader = True)
        IrtcHdr = fits_lib.cleanHdr(IrtcHdr)
        hdr, errstr = getSetupFits(app)
        IrtcHdr.update(hdr)
        IrtcHdr.update( {'DETECTOR': 'IRTC'} )
        fits_lib.writeSimpleFits( filename, frames, IrtcHdr)
    except AdOptError, e:
        raise AORetryException( code = e.code, errstr = "Cannot save PSF")
    except Exception, e:
        print e
        raise AORetryException( code = 0, errstr = "Cannot save PSF")

    app.log('saveIrtc(): done', app.LOG_LEV_INFO)

def centerCameralens_fast(app):
    '''
    Center the camera lens position, using the current pupils and without using the internal reference source.
    Returns a tuple with the estimated (x,y) residual error in micron.
    '''

    setupDevices.loadDevices( app, ['stagex', 'stagey', 'stagez'], check=True)

    xpos = data['stagex']
    ypos = data['stagey']
    zpos = data['stagez']

    oldPosX = app.stagex.getPos()
    oldPosY = app.stagey.getPos()
    oldPosZ = app.stagez.getPos()

    # Prepare ccd
    ccdFreq=102;
    ccdBin=1;
    TTmod=30;

    doSetLoopParams( app, ccdFreq, 0, ccdBin, TTmod)

    # Correct centering and focus
    centerPupils.centerStages( app)
    centerPupils.centerFocus( app)

    # Center camera lens
    errors = centerPupils.centerCameralens( app, iterations=20, threshold = 1.2)
    app.log('centerCameralens(): done, errors = %5.2f %5.2f micron' % (errors[0], errors[1]), app.LOG_LEV_INFO)

    # Put back stages and flowerpot to old state
    app.stagex.moveTo(oldPosX)
    app.stagey.moveTo(oldPosY)
    app.stagez.moveTo(oldPosY)
    app.stagex.waitTargetReached()
    app.stagey.waitTargetReached()
    app.stagez.waitTargetReached()





def centerCameralens( app):
    '''
    Centers the camera lens position, using the internal reference source.
    Returns a tuple with the estimated (x,y) residual error in micron.
    '''

    ccdFreq=100;
    ccdBin=1;
    TTmod=30;

    setupDevices.loadDevices( app, ['power', 'flowerpot', 'stagex', 'stagey', 'stagez', 'lamp', 'fw1'], check=True)

    app.log('centerCameralens(): starting', app.LOG_LEV_INFO)

    # Move stages to a predefined position

    data = calib.loadCalibData( calib.getCameraLensCalibFile(opstate='CALIBRATION'), check=['stagex', 'stagey', 'stagez'])

    xpos = data['stagex']
    ypos = data['stagey']
    zpos = data['stagez']

    oldPosX = app.stagex.getPos()
    oldPosY = app.stagey.getPos()
    oldPosZ = app.stagez.getPos()
    oldPosFw1 = app.fw1.getPos()
    app.stagex.moveTo(xpos)
    app.stagey.moveTo(ypos)
    app.stagez.moveTo(zpos)
    app.fw1.moveTo(0)

    # Prepare ccd
    doSetLoopParams( app, ccdFreq, 0, ccdBin, TTmod)

    # Set flower pot
    opState = app.flowerpot.getOperatingState()
    app.flowerpot.setOperatingState( app.flowerpot.OPSTATE_CALIBRATION)
    app.lamp.setIntensity(100, waitTimeout=100)

    app.stagex.waitTargetReached()
    app.stagey.waitTargetReached()
    app.stagez.waitTargetReached()
    app.fw1.waitTargetReached()

    # Correct centering and focus
    centerPupils.centerStages( app)
    centerPupils.centerFocus( app)


    # Center camera lens
    errors = centerPupils.centerCameralens( app, iterations=20, threshold = 1.2)
    app.log('centerCameralens(): done, errors = %5.2f %5.2f micron' % (errors[0], errors[1]), app.LOG_LEV_INFO)

    # Put back stages and flowerpot to old state
    app.stagex.moveTo(oldPosX)
    app.stagey.moveTo(oldPosY)
    app.stagez.moveTo(oldPosY)
    app.fw1.moveTo(oldPosFw1)
    app.stagex.waitTargetReached()
    app.stagey.waitTargetReached()
    app.stagez.waitTargetReached()
    app.fw1.waitTargetReached()

    app.flowerpot.setOperatingState(opState)

    return errors


def setSourceInternal( app, config, magnitude=None):
    '''
    Sets the calibration unit to either "CALIBRATION" or "TELESCOPE" state.
    In calibration state, an equivalent magnitude value can be specified for the calibration lamp.
    '''

    setupDevices.loadDevices( app, ['power', 'flowerpot'], check=True)
   
    if config == "CALIBRATION": 
        app.flowerpot.setOperatingState( app.flowerpot.OPSTATE_CALIBRATION)

    if config == "TELESCOPE":
        app.flowerpot.setOperatingState( app.flowerpot.OPSTATE_OBSERVATION)

    if config == "INTERNAL SOURCE":
        app.flowerpot.setOperatingState( app.flowerpot.OPSTATE_INTERNALSOURCE)

    if magnitude != None:
        app.log('setSourceInternal(): magnitude setting not implemented.', app.LOG_LEV_INFO)

def calibSlopeNullInternal(app, name=None, num=100, apply=True):
    '''
    Acquires a slope null using the current pupils, binning etc.
    Results are saved in $ADOPT_ROOT/calib/WFS/slopenulls with a timestamp.
    If a name is provided, a link with the specified name will be created too.

    Timestamps are always unique, so the file will be always created. If
    the name provided already exists, the link is not created and a warning
    is generated.
    '''

    selectHODark(app)  # Load appropriate dark frame
    selectSlopenull(app, '00zero1600.fits', matchLUT=False)  # Load zero slopenull

    app.log('calibSlopeNullInternal(): starting measure, %d frames' % num, app.LOG_LEV_INFO)
    setupDevices.loadDevices( app, ['ccd39', 'sc'], check=True)

    slopes = frames_lib.averageFrames( app.ccd39.get_frames(num, type='slopes'))
    hdr, errstr = getSetupFits(app)
    hdr.update( {'AVERAGED FRAMES': num})

    errstr = saveCalibFile( app, calib.slopenullDir(app.ccd39.xbin()), slopes, hdr, ensureExtension( app, name, '.fits'))


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
    hdr['HIERARCH ccd39.BINNING'] = app.ccd39.xbin()
    if matchLUT:
        hdr['HIERARCH sc.PUPILS'] = removePrefix( app.sc.get_pixellut(), calib.LUTsDir('ccd39'))[0:-7]

    name = getCalibFile( calib.slopenullDir(app.ccd39.xbin()), hdrmatch = hdr, name = name)

    app.sc.set_slopenull(name)
    app.log('selectSlopenull(): selected slopenull %s' % name, app.LOG_LEV_INFO)



def calibPupilsInternal( app, th1, th2, num=100, enlarge=0):
    '''
    Acquire current pupils. Frames will be acquired averaging <num> frames.

    Returns the directory name where the pupils have been saved. The directory
    is in the form $ADOPT_ROOT/calib/<side>/WFS/ccd39/bin<x>/<timestamp>
    where:
    <side> is the current side ('left' or 'right')
    <x> is the current binning (from 1 to 5)
    <timestamp> is the timestamp assigned to the pupils. Only this part is returned.
    <enlarge> is an optional enlarging of the diameter (default=0), for HOT style acquisition.
    The full path can be found using calib.LUTsDir('ccd39', bin=1)+'/'+timestamp
    '''

    selectHODark(app)

    setupDevices.loadDevices( app, ['ccd39', 'sc'], check=True)

    frame = frames_lib.averageFrames( app.ccd39.get_frames(num))
    curPupils = frames_lib.findPupils( frame, th1, th2, fourPupils=True)
    import makePupils
    cx = [0.0]*4
    cy = [0.0]*4
    diam = [0.0] *4
    for n in range(4):
        cx[n] = curPupils[n].cx
        cy[n] = curPupils[n].cy
        diam[n] = curPupils[n].diameter + enlarge / app.ccd39.xbin()
        app.log('Pup %d: %f - %f - %f' % (n, diam[n], cx[n], cy[n]), app.LOG_LEV_DEBUG)
    return makePupils.makePupilsFromCenterDiameter( diam, cx, cy, app.ccd39.dx(), app.ccd39.xbin())


def calibPupilsHOTInternal( app, th1=0.5, th2=0.5, num=100):
    '''
    Acquire new pupils, that will be used for HOT style acquisition.
    '''

    # Get new pupils
    timestamp = calibPupilsInternal( app, th1, th2, num, enlarge=6)

    # Load new pupils
    setHObinning( app, app.ccd39.xbin())

    time.sleep(1)
    app.tt.set(amp=4, freq=0) # [TODO] meglio fare un SetLoopParams, che comprende anche il binning??

    # Re-do dark frame
    calibHODarkInternal( app)

    return timestamp



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

    setupDevices.loadDevices( app, ['fw1', 'fw2','ccd39'], check=True)

    if setup:
        boardSetup.load( app, setup)

    app.ccd39.set_framerate(197)
    calibHODarkInternal(app)

    centerPupils.centerStages( app)
    centerPupils.centerFocus( app)
    fixCubeVignetting( app)
    centerPupils.centerStages( app)
    centerPupils.centerCameralens( app, iterations=20, threshold = 2.4)


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
        aoretry = "AdOpt exception: %s (%d) %s" % (e.errstr, e.code, AOConstants.AOErrType[e.code])
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


def setPowerOff(app):
    '''
    Goes into PowerOff state, safely turning off the system.
    '''

    return arbFunc( app, TurnSystemOff, (app,))


def setOperating( app, config="", boardSetup="", opticalSetup=False):
    '''
    Goes into Operating state, 
    '''

    try:
        return arbFunc( app, TurnSystemOn, (app, config, boardSetup, opticalSetup))
    except Exception, e:
        app.log('Serious error: non-trapped exception in arbFunc()', app.LOG_LEV_ERROR)
        app.log(str(e), app.LOG_LEV_ERROR)
        print e

def prepareAcquireRef( app, instr, mode, starxpos, starypos, telElevation, derotAngle, starMag, starColor, r0, v0):
    '''
    Prepare for reference start acquisition: move stages and setup ccd.
    '''
    return arbFunc( app, doPrepareAcquireRef, (app, instr, mode, starxpos, starypos, telElevation, derotAngle, starMag, starColor, r0, v0))

def acquireRef( app):
    '''
    Acquire reference source and prepare for closed loop operation.
    '''
    return arbFunc( app, doAcquireRef, (app,))

def enableDisturb( app, wfs=False, ovs=False):
    '''
    Enable/disable command disturbance on AdSec.
    '''
    return arbFunc( app, doEnableDisturb, (app, wfs, ovs))

def saveStatus( app, outputFile):
    '''
    Acquire reference source and prepare for closed loop operation.
    '''
    return arbFunc( app, doSaveStatus, (app, outputFile))

def closeLoop(app, nocheck=False):
    '''
    Closes the AO loop.
    '''
    return arbFunc( app, doCloseLoop, (app, nocheck))

def setLoopFreq(app, freq, decimation):
    '''
    Sets the AO loop frequency
    '''
    return arbFunc( app, doSetLoopFreq, (app, freq, decimation))

def setLoopParams(app, freq, decimation, binning, modulation, forceBinning):
    '''
    Sets the AO loop parameters (binning, frequency, modulation)
    '''
    return arbFunc( app, doSetLoopParams, (app, freq, decimation, binning, modulation, forceBinning))


def getTVSnap(app):
    '''
    Gets an image from the Technical viewer.
    '''
    return arbFunc( app, doGetTVSnap, (app,))

def checkRef(app):
    '''
    Checks the reference star position
    '''
    return arbFunc( app, doCheckRef, (app,))

def offsetXY(app, offsetx, offsety):
    '''
    Offsets the XY stages
    '''
    return arbFunc( app, offsetXYinternal, (app, offsetx, offsety))

def offsetZ(app, offsetz):
    '''
    Offsets the Z stage
    '''
    return arbFunc( app, offsetZinternal, (app, offsetz))

def correctModes(app, modes):
    '''
    Correct higher-order modes
    '''
    return arbFunc( app, correctModes, (app, modes))

def emergencyOff(app):
    '''
    Immediate power off
    '''
    return arbFunc( app, emergencyOffInternal, (app,))

def pauseLoop(app):
    '''
    Pauses the AO loop
    '''
    return arbFunc( app, doPauseLoop, (app,))


def resumeLoop(app):
    '''
    Resumes a paused AO loop.
    '''
    return arbFunc( app, doResumeLoop, (app,))

def stopLoop(app):
    '''
    Stops an AO loop.
    '''
    return arbFunc( app, doStopLoop, (app,))


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

def setSource(app, config, magnitude):
    '''
    Sets the source to use (reference or telescope).
    '''
    return arbFunc( app, setSourceInternal, (app, config, magnitude))

def calibSlopenull(app, name=None, num=1000, apply=True):
    '''
    Acquires a slope null using the current pupils, binning etc. averaging <num> frames.
    Results are saved in $ADOPT_ROOT/calib/WFS/slopenulls with a timestamp.
    If a name is provided, a link with the specified name will be created too.
    If apply is True, the new slope null will be immediately applied to the system.
    '''
    return arbFunc(app, calibSlopeNullInternal, (app, name, num, apply))

def calibHODark(app, name=None, num=100, apply=True):
    '''
    Acquires a dark frame using the current binning, exposure time etc. averaging <num> frames.
    Results are saved in $ADOPT_ROOT/calib/WFS/backgrounds with a timestamp.
    If a name is provided, a link with the specified name will be created too.
    If apply is True, the new dark will be immediately applied to the system.
    '''
    return arbFunc(app, calibHODarkInternal, (app, name, num, apply))

def calibTVDark(app, name=None, num=10, apply=True):
    return arbFunc(app, calibTVDarkInternal, (app, name, num, apply))

def calibIRTCDark(app, name=None, num=10, apply=True):
    return arbFunc(app, calibIRTCDarkInternal, (app, name, num, apply))

def savePsf( app, filename, num = 50):
    '''
    Save a ccd47 averaged frame into the specified filename.
    Format will be a float FITS file.
    '''
    return arbFunc( app, savePsfInternal, (app, filename, num))

def saveIrtc( app, filename, num = 50):
    '''
    Save an IRTC cube into the specified filename.
    Format will be an integer FITS file.
    '''
    return arbFunc( app, saveIrtcInternal, (app, filename, num))



def calibPupils(app, th1=0.5, th2=0.5, num=100):
    '''
    Acquires new pupils using the current binning etc. averaging <num> frames.
    Results are saved in $ADOPT_ROOT/calib/WFS/ccd39/LUTs in the appropriate binning subdirectory,
    using a timestamp.
    '''
    return arbFunc(app, calibPupilsInternal, (app, th1, th2, num))

def calibPupilsHOT(app, th1=0.5, th2=0.5, num=100):
    '''
    Acquires new pupils using the current binning etc. averaging <num> frames.
    Results are saved in $ADOPT_ROOT/calib/WFS/ccd39/LUTs in the appropriate binning subdirectory,
    using a timestamp.
    Specialized to prepare for HOT style acquisition
    '''
    return arbFunc(app, calibPupilsHOTInternal, (app, th1, th2, num))



    


#@Procedure: calibMovements
#
# Calibrates all mechanical movements in the WFS.
#@

def calibMovements(app):

    return arbFunc(app, calibMovementsInternal, (app,))



def refineAO(app, freq = None, binning = None, TTmod = None, Fw1Pos = None, Fw2Pos = None, decimation=0, checkCameralens = False):
    '''
    Changes AO loop parameters after a star has been acquired but before closing the loop.
    For all values, -1 means unchanged from current value.
    If <checkCameralens> is True, the camera lens position is checked and corrected before applying the settings.
    '''
    return arbFunc( app, refineAOInternal, (app, freq, binning, TTmod, Fw1Pos, Fw2Pos, decimation, checkCameralens ))

def refineLoop( app, params):
    '''
    Changes loop parameters while the loop is closed
    '''
    pass

def emergencyStop( app):
    '''
    Call to quickly interrupt whatever operations was in progress.
    '''
    pass



