#!/usr/bin/env python

import sys
from qt import *


from AdOpt.WfsGUI.engineeringGui import EngineeringGui

from AdOpt.WfsGUI.ccd39_gui import *
from AdOpt.WfsGUI.ccd47_gui import *
from AdOpt.WfsGUI.stages_gui import *
from AdOpt.WfsGUI.quick_selection import *
#from AdOpt.WfsGUI.wfs_operation import *
from AdOpt.WfsGUI.tt_gui import *
from AdOpt.WfsGUI.tt_lowlevel_gui import *
from AdOpt.WfsGUI.power_gui import *
from AdOpt.WfsGUI.filter_wheel1 import *
#from AdOpt.WfsGUI.filter_wheel2 import *
#from AdOpt.WfsGUI.filter_wheel3 import *
#from adc import *
from AdOpt.WfsGUI.rerot_gui import *
from AdOpt.WfsGUI.status_gui import *
from AdOpt.WfsGUI.temps_gui import *
from AdOpt.WfsGUI.mercury import *
from AdOpt.WfsGUI.cubestage import *
from AdOpt.WfsGUI.lamp_gui import *
from AdOpt.WfsGUI.adclow1 import *
from AdOpt.WfsGUI.adclow2 import *
from AdOpt.WfsGUI.adc_hilevel_gui import *
from AdOpt.WfsGUI.cameralensctrl_gui import *
from AdOpt.WfsGUI.tests_gui import *
from AdOpt.WfsGUI.restoreSetup import *
from AdOpt.WfsGUI.calib_gui import *

#from AdOpt.VisAOGUI.shutter_gui import *
from AdOpt.VisAOGUI.VisAOfilter_wheel2 import *
from AdOpt.VisAOGUI.VisAOfilter_wheel3 import *
from AdOpt.VisAOGUI.VisAOccd47_gui import *
from AdOpt.VisAOGUI.VisAOI_gui import *
from AdOpt.VisAOGUI.CRO_gui import *
from AdOpt.VisAOGUI.OffloadTest_gui import *
#from AdOpt.VisAOGUI.VisAOPwrCtrl_gui import *
## First and foremost, construct the QApplication object

app = QApplication( sys.argv)

class GUIMain( EngineeringGui):

    def __init__(self):

        EngineeringGui.__init__(self)

        self.setCaption('ADSEC hardware GUI')

        #dialogs = ['power_gui','ccd39_gui','VisAOccd47_gui', 'filter_wheel1', 'VisAOfilter_wheel2', 'VisAOfilter_wheel3','status_gui',
        #           'temps_gui', 'tt_gui', 'tt_lowlevel_gui', 'rerot_gui',
        #           'cubestage', 'mercury', 'stages_gui', 'lamp_gui', 'cameralensctrl_gui', 
        #             'restoreSetup', 'adclow1', 'adclow2', 'adc_hilevel_gui', 'tests_gui', 'quick_selection', 'calib_gui',
        #             'VisAOI_gui', 'CRO_gui']#'shutter_gui']

        dialogs = ['OffloadTest_gui', 'CRO_gui']

        self.dialog_list = []

        for dialog in dialogs:
            command = "self.dialog_list.append(%s(self))" % dialog 
            eval(command)
            self.dialog_list[-1].setCaption(self.dialog_list[-1].name)
            self.AddWindow( self.dialog_list[-1], id = dialog)       


        self.stack.raiseWidget(0)
        
# Start application

main = GUIMain()
app.setMainWidget(main)
main.show()

app.exec_loop()
