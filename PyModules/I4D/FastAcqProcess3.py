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
from I4D import Constant4d
# Process mesurements (.dat files) and output hdf5 format (.h5 files)
#    sourceDir: relative path of the '.dat' files
#    measPrefix: prefix of each file ('_nnnn' is automatically append)
#    nMeas: number of files to process
#
# DAT files are searched in 'Constants.I4D_DATA_PATH/sourceDir/' folder
# HDF5 files are stored in 'Constants.I4D_DATA_PATH/sourceDir/hdf5' folder
def datToH5(sourceDir, measPrefix, nMeas,want_frames):
    # Define source directory
	sourceAbsoluteDir = Constant4d.datadir + '/' + sourceDir + '/calibrated'
	print sourceAbsoluteDir
    #if Constants.I4D_CALIBRATE_RAW_FRAMES: 
     #   sourceAbsoluteDir += 'calibrated'
    #else:
     #   sourceAbsoluteDir += Constants.I4D_RAW_DATA_PATH
    
    # Check source directory
    #if not os.path.exists(sourceAbsoluteDir):#was: if not
     #   raise Exception, "Source directory doesn't exist"
    
    # Define dest directory  
	destAbsoluteDir = Constant4d.datadir + '/' + sourceDir + '/' + Constants.I4D_HDF5_DATA_PATH
	disp=CreateProgressWindow(title='HDF5 data reduction', maximum=nMeas)
	if not os.path.exists(destAbsoluteDir):
		os.mkdir(destAbsoluteDir)
    
    
    # Process
	for i in range(nMeas):
		bar=ProgressWindow.Update(disp, i, new_message='Analizing frames, %(c).2f minutes remaining'%{'c':(0.8*(nMeas-i)/60.)})
		fileName = '%s_%04d' % (measPrefix, i)
		fileNameF = '%s_frames_%04d' % (measPrefix, i)
		meas = FastAcqLoad.MeasurementFromDisk(sourceAbsoluteDir + '/' + fileName  + '.dat')
        	if meas:
            		# Do post-processing here: compute SURFACE
			dataset = meas.GetDataset('analyzed', 'dataset')#genraw era analyzed
			print 'Analized '+str(i)+'/'+str(nMeas)+' files'
			res = SaveMeasurement(dataset, destAbsoluteDir + '/' + fileName + '.h5')
			if not res:
				raise Exception, 'datToH5 failed saving analyzed measurement'
		if want_frames==1:
			try:
				meas.WriteToHDF5(destAbsoluteDir + '/' + fileNameF + '.h5')
			except Exception, e:
				raise Exception, 'datToH5 failed saving frames measurement'
	
	ProgressWindow.Close(disp)
# Todo if needed       
def datToFits():
    None
    
