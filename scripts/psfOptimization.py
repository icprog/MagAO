
# Script to optimize a PSF changing the slope null, using IRTC psf

# IM_file: complete path of IM file to use
# n_modes: first N modes of the IM file to use for optimization
# amp_factor: amplitude factor (over IM amplitude) to explore
# n_steps: number of steps in amplitude 

# Loop is assumed to be already closed with slopenull zero, and
# light levels setup to have a good psf.

from AdOpt import fits_lib
from AdOpt.wrappers import idl
import numpy

def optTest( app, pippo):
    print pippo
    print app.ccd39.xbin()


def optimizePSF( app, IM_file, n_modes, amp_factor, n_steps):

    # Temporary file for slopenull
    tmpSlopenullFile = '/tmp/slopenull.fits'

    im = fits_lib.readSimpleFits(IM_file)
    hdr = fits_lib.readFitsHdr(IM_file)

    modalCmdFile = hdr['M_DIST_F']
    modalCmd = fits_lib.readSimpleFits('/towerdata/adsec_calib/'+modalCmdFile)
    modalAmp = modalCmd.max(1)

    # Go back to raw slopes
    for m in range(n_modes):
        im[:,m] *= modalAmp[m]*2 

    # Base slopenull
    base = numpy.zeros(1600, dtype = numpy.float32)
   
    for m in range(n_modes):
        mode = im[:,m]

        fwhm = numpy.zeros( n_steps, dtype = numpy.Float32 )
 
        for s in range(n_steps):
            amp = (float(amp_factor)*2/n_steps)*s - amp_factor


            slpnull = base + mode * amp 
            fits_lib.writeSimpleFits(tmpSlopenullFile)

            # Load slopenull on to WFS
            app.sc.set_slopenull(tmpSlopenullFile)

            # Get a frame from IRTC
            img = app.irtc.get_frame(pathOnly = True)

            # Analyse with IDL
            cmd = 'psf_fwhm, "%s", FWHM = FWHM' % img
            idl.ex(cmd)
            fwhm[s] = idl.var.fwhm

            print 'Mode %d, amp %f --> fwhm %f' % (m, amp, idl.var.fwhm)

        bestPos = int( numpy.where( fwhm = fwhm.min())[0])

        # Calculate next base slopenull
        amp = (float(amp_factor)*2/n_steps)*bestPos - amp_factor
        base = base + mode * amp

        print 'Selected amp %f' % amp
             
    writeSimpleFits('slopenull_base.fits', base)    
