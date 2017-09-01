
# Per ogni modo:
#  Per ogni ampiezza:
#   Genera vettore SN modo/ampiezza
#   Applica vettore SN
#   Misura PSF (save elab-lib psf)
#   Ripeti

###################
#
# Configuration starts


# Which ccd to use (either 'IRTC' or 'PISCES')
ccd = 'PISCES'

# Test (do not apply slopenull and read frames, just print which slopenull would be applied)
test = False

# Modal basis and intmat to use (one intmat for each binning)

m2c = 'KL_v7'
intmats=['','20101024_092604', '20101119_180534', '', '']

# PSF position
cx = 600 # 475 # 165
cy = 619 # 555 # 133
mask = 50

# Modes to apply and relative amplitude (from the intmat signals)
modes = [2,3,4] #5,6,7,8,9,10,11] #,7,8,9,10]

amp_min = -0.4e-7
amp_max = 0.4e-7
amp_steps = 6   # Minimum 3, otherwise it will enter some infinite loop

if test:
    amp_steps=3

# CCD parameters: no. of frames and exposure time (seconds)
ccd_nf  = 1
ccd_exp = 10    # Exptime for pisces is ignored, use PiscesGui

# Filename to use for temporary slope offsets (in slopenull directory)
tmpfile = 'nc.fits'

# Output filename (final slope offset)
outfile = 'total_nc.fits'

# Resume from a certain point
resume=0      # From which mode to resume (0= first mode), 0 = do not resume
#res=[0]*resume
#res[0] = -2.14e-08
#res[1] = -2.67e-08
#res[2] = 3.46e-09



#
#
# Configuration ends
#
###################

b = ccd39.xbin()
intmat = intmats[ccd39.xbin()]

imfile = '/towerdata/adsec_calib/M2C/%s/RECs/Intmat_%s.fits' % (m2c, intmat)

im = fits_lib.readSimpleFits(imfile)

total_offset = im[:,0]*0  # Total slope offset vector
best = [0] * len(modes)              # Vector to hold the best modal coefficients

if resume >0:
    for i in range(resume):
        best[i] = res[i]
        total_offset += im[:,modes[i]] * best[i]

from AdOpt import irc
from AdOpt.wrappers import idl
import time

day = time.strftime('%Y%m%d')
datadir = '/towerdata/adsec_data/'+day

def apply_offset(sl_offset, hdr = {} , useFile = None):
    if useFile == None:
      useFile = tmpfile
    path = os.path.join( calib.slopenullDir(app.ccd39.xbin()), useFile)
    fits_lib.writeSimpleFits(path, sl_offset, hdr, overwrite=True)
    if not test:
      sc.set_slopenull(useFile)

def analyse(tracknums, amps):

  import numpy
  peaks = numpy.zeros(len(amps))
  fhwms = numpy.zeros(len(amps))
  energy = numpy.zeros(len(amps))
  for i in range(len(tracknums)):
    filename = datadir+'/Data_%s/%s.fits' % (tracknums[i], ccd.lower())
    idl.ex('psf_fwhm, "%s", FWHM = FWHM, ENERGY = ENERGY, MASK = %d, CX = %d, CY= %d, PEAK = PEAK'% (filename, mask, cx, cy))
    peaks[i] = idl.var.peak
    energy[i] = idl.var.energy
    fhwms[i] = idl.var.fwhm

  for i in range(len(tracknums)):
    print 'Step %d - amp %5.3g - peak %5.3f - peak/energy %5.3f' % (i, amps[i], peaks[i], peaks[i]/energy[i])

  ff = file('/tmp/peaks.txt', 'w')
  for i in range(len(tracknums)):
    ff.write('%5.3f\n' % peaks[i])
  ff.close()

  peaks = list(peaks)
  mm = max(peaks)
  if peaks.index(mm) == len(peaks)-1:
    ampmax = amps[-1]
  if peaks.index(mm) == 0:
    ampmax = amps[0]
  else:
    fit = numpy.polyfit(amps, peaks, 2)
    ampmax = -fit[1]/ (2*fit[0])

  print 'Maximum amp: %5.3g' % ampmax


  return ampmax
  

n_steps = amp_steps+1
amps = [0]*n_steps
amp_step = (amp_max-amp_min)/amp_steps
for m in range(resume, len(modes)):
  mode = modes[m]
  print
  print 'Starting mode %d' % mode
  print
  correction = 0.0
  while 1:
    for step in range(n_steps):
      amps[step] = amp_min + amp_step * step + correction
      sl_offset = im[:,mode] * amps[step]
      print 'Minmax mode offset: %5.3f, %5.3f' %( min(sl_offset), max(sl_offset))
      sl_offset += total_offset
      print 'Minmax total offset: %5.3f, %5.3f' %( min(sl_offset), max(sl_offset))
      apply_offset(sl_offset)
      if not test:
        time.sleep(1)
        if ccd=='irtc':
          irc.single_psf( ccd_exp, ccd_nf)
        else:
          irc.pisces_psf( ccd_nf)
        
    print 'Amps: ', amps
    tracknums = map( lambda x: x[-15:], sorted(os.listdir(datadir))[-n_steps:])
    best[m] = analyse(tracknums, amps)
    if (best[m] > amp_min + correction + amp_step) and (best[m] < amp_max + correction - amp_step):
      break
    if (best[m] <= amp_min + correction + amp_step):
      correction -= (amp_max-amp_min)/2
    else:
      correction += (amp_max-amp_min)/2
    print 'Correcting range to %05.3g' % correction
    if test:
        break
    
  total_offset += im[:,mode] * best[m]

tracknum = calib.getDateTime()+".fits" 
pupils = sc.get_pixellut()[-28:-7]  #extract pupil binning and tracknum

# Build fits header
hdr ={}
hdr['sc.PUPILS'] = pupils
hdr['nc.INTMAT'] = intmat
for m in range(len(modes)):
  hdr['nc.M%02d' % modes[m]] = '%5.3g' % best[m]

path = os.path.join( calib.slopenullDir(app.ccd39.xbin()), tracknum)
print 'Best modal coefficients:'
for m in range(len(modes)):
  print 'Mode ',modes[m],' coefficient %5.3g' % best[m]
print 'Saving total offset: '+path

apply_offset(total_offset, hdr, useFile = tracknum)


