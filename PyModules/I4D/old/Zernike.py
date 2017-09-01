###########################
##
## 4D automatic measurement script
##
## This script will take a series of interferograms and save them on disk. The delay
## between each measure and the total # of measurement is configurable. The script can send emails
## to selected people to signal start and end of the measurement.
##
## For each measurement, four files are saved:
##
## xxxx_intensity.txt: intensity map as a 2D map with ASCII-encoded (fixed width) floating poing numbers
## xxxx_surface.txt:   surface map as a 2D map with ASCII-encoded (fixed width) floating point numbers
## xxxx_zernikes.txt:  list of the first 36 zernike coefficients, ASCII-encoded (fixed width) floating point numbers
## xxxx_zernikes_names.txt: list of the first 36 zernike coefficients, with the name of each zernike polynomial next to each number.
##
## The first three files can be read from IDL with the READ_ASCII() instruction.
##
## Complete filenames are as follows:
##
## <prefix>_<n>_<type>.txt
##
## where <prefix> is the prefix specified in the configuration, <n> is the measurement number (starting from zero), and
## <type> may be 'intensity', 'surface', 'zernikes' or 'zernikes_names'.
##
## July 2007
##
## For info:
##  - Fabio Tosetti
##  - Alfio Puglisi
##
##


import sys
import time, types

# import all 4D libraries
from Scripting.App import *
from Scripting.Modify import *
from Scripting.Data import *
from Scripting.Config import *
from Scripting.Measurements import *
from Scripting.Show import *

from Commons import Constants
import Controller


###############################################
##
## Configuration starts
##

# Output directory for output processed data (intensity, surface, zernike)
# Raw and calibrated data are saved in Constants.I4D_RAW_DATA_PATH and Constants.I4D_CALIB_DATA_PATH
outDir = Constants.I4D_PROC_DATA_PATH

# Delay (in seconds) between measures. Total time is slightly higher due measurement overhead
# only valid for SLOW acquisition
#delay_s = 300     OBSOLETED

# Overall interferometer configuration (stores things like exp. time, zernike removal, mask, and so on).
config4D = 'c:/4d/config/default.ini'

# If nonzero, will send emails
useMail = 0

# From: address of emails (must be a valid Arcetri address).
mailFrom = '4D_Interferometer <puglisi@arcetri.astro.it>'

# List of people to mail when starting/finishing a measurement. Must be a python list with square brackets.
mailTo = ['puglisi@arcetri.astro.it', 'pieralli@arcetri.astro.it', 'tosetti@arcetri.astro.it']

##
## Configuration ends
##
##################################################################


# Add this to have the some python libs
sys.path.append('C:\python24\lib')
sys.path.append('C:\python24\lib\site-packages')
sys.path.append('C:\python24\DLLs')

import smtplib

# Add the I4D Pyro Server package location to path
sys.path.append('C:\Obelix_CVS\Supervisor\PyModules')

import numpy
import fits_lib

import Controller
import FastAcqLoad
import checkBurst

zernikeNames = ['Piston', 'Tilt X', 'Tilt Y', 'Defocus', 'Astig. X', 'Astig. Y', 'Coma X', 'Coma Y',
                'Primary Spherical', 'Trefoil X', 'Trefoil Y', 'Secondary Astig. X', 'Secondary Astig. Y',
                'Sec. Coma X', 'Sec. Coma Y', 'Sec. Spherical', 'Tetrafoil X', 'Tetrafoil Y', 
                'Sec. Trefoil X', 'Sec. Trefoil Y', 'Tertiary Astig. X', 'Tertiary Astig. Y',
                'Tertiaty Coma X', 'Tertiary Coma Y', 'Tertiary Spherical', 'Pentafoil X', 'Pentafoil Y',
                'Sec. Tetrafoil X', 'Sec. Tetrafoil Y', 'Tertiary Trefoil X', 'Tertiary Trefoil Y',
                'Quad. Astig. X', 'Quad. Astig. Y', 'Quad. Coma X', 'Quad. Coma Y', 'Quad. Spherical']



def Array2dToStr(arr):
    """
    Converts a 2d array to an ASCII string with fixed width numbers. Supports float and int arrays.
    Each array line ends with a linefeed.
    """

    s = ''
    if type(arr[0][0]) == types.FloatType:
        for x in range(len(arr)):            
            s2 = ['%9.5f  ' % v  for v in arr[x]]
            s += ''.join(s2)
            s += '\n'
    elif type(arr[0][0]) == types.IntType:
        for x in range(len(arr)):
            s2 = ['%10d   ' % v for v in arr[x]]
            s += ''.join(s2)
            s += '\n'
    return s

def Array1dToStr(arr):
    """
    Converts a 1d array to an ASCII string with fixed width numbers. Supports float and int arrays.
    The string ends with a linefeed.
    """

    s = ''
    if type(arr[0]) == types.FloatType:
        s = ''.join(['%9.5f    ' % v for v in arr])
    elif type(arr[0]) == types.IntType:
        s = ''.join(['%10d    ' % v for v in arr])

    s += '\n'

    return s

def Array1dToStrWithComments(arr, comments):
    """
    Converts a 1d array to an ASCII string with fixed width numbers. Supports float and int arrays.
    Each line of the string will contain a value and a comment. The comment array must be
    at least as long as the value array.
    """

    s = ''
    if type(arr[0]) == types.FloatType:
        for n in range(len(arr)):
            s += '%9.5f    %s\n' % (arr[n], comments[n])
    elif type(arr[0]) == types.IntType:
        for n in range(len(arr)):
            s += '%10d    %s\n' % (arr[n], comments[n])
    s += '\n'

    return s


def sendmail( message):
    if not useMail:
        return

    smtp = smtplib.SMTP('hercules.arcetri.astro.it')
    msgHdr = ("From: %s\r\nTo: %s\r\n\r\n" % (mailFrom, ", ".join(mailTo)))
    smtp.sendmail(mailFrom, mailTo, msgHdr + message)
    smtp.quit()

def saveZernikeMeas(fileName, measure, useFits = 1):
    prefix = "%s/%s" % (outDir, fileName)
    zernike = GetZernikeCoeff(measure)
    datasetInterf = measure.GetDataset('frames', 'frame1')
    datasetSurface = measure.GetDataset('zernikes', 'surface')

    if useFits:
        fits_lib.writeSimpleFits( prefix+'_intensity.fits', numpy.array(datasetInterf.GetArray()), {}, overwrite=True)
        fits_lib.writeSimpleFits( prefix+'_surface.fits', numpy.array(datasetSurface.GetArray().raw_data()), {}, overwrite=True)
    else:
        file(prefix+'_intensity.txt', 'w').write(Array2dToStr(datasetInterf.GetArray()))
        file(prefix+'_surface.txt', 'w').write(Array2dToStr(datasetSurface.GetArray().raw_data()))
    
    file(prefix+'_zernikes.txt', 'w').write(Array1dToStr(zernike))
    f = file(prefix+'_headers.txt', 'w')
    for k in measure.headers.keys():
        f.write('%s=%s\n' % (k, measure.GetHeader(k)))
    f.close()
        
    

    file(prefix+'_zernikes_names.txt', 'w').write(Array1dToStrWithComments(zernike, zernikeNames))
    


# Execute the acquisition and saving of surface, intensuty and zernike.
# Parameters:
#     controller: a Controller class instance
#     setNum:     number of set of measurements
#     setDelay:   delay in seconds beetwen sets
#     measPerSet: number of measures for each set  
#     measPrefix: prefix for saved data files.
def execute(controller, 
            setNum = Constants.I4D_DEFAULT_MEAS_SET_NUM,
            setDelay = Constants.I4D_DEFAULT_SET_DELAY_S, 
            measPerSet = Constants.I4D_DEFAULT_BURST_MEAS_NUM, 
            measPrefix = Constants.I4D_DATA_PREFIX, 
            processingType = Constants.I4D_ZERNIKE_ACQUIRE_ONLY,
            acceptedPerc = Constants.I4D_CHECK_BURST_ACCEPTED_PERCENT):

    # Load overall interferometer configuration
    OpenConfigFile(config4D)
    
    # Test if the first file exists - avoid overwriting old measurements
    #
    # Non funziona... forse ora va !
    try:
        filename = "%s/%s_%04d_04d_intensity.txt" % (outDir, measPrefix, 0, 0)
        dummy = os.stat(filename)
        raise Exception, 'Output file already exists'
    except:
        pass
    

    # burstOutputFolder = time.strftime('%Y%m%d-%H.%M-')+measPrefix
    burstOutputFolder = measPrefix

    str = "4D - Acquisizione zernike, surface e intensity \n\
           ---------------------------------------------- \n\
           L'interferometro sta per iniziare le misure...\n\n \
           Nome del set: %s\n \
           Numero set: %d\n \
           Numero misure per set: %d\nIntervallo: %d secondi\n \
           Tempo stimato per fine misura: %s\n \
           Dati salvati in: %s\n" \
           % (measPrefix, setNum, setDelay, measPerSet, time.ctime( time.time() + setNum*setDelay + Constants.I4D_ZERNIKE_START_DELAY_S), burstOutputFolder)
    print str
    sendmail(str)
    
    ### ACQUIRE INTERFEROGRAMS (RAW FRAMES) AND STORE THEM TO DISK ###
    if processingType == Constants.I4D_ZERNIKE_ACQUIRE_ONLY or \
       processingType == Constants.I4D_ZERNIKE_PROCESS_FULL:
        for i in range(setNum):
            filePrefix = "%s_%04d" % (measPrefix, i)
            controller.GetBurstToDisk(0, measPerSet, burstOutputFolder, filePrefix)
            time.sleep(setDelay)

    if processingType == Constants.I4D_ZERNIKE_COMPUTE_ONLY or \
       processingType == Constants.I4D_ZERNIKE_PROCESS_FULL:
        
        ### CALIBRATE RAW FRAMES AND STORE THEM 'calibrated' FOLDER ###
        # If required remove raw folder to save disk space
        # [to check] 'Calibrate' seems to to nothing... 
        controller.CalibrateMeasures(burstOutputFolder)
        if Constants.I4D_REMOVE_RAW_FRAMES:
            raw_full_path = Constants.I4D_RAW_DATA_PATH + '/' + burstOutputFolder
            raw_filenames = os.listdir(raw_full_path)
            for file in raw_filenames:
                os.remove(raw_full_path + '/' + file)
            os.rmdir(raw_full_path)

        ### PERFORM EXTRA PROCESSING ###
        # Load from disk and compute/save zernike, surface and intensity
        if not os.path.exists(Constants.I4D_PROC_DATA_PATH + '/' + burstOutputFolder):
            os.mkdir(Constants.I4D_PROC_DATA_PATH + '/' + burstOutputFolder)
        
        # Obtain the list of completed bursts
        burstCompletedList = checkBurst.doCheck(measPrefix, setNum, measPerSet, acceptedPerc)
        
        for i in range(setNum):
            # Check if the burst is completed: if not, skip it!
            if i in burstCompletedList:
                for j in range(measPerSet):
                    fileName = burstOutputFolder + '/%s_%04d_%04d' % (measPrefix, i, j)
                    meas = FastAcqLoad.MeasurementFromDisk(Constants.I4D_CALIB_DATA_PATH + '/' + fileName + '.dat')
                    if meas:
                        saveZernikeMeas(fileName, meas)
    
    sendmail("L'interferometro ha finito le misure del set %s!" % measPrefix)
    
   
# To run this script stand-alone 
def start(setNum = Constants.I4D_DEFAULT_MEAS_SET_NUM, 
          setDelay = Constants.I4D_DEFAULT_SET_DELAY_S, 
          measPerSet = Constants.I4D_DEFAULT_BURST_MEAS_NUM, 
          measPrefix = Constants.I4D_DATA_PREFIX,
          processingType = Constants.I4D_DEFAULT_ZERNIKE_PROCESSING_TYPE,
          acceptedPerc = Constants.I4D_CHECK_BURST_ACCEPTED_PERCENT):
    
    controller = Controller.Controller()
    execute(controller, setNum, setDelay, measPerSet, measPrefix, processingType, acceptedPerc)
    
    
if __name__ == 'MAIN':
    print 'Waiting %d seconds to start acquisition...' %Constants.I4D_ZERNIKE_START_DELAY_S
    time.sleep(Constants.I4D_ZERNIKE_START_DELAY_S)
    start();
