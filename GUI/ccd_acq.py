#!/usr/bin/python

import sys, os

from qt import *

from AdOpt import cfg, fits_lib, frames_lib, calib, WfsArbScripts, setupDevices
from AdOpt.WfsGUI.thAOAppGUI import thAOAppGUI
from AdOpt.QtDesigner.ccd_acq_template import *
from AdOpt.AOExcept import *

class ccd_acq(ccd_acq_template, thAOAppGUI):

    ## aoGui is an AOGUI, the main gui connected to MsgD, wich wait and 
    ## dispatch rtdb variable changes.
    def __init__(self, qApp, ccdname):

        thAOAppGUI.__init__( self, qApp, 'ccd_acq')
        self.canDoSlopes = False

        ccd_acq_template.__init__(self)

        if ccdname == 'ccd39':
            from AdOpt.hwctrl import ccd
            self.ccd = ccd.ccd('ccd39', self)
            self.canDoSlopes = True
        elif ccdname == 'ccd47':
            from AdOpt.hwctrl import ccd47
            self.ccd = ccd47.ccd47(self)
        elif ccdname == 'thorlabs':
            from AdOpt.hwctrl import thorlabs_ccd
            self.ccd = thorlabs_ccd.thorlabs_ccd(self)
        elif ccdname == 'thorlabs1':
            from AdOpt.hwctrl import thorlabs_ccd
            self.ccd = thorlabs_ccd.thorlabs_ccd(self, 1)
        elif ccdname == 'thorlabs2':
            from AdOpt.hwctrl import thorlabs_ccd
            self.ccd = thorlabs_ccd.thorlabs_ccd(self, 2)
        elif ccdname == 'irtc':
            from AdOpt.hwctrl import irtc_ccd
            self.ccd = irtc_ccd.irtc_ccd(self)
            
        else:
            raise Exception('Invalid CCD type "%s"' % ccdname)

        self.mute()
        setupDevices.loadDevices(self)
        self.unmute()

        self.ccdname = ccdname

        self.setCaption( ccdname+' frame acquisition')
        self.labelTitle.setText( ccdname+' frame aquisition')

        self.radioAverage.setChecked(True)
        self.radioSequence.setChecked(False)
        self.radioBackground.setChecked(False)
        self.radioSlopes.setChecked(False)

        if not self.canDoSlopes:
            self.radioSlopes.setEnabled(False)

        # Try to change into the last used directory
        try:
            f = file('/tmp/.ccdacq'+self.ccdname)
            lastdir= f.read()
            f.close()
            os.chdir(lastdir)
        except:
            pass
            

    def BrowseButton_clicked(self):

        if self.radioBackground.isChecked():
             dir = calib.BackgDir( self.ccdname, self.ccd.xbin())
        else:
             dir = ''

        filename = QFileDialog.getSaveFileName(dir)
        if filename != QString.null:
            f = filename.latin1()
            if not f[-5:] == '.fits':
                f = f + '.fits'
            self.FileEdit.setText(f)

            # Save last used directory
            file('/tmp/.ccdacq'+self.ccdname, 'w').write( os.path.dirname(f))


    def StartButton_clicked(self):

        filename = self.FileEdit.text().latin1() 
        if filename[-5:] != ".fits":
                filename = filename + ".fits"

        # Force background directory 
        if self.radioBackground.isChecked():
             dir = calib.BackgDir( self.ccdname, self.ccd.xbin())
             try:
                filename = filename[filename.rindex('/')+1:]
             except ValueError:
                # Happens if only the file name was written in the edit box
                pass
             filename = dir +'/' + filename 

        if os.access(filename, os.F_OK):
            ret = QMessageBox.information(self, 'File exists', 'file %s already exists. Overwrite?' % filename, QMessageBox.Yes, QMessageBox.No)
            if ret == QMessageBox.No:
                return

            os.unlink(filename)

        self.n = int( self.FrameNumberEdit.text().latin1())

        try:
            if self.radioBackground.isChecked():
                type = 'background'
            else:
                type = 'pixels'
            if self.radioSlopes.isChecked():
                type = 'slopes'

            if self.radioAverage.isChecked():
                frames = self.ccd.get_frames( self.n, type = type, callback = self.progress_callback, average=True, useFloat=True)
                message = 'Average of %d frames saved' % self.n
            else:
                frames = self.ccd.get_frames( self.n, type = type, callback = self.progress_callback)
                message = 'Sequence of %d frames saved' % self.n

            hdr, errstr = WfsArbScripts.getSetupFits(self)
            #if errstr != '':
            #    QMessageBox.warning( self, 'Warning', errstr) 

            if self.radioBackground.isChecked():
                frames = frames_lib.averageFrames(frames)
                message = 'Background (average of %d frames) saved' % self.n

            hdr.update( {'DETECTOR': self.ccd.get_detector_string()} )

            fits_lib.writeSimpleFits( filename, frames, hdr)
            QMessageBox.information(self, 'OK', message, QMessageBox.Ok)

            if self.radioBackground.isChecked():
                answer = QMessageBox.question(self, '', 'Do you want to use the new background immediately?', QMessageBox.Yes, QMessageBox.No)
                if answer == QMessageBox.Yes:
                    v = self.ReadVar( cfg.ccd_darkvariable[self.ccdname]+'.REQ')
                    self.SetVar( v, os.path.basename(filename))

        except AdOptError, e:
            QMessageBox.critical(self, 'Error', str(e), QMessageBox.Ok)

        self.ProgressBar.setProgress(0)

    def progress_callback(self, app, i):
        completed = i*100/self.n
        self.ProgressBar.setProgress(completed)
   
    def radioAverage_clicked(self):
        self.radioAverage.setChecked(True) 
        self.radioSequence.setChecked(False) 
        self.radioBackground.setChecked(False) 
        self.radioSlopes.setChecked(False) 

    def radioSequence_clicked(self):
        self.radioAverage.setChecked(False) 
        self.radioSequence.setChecked(True) 
        self.radioBackground.setChecked(False) 
        self.radioSlopes.setChecked(False) 

    def radioBackground_clicked(self):
        self.radioAverage.setChecked(False) 
        self.radioSequence.setChecked(False) 
        self.radioBackground.setChecked(True) 
        self.radioSlopes.setChecked(False) 

    def radioSlopes_clicked(self):
        self.radioAverage.setChecked(False) 
        self.radioSequence.setChecked(False) 
        self.radioBackground.setChecked(False) 
        self.radioSlopes.setChecked(True) 


if __name__ == '__main__':
    qApp = QApplication( sys.argv)

    if '-i' in sys.argv:
        idx = sys.argv.index('-i')
        del sys.argv[idx]  # -i
        del sys.argv[idx]  # identity

    if len(sys.argv)<2:
        print 'Please specify ccd type'
        sys.exit(0)
    
    main = ccd_acq(qApp, sys.argv[1])
    qApp.setMainWidget(main)
    main.show()
   
    qApp.exec_loop()
        
        


