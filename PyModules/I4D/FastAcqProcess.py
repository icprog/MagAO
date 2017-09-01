#!/usr/bin/env python

# @File: FastAcqProcess.py
# Defines a set of processing functions for data streamed to disk
# @

# import all 4D libraries
from Scripting.App import *
from Scripting.Modify import *
from Scripting.Data import *
from Scripting.Config import *
from Scripting.Measurements import *
from Scripting.Show import *

import FastAcqLoad

from Commons import Constants

# Process mesurements (.dat files) and output hdf5 format (.h5 files)
#    sourceDir: relative path of the '.dat' files
#    measPrefix: prefix of each file ('_nnnn' is automatically append)
#    nMeas: number of files to process
#
# DAT files are searched in 'Constants.I4D_DATA_PATH/sourceDir/' folder
# HDF5 files are stored in 'Constants.I4D_DATA_PATH/sourceDir/hdf5' folder
def datToH5(sourceDir, measPrefix, nMeas):
    # Define source directory
    sourceAbsoluteDir = Constants.I4D_DATA_PATH + '/' + sourceDir + '/' 
    if Constants.I4D_CALIBRATE_RAW_FRAMES: 
        sourceAbsoluteDir += Constants.I4D_CALIB_DATA_PATH
    else:
        sourceAbsoluteDir += Constants.I4D_RAW_DATA_PATH
    
    # Check source directory
    if not os.path.exists(sourceAbsoluteDir):
        raise Exception, "Source directory doesn't exist"
    
    # Define dest directory  
    destAbsoluteDir = Constants.I4D_DATA_PATH + '/' + sourceDir + '/' + Constants.I4D_HDF5_DATA_PATH
    
    if not os.path.exists(destAbsoluteDir):
        os.mkdir(destAbsoluteDir)
        
    # Process
    for i in range(nMeas):
        fileName = '%s_%04d' % (measPrefix, i)
        fileNameF = '%s_frames_%04d' % (measPrefix, i)
        meas = FastAcqLoad.MeasurementFromDisk(sourceAbsoluteDir + '/' + fileName  + '.dat')
        if meas:
            # Do post-processing here: compute SURFACE
            dataset = meas.GetDataset('analyzed', 'dataset')
            res = SaveMeasurement(dataset, destAbsoluteDir + '/' + fileName + '.h5')
            if not res:
                raise Exception, 'datToH5 failed saving analyzed measurement'
    	    try:
                 meas.WriteToHDF5(destAbsoluteDir + '/' + fileNameF + '.h5')
            except Exceprion, e:
		raise Exception, 'datToH5 failed saving frames measurement'

# Todo if needed       
def datToFits():
    None
    
