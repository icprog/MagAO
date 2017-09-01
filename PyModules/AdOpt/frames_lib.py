#!/usr/bin/env python2
#
#+File: frames_lib.py
#
# Python utility functions for frame calculations.
#
from numpy import *
import os, time, pyfits, types

from AdOpt import fits_lib, cfg, AOConstants



#+Function: saveFrame
#
# Saves a frame read from the BCU into a FITS file
#-

def saveFrame( frame, filename, dict = None):
    '''
    Save a frame or a series of frames on disk.
    Parameters:
        frame: frame array
        filename: name of the file to write (FITS file)
        dict: optional dict with file metadata (default: None)
    '''
       
    fits_lib.writeSimpleFits( filename, frame, dict)    


#+Function: averageFrames
#
# Computes the average of a series of frames, compressing the first index
#
# frames is assumed to be 3D series of frames. If a 2D series is passed,
# it will not be modified
#-
def averageFrames(frames, useFloat=False):

    if len(frames.shape) == 2:
        return frames
    if frames.shape[0] == 1:
        return frames[0,:,:]
    if frames.shape[0] == 0:
        return 0

    t = frames.dtype
    out_t = t
    if useFloat:
        t = float32
        out_t = t

    # Promote Int16 array to Int32 to avoid overflows
    if t == int16:
        t = int32

    n_frames = frames.shape[0]
    avg = frames[0,:,:].astype(t)

    if n_frames>1:
        for i in range(1,n_frames):
            avg += frames[i,:,:].astype(t)
        avg /= n_frames

    return avg.astype(out_t)


#+Function: splitQuadrants
#
# Given a frame, splits it into four quadrants
# returns a tuple with the four quadrants
#
# frame is assumed to be a bidimensional array
# the frame width must be divisible by two


def splitQuadrants(frame):

    q_dx = frame.shape[0]/2
    q_dy = frame.shape[1]/2

    quad1 = frame[0:q_dx, 0:q_dy]
    quad2 = frame[0:q_dx, q_dy:]
    quad3 = frame[q_dx:, 0:q_dy]
    quad4 = frame[q_dx:, q_dy:] 

    return (quad1, quad2, quad3, quad4)


#@Function: splitHalves
#
# Splits a frame into two halves

def splitHalves(frame):

    dy = frame.shape[1]/2

    half1 = frame[:,0:dy]
    half2 = frame[:,dy:]

    return (half1, half2) 
   

##@Function: findPupils
#
# Finds the center & radius of one or four pupils in a ccd frame
# If amoeba=True, uses the IDL amoeba routine called from pupilfit.pro,
# otherwise uses a threshold method.
# In the amoba algorithm, th1 and th2 are ignored, binning is required.
# In threshold algorithm, th1 and th2 are required, binning is ignored.

def findPupils( frame, th1, th2, fourPupils=False, chdir=None, app=None, binning=None, method='separate'):
    '''
        Finds center & radius of pupil in a ccd frame. Parameters:
            - frame: 2D ccd frame to analyze
            - th1:   first threshold
            - th2:   second threshold
            - fourPupils: if True, searches for four pupils instead of one

        Returns a list of pupil objects, each of which has defined the members: diameter, cx, cy and side.
        The optional app arguments allow logging to log file of the data.
    '''

    if not locals().has_key('idl'):
        from AdOpt.wrappers import idl

    start = time.time()

    if fourPupils:
        split =1
    else:
        split =0

    import tempfile, os
    a = tempfile.mkstemp('.fits','pupils_file','/tmp')
    os.close(a[0])
    tmp_fits_file = a[1]

    a = tempfile.mkstemp('','pupdata','/tmp')
    os.close(a[0])
    tmp_pupils = a[1]

    if chdir != None:
        idl_cmd = 'cd,\'%s\' & ' % chdir
    else:
        idl_cmd = ''

    if method == 'amoeba':
        idl_cmd += "wfs_pupilfit, '%s', %d, /QUIET, OUTFILE='%s', SPLIT=%d" % (tmp_fits_file, binning, tmp_pupils, split)
    elif method == 'separate':

        idl_cmd += "distanza_centri,'%s', IM_TYPE='FITS', m, w, diagonale_lato, media_d, /QUIET, OUTFILE='%s',"  % (tmp_fits_file, tmp_pupils)
        idl_cmd += "TH1=%7.3f, TH2=%7.3f, SPLIT=%d" % (float(th1), float(th2), split)

    elif method == 'sum':
        idl_cmd = "acqpupil_sum, '%s', SPLIT=%d, OUTFILE='%s', TH1=%7.3f, TH2=%7.3f, BINNING=%d" % (tmp_fits_file, split, tmp_pupils, float(th1), float(th2), binning)
        
    else:
        print 'Unknown method %s' % method

    # Clear old data, if any
    if os.access( tmp_fits_file, os.F_OK):
        os.unlink( tmp_fits_file)
    if os.access( tmp_pupils, os.F_OK):
        os.unlink( tmp_pupils)

    fits_lib.writeSimpleFits( tmp_fits_file, frame)

    idl.ex(idl_cmd)
    idl.ex('retall')

    try:
        pupils = readCurPupils( filename = tmp_pupils)
    finally:
        # Clear data in any case, and then propagate the exception
        if os.access( tmp_fits_file, os.F_OK):
            os.unlink( tmp_fits_file)
        if os.access( tmp_pupils, os.F_OK):
            os.unlink( tmp_pupils)

    end = time.time()
    if app:
        app.log('findPupils() elapsed time: %5.3f seconds' % (end-start))
        app.log('findPupils() data: '+str(pupils))

    return pupils

class pupil:
    def __repr__(self):
        if hasattr(self, 'dx'):
            return '%7.3f - %7.3f - %7.3f - %7.3f - %7.3f - %7.3f' % (self.cx, self.cy, self.dx, self.dy, self.obs, self.side)
        else:
            return '%7.3f - %7.3f - %7.3f - %7.3f' % (self.diameter, self.cx, self.cy, self.side )


def getCurPupilsFromRTDB(app, next=False):
    '''
    Reads pupil information from RTDB.
    '''

    pupils = []
    for pup in range(4):
        varname = cfg.varname('pupilcheck39', 'PUP%d' % pup)

        # Poll for next update if requested. Exit after 100 seconds.
        if next:
            count=0
            t = app.ReadVar(varname).MTime() 
            while app.ReadVar(varname).MTime() == t:
                time.sleep(1)
                count += 1
                if count>100:
                    raise AdOptError( AOConstants.AOErrCode['TIMEOUT_ERROR'], 'Timeout waiting for pupil update.')

        v = app.ReadVar(varname).Value()
        p = pupil()
        p.diameter, p.cx, p.cy, p.side, p.diffx, p.diffy = map( float, v)
        pupils.append(p)

    return pupils

def readCurPupils( filename):
    '''
    Reads pupil information from disk.
    '''

    lines = open( filename).readlines()
    n_pup = len(lines)

    pupils = []
    for n in range(n_pup):
        parts = lines[n].split()
        if len(parts) == 4:

            # Threshold algorithm with four numbers per pupil
            p = pupil()
            p.diameter = float(parts[0])
            p.cx = float(parts[1])
            p.cy = float(parts[2])
            p.side = float(parts[3])

            pupils.append(p)

        elif len(parts) == 6:

            # Amoeba algorithm with six numbers per pupil
            p = pupil()
            p.cx = float(parts[0])
            p.cy = float(parts[1])
            p.dx = float(parts[2])
            p.dy = float(parts[3])
            p.obs = float(parts[4])
            p.side = float(parts[5])
            p.diameter = (p.dx + p.dy)   # Retained for backward compatibility
           
            pupils.append(p)

        else:
            raise AdOptError( AOConstants.AOErrCode['MALFORMED_FILE_ERROR'], 'Pupil file format not recognized')

    return pupils

def get_pupils_delta(frame, useIndpup = None):
    '''
    Computes the amount of light decentering on the four ccd quadrants.
    Amplitude is normalized from -1.0 to +1.0.

    If an indpup file is specified, only uses the pixels inside the pupils
    Returns a (dx,dy) tuple.
    '''


    # Compute pixel sum for quadrant pairs top/bottom and left/right
    if useIndpup == None:
        q = splitQuadrants(frame)
        top = q[0].sum() + q[2].sum()
        bottom = q[1].sum() + q[3].sum()
        left = q[0].sum() + q[1].sum()
        right = q[2].sum() + q[3].sum()
        total = frame.sum()
    else:
        indpup = map( int, file(useIndpup).read().splitlines())
        nsubap = len(indpup)/4
        q3 = frame.flat[ indpup[ nsubap*0:nsubap*1]].sum()
        q4 = frame.flat[ indpup[ nsubap*1:nsubap*2]].sum()
        q2 = frame.flat[ indpup[ nsubap*2:nsubap*3]].sum()
        q1 = frame.flat[ indpup[ nsubap*3:nsubap*4]].sum()
        top = q1+q3
        bottom = q2+q4
        left = q1+q2
        right = q3+q4
        total = q1+q2+q3+q4
    

    if (right+left == 0) or (top+bottom==0):
        raise AdOptError( errstr='Cannot calculate intensity: some quadrants have 0 counts')

    # Compute X and Y differences
    dx = float(right - left) / total
    dy = float(top - bottom) / total
   
    return (dx,dy)

def get_pupils_focus(frame, pupils):
    '''
    Computes the amount of defocus looking at the pupil shape:
    each pupil is splitted in four and the totals of the center-ward quadrant
    and the opposite one are calculated. The difference is the amount of defocus.
    The four pupil values are averaged.
    '''

    debug = 0

    pup_defocus = [0.0] * 4

    for pup in range(4):

        cx = int(pupils[pup].cx)
        cy = int(pupils[pup].cy)
        diam = int(pupils[pup].diameter)

        x1 = cx - diam/2
        x2 = cx + diam/2
        y1 = cy - diam/2
        y2 = cy + diam/2

        if pup == 0:
            quad1 = frame[cx:x2, y1:cy]
            quad2 = frame[x1:cx, cy:y2]
        if pup == 1:
            quad1 = frame[x1:cx, y1:cy]
            quad2 = frame[cx:x2, cy:y2]
        if pup == 2:
            quad1 = frame[cx:x2, cy:y2]
            quad2 = frame[x1:cx, y1:cy]
        if pup == 3:
            quad1 = frame[x1:cx, cy:y2]
            quad2 = frame[cx:x2, y1:cy]

        pup_defocus[pup] = (float(quad1.sum()) - float(quad2.sum())) / ( float(quad1.sum()) + float(quad2.sum()))

        if debug:
            print 'pup %d - quadrants %d - %d -- defocus %5.3f' % (pup, quad1.sum(), quad2.sum(), pup_defocus[pup])

    defocus = reduce( lambda x,y: x+y, pup_defocus) /4
  
    if debug: 
        print 'get_pupil_focus(): total defocus = %5.3f' % defocus
    return defocus

def findStar(frame, completeInfo = False, fwhm_px=40):
    '''
    Analyze the frame and returns the x,y coordinates and the flux of the brightest star found.
    <fwhm> is the expected fwhm of the star in pixels. This parameter will be used to initialize
    the gaussian filter when using the find.pro algorithm.
    If <completeInfo> is set to True, returns a list of all stars found. Each
    star is an object with members x, y, flux, sharpness and roundness.
    If nothing is found, the list will be empty. In this case, if <completeInfo> is not set,
    three -1 numbers will be returned.
    '''

    if cfg.useStarFindPython:
        window = 50
        soglia = frame.max()
        pos = where( frame == soglia)
        cx = pos[0][0]
        cy = pos[1][0]
        x1 = cx - window/2
        x2 = cx + window/2
        y1 = cy - window/2
        y2 = cy + window/2

        if x1 <0:
            x1=0
        if y1 <0:
            y1 =0
        if x2 >= frame.shape[0]:
            x2 = frame.shape[0]-1
        if y2 >= frame.shape[1]:
            y2 = frame.shape[1]-1

        ww =frame[x1:x2,y1:y2]

        indx = indices((window,window))
        x = (ww*indx[1]).sum() / ww.sum()
        y = (ww*indx[0]).sum() / ww.sum()

        return x +x1, y+y1, 0

    elif cfg.useStarFindIDL:

        if not locals().has_key('idl'):
            from AdOpt.wrappers import idl
	

        # Save frame into a FITS file if needed
        if type(frame) != types.StringType:
            tmpImgFile = os.tmpnam() + '.fits'
            fits_lib.writeSimpleFits( tmpImgFile, frame, {})
            frame = tmpImgFile
        hdr = fits_lib.readFitsHdr(frame)
        img = fits_lib.readSimpleFits(frame)

        th = 100.0
        if img.max() <th:
            if completeInfo:
                return []
            else:
                return -1, -1, -1
        

        cmd = "find_star, '%s', %5.2f, %5.2f, nstars, x, y, flux, sharp, round, /USE_FIT, FIT_TH=%7.3f" % (frame, th, fwhm_px, th)
        idl.ex(cmd)
        idl.ex('retall')

        if 1:
            print 'Found stars: ',idl.var.nstars
            for n in range(idl.var.nstars):
                print 'X: %5.2f  Y: %5.2f  Flux: %6d  sharpness: %5.2f   roundness: %5.2f' % (idl.var.x[n], idl.var.y[n], int(idl.var.flux[n]), idl.var.sharp[n], idl.var.round[n])

        if completeInfo:
            class star:
                def __repr__(self):
                    return 'Pos: %5.2f,%5.2f - Flux: %5.2f Roundness: %5.2f' % (self.x, self.y, self.flux, self.round )

            stars=[]

            for n in range(idl.var.nstars):
                s = star()
                s.x = float(idl.var.y[n])
                s.y = float(hdr['NAXIS2'] - idl.var.x[n])
                s.flux = float(idl.var.flux[n])
                s.sharp = float(idl.var.sharp[n])
                s.round = float(idl.var.round[n])
                stars.append(s)

            return stars
        else:

            if idl.var.nstars>0:
                x = float(idl.var.y[0])
                y = float(hdr['NAXIS2'] - idl.var.x[0])
                flux = float(idl.var.flux[0])
                return x,y, flux
            else:
                return -1, -1, -1

def gaussFit( img, dark):
    '''
    Perform a 2D gaussian fit on an image using the fwhm_single routine.
    Returns a tuple with the estimated FWHM and the two x and y corrections
    to center the PSF on a pixel.

    <img> can be both a filename or an image. In the latter case, it will be saved in a temporary file for analysis.
    <dark> idem
    '''

    tmpImgFile = None
    tmpDarkFile = None

    import types, os

    if type(img) != types.StringType:
        tmpImgFile = os.tmpnam() + '.fits'
        fits_lib.writeSimpleFits( tmpImgFile, img, {})
        img = tmpImgFile
    if type(dark) != types.StringType:
        tmpDarkFile = os.tmpnam() + '.fits'
        fits_lib.writeSimpleFits( darkDarkFile, dark, {})
        dark = tmpDarkFile

    import idl
    cmd = "fwhm_single, '%s', '%s', coeff, fwhm, xcorr, ycorr" % ( img, dark)

    idl.ex(cmd)

    fwhm = idl.var.fwhm
    xcorr = idl.var.xcorr
    ycorr = idl.var.ycorr

    if tmpImgFile != None:
        os.unlink(tmpImgFile)
    if tmpDarkFile != None:
        os.unlink(tmpDarkFile)


    return (fwhm, xcorr, ycorr)

#@Function: rebin
#
# Rebins a 2d numarray to a new dimension.
#
# Final dimensions can be different for each axis, but must be an integer
# multiple or divisor of the starting dimension. For multiple dimensions,
# block-resampling is used. For divisors, sparse sampling (1 out of n) is used.
#@

def rebin( array,      #@P{array}:  input array, 2d
            xdim,      #@P{xdim}:   desired final dimension on the X axis
            ydim):     #@P{ydim}:   desired final dimension on the Y axis
                       #@R:         scaled array

    if xdim > array.shape[0]:
        xscale = xdim / array.shape[0]
        a2 = array.repeat(xscale,0)

    elif xdim < array.shape[0]:
        xstride = array.shape[0] / xdim
        a2 = array[::xstride,:]

    else:
        a2 = array

    if ydim > array.shape[1]:
        yscale = ydim / array.shape[1]
        return a2.repeat(yscale,1)

    elif ydim < array.shape[1]:
        ystride = array.shape[1] / ydim
        return a2[:,::ystride]

    else:
        return a2
        


