#!/usr/bin/env python

import sys
from qt import *


from AdOpt.WfsGUI.engineeringGui import EngineeringGui


from AdOpt.WfsGUI.point_and_source_impl import *
from AdOpt.WfsGUI.ho_arm_impl import *
from AdOpt.WfsGUI.tv_arm_impl import *
from AdOpt.WfsGUI.temps_gui import *

from AdOpt.WfsGUI.tests_gui import *
from AdOpt.WfsGUI.restoreSetup import *

## First and foremost, construct the QApplication object

app = QApplication( sys.argv)

class GUIMain( EngineeringGui):

    def __init__(self):

        EngineeringGui.__init__(self)

        dialogs = ['point_and_source_impl', 'ho_arm_impl', 'tv_arm_impl', 'temps_gui', 'restoreSetup', 'tests_gui']

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
