
# import all 4D libraries
from Scripting.App import *
from Scripting.Modify import *
from Scripting.Data import *
from Scripting.Config import *
from Scripting.Measurements import *
from Scripting.Show import *


from Commons import Constants
import FastAcqMeasure
import FastAcqLoad
import Zernike

# @Class: Controller
# Wrapper around the 4D python library to control the interferometer.
# @
class Controller:
    
    def __init__(self):
        self.logSource = "CONTROLLER I4D"

        self.measures = []
        self.measuresGot = 0


    def log(self, message):
        print "%s: %s" %(self.logSource, message)
       
       
    # Stupid test
    def test(self):
        self.log("Controller is working")   
        return "Controller is working"     



    # ---------- Config methods ----------- #



    # ---------- Single shot measurement methods ------------ #
    # All this methods work on memory, so the number of measures is limited
    # by the memory available in Windows

    # Tell the number of measures stored in server's ram
    def GetMeasuresCount(self):
        return self.measuresGot

    # Get a new measure and return the number of total measures
    # stored in server's ram
    def GetMeasure(self):
        self.measures.insert(self.measuresGot, Measure('frames'))
        self.measuresGot = self.measuresGot + 1
        return self.measuresGot

    # Compute the specified dataset of a measure stored in server's ram and 
    # return it as array.
    # DatasetName: 'frames', 'intensity', 'surface'  [to check]
    def GetDataset(self, measureIndex, datasetName):
        if measureIndex > (self.measuresGot - 1):
            self.log("Requested dataset '%s' of unexisting measure (index %d) !" %(datasetName, measureIndex))
            return None
        dataset = self.measures[measureIndex].GetDataset(datasetName)
        return GetArray(dataset)

    # Clear all the measures stored in server's ram
    def ClearMeasures(self):
        for i in range(self.measuresGot):
            self.measures[i] = None
        self.measuresGot = 0

    
    # ----------Burst measurement methods ------------ #
    # All this methods work on disk

    # Takes a number of measures (burst measure) and store them to server's disk
    # Return some info about the camera and the measures
    def GetBurstToDisk(self, enableTrigger, 
                       nMeasures = Constants.I4D_DEFAULT_BURST_MEAS_NUM, 
                       burstFolder = Constants.I4D_DATA_PREFIX,
                       filenamePrefix = Constants.I4D_DATA_PREFIX):
        try:
            self.log("Acquiring %d burst measurements to disk..." %nMeasures)
            (frameInfo, camInfo) = FastAcqMeasure.acquire(nMeasures, burstFolder, filenamePrefix, enableTrigger)
            self.log("Burst measurements succesfully acquired to disk !")
            return (frameInfo, camInfo)
        except Exception, e:
            self.log("Burst measurements failed (%s)" % str(e))
            raise

    def CalibrateMeasures(self, burstOutputFolder):
        try:
            self.log("Calibrating all measurements...")
            FastAcqMeasure.calibrate(burstOutputFolder)
            self.log("All measures succesfully calibrated on server disk !")
        except Exception, e:
            self.log("Measures calibration failed (%s)" % str(e))
            raise
        
    # Compute the surface of a measure stored on server's disk
    # and return it as array
    def GetSurfaceFromDisk(self, measureId = 0, filenamePrefix = Constants.I4D_DATA_PREFIX):
        try:
            self.log("Getting surface for measurement %d from disk..." %measureId)
            meas = FastAcqLoad.MeasurementFromDisk(os.path.join(Constants.I4D_CALIB_DATA_PATH, filenamePrefix + str("%04d" %measureId) + ".dat"))
            if meas:
                dataset = meas.GetDataset('analyzed')
                self.log("Surface succesfully got !") 
                return GetArray(dataset)
            else:
                self.log("Get surface failed") 
                return None
        except Exception, e:
            self.log("Get surface failed (%s)" %str(e))
            raise
        
        
    # ---------- 'Extended' measurement methods ------------ #
    
    def GetZernike(self, 
                   nSet = Constants.I4D_DEFAULT_MEAS_SET_NUM,
                   setDelay = Constants.I4D_DEFAULT_SET_DELAY_S,
                   nMeasures = Constants.I4D_DEFAULT_BURST_MEAS_NUM, 
                   filenamePrefix = Constants.I4D_DATA_PREFIX,
                   processingType = Constants.I4D_DEFAULT_ZERNIKE_PROCESSING_TYPE,
                   acceptedPerc = Constants.I4D_CHECK_BURST_ACCEPTED_PERCENT):
        try:
            self.log("Getting interferograms, intensity, surface and zernike (depending on processing type. Current=%d) for %d sets (%d measures/set) with delay %d s and prefix %s..." %(processingType, nSet, nMeasures, setDelay, filenamePrefix))
            Zernike.execute(self, nSet, setDelay, nMeasures, filenamePrefix, processingType, acceptedPerc)  
            self.log("Completed!")
        except Exception, e:
            self.log("Get zernike failed (%s)" %str(e))
            raise
    
    # ------------- other methods ------------- #

    def ShutdownServer(self):
        self.daemon.shutdown()
        
