#!/usr/bin/python
# Pyro I4D Zernike acquisition local startup
# Put this script in the 4D/scripts directory

import sys
import time

# Add the I4D Pyro Server package location to path
sys.path.append('C:\Obelix_CVS\Supervisor\PyModules')


from I4D import Zernike
from I4D.Commons import Constants

# All parameters are got from Commons.Constants
print 'Waiting %d seconds to start acquisition...' %Constants.I4D_ZERNIKE_START_DELAY_S
time.sleep(Constants.I4D_ZERNIKE_START_DELAY_S)
Zernike.start();

