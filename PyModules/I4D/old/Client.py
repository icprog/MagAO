#!/usr/bin/env python

# @File: Client.py
# Pyro Client for the 4D interferometer.
# Connect to Pyro Server to remotely call 4D python libraries.
# @

import Pyro.core
import Pyro.naming

from Commons import Constants

import sys, time, os.path
from Numeric import *


class Client:
    
    def __init__(self):
        
        print
        print "  ----------------------------- 4D INTERFEROMETER PYRO CLIENT -----------------------------"
    	print "  -----------------------------------------------------------------------------------------"
    	print "  Pyro config file: %s" %Pyro.config.PYRO_CONFIG_FILE
    	print "  Pyro storage:     %s" %Pyro.config.PYRO_STORAGE
    	print "  Pyro logging:     %s" %Pyro.config.PYRO_USER_LOGFILE
        print
        
        # Init logging facility
        self.userLog = Pyro.util.UserLogger()
        self.logSource = "CLIENT I4D"
        
        # Empty lines in log file !!!
        self.log("---------------- 4D INTERFEROMETER PYRO-CLIENT (TEST) ----------------")
        
        # Init the Pyro client
        Pyro.core.initClient()
        
        self.log("Getting proxy for '%s'..." % Constants.I4D_CONTROLLER_NAME)
        remoteCtrlUri = 'PYROLOC://' + Pyro.config.PYRO_HOST + ':' + str(Pyro.config.PYRO_PORT) + '/' + Constants.I4D_CONTROLLER_NAME
        self.remoteCtrl = Pyro.core.getProxyForURI(remoteCtrlUri)
        
        self.log(self.remoteCtrl.test())
        
        
    def log(self, message):
         self.userLog.msg(self.logSource, message)
         
         
    # Get a new measure and return the number of total measures
    # stored in server's ram
    def getMeasure(self, measuresNum):
        for i in xrange(measuresNum):
            print 'Getting measure %d/%d...' %(i+1, measuresNum)
            print 'Measure got: %d' %(self.remoteCtrl.GetMeasure())
            print ''
        return self.remoteCtrl.GetMeasureCount()

    # Compute the specified dataset of a measure stored in server's ram and 
    # return it as array
    # DatasetId: 0='frames', 1='intensity', 2='surface'  [to check]
    def getDataset(self, measureIndex, datasetId):
        
        datasetName = 'unknown'
        if datasetId == 0:
            datasetName = 'frames'
        elif datasetId == 1:
            datasetName = 'intensity'
        elif datasetId == 2:
            datasetName = 'surface'
        
        if datasetName != 'unknown':
            print 'Getting dataset %s for measure %d...' %(datasetName, measureIndex)
            datasetA = self.remoteCtrl.GetDataset(measureIndex, datasetName)
    
            if datasetA != None:
                centerX = datasetA.shape[0]/2
                centerY = datasetA.shape[1]/2
                radius = 5
                print 'Dataset (%s) subset: ' %str(datasetA.shape)
                print datasetA[centerX-radius:centerX+radius, centerY-radius:centerY+radius]
                print ''
                return datasetA
            else:
                print 'Measure %d not existing on the server !' %measureIndex

    # Compute all surfaces of the measures stored in server's ram and 
    # return it as a list of array
    def getAllSurfaces(self):
        surfaces = []
        for i in xrange(self.remoteCtrl.GetMeasuresCount()):
            surfaces.append(self.getSurface(i))
    
    # Clear all the measures stored in server's ram
    def clearAllMeasures(self):
        self.remoteCtrl.ClearMeasures()
        print 'All measures removed from server'
        
    # Takes a number of measures (burst measure) and store them to server's disk
    # Return some info about the camera and the measures
    def getBurstToDisk(self, enableTrigger, nMeas, filenamePrefix):
        print 'Getting burst measurement (%d  measures) to disk...' %nMeas
        (frameInfo, camInfo) = self.remoteCtrl.GetBurstToDisk(enableTrigger, nMeas, filenamePrefix)
        print 'Burst measurement succesfully completed ! Measures stored on server with prefix %s' %filenamePrefix

    def calibrateMeasures(self):
        print 'Starting measurement calibration...'
        self.remoteCtrl.CalibrateBurst()
        print 'Calibration done'

    # Compute the surface of a measure stored on server's disk 
    # and return it as array
    def getSurfaceFromDisk(self, measureId, filenamePrefix = Constants.I4D_DATA_PREFIX):
        print 'Getting surface for measurement %s from disk...' %(filenamePrefix + str("%04d" %measureId) + ".dat")
        surfaceA =  self.remoteCtrl.GetSurfaceFromDisk(measureId, filenamePrefix)
        if surfaceA != None:
            print 'Surface succesully got !'
            centerX = surfaceA.shape[0]/2
            centerY = surfaceA.shape[1]/2
            radius = 5
            print 'Surface (%s) subset: ' %str(surfaceA.shape)
            print surfaceA[centerX-radius:centerX+radius, centerY-radius:centerY+radius]
            print ''
            return surfaceA
        else:
            print 'Measurement %s not existing on the server !' %(filenamePrefix + str("%04d" %measureId) + ".dat")

    
    def getZernike(self, nSet, setDelay, nMeas, filenamePrefix, processingType, acceptedPerc):
        print 'Waiting %d seconds to start acquisition...' %Constants.I4D_ZERNIKE_START_DELAY_S
        time.sleep(Constants.I4D_ZERNIKE_START_DELAY_S)
        print 'Getting interferograms, intensity, surface and zernikes for %d sets (%d measures/set) with delay %d sec and prefix %s...' %(nSet, nMeas, setDelay, filenamePrefix)
        print 'Processing type: %d (1=acquire only, 2=compute zernike only (frames already acquired), 3=acquire and compute zernike)' %processingType
        self.remoteCtrl.GetZernike(nSet, setDelay, nMeas, filenamePrefix, processingType, acceptedPerc)
        print 'Completed!'

    def shutdownServer(self):
        self.remoteCtrl.ShutdownServer()
        print 'Server shutdown completed !'
        

if __name__=="__main__":
    
    startTime = time.asctime()

    if len(sys.argv) < 2:
        print 'Incorrect number of arguments: please specify a COMMAND'
        print 'Available COMMANDs: '
        #OLD UNUSEFUL COMMANDS
        #print '    GetMeasure [measures number = 1]'
        #print '        Acquire n measures and store them to server memory'
        #print '    GetSurface [measure index = 0]'
        #print '        Get the surface from a measure on server memory and return it locally as array' 
        #print '    GetAllSurfaces'
        #print '        Get all surfaces from all measures on server memory and return it locally as arrayof array'
        #print '    ClearMeasures'
        #print '        Remove all measures from server memory ' 
        #print '    GetBurstMeasure [enable trigger = 0] [measures number = %d] [files prefix = %s]' %(Constants.I4D_DEFAULT_BURST_MEAS_NUM, Constants.I4D_DATA_PREFIX)
        #print '        Acquire a burst of measures and store them to server disk'
        #print '    CalibrateMeasures: calibrates ALL files in the %s directory to %s directory' %(Constants.I4D_RAW_DATA_PATH, Constants.I4D_CALIB_DATA_PATH)
        #print '        Calibrate all measures previously acquired to server disk'
        #print '    GetSurfaceFromDisk [measure id = 0] [files prefix = %s]' %Constants.I4D_DATA_PREFIX
        #print '        Get the surface from a measure on server disk and return it locally as array'
        print '    GetZernike [set number = %d] [set delay = %d s] [measures per set = %d] [files prefix = %s] [processing type = %d] [accepted perc = %d]' %(Constants.I4D_DEFAULT_MEAS_SET_NUM, Constants.I4D_DEFAULT_SET_DELAY_S, Constants.I4D_DEFAULT_BURST_MEAS_NUM, Constants.I4D_DATA_PREFIX, Constants.I4D_DEFAULT_ZERNIKE_PROCESSING_TYPE, Constants.I4D_CHECK_BURST_ACCEPTED_PERCENT)
        print '        Acquire a set of burst measures and store them to server disk. Then calibrate them and optionally'
        print '        process them to obtain intensity, surface and zernike coeff'
        print '        Processing type: 1=acquire only, 2=compute zernike only (frames already acquired), 3=acquire and compute zernike'
        print '        Accepted perc: is the completed percentage of the burst to process it (if processing type is 2 or 3)'
        print '    ShutdownServer'
        sys.exit()
        
    client = Client()
 
    # Catch all exception that can occurs remotely (propageted locally by Pyro)
    try:
        # --- GetMeasure command --- #	    
        if sys.argv[1] == 'GetMeasure':
            # by default get a single measure
            measures = 1
            if len(sys.argv) == 3:
                measures = int(sys.argv[2])    
            client.getMeasure(measures)
    
        # --- GetSurface command --- #
        elif sys.argv[1] == 'GetSurface':
            # by default get the surface of measure 0
            measureIndex = 0
            if len(sys.argv) == 3:
                measureIndex = int(sys.argv[2])  
            client.getSurface(measureIndex) 
    
        # --- GetAllSurfaces --- #
        elif sys.argv[1] == 'GetAllSurfaces':
            client.getAllSurfaces()
    
        # --- Clear all measures from server --- #
        elif sys.argv[1] == 'ClearMeasures':
            client.clearAllMeasures()
    
        # --- Acquire a burst measurement to server disk --- #
        elif sys.argv[1] == 'GetBurstMeasure':
            # Default parameters
            nMeas =  Constants.I4D_DEFAULT_BURST_MEAS_NUM
            filenamePrefix = Constants.I4D_DATA_PREFIX
            enableTrigger = 0
            # Enable or not the trigger
            if len(sys.argv) >= 3:
                enableTrigger = (int(sys.argv[2])== 1)
            # Get the number of measures param
    	    if len(sys.argv) >= 4:
    	        nMeas = int(sys.argv[3])
            # Get the file name prefix param
            if len(sys.argv) >= 5:
                filenamePrefix = sys.argv[4]
            client.getBurstToDisk(enableTrigger, nMeas, filenamePrefix)
        
        # Calibrate all bursts previously acquired on disk
        elif sys.argv[1] == 'CalibrateMeasures':
            client.calibrateMeasures()

        # --- Get the surface dataset as array from a measure stored to disk --- #
        elif sys.argv[1] == 'GetSurfaceFromDisk':
            measureId = 0
            filenamePrefix = Constants.I4D_DATA_PREFIX
            # Get the measure id
            if len(sys.argv) >= 3:
                measureId = int(sys.argv[2])
            # Get the mesure filename prefix
            if len(sys.argv) >= 4:
                filenamePrefix = sys.argv[3]
            client.getSurfaceFromDisk(measureId, filenamePrefix = Constants.I4D_DATA_PREFIX)
        
         # --- Get the zernike, surface and intensity and store them on server --- #
        elif sys.argv[1] == 'GetZernike':
            nSet = Constants.I4D_DEFAULT_MEAS_SET_NUM
            setDelay = Constants.I4D_DEFAULT_SET_DELAY_S
            nMeas =  Constants.I4D_DEFAULT_BURST_MEAS_NUM
            filenamePrefix = Constants.I4D_DATA_PREFIX
            processingType = Constants.I4D_DEFAULT_ZERNIKE_PROCESSING_TYPE
            acceptedPerc = Constants.I4D_CHECK_BURST_ACCEPTED_PERCENT
            # Get the number of set
            if len(sys.argv) >= 3:
                nSet = int(sys.argv[2])
             # Get the delay beetwen sets
            if len(sys.argv) >= 4:
                setDelay = int(sys.argv[3])
            # Get the number of measures param
            if len(sys.argv) >= 5:
                nMeas = int(sys.argv[4])
            # Get the file name prefix param
            if len(sys.argv) >= 6:
                filenamePrefix = sys.argv[5]
            # Get the full process option
            if len(sys.argv) >= 7:
                processingType = int(sys.argv[6])
            # Get the accepted percentage of acquired frames for burst
            if len(sys.argv) >= 8:
                acceptedPerc = int(sys.argv[7])
            client.getZernike(nSet, setDelay, nMeas, filenamePrefix, processingType, acceptedPerc)
        
        # --- Shutdown the server
        elif sys.argv[1] == 'ShutdownServer':
    	    client.shutdownServer()
    
        # --- Unknown command --- 3
        else:
            print 'Unknow command !'
        
        print 'Start time: ' + startTime
        print 'End time: ' + time.asctime()	
    
    except Exception, e:
        print "Exception in command execution: %s" %str(e)
