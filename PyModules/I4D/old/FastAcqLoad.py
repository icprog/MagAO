"""
LoadFastAcq.py

This script demonstrates how to process measurement acquired using 4Sight's
Fast Acquisition capability. There are three sample sections below:

Single Measurement opens and displays a single fast-acquisition measurement.
Grouped Measurement opens and displays a number of measurements as a movie.
Averaged Measurement demonstrates processing a large number of measurements.
"""

from Scripting.App import *
from Scripting.Modify import *
from Scripting.Data import *
from Scripting.Config import *
from Scripting.Measurements import *
from Scripting.Show import *
from PhazeMonkey.Core import Store, Group, MMgr, Units
from PhazeMonkey.Core import Dataset as ds
import time, os.path
from PhazeMonkey.Util import FastAcq

'''
MeasurementFromDisk is a helper function that reads a FastAcq (.dat) 
file and returns a Measurement object.
'''
def MeasurementFromDisk(file_path, title='FAST ACQ Measurement', group=None):
    # Check if the file exists, otherwise retrun None
    if not os.path.exists(file_path):
        return None
    num = 0
    print 'MeasurementFromDisk: reading file %s' %file_path
    arrayAndInfo = FastAcq.ReadRawFromDisk(file_path)
    if arrayAndInfo is not None: # check that it worked
        (array, info) = arrayAndInfo
    
        hint = {'source_desc': 'Stored Interferograms', 'frame_full' : ds.Dataset(arry=array.astype('s')),
                'ordinal': num + 1, 'averaging': 1, 'seq_time': 0,
                'xpix': Units.UV(info.get('xpix_val', 1.), info.get('xpix_unit', 'px')),
                'maxint': 1<<int(info.get('WORD_SIZE', 10)),
                'wedge': info.get('wedge', 0.5),
                'wavelength': Units.UV(info.get('wavelength_val', 632.8), info.get('wavelength_unit', 'nm'))
                }
        meas = Measurement.Measurement('analyzed', hint=hint, title='Meas %d' % (num + 1),
                override_opts = {}, masksrc='runtime', group=group)
        
        meas.SetHeader('head_serial', info.get('head_serial'))
        meas.SetHeader('title', title)
        meas.SetHeader('date', info.get('date'))
        meas.SetHeader('seq_time', info.get('ACQ_TIME'))
        return meas
