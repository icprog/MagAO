#!/usr/bin/env python

from AdOpt.QtDesigner.Pisces_gui import *
from AdOpt.autoFocus47 import *
import sys, thread, time, glob
from AdOpt.WfsGUI.thAOAppGUI import *
from AdOpt.hwctrl import pisces_ccd
from AdOpt import irc, cfg, fits_lib



class PiscesGui(Pisces_gui, thAOAppGUI):

    def __init__(self, qApp):
        thAOAppGUI.__init__(self, qApp, 'PiscesGui')
        Pisces_gui.__init__(self)

        self.p = pisces_ccd.pisces_ccd(self)
        self.lock = thread.allocate_lock()
        self.biasexptime = 0.8
	thread.start_new_thread( self.update_thread, ())

        self.angles = [0,45,90,135,180,270,315]
        self.rots = []
        prefix = os.getenv('ADOPT_ROOT')+'/images/'
        for angle in self.angles:
	    self.rots.append( QPixmap( prefix + 'rot%d.png' % angle))
        self.pixmap = QPixmap(prefix+'pisces.png')
        self.counter=0
        self.countDown=0
        self.timeStart = 0
        self.lastStatus=''
        self.exptime =0
        self.nframes =0
        self.aoSavingVar = cfg.varname('optloopdiag','SAVING')
        self.closeLoopVar = cfg.varname('slopecompctrl','FASTLINK.ENABLE.CUR')
        self.ccdFreqVar  = cfg.varname('ccd39', 'FRMRT.CUR')
        self.savePisces =False



    def buttonExpTime_clicked(self):

        exptime = float(self.editExpTime.text().latin1())
        exptime -= self.biasexptime
        if exptime<0: exptime=0
        self.lock.acquire()
        self.p.setexptime(exptime)
        self.lock.release()	

    def buttonFilters_clicked(self):
        filter = int( self.comboFilters.currentText()[0])
        self.lock.acquire()
        self.p.setfilter(filter)
        self.lock.release()	

    def buttonCubeFrames_clicked(self):
        nframes = int( self.editCubeFrames.text().latin1())
        self.lock.acquire()
        self.p.setnframes(nframes)
        self.lock.release()	

    def buttonExpose_clicked(self):
        answ = QMessageBox.warning( self, "Confirm", "Confirm exposure of %3.1fs x %d frames?" % (self.exptime+self.biasexptime, self.nframes) , QMessageBox.Yes, QMessageBox.No)
        if answ == QMessageBox.Yes:
            self.labelFilename.setText('')
            self.lock.acquire()
            path = self.p.image(pathOnly=True, block=False)
            self.lock.release()	
            self.labelFilename.setText(path)

    def updateStatus(self, s):
        ss = ''
        self.nframes = s['nframes']
        self.exptime = s['exptime']
        self.totaltime = s['totaltime']
        self.status = s['status']
        if self.nframes>1:
            ss = 's'
        self.labelExpTime.setText('<b>%5.1f s</b>'% (self.exptime + self.biasexptime))
        self.labelFilter.setText('<b>Filter %s</b>'% s['filter'])
        self.labelCubeFrames.setText('<b>%s frame%s</b>' % ( self.nframes, ss))
        self.labelStatus.setText('<H1><b>%s</b></H1>'% self.status)
        if (self.status == 'BUSY') and (self.lastStatus == 'IDLE'):
            self.countDown = self.totaltime + self.nframes
            self.timeStart = time.time()
	    thread.start_new_thread( self.save_thread, ())
        self.lastStatus = self.status
        if s['status'] == 'BUSY':
            self.labelCountdown.setText('Time remaining: %d sec' % (self.countDown - (time.time() - self.timeStart)))
        else:
            self.labelCountdown.setText('')

    def setImage(self, pixmap):
        self.callAfter( self.buttonPixmap.setPixmap, (pixmap,))

    def update_thread(self):

        while 1:
            try:
                self.lock.acquire()
                s = self.p.allstatus()
                self.lock.release()	
	        self.callAfter( self.updateStatus, (s,))
                for i in range(5):
                    self.counter +=1 
                    if s['status'] == 'IDLE':
                        self.setImage( self.pixmap)
                    else:
                        n = self.counter % len(self.rots)
                        self.setImage( self.rots[n])
                    if i != 4:
                        time.sleep(0.1)
            except: 
                 time.sleep(1)

    def save_thread(self):
        '''
        When a PISCES exposure starts, we save an AO dataset
        '''
        isAoCube = self.ReadVar( self.aoSavingVar).Value()
        if isAoCube:
            print 'Cube triggered by AO system, skipping'
            return
        ccdFreq = self.ReadVar( self.ccdFreqVar).Value()
        isCloseLoop = self.ReadVar( self.closeLoopVar).Value()
        if not isCloseLoop:
            print 'Loop is open'
            return
        if ccdFreq > 500:
            ccdFreq = ccdFreq/2    # Correct for masterdiagnostic decimation
        nframes = self.totaltime * ccdFreq * 0.8   # 20% safety factor
        if nframes > 4000:
            nframes = 4000
        
        irc.aodata( nframes)
        datadir = '/towerdata/adsec_data/'
        daydir = sorted(os.listdir(datadir))[-1]
        tracknumdir = sorted(os.listdir('%s/%s' % (datadir, daydir)))[-1]
        self.outdir = '%s/%s/%s' % (datadir, daydir, tracknumdir)
        self.nFramesToSave = self.nframes

        # Wait for exposure completion
        while self.status != 'IDLE':
            time.sleep(0.5)
       
        files = sorted(glob.glob('/mnt/newdata/pisces*'))[-self.nFramesToSave:]
        cube, hdr = self.p.read_frames(files, getHeader = True)
        hdr = fits_lib.cleanHdr(hdr)
        hdr.update( {'DETECTOR': 'PISCES'} )

        fits_lib.writeSimpleFits( os.path.join(self.outdir, 'pisces.fits'), cube, hdr)




if __name__ == '__main__':
    qApp = QApplication( sys.argv)

    main = PiscesGui(qApp)
    qApp.setMainWidget(main)
    main.show()

    qApp.exec_loop()


