#!/usr/bin/env python

from qt import *

from cameralens_gui import CameraLensGui
from AdOpt import AOStatesType, AOStatesCode, AOMsgCode, cfg
from AdOpt.wrappers import msglib
import cfg_gui

class cameralensctrl_gui(CameraLensGui):
    
    def __init__(self, aoGui):
        CameraLensGui.__init__(self)
        
        # Init self parameters
        self.aoGui = aoGui
        self.ctrlname  = 'ttctrl'
        self.name = "Camera lens"

        cfg_gui.init()
        
        self._decimalFactor = 1    # One decimal digit os used. Used for the slider.
        
        # Disable the GUI until controller ready and the status is READY or OPERATING
        self._isEnabledGui = False
        self._isCtrlReady = False
        self.enableGui(True, force = True)
        
        # Default parameters (load from config file!)
        self._MIN_POSX_UM = -60
        self._MIN_POSY_UM = -60
        self._MAX_POSX_UM = 60
        self._MAX_POSY_UM = 60
        self._DEF_POSX_UM = 0
        self._DEF_POSY_UM = 0
        
        # Set camera lens parameters (limits, default values,...)
        self.sliderPosX.setMinValue(self._MIN_POSX_UM*self._decimalFactor)
        self.sliderPosX.setMaxValue(self._MAX_POSX_UM*self._decimalFactor)
        self.sliderPosX.setValue(self._DEF_POSX_UM*self._decimalFactor)    # Udpated at the first cur notify!
        self.sliderPosY.setMinValue(self._MIN_POSY_UM*self._decimalFactor)
        self.sliderPosY.setMaxValue(self._MAX_POSY_UM*self._decimalFactor)    
        self.sliderPosY.setValue(self._DEF_POSY_UM*self._decimalFactor)    # Udpated at the first cur notify!
        self.posXreqEdit.setText(str(self._MIN_POSX_UM))
        self.posYreqEdit.setText(str(self._MIN_POSY_UM))
        
        # Create the RTDB vars names
        self.varname_ctrlReady = cfg.clstat(self.ctrlname)
        self.varname_status = cfg.varname(self.ctrlname, 'STATUS')
        self.varname_posXreq = cfg.varname('ttctrl', 'POSX.REQ')
        self.varname_posYreq = cfg.varname('ttctrl', 'POSY.REQ')
        self.varname_posXcur = cfg.varname('ttctrl', 'POSX.CUR')
        self.varname_posYcur = cfg.varname('ttctrl', 'POSY.CUR')

        self.varname_ttctrlStatus = cfg.varname( 'ttctrl', 'STATUS')

        self.ttctrlReady = False

        
        # Ask to be notified on client ready
        self.connect(aoGui, PYSIGNAL("%d" % AOMsgCode['CLNREADY']), self.slotNotify)
        msglib.SendMessage("", AOMsgCode['WAITCLRDY'], cfg.taskname(self.ctrlname))
        
        print "Cameralensctrl_gui succesfully initialized"
        
        
    def enableGui(self, enable, force = False):
        if self._isEnabledGui != enable or force:
            self.sliderPosX.setEnabled(False)
            self.sliderPosY.setEnabled(False)
            self.posXreqEdit.setEnabled(enable)
            self.posYreqEdit.setEnabled(enable)
            self._isEnabledGui = enable
        
        
    ## Handle notifies from AOGui for rtdb variables change
    def slotNotify(self, args):
        (message) = args
        code = message['code']
        

        ## CLNREADY
        ##
        ## When the Stage control client is ready, start the variable management
        if code == AOMsgCode['CLNREADY']:
            
            # Ask to be notified on controller CLSTAT
            self.aoGui.Notify( self.varname_ctrlReady, self.slotNotify, getNow = 1)
            
            # Ask to be notified on STATUS
            self.aoGui.Notify( self.varname_status, self.slotNotify, getNow = 1)
            self.aoGui.Notify( self.varname_ttctrlStatus, self.slotNotify, getNow = 1)
        
        ## VARREPLY
        ##
        ## When one of the watched variables change, update our display
        else:

            varname = message['name']
            value = message['value']
            
            # CLSTAT
            if varname == self.varname_ctrlReady: 
                # Client READY
                if value == 2:    
                    # Ask to be notified on var change
                    self.aoGui.Notify( self.varname_posXcur, self.slotNotify, getNow = 1)
                    self.aoGui.Notify( self.varname_posYcur, self.slotNotify, getNow = 1)
                
                # Client UNREGISTERED or REGISTERED
                else:    
                    # This is needed if the controller dies
                    self.enableGui(True)
    
            # STATUS
            # Note that STATE_ERROR is caused by a failure in applying the
            # values in the controller: but this must allow to operate!
            elif varname == self.varname_status:  
                self.ctrlStatusLabel.setText(AOStatesType[value])
                if AOStatesCode["STATE_READY"] == value or AOStatesCode["STATE_OPERATING"] == value:
                    self._isCtrlReady = True
                    self.recalcEnable()
                else:
                    self._isCtrlReady = False
                    self.recalcEnable()
                
                    
            # POSX.CUR
            elif varname == self.varname_posXcur:
                self.posXcurLabel.setText('%5.2f' % (value*1e6))
                #self.sliderPosX.setValue(value*self._decimalFactor)
            
            # POSY.CUR
            elif varname == self.varname_posYcur:
                self.posYcurLabel.setText('%5.2f' % (value*1e6))
                #self.sliderPosY.setValue(value*self._decimalFactor)

            # TTCTRL controller status (we need it to be OPERATING otherwise our digital outputs don't work)
            elif varname == self.varname_ttctrlStatus:
                if value == AOStatesCode["STATE_OPERATING"]:
                    self.ttctrlReady = True
                else:
                    self.ttctrlReady = False
               
    def recalcEnable(self): 
        if self._isCtrlReady == False or self.ttctrlReady == False:
        #    self.enableGui(False)
        #    self.pushButton1.setPixmap( cfg_gui.pixmaps.pix_ko)
             pass
        else:
        #    self.enableGui(True)
        #    self.pushButton1.setPixmap( cfg_gui.pixmaps.pix_ok)
             pass
        
        
    def sliderPosX_sliderMoved(self,a0):
        self.posXreqEdit.setText(str(float(a0)/self._decimalFactor))
        self.applySettings()

    def sliderPosY_sliderMoved(self,a0):
        self.posYreqEdit.setText(str(float(a0)/self._decimalFactor))
        self.applySettings()
        
    def posXreqEdit_returnPressed(self):
        value = float(self.posXreqEdit.text().latin1())*self._decimalFactor
        self.sliderPosX.setValue(value)
        self.applySettings()
        
    def posYreqEdit_returnPressed(self):
        value = float(self.posYreqEdit.text().latin1())*self._decimalFactor
        self.sliderPosY.setValue(value)
        self.applySettings()
            
    def applySettings(self):
        try:
            posX = float(self.posXreqEdit.text().latin1())
            posY = float(self.posYreqEdit.text().latin1())
            msglib.SetVar(self.varname_posXreq, msglib.REAL_VARIABLE, 1, posX / 1e6)  
            msglib.SetVar(self.varname_posYreq, msglib.REAL_VARIABLE, 1, posY / 1e6)  
            msglib.SetIntVar(cfg.varname('ttctrl',"GO"), 1)

        except ValueError:
            print "Invalid no-float value for posX, posY... ignored!"
       
        
# Main
if __name__ == "__main__":
    from engineeringGui import EngineeringGui
    import sys

    app = QApplication(sys.argv) 
    g = EngineeringGui(name='cameralensgui')
    myGui = cameralensctrl_gui(g)
    app.setMainWidget(myGui)
    myGui.show()
    app.exec_loop()
