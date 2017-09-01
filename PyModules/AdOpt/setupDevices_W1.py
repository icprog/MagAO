#!/usr/bin/env python

import threading, thread

from AdOpt import AOExcept, AOConstants, processControl, cfg
from AdOpt.hwctrl import *


## --------- A thread with exception reporting

class mythread( threading.Thread):

    def run(self):
        self.exception = None
        try:
            threading.Thread.run(self)
        except AOExcept.AdOptError, e:
            self.exception = e 
        except Exception, e:
            self.exception = AOExcept.AdOptError( errstr = str(e))

class setupDevices_W1:

    def __init__(self):

        self._locks = {}

    def ident(self):
        return 'Setup unit W#1'

    def setup( self, app, devices, parallel = True):
        '''   
        Setup WFS devices. If they are already setup, don't do anything to that device.
        <devices> is a list of device names.

        In case of fatal error (i.e. network connection not found) raises an exception.
        In case of partial error (i.e. some devices working, others not) returs a tuple with:
        1) a list of devices not in the correct state
        2) an error string with a more detailed description.
        '''

        self._app = app

        self.message(self.ident())

        # Setup locking
        for dev in devices + ['flowerpot']:
            if not self._locks.has_key(dev):
                self._locks[dev] = thread.allocate_lock()
    
        # Clean up any previous locking that we may have from previous aborted runs.
        for k in self._locks.keys():
            if self._locks[k].locked():
                self._locks[k].release()


        faults = []
        errstr = ''

        try:
            # ------------ Devices which must be done one at a time
            # -- or are necessary for the second group


            if 'ts8sx' in devices:
                self.message( 'Waiting for network connection (left box)')
                try:
                    waitIPhost( self._app, 'ts8sx', waitTimeout = 60)
                except AOExcept.AdOptError, e:
                    if e.code == AOConstants.AOErrCode['TIMEOUT_ERROR']:
                        e.errstr = 'Missing network connection with ts8 sx. Check network cable and 110V line'
                    raise e
                except:
                    raise

                self._app.power._ctrl['powerboard'].waitActive()

            # - W#2 needs the power supply #3 turned on at this point
            if 'ps3' in self._app.power.list():
                self._app.power.turnon('ps3')

            # - W#1 needs the main power supply turned on at this point
            if 'main' in self._app.power.list():
                self._app.power.turnon('main')


            if 'ts8dx' in devices:

                self.message( 'Waiting for network connection (right box)')
                try:
                    waitIPhost( self._app, 'ts8dx', waitTimeout = 60)
                except AOExcept.AdOptError, e:
                    if e.code == AOConstants.AOErrCode['TIMEOUT_ERROR']:
                        e.errstr = 'Missing network connection with ts8 dx. Check network cable and 110V line'
                    raise e
                except:
                    raise

            self._app.arbMsg('WFS: setting up BCUs...')


            if 'bcu39' in devices:
                self.setupBcu39()

            if 'bcu47' in devices:
                self.setupBcu47()

            if 'visao' in devices:
                self.setupVisAO()

            self._app.arbMsg('WFS: setting up flowerpot...')

            if 'flowerpot' in devices:
                self.setupFlowerpot()

            if not parallel:

                if 'stages' in devices:
                    self.setupXYZStages()

                if 'adc' in devices:
                    self.setupADC()
 
                if 'lamp' in devices:
                    self.setupLamp()

                if 'ccd39' in devices:
                    self.setupCCD39()

                if 'ccd47' in devices:
                    self.setupCCD47()

                if 'cube' in devices:
                    self.setupCube()

                if 'fw' in devices:
                    self.setupFW()

                if 'rerot' in devices:
                    self.setupRerotator()

                if 'cameralens' in devices or 'lens' in devices:
                    self.setupCameralens()

                if 'tt' in devices:
                    self.setupTT()


            # ------------------- parallel setup
            else:

                threads = []
                timeout = 120 # maximum time in seconds for each thread


                if 'stages' in devices:
                    threads.append( mythread( target=self.setupXYZStages, args= (), name = 'setupXYZstages'))
                if 'adc' in devices:
                    threads.append( mythread( target=self.setupADC, args= (), name = 'setupADC'))
 
                if 'lamp' in devices:
                    threads.append( mythread( target=self.setupLamp, args= (), name = 'setupLamp'))

                if 'ccd39' in devices:
                    threads.append( mythread( target=self.setupCCD39, args= (), name = 'setupCCD39'))

                if 'ccd47' in devices:
                    threads.append( mythread( target=self.setupCCD47, args= (), name = 'setupCCD47'))

                if 'cube' in devices:
                    threads.append( mythread( target=self.setupCube, args= (), name = 'setupCube'))

                if 'fw' in devices:
                    threads.append( mythread( target=self.setupFW, args= (), name = 'setupFW'))

                if 'rerot' in devices:
                    threads.append( mythread( target=self.setupRerotator, args= (), name = 'setupRerotator'))

                if 'tt' in devices:
                    threads.append( mythread( target=self.setupTT, args = (), name = 'setupTT'))

                if 'cameralens' in devices or 'lens' in devices:
                    threads.append( mythread( target=self.setupCameralens, args = (), name = 'setupCameralens'))

                ########################
                # Start of parallel threads

                for t in threads:
                    t.start()

                self.message( 'Parallel setup started')

                self._app.arbMsg('WFS: homing all movements')

                for t in threads:
                    t.join(timeout)
                    if t.isAlive():
                        ## Device timeout. Add to faults
                        faults.append(t.getName())
                        errstr += '%s: setup did not complete within %d seconds\n' % (t.getName(), timeout)
    
                    if hasattr(t, 'exception'):
                        if t.exception != None:
                            # Setup thread exception
                            faults.append(t.getName())
                            errstr += '%s: %s\n' % (t.getName(), t.exception.errstr)

                self.message( 'Parallel setup done.')

                self._app.arbMsg('WFS: setup done.')

        except AOExcept.AdOptError, e:

            # Be sure of releasing locks if something happens
            for k in self._locks.keys():
                if self._locks[k].locked():
                    self._locks[k].release()

            print e
            self.message( e.__str__())
            raise

        return ( faults, errstr)


    def message( self, msg):

        self._app.log(msg)

        if hasattr(self._app, 'setStatus'):
            self._app.setStatus(msg)

    ## --------------- Individual WFS pieces setup

    def setupBcu39_prepare(self):
        '''
        Prepares bcu39 preconditions
        Override this on other W units with different preconditions.
        '''

        # This turn the entire AGW on for unit W#1

        processControl.startAndWait( self._app, 'powerboard')
        self._app.power._ctrl['powerboard'].waitActive()
        self._app.power.turnon('boxfans')
        self._app.power.turnon('linear')

    def setupBcu39(self):
        '''
        Prepare BCU 39 for operation.
        '''
    
        self.setupBcu39_prepare()
        self._locks['bcu39'].acquire()
    
        processControl.startAndWait( self._app, 'slopecompctrl')
    
        # If already setup, don't do anything
        if self._app.sc.isActive() and self._app.power.is_on('prog39'):
            self._locks['bcu39'].release()
            return
    
        self.message( 'Setting up BCU 39')

        if 'prog39' in self._app.power.list():
            self._app.power.turnon('prog39')
        if 'reset39' in self._app.power.list():
            self._app.power.turnoff('reset39')
    
        # Wait for bcu39 boot
        # If it does not start, try the cold-temperature workaround: reset and de-reset the bcu39 to allow starting
        try:
            waitIPhost( self._app, 'bcu39', waitTimeout = 20)
        except AOExcept.AdOptError, e:
            if e.code == AOConstants.AOErrCode['TIMEOUT_ERROR']:
                self._app.log("Timeout waiting for bcu39, trying restart...")
                self._app.power.turnon('reset39')
                self._app.power.turnoff('reset39')
                waitIPhost( self._app, 'bcu39', waitTimeout = 20)
    
        self._app.sc.waitActive()
    
        self.message( 'BCU 39 OK')
    
        self._locks['bcu39'].release()
    
    def setupBcu47_prepare(self):
        '''
        Prepares bcu47 preconditions
        Override this on other W units with different preconditions.
        '''
        self.setupBcu39()
        processControl.startAndWait( self._app, 'bcurelays')
        self._app.power._ctrl['bcurelays'].waitActive()
    
    def setupBcu47(self):
        '''
        Prepares BCU 47 for operation.
        '''
    
        self._locks['bcu47'].acquire()
        self.setupBcu47_prepare()
    
        processControl.startAndWait( self._app, 'techviewctrl')
    
        # If already setup, don't do anything
        if self._app.tv.isActive() and self._app.power.is_on('prog47'):
            self._locks['bcu47'].release()
            return
    
        self.message( 'Setting up BCU 47')
    
        # Otherwise raise the back switch flag and reset the BCU
        self._app.power.turnon('prog47')
        self._app.power.turnon('reset47')
        waitIPhost( self._app, 'bcu47', waitTimeout = 30, offline=True)
        self._app.power.turnoff('reset47')
        waitIPhost( self._app, 'bcu47', waitTimeout = 30)
    
        self._app.tv.waitActive()
        self.message( 'BCU 47 OK')
        self._locks['bcu47'].release()
    
    def setupFlowerpot_prepare(self):
        '''
        Prepares flowerpot preconditions
        Override this on other W units with different preconditions.
        '''
        pass
    
    def setupFlowerpot(self):
        '''
        Prepares flowerpot for operation.
        '''
    
        self._locks['flowerpot'].acquire()
        self.setupFlowerpot_prepare()
    
        processControl.startAndWait( self._app, 'powerboard')
        processControl.startAndWait( self._app, 'picctrl')
    
        self._app.power._ctrl['powerboard'].waitActive()
        if self._app.power.is_on('flowerpot') and self._app.power._ctrl['picctrl'].isActive():
            self._locks['flowerpot'].release()
            return
    
        self.message( 'Setting up flowerpot')
    
        self._app.power.turnon('flowerpot')
        self._app.power._ctrl['picctrl'].waitActive()
    
        self.message( 'Flowerpot OK')
    
        self._locks['flowerpot'].release()
    
    def setupLamp_prepare(self):
        '''
        Prepares lamp preconditions
        Override this on other W units with different preconditions.
        '''
        self.setupFlowerpot()
    
    def setupLamp(self):
        '''
        Prepares flowerpot lamp for operation.
        '''
    
        self.message( 'Setting up lamp')
    
        self._locks['lamp'].acquire()
        self.setupLamp_prepare()
    
        self._app.power.turnon('lamp')
    
        self.message( 'Lamp OK')
        self._locks['lamp'].release()
    
    def setupCCD39_prepare(self):
        '''
        Prepares ccd39 preconditions
        Override this on other W units with different preconditions.
        '''
        processControl.startAndWait( self._app, 'bcurelays')
        self._app.power._ctrl['bcurelays'].waitActive()
     
    def setupCCD39(self):
        '''
        Prepares ccd39 for operation
        '''
    
        self._locks['ccd39'].acquire()
        self.setupCCD39_prepare()
        self.setupBcu39()
    
        self.message( 'Setting up ccd 39')
     
        processControl.startAndWait( self._app, 'ccd39')
    
        self._app.power.turnon('fans_LJ')
        self._app.power.turnon('ccd39')
    
        self._app.sc.waitActive( timeout = 30)
        self._app.sc.stop()
    
        self._app.ccd39.waitActive( timeout = 30)
    
        from AdOpt import WfsArbScripts
        WfsArbScripts.setHObinning( self._app, 1) 
        self._app.ccd39.set_framerate(197)
    
        self.message( 'Ccd39 OK')
    
        self._locks['ccd39'].release()
    
    def setupCCD47_prepare(self):
        '''
        Prepares CCD47 preconditions
        Override this on other W units with different preconditions.
        '''
        pass
    
    
    def setupCCD47(self):
        '''
        Prepares ccd47 for operation
        '''
    
        self._locks['ccd47'].acquire()
    
        self.setupCCD47_prepare()
        self.setupBcu47()
    
        self.message( 'Setting up ccd 47')
    
        self._app.tv.waitActive( timeout=10)
        self._app.tv.stop()
        self._app.power.turnon('fans_LJ')
        self._app.power.turnon('ccd47')
    
        self._app.ccd47.waitActive( timeout = 30)
        self._app.tv.waitActive( timeout = 30)
    
        self._locks['bcu47'].acquire()
        from AdOpt import WfsArbScripts
        WfsArbScripts.setTVbinning( self._app, 1)
        self._locks['bcu47'].release()
    
        self.message( 'Ccd47 OK')
    
        self._locks['ccd47'].release()
    
    def setupCube_prepare(self):
        '''
        Prepares cube preconditions
        Override this on other W units with different preconditions.
        '''
        self.setupBcu39()
        self.setupFlowerpot()
        
    
    def setupCube(self):
        '''
        Prepares cube for operation.
        '''
    
        self._locks['cube'].acquire()
    
        self.setupCube_prepare()
    
        self.message( 'Setting up cube')
    
        processControl.startAndWait( self._app, 'mercury')
        processControl.startAndWait( self._app, 'cubestage')
        processControl.startAndWait( self._app, 'picctrl')
    
        self._app.power._ctrl['picctrl'].waitActive()
        self._app.power.turnon('cubestage')
        self._app.power.turnon('cuberot')
    
        self._app.cubestage.waitStatus( 'STATE_READY', waitTimeout = 60)
        self._app.cuberot.waitStatus( 'STATE_READY', waitTimeout = 60)
    
        self.message( 'Cube OK')
    
        self._locks['cube'].release()
    
    def setupADC_prepare(self):
        '''
        Prepares adc preconditions
        Override this on other W units with different preconditions.
        '''
        self.setupBcu39()
        processControl.startAndWait( self._app, 'bcurelays')
        self._app.power._ctrl['bcurelays'].waitActive()
        
    
    def setupADC(self):
    
        self._locks['adc'].acquire()
    
        self.setupADC_prepare()
        self.message( 'Setting up adc')
    
        self._app.power.turnon('adc')
        self._app.adc.waitStatus( 'STATE_READY', waitTimeout = 90)
    
        # Special ADC homing only for W1
        if cfg.subsystem == 'W1':
            self._app.adc.home( waitTimeout = 120)
    
        self.message( 'ADC OK')
    
        self._locks['adc'].release()
    
    def setupCameralens_prepare(self):
        '''
        Prepares camera lens preconditions
        Override this on other W units with different preconditions.
        '''
        self.setupTT()
    
    def setupCameralens(self):
        '''
        Prepare cameralens for operation.
        '''
    
        self._locks['cameralens'].acquire()
    
        self.setupCameralens_prepare()
    
        self.message( 'Setting up cameralens')
   
        processControl.startAndWait( self._app, 'ttctrl')
    
        self._locks['bcu47'].acquire()
        self._app.lens.waitActive( timeout=30)
        self._app.lens.moveTo(0,0, waitTimeout=5)
        self._locks['bcu47'].release()
    
        self.message( 'Cameralens OK')
    
        self._locks['cameralens'].release()
    
    def setupFW_prepare(self):
        '''
        Filter wheels preconditions.
        Override this on other W units with different preconditions.
        '''
        self.setupBcu39()
        processControl.startAndWait( self._app, 'bcurelays')
        self._app.power._ctrl['bcurelays'].waitActive()
        
    def setupFW(self):
        '''
        Prepare filter wheels for operation.
        '''
    
        self._locks['fw'].acquire()
        self.setupFW_prepare()
    
        self.message( 'Setting up filterwheels')
    
        processControl.startAndWait( self._app, 'filterwheel1')
        processControl.startAndWait( self._app, 'filterwheel2')
    
        self._app.power.turnon('fw')
        
        self._app.fw1.waitStatus( 'STATE_READY', waitTimeout = 60)
        self._app.fw2.waitStatus( 'STATE_READY', waitTimeout = 60)
    
        self.message( 'Filterwheels OK')
    
        self._locks['fw'].release()
    
    def setupRerotator_prepare(self):
        '''
        Rerotator preconditions.
        Override this on other W units with different preconditions.
        '''
        self.setupBcu39()
        processControl.startAndWait( self._app, 'bcurelays')
        self._app.power._ctrl['bcurelays'].waitActive()
        
    
    def setupRerotator(self):
        '''
        Prepares the pupil rerotator for operation.
        '''
        self._locks['rerot'].acquire()
        self.setupRerotator_prepare()
    
        self.message( 'Setting up rerotator')
    
        processControl.startAndWait( self._app, 'rerotator')
        self._app.power.turnon('rirot')
        
        self._app.rerot.waitStatus( 'STATE_READY', waitTimeout = 120)
    
        self.message( 'Rerotator OK')
    
        self._locks['rerot'].release()
    
    def setupTT_prepare(self):
        '''
        TT preconditions.
        Override this on other W units with different preconditions.
        '''
        self.setupBcu47()
        
    def setupTT(self):
        '''
        Prepares tip-tilt for operation.
        '''
    
        self._locks['tt'].acquire()
        self.setupTT_prepare()
    
        self.message( 'Setting up tip-tilt')
        processControl.startAndWait( self._app, 'ttctrl')
    
        if self._app.tt.isOperating():
            self._locks['tt'].release()
            self.message( 'Tip-tilt OK')
            return
    
    
        self._app.tt.waitActive(timeout=10)
        self._app.tt.set( amp=3, freq=300, offx=0, offy=0)
        self._app.tt.start()
    
        self.message( 'Tip-tilt OK')
    
        self._locks['tt'].release()
    
    def setupXYZStages_prepare(self):
        '''
        XYZ stages preconditions.
        Override this on other W units with different preconditions.
        '''
        self.setupBcu39()
        processControl.startAndWait( self._app, 'bcurelays')
        self._app.power._ctrl['bcurelays'].waitActive()
        self._app.power.turnon('idrive')
    
    
    def setupXYZStages(self):
        '''
        Prepares XYZ stages for operation.
        '''
    
        self._locks['stages'].acquire()
        self.setupXYZStages_prepare()
    
        self.message( 'Setting up XYZ stages')
    
        # Start control processes
        processControl.startAndWait( self._app, 'baysidex')
        processControl.startAndWait( self._app, 'baysidey')
        processControl.startAndWait( self._app, 'baysidez')
    
        # Wait for startup OK
        self._app.stagex.waitStatus( 'STATE_OFF', waitTimeout = 60)
        self._app.stagey.waitStatus( 'STATE_OFF', waitTimeout = 60)
        self._app.stagez.waitStatus( 'STATE_OFF', waitTimeout = 60)

        # Set parameters: stage enabled, control off after movements
        self._app.stagex.enable( enable = True, onaftermove = False)
        self._app.stagey.enable( enable = True, onaftermove = False)
        self._app.stagez.enable( enable = True, onaftermove = False)

        # Here wait for confirmation from controllers that they are ready.
        # For example, clearing of the "undervoltage" error
        import time
        time.sleep(5)

        # Move Z stage forward by a few mm - it fails when starting directly
        # from the home position by chance.
#        self._app.stagez.moveBy(5.0)
#        self._app.stagez.waitStatus( 'STATE_OPERATING', waitTimeout = 10)
#        self._app.stagez.waitStatus( 'STATE_OFF', waitTimeout = 10)
#    
#        # Start home movement
#        self._app.stagez.home()
#        self._app.stagey.home()
#        self._app.stagex.home()
#    
#        # Wait for start of homing movement - should start all three almost at the same time
#        self._app.stagex.waitStatus( 'STATE_HOMING', waitTimeout = 10)
#        self._app.stagey.waitStatus( 'STATE_HOMING', waitTimeout = 10)
#        self._app.stagez.waitStatus( 'STATE_HOMING', waitTimeout = 10)
#    
#        # Wait for all three to finish
#        self._app.stagex.waitStatus( 'STATE_OFF', waitTimeout=120)
#        self._app.stagey.waitStatus( 'STATE_OFF', waitTimeout=120)
#        self._app.stagez.waitStatus( 'STATE_OFF', waitTimeout=120)
#    
#        # Set default parameters: stage enabled, keep control after movements
#        self._app.stagex.enable( enable = True, onaftermove = True)
#        self._app.stagey.enable( enable = True, onaftermove = True)
#        self._app.stagez.enable( enable = True, onaftermove = True)
#   
#        self.message( 'XYZ stages OK')
#
#        self._locks['stages'].release()

 
def waitIPhost( app, name, waitTimeout = 0, offline = False):
        '''
        Waits until the specified host is up. Uses the PINGER module 
        variables to decide.
        If offline is True, waits for the specified host to go down instead of up.
        ''' 

        import cfg
        varname = 'pinger.%s.%s' % (cfg.side, name)

        if offline:
            app.WaitVarValue( varname, -1, timeout = waitTimeout)
        else:
            app.WaitVarFunc( varname, lambda x: x>0, timeout=waitTimeout)

def loadDevices( app, deviceList = None, check = False):
    '''
    Tries to load all Python interfaces into the AOApp.
    Returns a list of names of the interfaces successfully loaded.
    if <check> is true, any device not loaded will trigger an exception. Otherwise,
    they will be skipped and the application will have to check the returned list.

    To load only certain devices, use the deviceList arguments. If not devices
    are specified, all devices will be loaded.

    This function can be called multiple times with no errors. Devices already
    loaded will not be re-loaded.
    '''

    if not deviceList:
        deviceList = ['power', 'fw1', 'fw2', 'adc', 'ccd39', 'sc', 'ccd47', 'tv',
                      'rerot', 'cuberot', 'cubestage', 'stagex', 'stagey', 'stagez',
                      'tt', 'thorlabs', 'lamp', 'lens', 'flowerpot', 'temps'] # 'irtc', 'pisces']
    dev_ok = []

    devicemap = dict((
                ('power', 'agw_power.AGW_Power(app)'),
                ('fw1',   'simplemotor.SimpleMotor("filterwheel1", app)'), 
                ('fw2',   'simplemotor.SimpleMotor("filterwheel2", app)'), 
                ('adc',   'adc.Adc(app)'), 
                ('ccd39', 'ccd.ccd("ccd39", app)'),
                ('sc',    'slopecomp.slopecomp("slopecompctrl", app)'),
                ('ccd47', 'ccd47.ccd47(app)'),
                ('tv',    'framegrabber.framegrabber("techviewctrl", app)'),
                ('rerot', 'simplemotor.SimpleMotor("rerotator", app)'),
                ('cuberot', 'simplemotor.SimpleMotor("mercury", app)'),
                ('cubestage', 'simplemotor.SimpleMotor("cubestage", app)'),
                ('stagex', 'bayside_stage.bayside_stage("baysidex", app)'),
                ('stagey', 'bayside_stage.bayside_stage("baysidey", app)'),
                ('stagez', 'bayside_stage.bayside_stage("baysidez", app)'),
                ('tt',     'tt.TipTilt("ttctrl", app)'),
                ('lamp',   'lamp.Lamp("picctrl", app)'),
                ('lens',   'cameralens.CameraLens("ttctrl", app)'),
                ('thorlabs', 'thorlabs_ccd.thorlabs_ccd(app)'),
                ('flowerpot', 'calib_unit.calib_unit(app)'),
                ('irtc', 'irtc_ccd.irtc_ccd(app)'),
                ('pisces', 'pisces_ccd.pisces_ccd(app)'),
                ('gimbal', 'gimbal.gimbal("gimbal",app)'),
                ('shutter', 'shutter.shutter("shutterremote",app)'),
                ('temps', 'temps.temps(app)')
                ))

    loaded = []
    for device in deviceList:
        try:
            if hasattr(app, device) == False:
                app.log('Loading '+device)
                setattr( app, device, eval(devicemap[device]))
                loaded.append(device)
            dev_ok.append(device)
        except Exception,e:
            str = 'Exception loading device %s: %s %s' % (device, e, type(e))
            app.log(str)
            if check:
                raise AOExcept.AdOptError( errstr = str)

    if len(loaded)>0:
        app.log('Loaded devices: %s' % loaded)
    return dev_ok
 

