###
# Check if a set of acquired burst is complete (with respect to a given percentage), 
# and return a list burst indexes.
# Designed to be used AFTER Zernike.py script (or 'Client.py GetZernike' command).
#
# Syntax:
#    checkBurst MEAS_PREFIX BURST_NUM MEAS_NUM ACCEPTED_PERC
# where
#    MEAS_PREFIX is the measure prefix previously specified in Zernike.py (or 'Client.py GetZernike')
#    BURST_NUM is the number of burst previously specified...
#    MEAS_NUM is the number of measures previously specified...
#    ACCEPTED_PERC is the percentage of frames to be present to consider the burst completed
#

import os, sys
from Commons import Constants

def doCheck(filePrefix, burstNum, measNum, acceptedPerc):
    
    burstFolder = None
    burstRawFolder = Constants.I4D_RAW_DATA_PATH + '/' + filePrefix
    burstCalibFolder = Constants.I4D_CALIB_DATA_PATH + '/' + filePrefix
    print 'Checking if %d burst of %d measures correctly acquired to folder %s or %s...' %(burstNum, measNum, burstRawFolder, burstCalibFolder)
    if os.path.exists(burstRawFolder):
        burstFolder = burstRawFolder
    elif os.path.exists(burstCalibFolder):
        burstFolder = burstCalibFolder
   
    completedBursts = []
    if burstFolder != None:    
        
        ### Loop over sets (bursts) ###
        for burst in range(burstNum):
            measuresFound = 0
            ### Loop over measures ###
            for meas in range(measNum):
                if os.path.exists(burstFolder + '/' + filePrefix + '_%04d_%04d.dat' %(burst, meas)):
                    measuresFound = measuresFound+1
            #print 'Meas found:                 %d' %measuresFound
            #print 'Required meas (%d percent): %d' %(acceptedPerc, (measNum*acceptedPerc)/100)
            #print 'Exceeding meas:             %d' %(measuresFound-(measNum*acceptedPerc)/100)
            #print
            if measuresFound >= (measNum*acceptedPerc)/100:
                completedBursts.append(burst)
                
    return completedBursts       
       

if __name__ == '__main__':
    
    if len(sys.argv) != 5:
        print
        print "    Usage: checkBurst MEAS_PREFIX BURST_NUM MEAS_NUM ACCEPTED_PERC"
        print
        print "        MEAS_PREFIX is the measure prefix previously specified in Zernike.py (or 'Client.py GetZernike')"
        print "        BURST_NUM is the number of burst previously specified..."
        print "        MEAS_NUM is the number of measures previously specified..."
        print "        ACCEPTED_PERC is the percentage of frames acquired to consider the burst completed"
        sys.exit()
    
    completedBursts = doCheck(sys.argv[1], int(sys.argv[2]), int(sys.argv[3]), int(sys.argv[4]))
    print 'Succesfully completed %d/%d burst' %(len(completedBursts), int(sys.argv[2]))
    print 'Completed burst: ' + str(completedBursts)
        