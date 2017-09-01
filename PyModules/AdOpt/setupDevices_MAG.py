#!/usr/bin/env python
#
# Setupdevices overrides for unit W#2

from setupDevices_W1 import *
from processControl import *

class setupDevices_MAG(setupDevices_W1):

    def ident(self):
        return 'setup unit Magellan WFS'

    def setupMoxaDioCcd(self):
        try:
            waitIPhost( self._app, 'dioccd', waitTimeout = 60)
        except AOExcept.AdOptError, e:
            if e.code == AOConstants.AOErrCode['TIMEOUT_ERROR']:
                e.errstr = 'Missing network connection with Moxa ccd I/O. Check network cable and 110V line'
            raise e
        except:
            raise
        self._app.power._ctrl['moxadioccd'].waitActive()

    def setupMoxaDioDrv(self):
        try:
            waitIPhost( self._app, 'diodrv', waitTimeout = 60)
        except AOExcept.AdOptError, e:
            if e.code == AOConstants.AOErrCode['TIMEOUT_ERROR']:
                e.errstr = 'Missing network connection with Moxa drv I/O. Check network cable and 110V line'
            raise e
        except:
            raise
        self._app.power._ctrl['moxadiodrv'].waitActive()

    def setupVisAO(self):
        try:
            self._app.power.turnon('visaogimbal')
            self._app.power.turnon('visaomotors')
            self._app.power.turnon('visaoshutter')
            self._app.power.turnon('visaofan1')
            self._app.power.turnon('visaofan2')
            self._app.power.turnon('visaots8')
            self._app.power.turnon('visaocomp')
        except:
            raise


    def setupBcu39_prepare(self):
        '''
        Preconditions for bcu39 operation
        '''
        self.setupMoxaDioCcd()

    def setupBcu39(self):
        '''
        Prepare BCU 39 for operation.
        '''

        self.setupBcu39_prepare()
        self._locks['bcu39'].acquire()

        #processControl.startAndWait( self._app, 'slopecompctrl')

        # If already setup, don't do anything
        if self._app.sc.isActive() and self._app.power.is_on('bcu'):
            self._locks['bcu39'].release()
            return

        self.message( 'Setting up BCU 39')

        self._app.power.turnon('bcu')

        # Wait for bcu39 boot
        waitIPhost( self._app, 'bcu39', waitTimeout = 60)
        self._app.sc.waitActive()

        self.message( 'BCU 39 OK')
    
        self._locks['bcu39'].release()


    def setupBcu47_prepare(self):
        '''
        Prepares bcu47 preconditions
        Override this on other W units with different preconditions.
        '''
        self.setupMoxaDioCcd()
    
    def setupBcu47(self):
        '''
        Prepares BCU 47 for operation.
        '''
    
        self._locks['bcu47'].acquire()
        self.setupBcu47_prepare()
    
        # If already setup, don't do anything
        if self._app.tv.isActive() and self._app.power.is_on('bcu'):
            self._locks['bcu47'].release()
            return
    
        self.message( 'Setting up BCU 47')
    
        self._app.power.turnon('bcu')
        waitIPhost( self._app, 'bcu47', waitTimeout = 60)
    
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
    
        #processControl.startAndWait( self._app, 'powerboard')
        #processControl.startAndWait( self._app, 'picctrl')
    
        self.message( 'Setting up flowerpot')
    
        self._app.power.turnon('pic')
        time.sleep(5)
    
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
        self.setupMoxaDioCcd()
     
    def setupCCD39(self):
        '''
        Prepares ccd39 for operation
        '''
    
        self._locks['ccd39'].acquire()
        self.setupCCD39_prepare()
        self.setupBcu39()
    
        self.message( 'Setting up ccd 39')
     
        #processControl.startAndWait( self._app, 'ccd39')
    
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
        self.setupMoxaDioCcd()
    
    
    def setupCCD47(self):
        '''
        Prepares ccd47 for operation
        '''
    
        self._locks['ccd47'].acquire()
    
        self.setupCCD47_prepare()
    
        self.message( 'Setting up ccd 47')
    
        self._app.power.turnon('ccd47')
    
        self._app.ccd47.waitActive( timeout = 30)
    
        self.message( 'Ccd47 OK')
    
        self._locks['ccd47'].release()
    
    def setupCube_prepare(self):
        '''
        Prepares cube preconditions
        Override this on other W units with different preconditions.
        '''
        self.setupFlowerpot()
        
    
    def setupCube(self):
        '''
        Prepares cube for operation.
        '''
    
        self._locks['cube'].acquire()
    
        self.setupCube_prepare()
    
        self.message( 'Setting up cube')
    
        #processControl.startAndWait( self._app, 'mercury')
        #processControl.startAndWait( self._app, 'cubestage')
        #processControl.startAndWait( self._app, 'picctrl')
    
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
        self.setupMoxaDioDrv()
        
    
    def setupADC(self):
    
        self._locks['adc'].acquire()
    
        self.setupADC_prepare()
        self.message( 'Setting up adc')
    
        self._app.power.turnon('adc1')
        self._app.power.turnon('adc2')
        self._app.adc.waitStatus( 'STATE_READY', waitTimeout = 90)
    
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
   
        #processControl.startAndWait( self._app, 'ttctrl')
    
        self.message( 'Cameralens OK')
    
        self._locks['cameralens'].release()
    
    def setupFW_prepare(self):
        '''
        Filter wheels preconditions.
        Override this on other W units with different preconditions.
        '''
        self.setupMoxaDioDrv()
        
    def setupFW(self):
        '''
        Prepare filter wheels for operation.
        '''
    
        self._locks['fw'].acquire()
        self.setupFW_prepare()
    
        self.message( 'Setting up filterwheels')
    
        #processControl.startAndWait( self._app, 'filterwheel1')
        #processControl.startAndWait( self._app, 'filterwheel2')
    
        self._app.power.turnon('fw1')
        
        self._app.fw1.waitStatus( 'STATE_READY', waitTimeout = 60)
    
        self.message( 'Filterwheels OK')
    
        self._locks['fw'].release()
    
    def setupRerotator_prepare(self):
        '''
        Rerotator preconditions.
        Override this on other W units with different preconditions.
        '''
        self.setupMoxaDioDrv()
        
    
    def setupRerotator(self):
        '''
        Prepares the pupil rerotator for operation.
        '''
        self._locks['rerot'].acquire()
        self.setupRerotator_prepare()
    
        self.message( 'Setting up rerotator')
    
        #processControl.startAndWait( self._app, 'rerotator')
        self._app.power.turnon('rirot')
        
        self._app.rerot.waitStatus( 'STATE_READY', waitTimeout = 120)
    
        self.message( 'Rerotator OK')
    
        self._locks['rerot'].release()
    
    def setupTT_prepare(self):
        '''
        TT preconditions.
        Override this on other W units with different preconditions.
        '''
        self.setupBcu39()
        
    def setupTT(self):
        '''
        Prepares tip-tilt for operation.
        '''
    
        self._locks['tt'].acquire()
        self.setupTT_prepare()
    
        self.message( 'Setting up tip-tilt')
        #processControl.startAndWait( self._app, 'ttctrl')
    
        if self._app.tt.isOperating():
            self._locks['tt'].release()
            self.message( 'Tip-tilt OK')
            return
    
    
        self._app.tt.waitActive(timeout=30)
        self._app.tt.set( amp=0, freq=10, offx=0, offy=0)
        self._app.tt.start()
    
        self.message( 'Tip-tilt OK')
    
        self._locks['tt'].release()
    
    def setupXYZStages_prepare(self):
        '''
        XYZ stages preconditions.
        Override this on other W units with different preconditions.
        '''
        self.setupMoxaDioDrv()
        self._app.power.turnon('xstage')
        self._app.power.turnon('ystage')
        self._app.power.turnon('zstage')
    
    
    def setupXYZStages(self):
        '''
        Prepares XYZ stages for operation.
        '''
    
        self._locks['stages'].acquire()
        self.setupXYZStages_prepare()
    
        self.message( 'Setting up XYZ stages')
    
        # Start control processes
        #processControl.startAndWait( self._app, 'baysidex')
        #processControl.startAndWait( self._app, 'baysidey')
        #processControl.startAndWait( self._app, 'baysidez')
    
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
#        import time
#        time.sleep(5)
#
#        # Move Z stage forward by a few mm - it fails when starting directly
#        # from the home position by chance.
#        self._app.stagez.moveBy(5.0)
#        self._app.stagez.waitStatus( 'STATE_OPERATING', waitTimeout = 10)
#        self._app.stagez.waitStatus( 'STATE_OFF', waitTimeout = 10)
#    
#        # Start home movement

    def turnBcuOn(self):

        # 1. Turn on BCU

        print 'Turning BCU power ON'
        self._app.power.turnon('bcu')

        # 2. Wait until bcu is up

        print 'Waiting until BCU answers on the Ethernet...'
        setupDevices.waitIPhost( app, 'bcu39', waitTimeout=60)

        # 3. Force TT/CL rest (in case things were already on)

        print 'Ensuring that tip/tilt mirror is RESTed...'
        self._app.tt.mirrorSet(0)

        # 4. Turn on CCD

        print 'Turning ON ccd39'
        self._app.power.turnon('ccd39')

        # 5. Wait until CCD is online and stop it

        print 'Waiting until ccd39 answers on the serial line'
        self._app.ccd39.waitActive( timeout = 30)
        self._app.ccd39.stop()

        # 7. Set TT

        print 'Setting TT/CL...'
        self._app.tt.mirrorSet(1)

        # 8. Configure CCD

        print 'Configuring CCD with bin1x1, 200 Hz...'
        from AdOpt import WfsArbScripts
        WfsArbScripts.setHObinning( app, 1)
        self._app.ccd39.set_framerate(200)

