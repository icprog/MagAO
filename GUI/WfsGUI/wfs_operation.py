#!/usr/bin/env python
#
#@File: wfs_operation.py
#
# GUI tool for common WFS operations
#
# This window has buttons for saving frames, slopes and commands to disk directly
# from the pyramid wavefront sensor
#@

from qt import *
import time
from AdOpt.wrappers import msglib

#import BCUfunctions

#@Class: wfs_operation
#
# Main window for the GUI tool.
#@

class wfs_operation( QDialog):
    
    def __init__(self, AOwindow):
        QDialog.__init__(self)

        self.name = "Frame acquisition"

        self.backdir = "../config/backgrounds"
        self.slopesdir = "../config/slopes"

        ## Draw the widgets

        self.gridLayout = QGridLayout( self, 3, 4, 20)
        self.gridLayout.setAutoAdd(1)

        self.label0 = QLabel("Acquire background", self)
        self.buttonBackg = QPushButton("Save", self)
        self.text0 = QLabel("Frames to average:", self)
        self.backgNum = QLineEdit(self)
        self.backgNum.setText("10")
        self.connect( self.buttonBackg, SIGNAL("clicked()"), self.slotButtonBackg)

        self.label1 = QLabel("Acquire frame", self)
        self.buttonFrame = QPushButton("Save", self)
        self.text1 = QLabel("Frames to average:", self)
        self.framesNum = QLineEdit(self)
        self.framesNum.setText("10")
        self.connect( self.buttonFrame, SIGNAL("clicked()"), self.slotButtonFrame)

        self.label2 = QLabel("Acquire slopes", self)
        self.buttonSlopes = QPushButton("Save", self)
        self.text2 = QLabel("Frames to average:", self)
        self.slopesNum = QLineEdit(self)
        self.slopesNum.setText("10")
        self.connect( self.buttonSlopes, SIGNAL("clicked()"), self.slotButtonSlopes)

        self.label3 = QLabel("Acquire commands", self)
        self.buttonCommands = QPushButton("Save", self)
        self.text2 = QLabel("Frames to average:", self)
        self.commandsNum = QLineEdit(self)
        self.commandsNum.setText("10")
        self.connect( self.buttonCommands, SIGNAL("clicked()"), self.slotButtonCommands)

    #@@Method: chooseFileAndSavee
    #
    # Lets the user name a file where to save the frame
    #
    # Returns the full path selected, if any, otherwise an empty string
    #@


    def chooseFileAndSave(self, startdir, frame, auto_filename=''):
        if auto_filename != "":
            startdir = startdir + "/" + auto_filename
        filename = QFileDialog.getSaveFileName( startdir, "", self, "", "Choose a file")
        if type(filename) != "NoneType":
            f = filename.latin1()
            if f[-5:] != ".fits":
                f = f + ".fits"
            dict={}
            #BCUfunctions.saveFrame( frame, f, dict)
            return f
        return ""


    #@@Method: getFilename
    #
    # Extracts a filename from a fullpath string
    #@

    def getFilename(self, fullpath):
        pos = fullpath.rfind("/")
        if pos >=0:
            return fullpath[pos+1:]
        else:
            return ""

    #@@Method: slotButtonFrame
    #
    # Starts the script to capture a frame
    #@
    def slotButtonFrame(self):
        num = int(self.framesNum.text().latin1())
        #frames = BCUfunctions.getFrames(num)
   
        fullpath = self.chooseFileAndSave( self.backdir, frames )

    #@@Method: slotButtonBackg
    #
    # Starts the script to capture a background frame
    #@
    def slotButtonBackg(self):
        num = int(self.backgNum.text().latin1())
#        frames = BCUfunctions.getFrames(num, raster_order=0)

        # TODO Auto-generated filename: ccd_freq
        fullpath = self.chooseFileAndSave( self.backdir, 0)#BCUfunctions.averageFrames(frames) )
    
        self.setVariable( "Do you want to set this frame as the new background?", "WFS01DARK", fullpath)

    #@@Method: slotButtonSlopes
    #
    # Starts the script to capture a slope frame
    #@
    def slotButtonSlopes(self):
        num = int(self.slopesNum.text().latin1())
#        slopes = BCUfunctions.getSlopes(num)

        # TODO Auto-generated filename: ccd_freq, modulation
        fullpath = self.chooseFileAndSave( self.slopesdir, 0)#BCUfunctions.averageFrames(slopes))
        self.setVariable( "Do you want to set this frame as the new slopes null?", "WFS01SLPNULL", fullpath)


    #@@Method: slotButtonCommands
    #
    # Starts the script to capture a command frame
    #@
    def slotButtonCommands(self):
        num = int(self.commandsNum.text().latin1())
        commands = BCUfunctions.getCommands(num)

        fullpath = self.chooseFileAndSave( ".", BCUfunctions.averageFrames(commands))


    #@@Method: setVariable
    #
    # Sets a variable value to a filename, asking for user confirmation and stripping the path (if any)
    #@

    def setVariable(self, message, varname, fullpath):

        if fullpath == "":
            return

        answer  = QMessageBox.information ( self, "", message, QMessageBox.Yes, QMessageBox.No, QMessageBox.NoButton )
        if answer == QMessageBox.Yes:
            filename = self.getFilename(fullpath)
            if filename != "":
                msglib.SetVar( varname, msglib.CHAR_VARIABLE(), len(filename), filename)


        
