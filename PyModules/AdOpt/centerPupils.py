#!/usr/bin/env python

# Pupil centering procedure
#
# Procedure to remove tip-tilt and focus offsets from the ccd39 pupils. Can use either the XYZ stages or the tip-tilt mirror.
# If the movement to use (stages or tip-tilt) is not specified, it will attempt to use all: first the XY stages
# for big errors, then the tip-tilt mirror for small errors, then the Z stage for focus.
#
# API:
#
#  calibCentering()   -   calibration procedure.
#                         The calibration will register the stages/tip-tilt movements wrt. the ccd39 pupils.
#                         To work, the ccd39 must be on with light from either a reference source or a star. Values
#                         will be written in a calibration file, and read back from centering procedure.
#
#  centerPupils()  -    center and/or focus pupils.
#                       Move the stages, tip-tilt mirror or both (depending on the arguments) so that the four pupils are of equal intensity
#
#
# Can be run from command line too
#
# centerPupils.py [-calib] [-center] [-stages] [-cameralens] [-tt] 

from AdOpt import calib, frames_lib, setupDevices, cfg, AOExcept, AOConstants, AOVar
from AdOpt.wrappers import drawlib
import time, math

autocenter_timeout = 60     # Time without updates after which an autocenter process is considered defunct

def calibCentering( app, useStages = True, useTT = True, useFocus = False, stageMovement = None, ttMovement = None):
    '''
    The calibration will register the stages/tt movements wrt. the ccd39 pupils.
    To work, the ccd39 must be on with light from either a reference source or a star.
    The light should be roughly centered so that changing by a small amount all four pupils are always illuminated.
    The current amplitude and frequency are not changed.

    Values will be written in a calibration file, and read back from centering procedure.

    <stageMovement> is the stage movement amplitude, in millimeters. Defaults to the current calib file
    <ttMovement> is the tip-tilt mirror movement amplitude, in volt. Defaults to the current calib file
    '''
    
    if useStages:
        calibStagesCentering( app, stageMovement)

    if useTT:
        calibTTCentering( app, ttMovement)

    if useFocus:
        calibStagesFocus( app, stageMovement)

def calibStagesFocus( app, stageMovement = None):
    '''
    Calibrate ccd39 defocus moving the Z stage.
    '''

    # Load required modules
    setupDevices.loadDevices( app, ['ccd39','stagez', 'tt'])

    # Set default values
    data = {}
    data['numFrames'] = 10
    data['gain']      = -0.7
    data['stageMovement'] = 1.000
    data['maxError'] = 0.050

    # The calibration movement is done at constant modulation (6 lambda/D).
    modPsf = 6
    modVolt = app.tt.modulationLambdaDF2V( modPsf, app.ccd39.get_framerate())
    app.tt.set( amp = modVolt)

    # Load existing calib, if any
    data.update( calib.loadCalibData( calib.focusStagesCalibFile( modPsf), allowFail = True))

    data['modPsf'] = modPsf

    # Manual overrides
    if stageMovement != None:
        data['stageMovement']  = stageMovement

    # Save overrides, if any
    calib.saveCalibData( calib.focusStagesCalibFile( modPsf), data, 'Stages focusing calibration')

    app.log('calibStagesFocus(): starting stage focusing, movement amount = %5.3f mm' % data['stageMovement'])

    zpos = app.stagez.getPos()
    app.stagez.enable( onaftermove = True)

    app.log('calibStagesFocus(): moving to +Z')

    app.stagez.moveTo( zpos + data['stageMovement'], waitTimeout=10)
    pupils = calib.getCurPupils( app)
    focus1 = frames_lib.get_pupils_focus( frames_lib.averageFrames( app.ccd39.get_frames(data['numFrames'])), pupils)
    app.log('calibStagesFocus(): focus: %5.3f' % focus1)

    app.log('calibStagesFocus(): moving to -Z')

    app.stagez.moveTo( zpos - data['stageMovement'], waitTimeout=10)
    pupils = calib.getCurPupils( app)
    focus2 = frames_lib.get_pupils_focus( frames_lib.averageFrames( app.ccd39.get_frames(data['numFrames'])), pupils)
    app.log('calibStagesFocus(): focus: %5.3f' % focus1)

    # Leave stages as they were in the beginning
    app.log('calibStagesFocus(): moving to original position')

    app.stagez.moveTo( zpos, waitTimeout = 10)

    data['offz_focus'] = (focus1-focus2) / data['stageMovement']

    calib.saveCalibData( calib.focusStagesCalibFile( modPsf), data, title='Stage focus calibration')

    app.log('calibStagesFocus(): done')
    



def calibStagesCentering( app, stageMovement = None):
    '''
    Calibrates pupil centering on ccd39 moving the XYZ stages.
    '''

    # Load required modules
    setupDevices.loadDevices( app, ['ccd39','stagex', 'stagey', 'tt', 'rerot'])

    # Set default values
    data = {}
    data['numFrames'] = 10
    data['gain']      = -0.2     # Default low gain
    data['stageMovement'] = 0.030
    data['maxError'] = 0.030

    # The calibration movement is done at constant modulation (6 lambda/D).
    modPsf = 6
    #modVolt = app.tt.modulationLambdaDF2V( modPsf, app.ccd39.get_framerate())
    #app.tt.set( amp = modVolt)

    # Load existing calib, if any
    data.update( calib.loadCalibData( calib.centerStagesCalibFile( modPsf), allowFail = True))

    data['modPsf'] = modPsf

    # Manual overrides
    if stageMovement != None:
        data['stageMovement']  = stageMovement

    # Save overrides, if any
    calib.saveCalibData( calib.centerStagesCalibFile( modPsf), data, 'Stages centering calibration')

    app.log('calibStagesCentering(): moving rerotator to zero position')
    app.rerot.moveTo(0, waitTimeout=60)

    app.log('calibStagesCentering(): starting stage centering, movement amount = %5.3f mm' % data['stageMovement'])

    xpos = app.stagex.getPos()
    ypos = app.stagey.getPos()

    app.stagex.enable( onaftermove = True)
    app.stagey.enable( onaftermove = True)

    app.log('calibStagesCentering(): moving to +X')

    app.stagex.moveTo( xpos + data['stageMovement'], waitTimeout=10, accuracy=0.005)
    time.sleep(2)
    dx1, dy1 = frames_lib.get_pupils_delta( frames_lib.averageFrames( app.ccd39.get_frames(data['numFrames'])),  useIndpup = calib.CurIndpup('ccd39'))

    app.log('calibStagesCentering(): moving to -X')

    app.stagex.moveTo( xpos - data['stageMovement'], waitTimeout=10, accuracy=0.005)
    time.sleep(2)
    dx2, dy2 = frames_lib.get_pupils_delta( frames_lib.averageFrames( app.ccd39.get_frames(data['numFrames'])),  useIndpup = calib.CurIndpup('ccd39'))

    # Leave stages as they were in the beginning
    app.log('calibStagesCentering(): moving to original position')
    app.stagey.moveTo( ypos, waitTimeout=10)

    x1 = dx1-dx2
    y1 = dy1-dy2

    mag = math.sqrt(x1*x1 + y1*y1) / (data['stageMovement']*2)
    ang = math.atan2(x1,y1)
    
    data['offx_dx'] = (dx1-dx2) / data['stageMovement']
    data['offx_dy'] = (dy1-dy2) / data['stageMovement']
    data['mag'] = mag
    data['ang'] = ang

    calib.saveCalibData( calib.centerStagesCalibFile( modPsf), data, 'Stages centering calibration')

    app.log('calibStagesCentering(): done')
    


def calibTTCentering( app, ttMovement = None):
    '''
    Centers pupils on ccd39 moving the tip-tilt mirror
    '''

    # Load required modules
    setupDevices.loadDevices( app, ['ccd39','tt','rerot'])

    # Set default values
    data = {}
    data['numFrames'] = 10
    data['gain']      = -0.7 
    data['ttMovement'] = 0.5
    data['maxError'] = 0.002

    # The calibration movement is done at constant modulation (6 lambda/D).
    modPsf = 6
    modVolt = app.tt.modulationLambdaDF2V( modPsf, app.ccd39.get_framerate())
    app.tt.set( amp = modVolt)

    # Load existing calib, if any
    data.update( calib.loadCalibData( calib.centerTTCalibFile( modPsf), allowFail = True))

    data['modPsf'] = modPsf

    # Manual overrides
    if stageMovement != None:
        data['stageMovement']  = stageMovement

    # Save overrides, if any
    calib.saveCalibData( calib.centerTTCalibFile( modPsf), data, 'TT centering calibration')


    app.log('calibTTCentering(): moving rerotator to zero position')
    app.rerot.moveTo(0, waitTimeout=60)

    app.log('calibTTCentering(): starting TT centering, movement amount = %5.3f volts' % data['ttMovement'])

    amp = app.tt.amp()
    freq = app.tt.freq()
    offx = app.tt.offx()
    offy = app.tt.offy()  
    ang  = app.rerot.getPos()*3.1415/180.0 *2  # Pupil rotates by 2x the rerotator angle

    app.log('calibTTCentering(): moving to +X')

    app.tt.set( amp, freq, offx + data['ttMovement'], offy)
    dx1, dy1 = frames_lib.get_pupils_delta( frames_lib.averageFrames( app.ccd39.get_frames(data['numFrames'])), useIndpup = calib.CurIndpup('ccd39' ))

    app.log('calibTTCentering(): moving to -X')

    app.tt.set( amp, freq, offx - data['ttMovement'], offy)
    dx2, dy2 = frames_lib.get_pupils_delta( frames_lib.averageFrames( app.ccd39.get_frames(data['numFrames'])),  useIndpup = calib.CurIndpup('ccd39'))

    app.log('calibTTCentering(): moving to +Y')

    app.tt.set( amp, freq, offx, offy + data['ttMovement'])
    dx3, dy3 = frames_lib.get_pupils_delta( frames_lib.averageFrames( app.ccd39.get_frames(data['numFrames'])),  useIndpup = calib.CurIndpup('ccd39'))

    app.log('calibTTCentering(): moving to -Y')

    app.tt.set( amp, freq, offx, offy - data['ttMovement'])
    dx4, dy4 = frames_lib.get_pupils_delta( frames_lib.averageFrames( app.ccd39.get_frames(data['numFrames'])), useIndpup = calib.CurIndpup('ccd39'))
 
    # Leave tt as it was in the beginning
    app.log('calibTTCentering(): moving to original position')

    app.tt.set( amp, freq, offx, offy)

    data['offx_dx'] = (dx1-dx2) / data['ttMovement']
    data['offx_dy'] = (dy1-dy2) / data['ttMovement']
    data['offy_dx'] = (dx3-dx4) / data['ttMovement']
    data['offy_dy'] = (dy3-dy4) / data['ttMovement']

    calib.saveCalibData( calib.centerTTCalibFile( modPsf), data, 'Tip-tilt centering calibration')

    app.log('calibTTCentering(): done')
    

def setStagesCenterStatus( app, status):
    statusVar = AOVar.AOVar(name=cfg.side+'.WFS.stages_autocenter')
    statusVar.SetValue(status)
    app.WriteVar(statusVar)

def setFocusCenterStatus( app, status):
    statusVar = AOVar.AOVar(name=cfg.side+'.WFS.stages_autofocus')
    statusVar.SetValue(status)
    app.WriteVar(statusVar)

def setLensCenterStatus( app, status):
    statusVar = AOVar.AOVar(name=cfg.side+'.WFS.lens_autocenter')
    statusVar.SetValue(status)
    app.WriteVar(statusVar)

def getStagesCenterStatus( app):
    try: 
        var = app.ReadVar( cfg.side+'.WFS.stages_autocenter')
	now = time.time()
	if now - var.MTime() > autocenter_timeout:
		return 0
	else:
        	return app.ReadVar(statusVar).Value()
    except:
        return 0

def getFocusCenterStatus( app):
    try:
        var = app.ReadVar( cfg.side+'.WFS.stages_autofocus')
	now = time.time()
	if now - var.MTime() > autocenter_timeout:
		return 0
	else:
        	return app.ReadVar(statusVar).Value()
    except:
        return 0

def getLensCenterStatus( app):
    try: 
        statusVar = cfg.side+'.WFS.lens_autocenter'
        return app.ReadVar(statusVar).Value()
    except:
        return 0




def centerPupils( app, useStages = True, useTT = True, useFocus = False, useCameralens = False, cameraLensTh = 0):
    '''
    Move the XYZ stages and/or the tip-tilt mirror so that the four pupils are of equal intensity
    '''
    if useStages:
        centerStages( app)
    if useTT:
        centerTT( app)
    if useFocus:
        centerFocus( app)
    if useCameralens:
        centerCameralens( app, threshold = cameraLensTh)

def centerStages( app):
    '''
    Centers the ccd39 pupils using the XYZ stages.
    '''

    if getStagesCenterStatus(app):
        return
    setStagesCenterStatus(app,1)

    try:
        # Load required modules
        setupDevices.loadDevices( app, ['ccd39', 'stagex', 'stagey', 'tt', 'rerot', 'sc'])

        keylist = ['numFrames', 'offx_dx', 'offx_dy', 'offy_dx', 'offy_dy', 'gain' ]
        calibModPsf = 6
        try:
            print calib.centerStagesCalibFile(calibModPsf)
            data = calib.loadCalibData( calib.centerStagesCalibFile(calibModPsf), check=keylist)
        except AOExcept.AdOptError:
            app.log('Error: cannot locate calibration for default modulation %5.3f psf' % calibModPsf)
            setStagesCenterStatus(app,0)
            return

        g2 = app.tt.amp()

        app.stagex.enable( onaftermove = True)
        app.stagey.enable( onaftermove = True)

        app.log( 'centerStages(): starting stages centering procedure')

        xpos = app.stagex.getPos()
        ypos = app.stagey.getPos()
        rerotAng  = (app.rerot.getPos())*3.1415/180.0 *2  # Pupil rotates by 2x the rerotator angle

        bin = app.ccd39.xbin()
        cx = (app.ccd39.dx()/bin)/2
        cy = (app.ccd39.dy()/bin)/2

        while not app.TimeToDie():

            dx, dy = frames_lib.get_pupils_delta( app.ccd39.get_frames(data['numFrames'], average=True, useFloat=True), useIndpup = calib.CurIndpup('ccd39'))

            mag = math.sqrt(dx*dx + dy*dy)
            ang = math.atan2(dx,dy)
            
            print 'Current error:', mag, math.atan2(dx,dy)

            soglia = 0.1
            if (abs(dx) < soglia) and (abs(dy) < soglia):
                app.log( 'centerStages(): target reached.')
                drawlib.sendCircle( "ccdviewer_ccd39", cx, cy, 3, 0x00FF00, 2, 0)
                setStagesCenterStatus(app,0)
                return

            useAng = rerotAng + (data['ang'] - ang)
            
            xError = mag / data['mag'] * math.cos(useAng)
            yError = mag / data['mag'] * math.sin(useAng) * -1  # To have the right direction
            
            app.log( 'centerStages(): X,Y errors = %5.3f, %5.3f' % (xError, yError))

            drawlib.sendArrow( "ccdviewer_ccd39", cx, cy, cx-xError*100/bin, cy+yError*100/bin, 0x00FF00, 2, 0)

            app.log( 'centerStages(): correcting by  %5.3f, %5.3f' % (xError, yError))

            xError = xError * data['gain'] *g2
            yError = yError * data['gain'] *g2

            xpos += xError
            ypos += yError

            app.log( 'centerStages(): moving to %5.3f, %5.3f' % (xpos, ypos))

            app.stagex.moveTo(xpos)
            app.stagey.moveTo(ypos) 
            app.stagex.waitTargetReached( checkStatus = True)
            app.stagey.waitTargetReached( checkStatus = True)

            time.sleep(3)

    except:
        pass

    setStagesCenterStatus(app,0)

def centerCameralens( app, iterations=-1, threshold = 0):
    '''
    Centers pupils on ccd39 using the camera lens.
    If <iterations> is specified, it will exit after the specified no. of iterations, otherwise runs forever (or until below the threshold)
    Information is taken from RTDB variables, which are normally updated by the pupilcheck process.
    If <threshold> is specified, is the exit threshold in microns (both x and y errors must be under threshold).
    If it exits, returns a tuple with the estimated (x,y) residual error in micron.
    '''

    if getLensCenterStatus(app):
        return (0,0)
    
    setLensCenterStatus( app, 1)

    try:
        # Load required modules
        setupDevices.loadDevices( app, ['ccd39','lens'])

        pix_micron = 24.0  # Pixel size in microns (bin 1x1)
        gain = 0.7         # Loop gain

        bin = app.ccd39.xbin()
        cx = (app.ccd39.dx()/bin)/2
        cy = (app.ccd39.dy()/bin)/2

        while (not app.TimeToDie()) and iterations != 0:

            x_mov=0
            y_mov=0

            curPups = frames_lib.getCurPupilsFromRTDB(app, next=True)

            dx = -(curPups[0].diffx + curPups[1].diffx + curPups[2].diffx + curPups[3].diffx)/4.0
            dy = (curPups[0].diffy + curPups[1].diffy + curPups[2].diffy + curPups[3].diffy)/4.0 

            x_err = dx*pix_micron*bin 
            y_err = dy*pix_micron*bin
            app.log( 'centerCameralens(): errors in um: %3.1f, %3.1f' % (x_err, y_err))
            print 'centerCameralens(): errors in um: %3.1f, %3.1f' % (x_err, y_err)
 
            # Exit if under threshold
            if threshold>0:
                if (abs(x_err)<threshold) and (abs(y_err)<threshold):
                    drawlib.sendCircle( "ccdviewer_ccd39", cx, cy, 3, 0x00FF00, 2, 0)
                    break

            x_mov = -x_err * gain
            y_mov = -y_err * gain
       
            app.log( 'centerCameralens(): correcting by %3.1f, %3.1f microns' % (x_mov, y_mov))
            print 'centerCameralens(): correcting by %3.1f, %3.1f microns' % (x_mov, y_mov)
            app.lens.moveBy( x_mov, y_mov)

            xdim = min(x_err,5)
            ydim = min(y_err,5)
            drawlib.sendArrow( "ccdviewer_ccd39", cx-cx/2, cy-cy/2, cx-cx/2-xdim*2/bin, cy-cy/2-ydim*2/bin, 0x00FF00, 2, 0)
            drawlib.sendArrow( "ccdviewer_ccd39", cx-cx/2, cy+cy/2, cx-cx/2-xdim*2/bin, cy+cy/2-ydim*2/bin, 0x00FF00, 2, 0)
            drawlib.sendArrow( "ccdviewer_ccd39", cx+cx/2, cy-cy/2, cx+cx/2-xdim*2/bin, cy-cy/2-ydim*2/bin, 0x00FF00, 2, 0)
            drawlib.sendArrow( "ccdviewer_ccd39", cx+cx/2, cy+cy/2, cx+cx/2-xdim*2/bin, cy+cy/2-ydim*2/bin, 0x00FF00, 2, 0)
            iterations -= 1

    except:
        pass

    setLensCenterStatus( app, 0)
    return (x_err+x_mov, y_err+y_mov)


def centerFocus( app):
    '''
    Focus the ccd39 pupils using the XYZ stages.
    '''

    if getFocusCenterStatus(app):
        return

    setFocusCenterStatus(app,1)

    try:
        # Load required modules
        setupDevices.loadDevices( app, ['ccd39', 'stagez', 'tt'])

        # Try to find a calibration for our current modulation
        modPsf = app.tt.modulationVF2Psf( app.ccd39.get_framerate(), app.tt.amp())
        if modPsf == 0:
            app.log('Error: cannot autocenter without modulation')
            setFocusCenterStatus(app,0)
            return

        keylist = ['numFrames', 'offz_focus', 'maxError', 'gain']
        try:
            data = calib.loadCalibData( calib.focusStagesCalibFile(modPsf), check=keylist)
        except AOExcept.AdOptError:
            app.log('Cannot locate calibration for modulation=%5.3f psf. Rescaling default calibration...' % modPsf)
            defaultModPsf = 6
            try:
                data = calib.loadCalibData( calib.focusStagesCalibFile(defaultModPsf), check=keylist)
            except AOExcept.AdOptError:
                app.log('Error: cannot locate calibration for default modulation %5.3f psf' % defaultModPsf)
                setStagesFocusStatus(app,0)
                return
            data['offz_focus'] /= defaultModPsf/modPsf

        app.log( 'centerFocus(): starting stages focusing procedure')

        app.stagez.enable( onaftermove = True)
        zpos = app.stagez.getPos()

        bin = app.ccd39.xbin()
        cx = (app.ccd39.dx()/bin)/2
        cy = (app.ccd39.dy()/bin)/2

        while not app.TimeToDie():
            pupils = calib.getCurPupils( app)
            focus = frames_lib.get_pupils_focus( frames_lib.averageFrames( app.ccd39.get_frames(data['numFrames'])), pupils)

            zError = focus * data['offz_focus']

            app.log( 'centerFocus(): Z error = %5.3f' % zError)

            # Done?
            if abs(zError) < data['maxError']:
                app.log( 'centerFocus(): target reached.')
                drawlib.sendCircle( "ccdviewer_ccd39", cx, cy, 3, 0x00FF00, 2, 0)
                setFocusCenterStatus(app,0)
                return

            zError = zError * data['gain']

            app.log( 'centerFocus(): correcting by %5.3f' % zError)

            d_out=abs(zError)*20/bin
            if d_out<3:
                d_out=3
            d_in =abs(zError)*2/bin
            if d_in<1:
                d_in=1
        
            if zError <0:
                drawlib.sendArrow( "ccdviewer_ccd39", cx+d_in, cy-d_in, cx+d_out, cy-d_out, 0x00FF00, 2, 0)
                drawlib.sendArrow( "ccdviewer_ccd39", cx+d_in, cy+d_in, cx+d_out, cy+d_out, 0x00FF00, 2, 0)
                drawlib.sendArrow( "ccdviewer_ccd39", cx-d_in, cy-d_in, cx-d_out, cy-d_out, 0x00FF00, 2, 0)
                drawlib.sendArrow( "ccdviewer_ccd39", cx-d_in, cy+d_in, cx-d_out, cy+d_out, 0x00FF00, 2, 0)
            else:
                drawlib.sendArrow( "ccdviewer_ccd39", cx+d_out, cy-d_out, cx+d_in, cy-d_in, 0x00FF00, 2, 0)
                drawlib.sendArrow( "ccdviewer_ccd39", cx+d_out, cy+d_out, cx+d_in, cy+d_in, 0x00FF00, 2, 0)
                drawlib.sendArrow( "ccdviewer_ccd39", cx-d_out, cy-d_out, cx-d_in, cy-d_in, 0x00FF00, 2, 0)
                drawlib.sendArrow( "ccdviewer_ccd39", cx-d_out, cy+d_out, cx-d_in, cy+d_in, 0x00FF00, 2, 0)

            zpos += zError

            app.log( 'centerFocus(): moving to %5.3f' % zpos)

            app.stagez.moveTo(zpos)
            app.stagez.waitTargetReached( checkStatus = True)

    except:
        pass

    setFocusCenterStatus(app,0)

def centerTT( app):
    '''
    Move the tip-tilt so that the four pupils are of equal intensity
    '''
    setupDevices.loadDevices( app, ['ccd39', 'tt', 'rerot'])

    # Try to find a calibration for our current modulation
    modPsf = app.tt.modulationVF2Psf( app.ccd39.get_framerate(), app.tt.amp())
    if modPsf == 0:
        app.log('Error: cannot autocenter without modulation')
        return

    keylist = ['offx_dx', 'offx_dy', 'offy_dx', 'offy_dy', 'gain', 'numFrames', 'maxError']
    try:
        data = calib.loadCalibData( calib.centerTTCalibFile(modPsf), check=keylist)
    except AOExcept.AdOptError:
        print 'error 1'
        app.log('Cannot locate calibration for modulation=%5.3f psf. Rescaling default calibration...' % modPsf)
        defaultModPsf = 6
        try:
            data = calib.loadCalibData( calib.centerTTCalibFile(defaultModPsf), check=keylist)
        except AOExcept.AdOptError:
            print 'error 2'
            app.log('Error: cannot locate calibration for default modulation %5.3f psf' % defaultModPsf)
            return
        data['offx_dx'] /= defaultModPsf/modPsf
        data['offx_dy'] /= defaultModPsf/modPsf
        data['offy_dx'] /= defaultModPsf/modPsf
        data['offy_dy'] /= defaultModPsf/modPsf


    
    amp = app.tt.amp()
    freq = app.tt.freq()
    offx = app.tt.offx()
    offy = app.tt.offy()
    ang  = (app.rerot.getPos()-132)*3.1415/180.0  *2  # Pupil rotates by 2x the rerotator angle

    app.log( 'centerTT(): starting TT centering procedure')

    while not app.TimeToDie():
        dx, dy = frames_lib.get_pupils_delta( frames_lib.averageFrames( app.ccd39.get_frames(data['numFrames'])))

        v_len1 = math.sqrt(data['offx_dx']*data['offx_dx'] + data['offx_dy']*data['offx_dy'])
        v_len2 = math.sqrt(data['offy_dx']*data['offy_dx'] + data['offy_dy']*data['offy_dy'])

        x = (dx*data['offx_dx'] + dy*data['offx_dy']) / v_len1
        y = (dx*data['offy_dx'] + dy*data['offy_dy']) / v_len2

        xError = x*math.cos(ang) + y*math.sin(ang)
        yError = -x*math.sin(ang) + y*math.cos(ang)

        app.log( 'centerTT(): X,Y errors = %5.3f, %5.3f' % (xError, yError))
    
        # Done?
        if (abs(xError) < data['maxError']) and (abs(yError) < data['maxError']):
            app.log( 'centerTT(): target reached.')
            return

        xError = xError * data['gain'] 
        yError = yError * data['gain'] 

        app.log( 'centerTT(): correcting by %5.3f, %5.3f' % (xError, yError))
    
        offx += xError
        offy += yError
    
        app.tt.set( amp, freq, offx, offy)

        time.sleep(0.1)


def usage():
    print 'Usage:  centerPupils.py [-calib | -center] [-stages] [-tt] [-focus] [-cameralens] [-th <n>]'

def run():
    import sys
    from AdOpt import thAOApp

    if len(sys.argv)<2:
        usage()
        sys.exit(0)

    useStages = False
    useTT     = False
    useFocus  = False
    useCameralens = False
    cameraLensTh=0

    doCenter = False
    doCalib  = False

    if '-center' in sys.argv:
        doCenter = True
    if '-calib' in sys.argv:
        doCalib = True

    if '-stages' in sys.argv:
        useStages = True
    if '-tt' in sys.argv:
        useTT = True
    if '-focus' in sys.argv:
        useFocus = True
    if '-cameralens' in sys.argv:
        useCameralens = True
    if '-th' in sys.argv:
        cameraLensTh = float(sys.argv[ sys.argv.index('-th')+1])

    if useStages == False and useTT == False and useFocus == False and useCameralens == False:
        useStages = True
        useTT = True
        useFocus = True

    app = thAOApp('autocenter', argv = sys.argv)
    
    if doCalib:
        calibCentering(app, useStages = useStages, useTT = useTT, useFocus = useFocus)
    elif doCenter:
        centerPupils(app, useStages = useStages, useTT = useTT, useFocus = useFocus, useCameralens = useCameralens, cameraLensTh = cameraLensTh)
    else:
        usage()

if __name__ == "__main__":
    run()

    


