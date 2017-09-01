#!/usr/bin/env python

# Standalone CCD viewer for the AdOpt system


import qt, sys
import thread, time, types, os, math
import numpy

import gc
#gc.set_debug( gc.DEBUG_LEAK | gc.DEBUG_STATS)
gc.disable()


from AdOpt import cfg, processControl, fits_lib, makePupils, calib
from AdOpt.WfsGUI import pixelview, cfg_gui, pupilshift
from AdOpt.AOConstants import *
from AdOpt.AOExcept import *

from qt import *
from AdOpt.WfsGUI.thAOAppGUI import *

from AdOpt.widgets.rtdbStatusLight import *


class plotWidget( qt.QLabel):
    def __init__(self, parent, num):
        qt.QLabel.__init__(self, parent)
        self._num = num

    # Redirect automatic drawing to show the last frame
    def drawContents(self, painter):
        pixelview.replot( self._num)

    # Avoid any other automatic drawing
    def drawFrame( self, painter):
        pass    

    def setText( self, text):
        pass

    def resizeEvent(self, ev):
        s = ev.size()
        pixelview.replot( self._num, s.width(), s.height())


class ccd_window( qt.QScrollView):

    def __init__(self, parent):
        qt.QScrollView.__init__(self, parent)
        self.parent = parent
       
        self.frame = frame_window( None, self.viewport())
        self.addChild(self.frame)
    


class frame_window( qt.QLabel):

    def __init__(self, name, parent):
        #qt.QLabel.__init__(self, name, parent)
        qt.QLabel.__init__(self, parent)
        self.setWFlags( qt.Qt.WNoAutoErase)
        self.acqPupilWidth = True
        self.w = self.parent().parent().parent
        self.px = -1
        self.py = -1

        # Rect drawing data
        self.px1 = -1
        self.px2 = -1
        self.py1 = -1
        self.py2 = -1
        self.tracking = False
        self.zoomRect = QRect( 0, 0, 0, 0)
        pixelview.setZoomingRect( 0, 0, 0, 0)


    # Support for rect drawing
    def saveMousePos(self, event):
        self.px = float(event.pos().x()) / self.w.dx
        self.py = float(event.pos().y()) / self.w.dy 

    def mousePressEvent( self, event):
        '''
        Save current position (in screen pixels) and start rect tracking.
        '''
        self.px1 = event.pos().x()
        self.py1 = event.pos().y() 
        self.tracking = True

    def mouseReleaseEvent( self, event):
        '''
        Stop rect tracking and trigger zoom to the drawn rect.
        '''
        self.tracking = False

        # Abort in case of incorrect zooms
        if self.zoomRect.width()<1 or self.zoomRect.height<1:
            self.zoomRect = QRect( 0, 0, 0, 0)
            pixelview.setZoomingRect( 0, 0, 0, 0)
            return

        self.zoomToRect()


    def zoomToRect(self):

        # Calculate new center (in ccd pixels)
        cx = self.zoomRect.center().x() / self.w.dx
        cy = self.zoomRect.center().y() / self.w.dy

        # Calculate new zoom factor
        npixels = self.zoomRect.width() / self.w.dx
        wantedzoom = self.w.viewer.visibleWidth() / npixels
        if wantedzoom > self.w.maxscale:
            newzoom  = self.w.maxscale
        else:
            newzoom = wantedzoom
	if newzoom==0:
		newzoom=1
	if newzoom<1:
		newzoom = -(1/newzoom)
		

        # Calculate equivalent rect dimensions
        w = self.zoomRect.width() * (wantedzoom/float(newzoom)) / self.w.dx
        h = self.zoomRect.height() * (wantedzoom/float(newzoom))  / self.w.dy

        # Calc window origin in pixels
        x1 = cx - (w/2) 
        y1 = cy - (h/2) 
        if x1<0:
            x1=0
        if y1<0:
            y1=0

        # Set new zoom
        self.w.slider.setValue( newzoom)

        # Move window origin
        self.w.viewer.setContentsPos(x1*self.w.dx, y1*self.w.dy)

        # Remove tracking display 
        self.zoomRect = QRect( 0, 0, 0, 0)
        pixelview.setZoomingRect( 0, 0, 0, 0)

    def mouseMoveEvent( self, event):
        '''
        Mouse move event: show current pixel value and, if tracking, update rect coordinates with the current position (in screen pixels)
        '''
        self.saveMousePos(event)
        self.setMouseValue()

        self.px2 = event.pos().x()
        self.py2 = event.pos().y()

        if self.tracking:
            self.zoomRect = QRect( QPoint( self.px1, self.py1), QPoint( self.px2, self.py2)).normalize()
            if self.zoomRect.width()>1 and self.zoomRect.height()>1:
                pixelview.setZoomingRect( self.zoomRect.left(), self.zoomRect.top(), self.zoomRect.right(), self.zoomRect.bottom())
             


    # Show last pixel coords and current value
    def setMouseValue(self):
        w = self.parent().parent().parent

        if (self.px >=0) and (self.py>=0) and (self.px < w.nx) and (self.py < w.ny):

            value = pixelview.getPixelValue( int(self.px), int(self.py))
            if type(value) == types.IntType:
                display = "Pixel value: %d - X: %5.1f - Y: %5.1f" % (value, self.px, self.py)
            else:
                if value >0:
                    sign = "+"
                else:
                    sign = "-"
                    value = -value

                display = "Slope value: %s%4.2e - X: %5.1f - Y: %5.1f" % (sign, value, self.px, self.py)

            w.pixelvalue = display
            w.callAfter( w.setPixelValue, ())
            if hasattr(w, 'plotY'):
                w.callAfter( pixelview.fillCutPlots, (int(self.px), int(self.py)))
                w.callAfter( w.plotX.drawContents, (0))
                w.callAfter( w.plotY.drawContents, (0))
                pass
        else:
            return None
    
    # Redirect automatic drawing to show the last frame
    def drawContents(self, painter):
        self.parent().parent().parent.redrawPixels()

    # Avoid any other automatic drawing
    def drawFrame( self, painter):
        pass    

    def setText( self, text):
        pass


class ccd_window( qt.QScrollView):

    def __init__(self, parent):
        qt.QScrollView.__init__(self, parent)
        self.parent = parent
       
        self.frame = frame_window( None, self.viewport())
        self.addChild(self.frame)
        self.frame.setMouseTracking(1)

        flags = self.getWFlags();
        flags |= qt.QWidget.WNoAutoErase
        self.setWFlags(flags)

    def mouseMoveEvent(self, event):
        pass

    def resize( self, x, y):
        self.frame.resize(x,y)

    def viewportResizeEvent(self, event):
       pixelview.setViewport( self.contentsX(), self.contentsY(), self.visibleWidth(), self.visibleHeight())


#+Class: ccd_viewer
#
# Finestra che implementa il visore dei pixels
#-

class ccd_viewer( thAOAppGUI):

    def close(self, alsoDelete):
	self.timetodie = True
        self.die()
        return QMainWindow.close( self, alsoDelete)

    def __init__(self, qApp):

        # Select ccd type
        if len(sys.argv)<2:
            print 'Please specify ccd type'
            sys.exit(0)

	if '-i' in sys.argv:
		idx = sys.argv.index('-i')
		del sys.argv[idx]
		del sys.argv[idx]

        ccd = sys.argv[1]
        self.ccdName = ccd
        self.fw1pos=0
	self.ccdgain=0

	self.timetodie = False

        thAOAppGUI.__init__(self, qApp, 'CCDVIEW'+ccd, multi=True)
        cfg_gui.init()
        self.lock = thread.allocate_lock()
        self.lastLiveFlag = False

        self.acqData = calib.loadCalibData( calib.sourceAcqCalibFile(), check=['refMagFlux', 'refMag'])


        # Crea la finestra
        self.gridLayout = qt.QGridLayout( self, 24, 4, 10)
        self.gridLayout.setRowStretch( 11, 0)
        self.gridLayout.setRowStretch( 13, 1)
        self.gridLayout.setColStretch(0,1)
        self.gridLayout.setColStretch(1,1)
        self.gridLayout.setColStretch(2,0)

        self.name = "CCD viewer"
        self.currentrec = ""
        self.pixelsReceived=False

        self.varname_curxbin = cfg.varname( ccd, 'XBIN.CUR')
        self.varname_curybin = cfg.varname( ccd, 'YBIN.CUR')
        self.varname_dx = cfg.varname( ccd, 'DX')
        self.varname_dy = cfg.varname( ccd, 'DY')
        self.varname_dy = cfg.varname( ccd, 'DY')
        self.varname_fw1pos = cfg.varname('filterwheel1', 'POS.CUR')
        self.varname_ccdgain = cfg.varname('ccd39', 'GAIN.CUR')
        self.invertxy = False
	self.showStagesStatus = False
	self.showPixelLut = False
	self.ccdStatus = 0
        self.curSlopenull = ''

        self.refreshInterval=50

        if ccd.lower()=='test':
            self.setCaption("CCD39 Viewer")
            self.bufname = 'PRODC-01:PIPPO'
            self.ccd_type = 39
            self.minscale = 1
            self.maxscale = 19
            # Imposta le dimensioni dei pixels ad un default ragionevole
            self.xbin = 1
            self.ybin = 1
            self.ccdW = 80
            self.ccdH = 80
            self.canChangeBinning = False
            self.showSlopes = True
            self.showRms = True
            self.showRealtimePupils = False
            self.showCloopPupils = True
            self.canAutoCenter = True
            self.doFindStar = False
            self.showCrux = False
            self.acceptableLiveErrors = 5
            self.showCuts = False
            self.logScale = False
            self.varname_darkvar = cfg.ccd_darkvariable['ccd39']+'.CUR'
            self.varname_slopenull = cfg.varname('slopecompctrl','SLOPENULL.CUR')
            self.varname_pixellut = cfg.varname('slopecompctrl','PIXELLUT.CUR')
            self.VarNotif( self.varname_pixellut, self.slotPixelLut)
            self.VarNotif( self.varname_slopenull, self.slotSlopenull)

        elif ccd.lower()=='ccd39':
            self.setCaption("CCD39 Viewer")
            self.bufname = cfg.ccd_framebuffer[39]
            self.ccd_type = 39
            self.minscale = 1
            self.maxscale = 19
            # Imposta le dimensioni dei pixels ad un default ragionevole
            self.xbin = self.ReadVar( self.varname_curxbin).Value()
            self.ybin = self.ReadVar( self.varname_curybin).Value()
            self.ccdW = self.ReadVar( self.varname_dx).Value()
            self.ccdH = self.ReadVar( self.varname_dy).Value()
            self.canChangeBinning = True
            self.showSlopes = True
            self.showRms = True
            self.showRealtimePupils = True
            self.showCloopPupils = True
            self.canAutoCenter = True
            self.doFindStar = False
            self.showCrux = False
            self.acceptableLiveErrors = 5
            self.showCuts = False
            self.logScale = False
            self.varname_darkvar = cfg.ccd_darkvariable['ccd39']+'.CUR'
            self.varname_slopenull = cfg.varname('slopecompctrl','SLOPENULL.CUR')
            self.varname_pixellut  = cfg.varname('slopecompctrl','PIXELLUT.CUR')
            self.varname_status    = cfg.varname('ccd39','STATUS')
            self.VarNotif( self.varname_pixellut,  self.slotPixelLut)
            self.VarNotif( self.varname_slopenull, self.slotSlopenull)
            self.VarNotif( self.varname_status,    self.slotStatus)
            self.showStagesStatus = True
	    self.showPixelLut = True


        elif ccd.lower()=='ccd47':
            # Defaults to ccd47 bin 1x1
            self.setCaption("CCD47 Viewer")
            self.bufname = cfg.ccd_framebuffer[47] + "1048576"
            self.ccd_type = 47
            self.minscale = -5
            self.maxscale = +5
            self.xbin = 1
            self.ybin = 1
            self.ccdW = 1024
            self.ccdH = 1024
            self.canChangeBinning = True
            self.showSlopes = False
            self.showRms = False
            self.showRealtimePupils = False
            self.showCloopPupils = False
            self.canAutoCenter = False
            self.doFindStar = True
            self.showCrux = False
            self.showCuts = True
            self.logScale = True
            self.refreshInterval=50
            self.acceptableLiveErrors = 50 # very slow ccd
            self.varname_darkvar = cfg.ccd_darkvariable['ccd47']+'.CUR'
            self.varname_status    = cfg.varname('ccd47','STATUS')
            self.VarNotif( self.varname_status,    self.slotStatus)


        elif ccd.lower()=='thorlabs1':
            self.setCaption("Thorlabs Viewer")
            self.bufname = "ThorlabsProxy01:FRAME"
            self.ccd_type = 100
            self.minscale = -5
            self.maxscale = +5
            self.ccdW = 1024
            self.ccdH = 768
            self.xbin = 1
            self.ybin = 1
            self.canChangeBinning = False
            self.showSlopes = False
            self.showRms = False
            self.showRealtimePupils = True
            self.showCloopPupils = False
            self.canAutoCenter = False
            self.doFindStar = True
            self.showCrux = False
            self.showCuts = True
            self.logScale = True
            self.acceptableLiveErrors = 10
            self.varname_darkvar = ''
            self.invertxy = True

        elif ccd.lower()=='thorlabs2':
            self.setCaption("Thorlabs Viewer")
            self.bufname = "ThorlabsProxy02:FRAME"
            self.ccd_type = 100
            self.minscale = -5
            self.maxscale = +5
            self.ccdW = 1024
            self.ccdH = 768
            self.xbin = 1
            self.ybin = 1
            self.canChangeBinning = False
            self.showSlopes = False
            self.showRms = False
            self.showRealtimePupils = True
            self.showCloopPupils = False
            self.canAutoCenter = False
            self.doFindStar = False
            self.showCrux = False
            self.showCuts = True
            self.logScale = True
            self.acceptableLiveErrors = 10
            self.varname_darkvar = ''
            self.invertxy = True

        elif ccd.lower()=='irtc':
            self.setCaption("IRTC Viewer")
            self.bufname = "IrtcProxy00:FRAME"
            self.ccd_type = 200
            self.minscale = -5
            self.maxscale = +5
            self.ccdW = 320
            self.ccdH = 256
            self.xbin = 1
            self.ybin = 1
            self.canChangeBinning = False
            self.showSlopes = False
            self.showRms = False
            self.showRealtimePupils = False
            self.showCloopPupils = False
            self.canAutoCenter = False
            self.doFindStar = True
            self.showCrux = False
            self.showCuts = True
            self.logScale = True
            self.acceptableLiveErrors = 10
            self.varname_darkvar = cfg.ccd_darkvariable['irtc']+'.REQ'
            self.invertxy = True

        else:
            print "FATAL: unknown CCD type!!"
            sys.exit(0)

        if self.xbin == 0: self.xbin =1
        if self.ybin == 0: self.ybin =1
        self.nx = self.ccdW / self.xbin
        self.ny = self.ccdH / self.ybin

        print 'Using buffer: '+self.bufname

        self.pixeldata = {}
        self.pixeldata['counter'] = 0
        self.pixeldata['data'] = ''

        self.avg_background=0
        self.sloperms=0
        self.framerate=1

        maxsize = qt.QSize(500,20)

        # Crea lo slider di ingrandimento
        self.slider = qt.QSlider( self.minscale, self.maxscale, 2, 1, qt.QSlider.Horizontal, self, "Zoom")
        self.gridLayout.addWidget( self.slider, 1, 0)
        self.slider.setTickInterval(2)
        self.slider.setTickmarks( qt.QSlider.Above)
        self.slider.setMaximumSize(maxsize)
        self.connect( self.slider, qt.SIGNAL("valueChanged(int)"), self.slotSlider)

        self.sliderL = qt.QLabel( "Zoom", self)
        self.sliderL.setMaximumSize(maxsize)
        self.gridLayout.addWidget( self.sliderL, 0, 0)

        # Crea lo slider di bit-window
        self.bitwindow_slider = qt.QSlider( 1, 400, 1, 10, qt.QSlider.Horizontal, self, "Amplifier")
        self.gridLayout.addWidget( self.bitwindow_slider, 1, 1)
        self.bitwindow_slider.setTickInterval(10)
        self.bitwindow_slider.setTickmarks( qt.QSlider.Above)
        self.bitwindow_slider.setMaximumSize(maxsize)
        self.connect( self.bitwindow_slider, qt.SIGNAL("valueChanged(int)"), self.slotBitWindowSlider)

        self.sliderbL = qt.QLabel( "Amplifier", self);
        self.sliderbL.setMaximumSize(maxsize)
        self.gridLayout.addWidget( self.sliderbL, 0, 1)

        # Crea il bottone di start/stop 
        self.liveLabel = qt.QLabel( "Live", self)
        self.gridLayout.addWidget( self.liveLabel, 2, 3)

        # Show/Hide selected pupils and background subtraction
        if self.showRealtimePupils:
            self.showrtpupils = qt.QCheckBox("Show realtime pupils", self)
            self.gridLayout.addWidget( self.showrtpupils, 6, 3)

        if self.showCloopPupils:
            self.showpupils = qt.QCheckBox("Show cloop pupils", self)
            self.gridLayout.addWidget( self.showpupils, 7, 3)

        # Slopes display
        if self.showSlopes:
            self.check_showslopes = qt.QCheckBox("Show slopes", self)
            self.gridLayout.addWidget( self.check_showslopes, 8, 3)

        # Log scale
        if self.logScale:
            self.checkLogScale = qt.QCheckBox("Log scale", self)
            self.gridLayout.addWidget( self.checkLogScale, 9, 3)

        self.subtractbackground = qt.QCheckBox("Subtract background", self)
        self.subtractbackground.setChecked(1)
        self.gridLayout.addWidget( self.subtractbackground, 10, 3)

        # Crea il "bottone" che fa da visore e riempilo
        self.viewer = ccd_window( self)
        self.gridLayout.addMultiCellWidget( self.viewer, 2, 15, 0, 1)

        # Pupil threshold
        if self.showRealtimePupils:
            self.pup_diff = qt.QCheckBox("Show position difference", self)
            self.gridLayout.addWidget( self.pup_diff, 14, 3)
            self.pup_diff.setChecked(0)

            self.pupdata0 = ""
            self.pupdata1 = ""
            self.pupdata2 = ""
            self.pupdata3 = ""

        if self.showRealtimePupils:
            self.pupdata = qt.QLabel(self, "Pup data")
            self.gridLayout.addWidget( self.pupdata, 15, 3)

        self._stageAutocenterVarname = cfg.clstat('autocenter_stages')
        self._stageAutofocusVarname  = cfg.clstat('autofocus_stages')

        if self.canAutoCenter:
            self.autocenterButton = qt.QPushButton( "Autocenter", self)
            self.gridLayout.addWidget( self.autocenterButton, 21, 0)
            self.connect( self.autocenterButton, qt.SIGNAL("clicked()"), self.slotAutocenter)

            self.autocenterStatus = rtdbStatusLight( self, "autocenter", xsize=100)
            self.gridLayout.addWidget( self.autocenterStatus, 21, 1)
            self.autocenterStatus.setup(" ", self, None, ontext="Running", offtext="Idle", oncolor= cfg_gui.colors.green, offcolor=QColor(192,192,192))

            self.stopAutocenterButton = qt.QPushButton( "Stop autocenter/focus", self)
            self.gridLayout.addWidget( self.stopAutocenterButton, 21, 3)
            self.connect( self.stopAutocenterButton, qt.SIGNAL("clicked()"), self.slotStopAutocenter)
 
            self.shiftPupilsButton = qt.QPushButton( "Shift pupils", self)
            self.gridLayout.addWidget( self.shiftPupilsButton, 22, 3)
            self.connect( self.shiftPupilsButton, qt.SIGNAL("clicked()"), self.slotShiftPupils)
 
            self.autofocusButton = qt.QPushButton( "Autofocus", self)
            self.gridLayout.addWidget( self.autofocusButton, 22, 0)
            self.connect( self.autofocusButton, qt.SIGNAL("clicked()"), self.slotAutofocus)

            self.autofocusStatus = rtdbStatusLight( self, "autofocus", xsize=100)
            self.autofocusStatus.setup( " ", self, None, ontext="Running", offtext="Idle", oncolor = cfg_gui.colors.green, offcolor=QColor(192,192,192))
            self.gridLayout.addWidget( self.autofocusStatus, 22, 1)

            self.VarNotif( self._stageAutocenterVarname, self.slotNotify, retry = True)
            self.VarNotif( self._stageAutofocusVarname,  self.slotNotify, retry = True)


        if self.doFindStar:
            self.dofindstars = qt.QPushButton( "Find stars", self)
            self.gridLayout.addWidget( self.dofindstars, 15, 3)
            self.connect( self.dofindstars, qt.SIGNAL("clicked()"), self.slotFindStars)
            # Load ccd object
            if self.ccd_type == 47:
                from AdOpt.hwctrl import ccd47
                self.ccd = ccd47.ccd47(self)
            if self.ccd_type == 100:
                from AdOpt.hwctrl import thorlabs_ccd
                self.ccd = thorlabs_ccd.thorlabs_ccd(self,1)

        # Cut plots
        if self.showCuts:
            self.plotX = plotWidget( self, 0)
            self.plotX.setMinimumHeight(50)
            self.gridLayout.addMultiCellWidget( self.plotX, 16, 16, 0, 1)

            self.plotY = plotWidget( self, 1)
            self.plotY.setMinimumWidth(50)
            self.gridLayout.addMultiCellWidget( self.plotY, 2, 15, 2, 2)

            pixelview.initplot( self.plotX.winId(), self.plotY.winId())

        if self.showRms:
            self.plotX = plotWidget( self, 0)
            self.plotX.setMinimumHeight(50)
            self.gridLayout.addMultiCellWidget( self.plotX, 16, 16, 0, 1)

            pixelview.initplot( self.plotX.winId())


        # Label under the ccd display with cursor coordinates and pixel values
        self.pixelvalueLabel = qt.QLabel(self)
        self.gridLayout.addMultiCellWidget( self.pixelvalueLabel, 17, 17, 0, 1)

        # Label with shm buffer name
        self.slopermsLabel = qt.QLabel(self)
        self.gridLayout.addMultiCellWidget( self.slopermsLabel, 18, 18, 0, 1)

        # Label with pupil intensity
        self.pupIntensityLabel = qt.QLabel(self)
        self.gridLayout.addMultiCellWidget( self.pupIntensityLabel, 19, 19, 0, 1)

        # Frame acquisition button
        self.frameacq = qt.QPushButton("Save frames", self)
        self.gridLayout.addWidget( self.frameacq, 17, 3)
        self.connect( self.frameacq, qt.SIGNAL("clicked()"), self.slotFrameAcq)

        # Various status indicators
        if self.showStagesStatus:
            self.statusind = qt.QLabel("", self)
            self.gridLayout.addWidget( self.statusind, 18, 3)
        if self.showPixelLut:
            self.pixellut = qt.QLabel("Pupils: None", self)
            self.gridLayout.addWidget( self.pixellut, 19, 3)

        # Crux labels

        if self.showCrux:
            self.cruxTitleX = qt.QLabel("Crux X", self)
            self.gridLayout.addWidget( self.cruxTitleX, 20, 3)
            self.cruxX = qt.QLineEdit("", self)
            self.gridLayout.addWidget( self.cruxX, 21, 3)
            self.connect( self.cruxX, qt.SIGNAL("textChanged(const QString &)"), self.slotCrux)

            self.cruxTitleY = qt.QLabel("Crux Y", self)
            self.gridLayout.addWidget( self.cruxTitleY, 22, 3)
            self.cruxY = qt.QLineEdit("", self)
            self.gridLayout.addWidget( self.cruxY, 23, 3)
            self.connect( self.cruxY, qt.SIGNAL("textChanged(const QString &)"), self.slotCrux)

        self.diam = [0.0, 0.0, 0.0, 0.0]
        self.cx = [0.0, 0.0, 0.0, 0.0]
        self.cy = [0.0, 0.0, 0.0, 0.0]
        self.diffcx = [0.0, 0.0, 0.0, 0.0]
        self.diffcy = [0.0, 0.0, 0.0, 0.0]

        #print "Dim: "+str(self.viewer.size().width())+","+str(self.viewer.size().height())
        self.dx = self.viewer.size().width() / self.nx
        self.dy = self.viewer.size().height() / self.ny
        if self.dx <1:
            self.dx =1
        if self.dy <1:
            self.dy =1

        self.pixelvalue=""

        # Setup del viewer C++

        pixelview.init( self.ccd_type)

        # Setup running mean intensity
        self.intensity_buf = numpy.zeros((100), dtype=numpy.float32)
        self.intensity_count=0
        self.sloperms_buf = numpy.zeros((100), dtype=numpy.float32)
        self.sloperms_count=0

        self.varname_frmrt = cfg.varname( self.ccdName, 'FRMRT.CUR')
        self.varname_pup0 =  cfg.varname( 'pupilcheck39', 'PUP0')
        self.varname_pup1 =  cfg.varname( 'pupilcheck39', 'PUP1')
        self.varname_pup2 =  cfg.varname( 'pupilcheck39', 'PUP2')
        self.varname_pup3 =  cfg.varname( 'pupilcheck39', 'PUP3')
        self.varname_tv_npixels = None    # Initialized only for CCD 47


        if self.canChangeBinning:
            self.VarNotif( self.varname_curxbin, self.slotNotify)
            self.VarNotif( self.varname_curybin, self.slotNotify)
            self.VarNotif( self.varname_dx, self.slotNotify)
            self.VarNotif( self.varname_dy, self.slotNotify)
            self.VarNotif( self.varname_frmrt, self.slotNotify)

            self.slotNotify( self.ReadVar(self.varname_dx))
            self.slotNotify( self.ReadVar(self.varname_dy))
            self.slotNotify( self.ReadVar(self.varname_curxbin))
            self.slotNotify( self.ReadVar(self.varname_curybin))
            self.slotNotify( self.ReadVar(self.varname_frmrt))

        try:
            self.VarNotif( self.varname_darkvar, self.slotNotify)
            self.slotNotify( self.ReadVar(self.varname_darkvar))

            if ccd.lower() == 'ccd39':
                self.slotSlopenull( self.ReadVar(self.varname_slopenull))
        except:
            pass

        if self.showRealtimePupils:
            self.VarNotif( self.varname_pup0, self.slotNotify, retry = True)
            self.VarNotif( self.varname_pup1, self.slotNotify, retry = True)
            self.VarNotif( self.varname_pup2, self.slotNotify, retry = True)
            self.VarNotif( self.varname_pup3, self.slotNotify, retry = True)

            try:
                self.slotNotify( self.ReadVar(self.varname_pup0))
                self.slotNotify( self.ReadVar(self.varname_pup1))
                self.slotNotify( self.ReadVar(self.varname_pup2))
                self.slotNotify( self.ReadVar(self.varname_pup3))
            except:
                pass

        if ccd=='ccd47':
            self.varname_tv_npixels =  cfg.varname( cfg.techviewer_frames_producer, "TV_NPIXELS")
            self.VarNotif( self.varname_tv_npixels, self.slotNotify)
            print "Technical Viewer: requested notify of RTDBVar %s" % self.varname_tv_npixels
            self.varreplyHandler( self.ReadVar( self.varname_tv_npixels))

        self.gain0 = 0
        if ccd=='ccd39':
            self.varname_gain = 'ADSEC.L.G_GAIN_A@M_ADSEC'
#            self.VarNotif( self.varname_gain, self.slotGain)
#            self.slotGain( self.ReadVar( self.varname_gain)) # AP removed 20161103

        # Set default values
        if self.showStagesStatus:
            self.stages={}
            self.varname_baysidex = cfg.varname('baysidex', 'STATUS')
            self.varname_baysidey = cfg.varname('baysidey', 'STATUS')
            self.varname_baysidez = cfg.varname('baysidez', 'STATUS')
            self.VarNotif( self.varname_baysidex, self.slotStages)
            self.VarNotif( self.varname_baysidey, self.slotStages)
            self.VarNotif( self.varname_baysidez, self.slotStages)

            self.slotStages( self.ReadVar(self.varname_baysidex))
            self.slotStages( self.ReadVar(self.varname_baysidey))
            self.slotStages( self.ReadVar(self.varname_baysidez))

        if self.showPixelLut:
            self.slotPixelLut( self.ReadVar(self.varname_pixellut))

        if hasattr(self, 'varname_status'):
            self.slotStatus( self.ReadVar(self.varname_status))

        if self.showCloopPupils:
            self.pupilshift = pupilshift.PupilShiftGui(self)
            self.pupilshift.hide()

        # Start worker thread
        thread.start_new_thread( self.worker_thread, ())

        # Show the window
        self.show()

        # Crea il timer per la visualizzazione
        self.timer = qt.QTimer( self)
        self.connect( self.timer, qt.SIGNAL("timeout()"), self.slotTimer)

        # Start live view
        self.timer.start( self.refreshInterval)

        self.slotSlider(1)
        self.slotBitWindowSlider(10)

    def readFw1(self):
        try:
            self.fw1pos = int(round(self.ReadVar(self.varname_fw1pos).Value()))
        except:
            self.fw1pos=0
        if self.fw1pos<0:
            self.fw1pos=0

    def readCcdGain(self):
        try:
            self.ccdgain = int(round(self.ReadVar(self.varname_ccdgain).Value()))
        except:
            self.ccdgain=0
        if self.ccdgain<0:
            self.ccdgain=0

    def setHotSpot(self):
        self.lock.acquire()
        try:
            calib_filename = calib.sourceAcqCalibFile(round(self.fw1pos))
            data = calib.loadCalibData( calib_filename, check=['xrefpos', 'yrefpos'])
            pixelview.setHotSpot(int(1023-data['yrefpos']),int(data['xrefpos']))
            print 'Setting hot spot:', data['xrefpos'], data['yrefpos']
        except Exception, e:
            print 'Exception setting hot spot: ',e
        self.lock.release()

    def slotPupSplit(self):
        self.writeTH()

    def slotAutocenter(self):
        processControl.startProcessByName('autocenter_stages')

    def slotStopAutocenter(self):
        processControl.stopProcessByName('autocenter_stages')
        processControl.stopProcessByName('autofocus_stages')

    def slotAutofocus(self):
        processControl.startProcessByName('autofocus_stages')

    def slotShiftPupils(self):
        if self.pupilshift.isVisible():
            self.pupilshift.hide()
        else:
            self.pupilshift.show()

    def slotZoomOut(self):
        rect = QRect( self.viewer.contentsX(), self.viewer.contentsY(), self.viewer.visibleWidth(), self.viewer.visibleHeight())
        incx = rect.width()/2
        incy = rect.height()/2
        rect.addCoords( -incx, -incy, incx, incy)
        self.viewer.frame.zoomRect = rect
        self.viewer.frame.zoomToRect()

    def slotFindStars(self):
        from AdOpt import frames_lib
        frame = self.ccd.get_frames(3, average=True, subtractDark=True)
        s = frames_lib.findStar(frame, completeInfo=True)
        # Fix display coordinates
        desc = ''
        for star in s:
            desc += str(star)+"\n"
        if len(s) == 0:
            desc = 'No stars found'
        self.pupIntensityLabel.setText(desc)

    def slotCrux(self, str):
        strx = self.cruxX.text().latin1()
        stry = self.cruxY.text().latin1()
        try:
            x = float(strx)
        except ValueError:
            x=0
        try:
            y = float(stry)
        except ValueError:
            y=0

        print 'Setting crux to',x,y
       
        pixelview.setCrux( x, y)
        

    def computeStatusInd(self):
	s = 'Stages: '
	if (self.stagesOn):
	   s += '<font color=red><b>ON</b></font>'
	else:
           s += '<font color=lightgra">Off</font>'
	return s

    def readTH(self):
        return calib.getPupTH( ccd = self.ccdName)

    def pupSplit(self):
        try:
            split = cfg.cfg['pupilcheck39']['pupSplit'].Value()
        except:
            split=1
        return split

    def setPixelValue( self):
        self.lock.acquire()
        self.pixelvalueLabel.setText( self.pixelvalue)
        self.lock.release()

    def setSlopeRms( self, erase=False):
        if erase:
            self.slopermsLabel.setText("")
        else:

            buflen = self.sloperms_buf.shape[0]
            self.sloperms_buf[ self.sloperms_count % buflen ] =  pixelview.getSlopeRms()
            ii = self.sloperms_buf.sum() / min(buflen, self.sloperms_count)
            self.sloperms_count +=1

            self.slopermsLabel.setText( "Slope rms: %4.3e" %ii)

    def slotPixelLut( self, var):
        v = var.Value()[-22:-7]
        if v == '':
            v = 'None'
        self.curPupils = v
        self.callAfter( self.pixelLutHandler, var)

    def slotStages(self, var):
        self.stages[var.Name()] = var.Value()
        self.stagesOn=False
        for v in self.stages.keys():
            if self.stages[v] >5:
                self.stagesOn = True
                break
	ind = self.computeStatusInd()
	self.callAfter( self.statusind.setText, ind)

    def slotGain(self, var):
        filename = var.Value()
        if filename[-10:] == 'gain0.fits':
            self.gain0=1 
        else:
            self.gain0=0

    def slotSlopenull( self, var):
        path = calib.slopenullDir( self.xbin) + '/' + var.Value()
        pixelview.setSlopesnull(path)
        self.curSlopenull = var.Value()
        self.callAfter( self.refreshPupilsSlopenull, ())

    def slotStatus( self, var):
        self.ccdStatus = var.Value()
        self.callAfter( self.setLiveFlag, ())
        
    def slotNotify( self, var):
        self.callAfter( self.varreplyHandler, var)

    def formatPupil( self, diam, cx, cy, lato, diffcx, diffcy):
        if self.pup_diff.isChecked() != 0:
            _cx = diffcx
            _cy = diffcy
            _nx = 0
            diff=True
        else:
            _cx = cx
            _cy = cy
            _nx = self.nx
            diff = False

        if not self.invertxy:
            _tempx = _cx
            _cx = _cy
            _cy = _nx - _tempx
            if not diff:
                _cy -= 1

        return "Diam: %5.2f CX: %5.2f  CY: %5.2f Side:%5.2f" % ( diam, _cx, _cy, lato)

    def pixelLutHandler( self, var):
        self.setupViewer()        
        self.refreshPupilsSlopenull()

    def refreshPupilsSlopenull(self):
        self.pixellut.setText('Pupils: %s\nSlopenulls: %s' % (self.curPupils, self.curSlopenull))

    def varreplyHandler( self, var):

        varname = var.Name()
        vtype  = var.Type()
        owner  = var.Owner() or 'global'
        value  = var.Value()

        recalc = False

        if varname == self._stageAutocenterVarname:
            self.autocenterStatus.update(value)
            return
        if varname == self._stageAutofocusVarname:
            self.autofocusStatus.update(value)
            return


        # --------- Binning and/or ccd size change

        if varname == self.varname_curxbin:
            self.xbin = int(value)
            recalc = True

        if varname == self.varname_curybin:
            self.ybin = int(value)
            recalc = True

        if varname == self.varname_dx:
            self.ccdW = int(value)
            recalc = True

        if varname == self.varname_dy:
            self.ccdH = int(value)
            recalc = True

        if recalc:
            if self.xbin == 0 or self.ybin == 0:
                return

            temp_nx = int( self.ccdW / self.xbin )
            temp_ny = int( self.ccdH / self.ybin )
            if temp_nx != self.nx or temp_ny != self.ny:

                print 'New binning: ',self.xbin, self.ybin, temp_nx, temp_ny

                self.nx = temp_nx
                self.ny = temp_ny
                self.setupViewer()

            return


        if varname == self.varname_frmrt:
            self.framerate = int(value)
            return

            # New dark to be subtracted from display
        if varname == self.varname_darkvar and value != "":
        
            path = calib.BackgDir( self.ccdName, self.xbin) + '/' + value
            print "Setting background "+path
            pixelview.setDark(path)
   
   
        # Pupil data
        if varname == self.varname_pup0:
            self.diam[0], self.cx[0], self.cy[0], self.lato0, self.diffcx[0], self.diffcy[0] = value
            self.pupdata0 = self.formatPupil( self.diam[0], self.cx[0], self.cy[0], self.lato0, self.diffcx[0], self.diffcy[0])

            if self.pupSplit() == 0:
                self.lock.acquire()
                self.pupdata.setText(self.pupdata0)
                self.lock.release()

        if varname == self.varname_pup1:
            self.diam[1], self.cx[1], self.cy[1], self.lato1, self.diffcx[1], self.diffcy[1] = value
            self.pupdata1 = self.formatPupil( self.diam[1], self.cx[1], self.cy[1], self.lato1, self.diffcx[1], self.diffcy[1])

        if varname == self.varname_pup2:  
            self.diam[2], self.cx[2], self.cy[2], self.lato2, self.diffcx[2], self.diffcy[2] = value
            self.pupdata2 = self.formatPupil( self.diam[2], self.cx[2], self.cy[2], self.lato2, self.diffcx[2], self.diffcy[2])

        if varname == self.varname_pup3: 
            self.diam[3], self.cx[3], self.cy[3], self.lato3, self.diffcx[3], self.diffcy[3] = value
            self.pupdata3 = self.formatPupil( self.diam[3], self.cx[3], self.cy[3], self.lato3, self.diffcx[3], self.diffcy[3])

            medio = "Averages: diameter %5.2f, side %5.2f" % ( (self.diam[0]+self.diam[1]+self.diam[2]+self.diam[3])/4.0, (self.lato0+self.lato1+self.lato2+self.lato3)/4.0)
            if self.pup_diff.isChecked():
                avg_diffx = (self.diffcx[0]+self.diffcx[1]+self.diffcx[2]+self.diffcx[3])/4.0
                avg_diffy = (self.diffcy[0]+self.diffcy[1]+self.diffcy[2]+self.diffcy[3])/4.0

                if not self.invertxy:
                    tmp = avg_diffx
                    avg_diffx = avg_diffy
                    avg_diffy = tmp
                    
                medio += "\nDiff x: %5.2f Diff y: %5.2f" % (avg_diffx, avg_diffy)

            self.lock.acquire()
            self.pupdata.setText(self.pupdata0 + "\n" + self.pupdata1 + "\n" +
                                 self.pupdata2 + "\n" + self.pupdata3 + "\n" + medio)
            self.lock.release()
                
        # CCD47 Frame size changed
        if varname == self.varname_tv_npixels:
            print "----------------------------"
            print "Changing frame size to %d px..." %value
            self.bufname = cfg.ccd_framebuffer[ self.ccd_type] + str(value)
            print 'Using buffer: '+self.bufname
            # Setup del viewer C++
            self.pixelsReceived=False

            #self.lock.acquire()
            #pixelview.init( self.ccd_type)
            #self.lock.release()

            self.setupViewer()
            pixelview.setBitWindow(self.bitwindow_slider.value())



    def slotTimer(self):

        # Ensure viewport changes are tracked

        self.lock.acquire()
        pixelview.setViewport( self.viewer.contentsX(), self.viewer.contentsY(), self.viewer.visibleWidth(), self.viewer.visibleHeight())
        self.lock.release()

        self.redrawPixels()

        # Get info about the pixel under the mouse pointer

    def setupViewer(self):
        self.lock.acquire()
        self.viewer.resize( self.nx * self.dx, self.ny * self.dy)

        pixelview.setup( self.nx, self.ny, self.dx, self.dy, self.xbin, \
                         self.viewer.contentsX(), self.viewer.contentsY(), self.viewer.visibleWidth(), self.viewer.visibleHeight())
        self.viewer.frame.setMouseTracking(1)

        self.lock.release()

    # Evento per lo slider: cambia le dimensioni del viewer e della pixmap
    def slotSlider(self, value):
        # Zero or negative means fractionary values (1/2, 1/3, etc.)
        if value <1:
            value = -1.0/(value-2)

        self.dx = value
        self.dy = value

        self.setupViewer()
        self.redrawPixels()

    # Evento per lo slider di bit window: cambia la bitwindow
    def slotBitWindowSlider(self, value):
        self.bitwindow = value
        pixelview.setBitWindow( value)
        self.redrawPixels()

    # Ridisegna i pixels, ma senza chiederne di nuovi al CCD 
    def redrawPixels( self, arg=None):

        if self.pixelsReceived:


            if self.showCloopPupils:
                showPupils = self.showpupils.isChecked()
            else:
                showPupils = False

            if self.showRealtimePupils:
                showRtPupils = self.showrtpupils.isChecked()
            else:
                showRtPupils = False

            self.lock.acquire()
            pixelview.setViewport( self.viewer.contentsX(), self.viewer.contentsY(), self.viewer.visibleWidth(), self.viewer.visibleHeight())
            useLogScale = 0
            if self.logScale:
                useLogScale = self.checkLogScale.isChecked()
            pixelview.drawPixels( showPupils, showRtPupils, self.pupSplit(), self.diam, self.cx, self.cy, useLogScale, self.gain0)

            pixelview.blitImage( self.viewer.frame.winId())

            if self.ccd_type == 39:
                pupIntensity = pixelview.getPupilIntensity()
                buflen = self.intensity_buf.shape[0]
                if self.pupSplit():
                    self.intensity_buf[ self.intensity_count % buflen ] = (pupIntensity[0]+pupIntensity[1]+pupIntensity[2]+pupIntensity[3])/2.0
                    intensityStr = 'Average pupil intensity (counts): %d - %d - %d - %d' % (pupIntensity[0], pupIntensity[1], pupIntensity[2], pupIntensity[3])
                else:
                    intensityStr = 'Average pupil intensity (counts): %d' % pupIntensity[0]
                    self.intensity_buf[ self.intensity_count % buflen] = pupIntensity[0]*2.0

                ii = self.intensity_buf.sum() / min(buflen, self.intensity_count)
                self.intensity_count += 1

                intensityStr += '\nTotal incoming light: %5.1f photons/subap' % ii

                # Calc magnitude
                # This code should be replaced with WfsArbScripts.ccd39Mag(), which must be factored out.
                nsubap = pixelview.getPupilIntensityCount()[0]
                photBin1=0
                if self.xbin>0:
                    photBin1 = ii * self.framerate / (self.xbin * self.xbin)

		# Correct for filterwheel #1 position
		photBin1 *= self.acqData['fw1_%d'% self.fw1pos]

		before = photBin1
		# Correct for filterwheel #1 position
		photBin1 *= self.acqData['gain_%d'% self.ccdgain]

                if (ii>5) and (photBin1 >0) and (photBin1 < 1e10):
                    mag = '%5.1f' % (2.5*math.log10( float(self.acqData['refMagFlux']) / photBin1)+ self.acqData['refMag'])
                else:
                    mag = 'n/a'

                # Do not show magnitude, since it is often wrong!
                intensityStr += '\nEstimated magnitude = ' + mag

                self.pupIntensityLabel.setText( intensityStr)

                self.setSlopeRms()

            self.lock.release()

        else:
            print 'redrawPixels(): pixels not ready'

    # Updates the Live (green) / Not live (red) value.
    # If the ccd is not integrating frames, the current ccd status is shown in gray

    def setLiveFlag(self, flag=None):

        self.lock.acquire()

	# If called without a new flag value, use the last one
        if flag==None:
            flag = self.lastLiveFlag
        self.lastLiveFlag = flag

        if self.ccdStatus != AOStatesCode["STATE_OPERATING"]:
            self.liveLabel.setEraseColor( cfg_gui.colors.gray)
            self.liveLabel.setText( AOStatesType[self.ccdStatus])
        elif flag:
            self.liveLabel.setEraseColor( cfg_gui.colors.green)
            self.liveLabel.setText("Live")
        else:
            self.liveLabel.setEraseColor( cfg_gui.colors.red)
            self.liveLabel.setText("Not live")
        self.lock.release()

    def slotFrameAcq(self):
        if self.ccd_type == 39:
            processControl.startProcessByName('ccd39acq', multi=1)
        elif self.ccd_type == 47:
            processControl.startProcessByName('ccd47acq', multi=1)
        elif self.ccd_type == 100:
            if self.ccdName == 'thorlabs1':
                processControl.startProcessByName('thorlabs1acq', multi=1)
            if self.ccdName == 'thorlabs2':
                processControl.startProcessByName('thorlabs2acq', multi=1)
        elif self.ccd_type == 200:
            processControl.startProcessByName('irtcacq', multi=1)

        

    # Worker thread:
    # - receives pixels from shared buffer
    # - prepares pixel buffer for display

    def worker_thread(self):

        liveFlag = False
        liveErrors = 0
        count=0

        while not self.timetodie:

            try:
                self.pixeldata = self.BufRead( self.bufname)

            except AdOptError, e:
                print e
                liveErrors +=1
                if liveErrors > self.acceptableLiveErrors:
                    liveFlag = False
                    self.callAfter( self.setLiveFlag, liveFlag)
                time.sleep(0.1)
                continue
            liveErrors = 0

            if self.showSlopes:
                showSlopes = self.check_showslopes.isChecked()
            else:
                showSlopes = False

	    self.pixelsReceived = False
            checkWhiteFrame=0
            if self.ccdName == 'ccd47':
                checkWhiteFrame=0

            if pixelview.preparePixels( self.pixeldata['data'], self.subtractbackground.isChecked(), showSlopes, checkWhiteFrame) != 0:
                print 'White frame rejected'
                time.sleep(0.01)
                continue

            self.pixelsReceived=True
            #print self.ccdName
            if self.ccdName == 'ccd39':
                self.callAfter( self.plotX.drawContents, (0))

            self.callAfter( self.viewer.frame.setMouseValue, ())


            if liveFlag == False:
                liveFlag = True
                self.callAfter( self.setLiveFlag, liveFlag)

            count+=1
            if count%5 ==0:
                self.callAfter( self.readFw1, ())
                self.callAfter( self.readCcdGain, ())
                if self.ccdName == 'ccd47':
                    self.callAfter( self.setHotSpot, ())
            time.sleep(0.1)



# Start application

qApp = QApplication( sys.argv)
main = ccd_viewer(qApp)
main.setMinimumWidth(500)
qApp.setMainWidget(main)
main.show()

qApp.exec_loop()
                                
