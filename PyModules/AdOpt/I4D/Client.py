#!/usr/bin/env python

# @File: Client.py
# Pyro Client for the 4D interferometer.
# Connect to Pyro Server to remotely call 4D python libraries.
# @

import Pyro.core
import Pyro.naming

import os

from Commons import Constants	

class Client:
    
    def __init__(self):
        
        # Init logging facility
        # Use the environment variable ADOPT_LOG for the logging path 
        Pyro.config.PYRO_USER_LOGFILE = os.getenv("ADOPT_LOG", "/tmp") + "/Client_Pyro_userlog.log"
        self.userLog = Pyro.util.UserLogger()
        self.logSource = "CLIENT I4D"
        
        # To run Client on server machine (4D PC)
        #Pyro.config.PYRO_HOST = "127.0.0.1"
        
        print
        print "[CLIENT I4D] ------------------------------ 4D INTERFEROMETER PYRO CLIENT ------------------------------"
    	print "[CLIENT I4D] -------------------------------------------------------------------------------------------"
    	print "[CLIENT I4D] Pyro config file: %s" %Pyro.config.PYRO_CONFIG_FILE
        print "[CLIENT I4D] Pyro server:      %s" %Pyro.config.PYRO_HOST
    	print "[CLIENT I4D] Pyro storage:     %s" %Pyro.config.PYRO_STORAGE
    	print "[CLIENT I4D] Pyro logging:     %s" %Pyro.config.PYRO_USER_LOGFILE
        print
        
        # Empty lines in log file !!!
        self.log("---------------- 4D INTERFEROMETER PYRO-CLIENT ----------------")
        
        # Init the Pyro client
        Pyro.core.initClient()
        
        self.log("Storing proxy address for '%s'..." % Constants.I4D_CONTROLLER_NAME)
        #Pyro.config.PYRO_HOST = "193.206.155.78" # Defined as environment variable
        self._remoteCtrlUri = 'PYROLOC://' + Pyro.config.PYRO_HOST + ':' + str(Pyro.config.PYRO_PORT) + '/' + Constants.I4D_CONTROLLER_NAME
        print "RemoteCtrlUri:" + self._remoteCtrlUri
        
        # ATTENTION: doesn't store a global reference to the proxy because Pyro
        #            doesn't allow proxy sharing among multiple threads, and the 
        #            InterferometerCtrl needs to use this Client in different 
        #            threads
        
        # Try to use the proxy to be sure the 4D Python/Pyro server is running
        remoteCtrl = self._retrieveProxy()
        try:
            self.log("Pinging remote controller...") 
            remoteCtrl._setTimeout(2)
            remoteCtrl.ping()
            remoteCtrl._setTimeout(None)
            remoteCtrl._release()
        except Exception, e:
            self.log("%s" % str(e))
            self.log("Client initialization failed") 
            remoteCtrl._release()
            raise Exception, "Client initialization failed (%s)" % str(e)
        
        self.log("Client succesfully initialized") 
        
        
    def log(self, message):
         self.userLog.msg(self.logSource, message)
        
        
    def _retrieveProxy(self):
        # Retrieve proxy at runtime to allow each different threads
        # to call these methods!
        self.log('Getting proxy for %s' % self._remoteCtrlUri)
        remoteCtrl = Pyro.core.getProxyForURI(self._remoteCtrlUri)
        remoteCtrl._setTimeout(Constants.I4D_CONNECTION_CLIENT_TIMEOUT_S)
        return remoteCtrl
         
         
    # Could return result
    def setConfigFile(self, configFile):
        remoteCtrl = self._retrieveProxy()
        try:
            self.log('Setting config file: %s' % configFile)
            if remoteCtrl.loadConfig(configFile):
                self.log('Config file succesfully set!')
            else:
                self.log('Config file not set')
            remoteCtrl._release()
        except Exception, e:
            self.log('%s' % str(e))
            remoteCtrl._release()
            raise Exception, "Config file setting failed (%s)" %  str(e)
       
       
    def enableTrigger(self, enable):
        remoteCtrl = self._retrieveProxy()
        try:
            self.log('Setting trigger to %d' % enable)
            remoteCtrl.setTrigger(enable)
            remoteCtrl._release()
        except Exception, e:
            self.log('%s' % str(e))
            remoteCtrl._release()
            raise Exception, "Trigger setting failed (%s)" %  str(e)
      
    def setPostProcessing(self, ppType):
        remoteCtrl = self._retrieveProxy()
        try:
            self.log('Setting post-processing to %s' % ppType)
            remoteCtrl.setPostProcessing(ppType)
            remoteCtrl._release()
        except Exception, e:
            self.log('%s' % str(e))
            remoteCtrl._release()
            raise Exception, "Post processing setting failed (%s)" %  str(e)
        
    # Takes a burst of measurements and store them to server's disk
    # Returns the destination path
    def getBurstToDisk(self, nMeas, destPath = None):
        remoteCtrl = self._retrieveProxy()
        try:
            self.log('Getting burst (%d meas) to disk...' %nMeas)
            (destPath, frameInfo, camInfo) = remoteCtrl.GetBurstToDisk(nMeas, destPath)
            remoteCtrl._release()
            self.log('Burst succesfully completed: meas stored on server (%s)' % destPath)
            # At the moment ignores frameInfo and camInfo
            return destPath
        except Exception, e:
            self.log('%s' % str(e))
            remoteCtrl._release()
            raise Exception, "Burst measurement failed (%s)" %  str(e)
        
        
    # Takes a burst of measurements and store them to server's disk
    # Returns the destination file name
    def getMeasurementToDisk(self, measPrefix):
        remoteCtrl = self._retrieveProxy()
        try:
            self.log('Getting single meas to disk...')
            destFileName = remoteCtrl.GetMeasurementToRam(measPrefix)
            remoteCtrl._release()
            self.log('Single meas succesfully completed: meas stored on server (%s)' % destFileName)
            # At the moment ignores frameInfo and camInfo
            return destFileName
        except Exception, e:
            self.log('%s' % str(e))
            remoteCtrl._release()
            raise Exception, "Single measurement failed (%s)" %  str(e)
    
    # Ask for data post-processing
    # Returns a masurement's filename or burst's path
    def postProcess(self, sourceDir, sourceFilePrefix, nMeas):
        remoteCtrl = self._retrieveProxy()
        try:
            self.log('Post-processing measurements...')
            dest = remoteCtrl.PostProcess(sourceDir, sourceFilePrefix, nMeas)
            remoteCtrl._release()
            if dest != '':
                self.log('Processed files succesfully generated on server (%s)' %dest)
            else:
                 self.log('Files not processed')
            return dest
        except Exception, e:
            self.log('%s' % str(e))
            remoteCtrl._release()
            raise Exception, "Post-processing failed (%s)" %  str(e)

    # Capture
    def capture(self, n, name = 'acq'):
        remoteCtrl = self._retrieveProxy()
        try:
            self.log('Capturing (%d meas) ...' %n)
            destPath = remoteCtrl.capture(n, name)
            self.log('Capture completed: stored on server (%s)' % destPath)
            return destPath
        except Exception, e:
            self.log('%s' % str(e))
            remoteCtrl._release()
            raise Exception, "Capture failed (%s)" %  str(e)

    # Produce
    def produce(self, name):
        remoteCtrl = self._retrieveProxy()
        try:
            self.log('Producing  ...')
            dest = remoteCtrl.produce(name)
            self.log('Produce completed: %s' % dest)
            return dest
        except Exception, e:
            self.log('%s' % str(e))
            remoteCtrl._release()
            raise Exception, "Produce failed (%s)" %  str(e)
    
