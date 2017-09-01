# @File: Commons.py
# Set of commons info needed by Pyro Server and Client.
# @

import os.path

class Constants:
    
    # Pyro remote object name
    I4D_CONTROLLER_NAME = "I4D_Controller"
    
    # Server data path
    I4D_DATA_PATH = "D:/4D/Data"
    I4D_RAW_DATA_PATH = I4D_DATA_PATH + '/' + 'raw'
    I4D_CALIB_DATA_PATH = I4D_DATA_PATH + '/' + 'calibrated'
    I4D_PROC_DATA_PATH = I4D_DATA_PATH + '/' + 'processed'
    
    # Server data default prefix
    I4D_DATA_PREFIX = "MEAS"
    
    I4D_DEFAULT_MEAS_SET_NUM = 2
    I4D_DEFAULT_SET_DELAY_S = 2
    I4D_DEFAULT_BURST_MEAS_NUM = 10
    
    # Remove raw frames after calibrating them
    I4D_REMOVE_RAW_FRAMES = 0
    
    #Processing types for GetZernike
    I4D_ZERNIKE_ACQUIRE_ONLY = 1
    I4D_ZERNIKE_COMPUTE_ONLY = 2
    I4D_ZERNIKE_PROCESS_FULL = 3
    I4D_DEFAULT_ZERNIKE_PROCESSING_TYPE = I4D_ZERNIKE_ACQUIRE_ONLY
    
    # The minimum percentage of frames to consider the burst completed (0-100)
    I4D_CHECK_BURST_ACCEPTED_PERCENT = 80
    
    # Delay before LocalStartup execute 
    I4D_ZERNIKE_START_DELAY_S = 5
    
