#!/usr/bin/env python

from AdOpt.QtDesigner.fastlinktest import *
from AdOpt.WfsGUI.thAOAppGUI import *
from AdOpt.WfsGUI import cfg_gui
from AdOpt import cfg, setupDevices, calib
from qt import QTimer, SIGNAL, QColor
import os


class FastlinkTestGui(FastlinkTest, thAOAppGUI):

    def __init__(self, qApp):
        thAOAppGUI.__init__(self, qApp, 'FastlinkTest')
        FastlinkTest.__init__(self)

        cfg_gui.init()

        setupDevices.loadDevices( self, ['sc'], check=True)
        self.prefix = os.path.join( calib.WfsCalibDir(), 'dsp_programs')
        self.switchFile = 'sl_fl_cmd_switch.txt'
        self.testFile = 'sl_fl_cmd_test.txt'

        self.timer = QTimer( self)
        self.connect( self.timer, SIGNAL("timeout()"), self.slotTimer)
        self.timer.start(500)

        self.testing=False
        self._adsecStatus = None

        self._focalStationVarname = 'adsecarb.L.FOCAL_STATION@M_ADSEC'
        self._adsecarbVarname = 'adsecarb.L.FSM_STATE@M_ADSEC'
        self.VarNotif( self._focalStationVarname,  self.slotNotify)
        self.VarNotif( self._adsecarbVarname,  self.slotNotify)
        self.slotNotify( self.ReadVar(self._focalStationVarname))
        self.slotNotify( self.ReadVar(self._adsecarbVarname))


    def slotNotify( self, var):
        self.callAfter( self.varreplyHandler, var)

    def varreplyHandler( self, var):

        varname = var.Name()
        vtype  = var.Type()
        owner  = var.Owner() or 'global'
        value  = var.Value()

        if varname == self._focalStationVarname:
            if value=='null':
                self.labelSwStatus.setText("Disabled")
                self.labelSwStatus.setPaletteForegroundColor( cfg_gui.colors.darkgray)
            else:
                self.labelSwStatus.setText("Enabled")
                self.labelSwStatus.setPaletteForegroundColor( cfg_gui.colors.darkgreen)
            return

        if varname == self._adsecarbVarname:
            self._adsecStatus = value

    def buttonEnable_clicked(self):

        if self._adsecStatus != 'Ready':
            QMessageBox.information(self, 'Wrong adsec status', 'Shell must be RIPped and in Ready state')
            return

        self.sc.fl_start()
        self.refresh(0)

    def buttonDisable_clicked(self):
        self.sc.fl_stop()
        self.refresh(0)

    def buttonTest_clicked(self):

        self.sc.set_fl_cmd( os.path.join( self.prefix, self.testFile))
        self.refresh(0)

    def buttonSwitch_clicked(self):
        self.sc.set_fl_cmd( os.path.join( self.prefix, self.switchFile))
        self.refresh(0)

    def buttonSwEnable_clicked(self):
        cmd = 'ssh magadsecsup "echo \'print, select_wfs_port(1)\' | terminal"'
        line = os.popen(cmd).read()
        self.labelSwStatus.setText("Enabled")
        self.labelSwStatus.setPaletteForegroundColor( cfg_gui.colors.darkgreen)

    def buttonSwDisable_clicked(self):
        cmd = 'ssh magadsecsup "echo \'print, select_wfs_port(0)\' | terminal"'
        line = os.popen(cmd).read()
        self.labelSwStatus.setText("Disabled")
        self.labelSwStatus.setPaletteForegroundColor( cfg_gui.colors.darkgray)


    def buttonFaultRecovery_clicked(self):
        answ = QMessageBox.warning( self, "Confirm", "The fault recovery procedure will reset all BCUs\nand takes about 1 minute to complete.\nDo you really want to do this?", QMessageBox.Yes, QMessageBox.No)
        if answ != QMessageBox.Yes:
            return

        cmd = 'ssh magadsecsup "echo \'print, fsm_fault_recovery(/auto)\' | terminal"'
        line = os.popen(cmd).read()
        self.buttonSwEnable_clicked()



    def slotTimer(self):
        self.callAfter( self.refresh, (0,))
        if self.testing:
            self.callAfter( self.readSwitch, (0,))

    def refresh(self, dummy):
        
        status = self.sc.get_fl_status()
        if status:
            text = 'Enabled'
            color = cfg_gui.colors.darkgreen
        else:
            text = 'Disabled'
            color = cfg_gui.colors.darkgray
        self.labelStatus.setText(text)
        self.labelStatus.setPaletteForegroundColor( color)


        cmd = self.sc.get_fl_cmd()
        cmd = cmd[cmd.rfind('_')+1:-4]
        if (cmd=='test'):
           color = cfg_gui.colors.darkgreen
           text = 'Testing'
        else:
           color = cfg_gui.colors.darkgray
           text = 'Closed loop'

        self.labelConfig.setText(text)
        self.labelConfig.setPaletteForegroundColor( color)

        self.testing =  (status) and (cmd == 'test')

    def readSwitch(self, dummy):

        cmd = 'ssh magadsecsup "echo fiber2 | terminal"'
        line = os.popen(cmd).read()
        line = line.replace('AdOpt>','')
        self.editSwitch.append(line)

       

if __name__ == '__main__':
    qApp = QApplication( sys.argv)

    main = FastlinkTestGui(qApp)
    qApp.setMainWidget(main)
    main.show()

    qApp.exec_loop()


