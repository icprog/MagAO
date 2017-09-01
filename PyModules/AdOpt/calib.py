#!/usr/bin/env python

import os, time
from AdOpt import cfg, frames_lib, AOExcept, AOConstants, fits_lib

###############################################################################################################
# Root paths and directories

root = os.environ['ADOPT_ROOT'].rstrip('/')
if os.environ.has_key('IDL_STARTUP'):
  idlroot = os.path.dirname(os.environ['IDL_STARTUP']).rstrip('/')
else:
  idlroot=''


# Directory access functions

def calibDir():
    return root + '/calib'

def WfsCalibDir():
    return calibDir() + '/wfs/current'

def BackgDir(ccd, bin):
    return WfsCalibDir() + '/%s/backgrounds/bin%d' % (ccd, bin)

def LUTsDir(ccd, bin=None):
    if bin:
        return WfsCalibDir() + '/%s/LUTs/bin%d' % (ccd, bin)
    else:
        return WfsCalibDir() + '/%s/LUTs' % ccd

def gainsDir():
    return WfsCalibDir()+'/gains'

def slopenullDir(bin):
    return WfsCalibDir() + '/slopenulls/bin%d' % bin 

def setupsDir():
    return WfsCalibDir()+'/setups/'

def autoCenterDir():
    return WfsCalibDir()+'/autocenter/'

def cameraLensCalibDir():
    return autoCenterDir()+'centerLens/'

def ttDir():
    return WfsCalibDir()+'/tt'

def adcDir():
    return WfsCalibDir()+'/adc'

def AODir():
    return WfsCalibDir()+'/ao'

def modalBasisDir(modalbasis):
    return '/towerdata/adsec_calib/M2C/%s/' % modalbasis

def recFile(modalbasis, rec):
    if len(rec)<4 or rec[0:4] != 'Rec_':
        rec = 'Rec_'+rec
    return modalBasisDir(modalbasis)+'RECs/'+rec

def nModes(recfile):
    try:
        hdr = fits_lib.readFitsHdr(recfile)
    except:
        raise AOExcept.AdOptError( code = AOConstants.AOErrCode['FILE_ERROR'], errstr = 'Cannot read reconstructor file: %s' % recfile)
    try:
        nmodes = int(float(hdr['IM_MODES']))
    except:
        raise AOExcept.AdOptError( code = AOConstants.AOErrCode['FILE_ERROR'], errstr = 'Cannot find keyword IM_MODES in reconstructor file. Modal basis: %s - Rec file: %s' % (modalbasis, rec))
    return nmodes


# File access functions

def CurBackgFilename(ccd):
    return BackgDir( ccd)+'/current'

def CurLUTlink(ccd):
    return LUTsDir( ccd) + '/current'

def CurDisplayLUT(ccd):
    return LUTsDir( ccd) + '/current/display'

def CurSlopeXLUT(ccd):
    return LUTsDir( ccd) + '/current/slopex'

def CurSlopeYLUT(ccd):
    return LUTsDir( ccd) + '/current/slopey'

def CurBcuLUT(ccd):
    return LUTsDir( ccd) + '/current/bcuLUT'

def CurIndpup(ccd):
    return LUTsDir( ccd) + '/current/indpup'

def CurPupdata(ccd):
    return LUTsDir( ccd) + '/current/pupdata.txt'

def CurSlopenull():
    return slopenullDir() + '/current'

def fovFile():
    '''
    Returns the path of the FoV calibration file.
    '''
    return WfsCalibDir()+'/fov.txt'


############################
# Measurements

measDir = os.getenv('ADOPT_MEAS')

def getDateTime( addTimestamp = True, onlyTime = False):

    if onlyTime:
        return time.strftime('%H%M%S', time.localtime())
    if addTimestamp:
        return time.strftime('%Y%m%d_%H%M%S', time.localtime())
    else:
        return time.strftime('%Y%m%d', time.localtime())

def getTodayMeasDir(name, addTimestamp = True):

    dir = os.path.join(measDir, name)

    # Errors here are not fatal
    try:
        os.makedirs(dir)
    except OSError, e:
        pass

    path = os.path.join( dir, getDateTime( addTimestamp))

    # Here we do not trap errors - we should make a new directory every time
    os.makedirs(path)

    return path

# Returns the measurement directory for the specified calibration.
# If a tracking number is specified, returns the path to that tracking number.
# Otherwise, returns the higher-level directory

def getMeasDir( name, tracknum = None):
    if not tracknum:
        return os.path.join(measDir, name)
    else:
        return os.path.join(measDir, name, tracknum)

def getNumberedFilename(filename, extension, digits=None):
    '''
        Returns an auto-numbered filename which will not overwrite any existing file.
        The numbered filename is in the format:
            <filename>_xx.<extension>

        where xx is the auto-number. If a # of digits is requested, leading zeros will
        be added as needed.
    '''

    if not digits: 
        format = '%s_%%d.%s' % (filename, extension)
    else:
        format = '%s_%%0%dd.%s' % (filename, digits, extension) 

    counter = 1
    while 1:
        new_filename = format % counter
        if not os.path.exists(new_filename):
            return new_filename
        counter += 1


############################
# Auto center/focus/etc.

def sourceAcqCalibFile(fwpos=None):
    if fwpos != None:
        return WfsCalibDir()+'/autocenter/sourceAcq/sourceAcq_filter%d.conf' % fwpos
    else:
        return WfsCalibDir()+'/autocenter/sourceAcq/sourceAcq.conf'
    
sourceAcqTimeout   = 60  # Timeout for various stage movements in the source acq procedure, in seconds

def centerTTCalibFile( modAmp):
    return autoCenterDir()+'centerTT/centerTT_mod%5.3f.conf' % float(modAmp)

def centerStagesCalibFile( modAmp):
    return autoCenterDir()+'centerStages/centerStages_mod%5.3f.conf' % float(modAmp)

def focusStagesCalibFile( modAmp):
    return autoCenterDir()+'focusStages/focusStages_mod%5.3f.conf' % float(modAmp)

###################################
# FoV description

def writeFov( xmin=None, ymin=None, xmax=None, ymax=None, xc=None, yc=None):
    '''
    Updates the FoV calibration file with new data.
    '''

    data=readFov()
    newdata = {}
    for x in ['xmin','ymin','xmax','ymax','xc','yc']:
        if locals()[x] != None:
            newdata[x] = float(locals()[x])

    saveCalibData( fovFile(), newdata, title='FoV description')


def readFov( allowFail=True):
    '''
    Reads the FoV calibration file. Returns a dictionary with the following keys:
    'xmin': lower X limit
    'ymin': lower Y limit
    'xmax': higher X limit
    'ymax': higher Y limit
    'cx'  : FoV center X position
    'cy'  : FoV center Y position 
    '''

    return loadCalibData( fovFile(), allowFail=allowFail)


######################################
# Pupils

# Returns currently calibrated pupils

def getCurPupils(app):

    pups = frames_lib.readCurPupils( CurPupdata('ccd39'))
    for pup in pups:
        pup.diameter *= 2.0
    return pups

# Returns a previously calibrated pupils

def getPupils( bin, tracknum):

    pups = frames_lib.readCurPupils( LUTsDir( 'ccd39', bin=bin) + '/'+tracknum+'/pupdata.txt')
    for pup in pups:
        pup.diameter *= 2.0
    return pups


#####################################
# Tip-tilt
#
# Returns tip-tilt calibration filename

def getTTcalibFile():
    return ttDir()+'/calib.txt'

####################################
# ADC
#
# Returns the adc calibration filename

def getADCcalibFile():
	return adcDir()+'/calib.txt'

#################################
# AO Parameters look up table

def getAOParamsFile( instr = None):
    if instr == None:
        instr = 'IRTC'
    return AODir()+"/table_%s.txt" % str(instr)
    

################################################
#
# Generic calib data load/save

def saveCalibData( filename, dataDict, title=None):
    '''
    Saves calibration data to disk. Reads old values, if any, and only updates the ones given as parameters.
    '''

    import time
    from AdOpt import AOConfig

    # Read back old values (if any)
    data = loadCalibData(filename, allowFail = True)

    data.update(dataDict)

    data['time'] = time.asctime()


    hdr = '''
# THIS FILE IS AUTOMATICALLY GENERATED BY CALIBRATION PROCEDURES.
# DO NOT EDIT MANUALLY!! (unless you know what you are doing)
#
# Please use the calibration procedures to update values.

'''

    if title != None:
        hdr = '# '+title+'\n#\n' + hdr 

    AOConfig.write_config( filename, data, hdr)



def loadCalibData( filename, allowFail = False, check=None):
    '''
    Loads calibration data from disk. Returns a dictionary of values.
    if <allowMissing> is True, it will fail silently if the file does not
    exist or contain errors. Otherwise, an exception is thrown 
    Check can be a list of keywords whose existence in the file will be checked, and a AdOptError exception raised if not found
    '''
    from AdOpt import AOConfig

    try:
        data = AOConfig.read_config(filename, useAOVar = False)
    except Exception, e:
        if not allowFail:
            print 'Problem reading file',filename
            raise AOExcept.AdOptError( code = AOConstants.AOErrCode['FILE_ERROR'], errstr = 'Cannot read file %s' % filename)
        else:
            return {}

    if check:
        for key in check:
            if not data.has_key(key):
                raise AOExcept.AdOptError( code = AOConstants.AOErrCode['WFSARB_MISSING_CALIB'], errstr = 'Missing keyword %s in config file %s' % (str(key), filename))

    return data


def ensureExtension( filename, extension):
    '''
    Add the extension if the filename does not have it.
    '''

    if filename == None:
        return None

    if extension[0] != '.':
        extension = '.'+extension

    if len(filename) < len(extension):
        return filename + extension
    if filename[-len(extension):] != extension:
        return filename + extension
    else:
        return filename




