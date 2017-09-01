
import os, time, stat
from AdOpt import calib
from AdOpt.QtDesigner.restoreSetup_template import *

class restoreSetup(restoreSetup_template):

    def __init__(self, parent):
        restoreSetup_template.__init__(self, parent)

        self.name = 'Board setup'
        self.opticalSetupCheck.setChecked(0)
        self.refreshList()


    def saveButton_clicked(self):
       filename = self.editFilename.text().latin1().replace(' ', '_').replace('"','_')
       if filename != '':
            command = os.environ['ADOPT_ROOT']+"/bin/runBoardSetup.py -save \"%s\"" % (calib.setupsDir() + filename)
            os.system(command)
            self.refreshList()

            QMessageBox.information(self, "Configuration saved", "Current configuration saved as %s" % filename, QMessageBox.Ok)

    def filename2display(self, filename):
	'''
	Converts a filename into a display string.
	'''
	mtime = os.stat( calib.setupsDir()+filename)[stat.ST_MTIME]
	timestr = time.strftime('%d %b %y', time.gmtime(mtime))
	return '%s - %s' % (timestr, filename)
	

    def display2filename(self, display):
	'''
	Converts a display string into a filename.
	'''
	return display[12:]

    def refreshList(self):
        self.setupList.clear()
        prefix = calib.setupsDir()
        lst = os.listdir( prefix)
        files = {}
        for name in lst:
            if name != 'CVS':   # Source management getting in the way...
                t = os.stat( prefix+name)[stat.ST_MTIME]
                while files.has_key(t):
                    t += 1
                files[t] = name
        kk = files.keys()
        kk.sort()    # This is required because some Pyhton versions lack sorted()
        for date in kk:
            self.setupList.insertItem( self.filename2display( files[date]))

    def restoreButton_clicked(self):
        filename = self.display2filename( self.setupList.currentText().latin1())
        if filename != QString.null:

            if self.opticalSetupCheck.isChecked():
                command = os.environ['ADOPT_ROOT']+'/bin/thaoshell.py -name loadSetup -e \\"towerOpticalSetup(app, setup=\'%s\')\\"' % (calib.setupsDir()+filename)
            else:
                command = os.environ['ADOPT_ROOT']+'/bin/runBoardSetup.py -load "%s"' % (calib.setupsDir()+filename)
            command = "xterm -T Restore -e \""+command+"\""
            print command
            os.system(command+' &')

            #self.labelLastSetup.setText(filename)

            #QMessageBox.information(self, "Configuration restored", "Configuration %s restored" % filename, QMessageBox.Ok)

    def setupList_highlighted(self,a0):
        filename = self.display2filename( self.setupList.currentText().latin1())
        if filename != QString.null:
            ff = calib.setupsDir()+filename
            contents = file(ff).readlines()
            self.textEdit1.setText(''.join(contents))



    def deleteButton_clicked(self):
        filename = self.display2filename( self.setupList.currentText().latin1())
        if filename != QString.null:

            answer = QMessageBox.question( self, "Confirm delete", "Really delete setup %s ?" % filename, QMessageBox.Yes, QMessageBox.No)

            if answer == QMessageBox.Yes:
                fullpath = calib.setupsDir()+filename
                os.unlink(fullpath)
                self.refreshList()



        

