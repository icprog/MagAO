#
# Script di acceptance test

class point:
  def __init__(self, lamp, aux, irtc, disturb = 'auto'):
    self.lamp = lamp
    self.aux  = aux
    self.irtc = irtc
    self.disturb = disturb

# Measure definition. Each point has three arguments:
# 2. Lamp setting
# 3. Aux FW setting
# 4. IRTC exposure time (s)

points = {}

#points[8.5] = point( 126, 6, 0.5) # seeing 0.6, TTM
#points[8.5] = point( 126, 6, 0.05) # seeing 0.4
#points[8.5] = point( 126, 6, 0.05) # seeing 0.6
points[8.5] = point( 126, 6, 0.05) # seeing 0.8
#points[8.5] = point( 126, 6, 0.10) # seeing 1.2 
#points[8.5] = point( 126, 6, 0.10) # seeing 1.5 
#points[9.5] = point( 111, 6, 0.07)  # seeing 0.8
#points[10.5] = point( 130, 1, 0.07)
#points[11.5] = point( 115, 1, 0.100)
#points[12.5] = point( 104, 1, 0.500) # seeing 0.6, TTM 
#points[12.5] = point( 104, 1, 0.100) # seeing 0.4
#points[12.5] = point( 104, 1, 0.100) # seeing 0.6
#points[12.5] = point( 104, 1, 0.150) # seeing 0.8
#points[12.5] = point( 104, 1, 0.300) # seeing 1.2
#points[12.5] = point( 104, 1, 0.500) # seeing 1.5
#points[13.5] = point( 119, 2, 0.100)
#points[14.5] = point( 115, 3, 0.150) # seeing 0.4
#points[14.5] = point( 115, 3, 0.150) # seeing 0.6
#points[14.5] = point( 115, 3, 0.300) # seeing 0.8
#points[14.5] = point( 115, 3, 0.600) # seeing 1.2
#points[14.5] = point( 115, 3, 0.600) # seeing 1.5
#points[15.5] = point( 106, 3, 0.400)
#points[16.5] = point( 127, 5, 0.400)

#points[9.5] = point( 255, 3, 0.050)    # fibra 200um mag 9.5
#points[10.5] = point( 91, 3, 0.100)    # fibra 200um mag 10.5

#points[8.5] = point( 98, 3, 0.050)   # fibra 365um  mag 8.5
#points[9.5] = point( 92, 3, 0.100)   # fibra 365um  mag 9.5
#points[10.5] = point( 87, 3, 0.250)   # fibra 365um  mag 10.5

#points[8.5] = point( 102, 5, 0.100)   # fibra 1mm  mag 8.5
#points[9.5] = point( 94, 5, 0.200)   # fibra 1mm  mag 9.5
#points[10.5] = point( 87, 5, 0.500)   # fibra 1mm  mag 10.5

#points[8.5] = point( 94, 3, 0.05)   # fibra 600um  mag 8.5
#points[9.5] = point( 89, 3, 0.100)   # fibra 600um  mag 9.5
#points[10.5] = point( 85, 3, 0.200)   # fibra 600um  mag 10.5

# aomode can be either 'ACE-AO' or 'TTM-AO'
#aomode = 'ACE-AO'
aomode = 'ACE-AO'

# IRTC exposure in seconds
irtc_sec = 15.0

# Spot voluto
# True : rightmost (correct) spot
# False: leftmost  ("wrong") spot
use_rightspot = True

# Z focus position for each AUX filter
#zfocus = [50, 55.0, 53.6, 54.3, 50.0, 54.20, 55.65]
zfocus = [50, 55.0, 53.6, 54.3, 50.0, 33.00, 55.65]
if not use_rightspot:
   for i in range(len(zfocus)):
      zfocus[i] -= (55.65-38.00)

# Expected z focus after PresetAO (from board setup)
zsetup = 50.0

# Stage X and Y position to be in the narrow-field IRTC center
xc = -80.3 
if use_rightspot:
   yc =  36.5   # Spot centrale
else:
   yc =  40.0   # Spot sbagliato

# Autogain mode: either 'old' or 'new'
autogain_mode = 'old'

# Available disturbance frequencies
disturbs = [625,800,1000]

# Seeing realization: either 1983 or 2891
seed = 2891

# Seeing to use
# Available: '0.4', 0.6', '0.8', '1.0', '1.2', '1.5', 'OFF'
seeing = '0.8'

# Disturb file pattern
disturb_pattern = 'dist_flao2aatm_s%s_L040.0_v15.0_ovfreq%d.00_sd%d.fits'
#disturb_pattern = 'dist_flao2aatm+vib_s%s_L040.0_v15.0_ovfreq%d.00_sd%d_vibtr_20110427_150440.fits'

from AdOpt import irc
import os, time, sys

def closest(target, collection) :
  return min((abs(target - i), i) for i in collection)[1]

for mag in sorted(points.keys()):

  p = points[mag]

  # Set lamp and aux FW. Correct focus

  print 'Setting LAMP to %d and AUX filterwheel to %d' % (p.lamp, p.aux)
  os.system('aux.sh %d' % p.aux)
  lamp.setIntensity( p.lamp, waitTimeout=120)

  # Setup and close AO loop

  irc.setReference(mag)
  irc.presetAO(aomode)
  irc.acquireRefAO()

  time.sleep(5)

  # Apply disturbance

  if seeing != 'OFF':
    hz = ccd39.get_framerate()
    if hz > 450:
      dist_hz = closest(hz, disturbs)
      mode = 'sync'
    elif hz >=300 and hz<320:
      dist_hz = 625
      mode = 'ovs'
    else:
      dist_hz = 800
      mode = 'ovs'
    print 'Ccd freq is %d Hz -> selected disturbance %d, mode %s, seeing %s, seed %d' % (hz, dist_hz, mode, seeing, seed)
    irc.disturb( filename = disturb_pattern % (seeing, dist_hz, seed), mode = mode)
  else:
    irc.disturb(mode='off')  

  irc.startAO()

  time.sleep(10)  # Centraggio lente di camera...
  irc.cameralens(False)
  irc.autogain( autogain_mode)
  irc.cameralens(True)

  # Correct focus and centering
  irc.offsetZ(zfocus[p.aux]- zsetup)

  x= app.stagex.getPos()
  y= app.stagey.getPos()
  irc.offsetXY( xc-x, yc-y)
  
  # Acquire IRTC tracknums
  nframes = int( irtc_sec / p.irtc)
  irc.psf( p.irtc, nframes)

  irc.stopAO()

