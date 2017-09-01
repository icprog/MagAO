#
# Slow diagnostic GUI
# The GUI implements a tree_gui object. A leaf of the tree
# is a Board object (e.g. crate, bcu, dsp, siggen)
#

from qt import *
from tree import *
from tree_gui import *
from fpformat import *

import sys
#from random import * #DELETEME
#from datetime import * #DELETEME
#from time import * #DELETEME

class form_crate(QWidget):
    def __init__(self,parent = None,name = None,fl = 0):
        QWidget.__init__(self,parent,name,fl)
        self.layout = QVBoxLayout(self,10)
        self.layout.setAutoAdd(1)

		# groupbox containing crate configuration info
        self.groupBox7    = QGroupBox(2, Qt.Horizontal, self, "group7")
        self.txtCrateCfg1 = QLabel(self.groupBox7,"txtCrateCfg1")
        self.txtCrateCfg2 = QLabel(self.groupBox7,"txtCrateCfg2")	

        # groupbox containing threshold current info
        self.groupBox8 = QGroupBox(4, Qt.Horizontal, self, "group8")
        self.txtLThrsPos  = QLabel(self.groupBox8,"txtLThrsPos")
        self.txtLThrsNeg  = QLabel(self.groupBox8,"txtLThrsNeg")
        self.txtTThrsPos  = QLabel(self.groupBox8,"txtTThrsPos")
        self.txtTThrsNeg  = QLabel(self.groupBox8,"txtTThrsNeg")
#self.languageChange()
    
    def updateDisplay(self,diagn):
        self.groupBox7.setTitle("Crate Configuration")
        self.groupBox8.setTitle("Threshold Current (mA)")
        self.txtCrateCfg1.setText("255  128  252  128    0    2    4    6    8   10")
        self.txtCrateCfg2.setText(" 12   14   16   18   20   22   24   26  128  128")
        self.txtLThrsPos.setText("Local pos:  1000")
        self.txtLThrsNeg.setText("Local neg: -1000")
        self.txtTThrsPos.setText("Local pos:  1000")
        self.txtTThrsNeg.setText("Local neg: -1000")
        

class form_bcu(QWidget):
    def __init__(self,parent = None,name = None,fl = 0):
        QWidget.__init__(self,parent,name,fl)
        self.layout = QVBoxLayout(self,10)
        self.layout.setAutoAdd(1)
		
		# a first groupbox of 1 horizontal strip containing generic info
        self.groupBox1 = QGroupBox(3, Qt.Horizontal, self, "group1")
        self.txtCrateID   = QLabel(self.groupBox1,"txtCrateID")	
        self.txtWhoAmI    = QLabel(self.groupBox1,"txtWhoAmI")	
        self.txtSoftVers  = QLabel(self.groupBox1,"txtSoftVers")	
        self.txtLogVers   = QLabel(self.groupBox1,"txtLogVers")	
        self.txtMAC       = QLabel(self.groupBox1,"txtMAC")	
        self.txtIP        = QLabel(self.groupBox1,"txtIP")	
        self.txtSerial    = QLabel(self.groupBox1,"txtSerial")	
        self.txtFrmCntr   = QLabel(self.groupBox1,"txtFrmCntr")	
		
		# groupbox containing temperature info
        self.groupBox2 = QGroupBox(4, Qt.Horizontal, self, "group2")
        self.txtStratix   = QLabel(self.groupBox2,"txtStratix")	
        self.txtPower     = QLabel(self.groupBox2,"txtPower")	
        self.txtHumidity  = QLabel(self.groupBox2,"txtHumidity")
        self.txtDummy1    = QLabel(self.groupBox2,"txtDummy1")
        self.txtIn0Temp   = QLabel(self.groupBox2,"txtIn0Temp")	
        self.txtIn1Temp   = QLabel(self.groupBox2,"txtIn1Temp")	
        self.txtOut0Temp  = QLabel(self.groupBox2,"txtOut0Temp")	
        self.txtOut1Temp  = QLabel(self.groupBox2,"txtOut1Temp")	
		
		# groupbox containing voltage info
        self.groupBox3 = QGroupBox(3, Qt.Horizontal, self, "group3")
        self.txtVccL   = QLabel(self.groupBox3,"txtVccL")	
        self.txtVccA   = QLabel(self.groupBox3,"txtVccA")	
        self.txtVssA   = QLabel(self.groupBox3,"txtVssA")	
        self.txtVccP   = QLabel(self.groupBox3,"txtVccP")	
        self.txtVssP   = QLabel(self.groupBox3,"txtVssP")	
	
		# groupbox containing current info
        self.groupBox4 = QGroupBox(3, Qt.Horizontal, self, "group4")
        self.txtIccL   = QLabel(self.groupBox4,"txtIccL")	
        self.txtIccA   = QLabel(self.groupBox4,"txtIccA")	
        self.txtIssA   = QLabel(self.groupBox4,"txtIssA")	
        self.txtIccP   = QLabel(self.groupBox4,"txtIccP")	
        self.txtIssP   = QLabel(self.groupBox4,"txtIssP")	
		
		# groupbox containing total current info
        self.groupBox5    = QGroupBox(3, Qt.Horizontal, self, "group5")
        self.txtTotIccP   = QLabel(self.groupBox5,"txtTotIccP")	
        self.txtTotIssP   = QLabel(self.groupBox5,"txtTotIssP")	
        self.txtTotIP     = QLabel(self.groupBox5,"txtTotIP")	
		
		# groupbox containing digital i/o info
        self.groupBox6 = QGroupBox(3, Qt.Horizontal, self, "group6")
        self.txtDgtIO  = QLabel(self.groupBox6,"txtDgtIO")	
        self.txtRel1   = QLabel(self.groupBox6,"txtRel1")	
        self.txtRel2   = QLabel(self.groupBox6,"txtRel2")	
		
    def updateDisplay(self,diagn):
        self.groupBox1.setTitle("General")
        self.groupBox2.setTitle("Temperature (C)")
        self.groupBox3.setTitle("Voltage (mV)")
        self.groupBox4.setTitle("Current (mA)")
        self.groupBox5.setTitle("Total Current (mA)")
        self.groupBox6.setTitle("Digital I/O")

        stringa = "CrateID: %d" % (diagn.crateID)
        self.txtCrateID.setText(stringa)
        stringa = "WhoAmI: %d" % (diagn.who_ami)
        self.txtWhoAmI.setText(stringa)
        stringa = "Software Vers: %d.%d.(%d)" % \
                   (diagn.software_release[0],\
                    diagn.software_release[1],\
                    diagn.software_release[2])
        self.txtSoftVers.setText(stringa)
        stringa = "Logic Vers: %d.%d" % \
                   (diagn.logic_release[0],\
                    diagn.logic_release[1])
        self.txtLogVers.setText(stringa)
        stringa = "MAC: %02X:%02X:%02X:%02X:%02X:%02X" %\
                  (diagn.mac_address[0], diagn.mac_address[1],\
                   diagn.mac_address[2], diagn.mac_address[3],\
                   diagn.mac_address[4], diagn.mac_address[5])
        self.txtMAC.setText(stringa)
        stringa = "IP: %d.%d.%d.%d" % \
                   (diagn.ip_address[0], diagn.ip_address[1],\
                    diagn.ip_address[2], diagn.ip_address[3])
        self.txtIP.setText(stringa)
        stringa = "Serial: %d" % (diagn.serial_number)
        self.txtSerial.setText(stringa)
        stringa = "Frame: %d" % (diagn.frames_counter)
        self.txtFrmCntr.setText(stringa)

        stringa = "Stratix: %.2f" % (diagn.stratix_temp)
        self.txtStratix.setText(stringa)
        stringa = "Power: %.2f" % (diagn.power_temp)
        self.txtPower.setText(stringa)
        stringa = "Humidity: %.1f" % (diagn.ext_umidity) 
        self.txtHumidity.setText(stringa)
        self.txtDummy1.setText("")
        stringa = "In 0: %.2f" % (diagn.in0_temp)
        self.txtIn0Temp.setText(stringa)
        stringa = "In 1: %.2f" % (diagn.in1_temp)
        self.txtIn1Temp.setText(stringa)
        stringa = "Out 0: %.2f" % (diagn.out0_temp)
        self.txtOut0Temp.setText(stringa)
        stringa = "Out 1: %.2f" % (diagn.out1_temp)
        self.txtOut1Temp.setText(stringa)
	
        stringa = "V Logic: %d" % (diagn.voltage_vccl)
        self.txtVccL.setText(stringa)	
        stringa = "V+ Analog: %d" % (diagn.voltage_vcca)
        self.txtVccA.setText(stringa)	
        stringa = "V- Analog: %d" % (diagn.voltage_vssa)
        self.txtVssA.setText(stringa)	
        stringa = "V+ Power: %d" % (diagn.voltage_vccp)
        self.txtVccP.setText(stringa)	
        stringa = "V- Power: %d" % (diagn.voltage_vssp)
        self.txtVssP.setText(stringa)	
		
        stringa = "I Logic: %d" % (diagn.current_vccl)
        self.txtIccL.setText(stringa)	
        stringa = "I+ Analog: %d" % (diagn.current_vcca)
        self.txtIccA.setText(stringa)	
        stringa = "I- Analog: %d" % (diagn.current_vssa)
        self.txtIssA.setText(stringa)	
        stringa = "I+ Power: %d" % (diagn.current_vccp)
        self.txtIccP.setText(stringa)	
        stringa = "I- Power: %d" % (diagn.current_vssp)
        self.txtIssP.setText(stringa)	
		
        stringa = "I+ Power: %d" % (diagn.tot_current_vccp)
        self.txtTotIccP.setText(stringa)	
        stringa = "I- Power: %d" % (diagn.tot_current_vssp)
        self.txtTotIssP.setText(stringa)	
        stringa = "? Power: %d" % (diagn.tot_current_vp)
        self.txtTotIP.setText(stringa)	
		
        stringa = "??? dgtio: %d" % (diagn.bck_digitalIO)
        self.txtDgtIO.setText(stringa)		
        stringa = "Rele1: %d" % (diagn.relais_board_out)
        self.txtRel1.setText(stringa)	
        stringa = "Rele2: %d" % (diagn.relais_board_out_1)
        self.txtRel2.setText(stringa)	

class form_dsp(QWidget):
    def __init__(self,parent = None,name = None,fl = 0):
        QWidget.__init__(self,parent,name,fl)
        self.layout = QVBoxLayout(self,10)
        self.layout.setAutoAdd(1)

# a first groupbox of 1 horizontal strip containing generic info
        self.groupBox1 = QGroupBox(2, Qt.Horizontal, self, "group1")
        self.txtWhoAmI    = QLabel(self.groupBox1,"txtWhoAmI")	
        self.txtSerial    = QLabel(self.groupBox1,"txtSerial")	
        self.txtSoftVers  = QLabel(self.groupBox1,"txtSoftVers")	
        self.txtLogVers   = QLabel(self.groupBox1,"txtLogVers")	

# groupbox containing temperature info
        self.groupBox2 = QGroupBox(4, Qt.Horizontal, self, "group2")
        self.txtStratix   = QLabel(self.groupBox2,"txtStratix")	
        self.txtPower     = QLabel(self.groupBox2,"txtPower")	
        self.txtDsp       = QLabel(self.groupBox2,"txtDsp")
        self.txtDriver    = QLabel(self.groupBox2,"txtDriver")

# groupbox containing coil current info
        self.groupBox3 = QGroupBox(4, Qt.Horizontal, self, "group3")
        self.txtCoil0  = QLabel(self.groupBox3,"txtCoil0")	
        self.txtCoil1  = QLabel(self.groupBox3,"txtCoil1")	
        self.txtCoil2  = QLabel(self.groupBox3,"txtCoil2")	
        self.txtCoil3  = QLabel(self.groupBox3,"txtCoil3")	
        self.txtCoil4  = QLabel(self.groupBox3,"txtCoil4")	
        self.txtCoil5  = QLabel(self.groupBox3,"txtCoil5")	
        self.txtCoil6  = QLabel(self.groupBox3,"txtCoil6")	
        self.txtCoil7  = QLabel(self.groupBox3,"txtCoil7")

    def updateDisplay(self, diagn):
        self.groupBox1.setTitle("General")
        self.groupBox2.setTitle("Temperature (C)")
        self.groupBox3.setTitle("Coil Current (mA)")

        stringa = "WhoAmI: %d" % (diagn.who_ami)
        self.txtWhoAmI.setText(stringa)

        stringa = "Software Vers: %d.%d.(%d)" % \
                   (diagn.software_release[0],\
                    diagn.software_release[1],\
                    diagn.software_release[2])
        self.txtSoftVers.setText(stringa)

        stringa = "Logic Vers: %d.%d" % \
                   (diagn.logic_release[0],\
                    diagn.logic_release[1])
        self.txtLogVers.setText(stringa)
        
        stringa = "Serial: %d" % (diagn.serial_number)
        self.txtSerial.setText(stringa)
        
        stringa = "Stratix: %.2f" % (diagn.stratix_temp)
        self.txtStratix.setText(stringa)
        stringa = "DSP: %.2f" % (diagn.power_temp)
        self.txtDsp.setText(stringa)
        stringa = "Power: %.2f" % (diagn.dsps_temp)
        self.txtPower.setText(stringa)
        stringa = "Driver: %.2f" % (diagn.driver_temp)
        self.txtDriver.setText(stringa)
        stringa = "Coil #0: %d" % (diagn.coil_current[0])
        self.txtCoil0.setText(stringa)
        stringa = "Coil #1: %d" % (diagn.coil_current[1])
        self.txtCoil1.setText(stringa)
        stringa = "Coil #2: %d" % (diagn.coil_current[2])
        self.txtCoil2.setText(stringa)
        stringa = "Coil #3: %d" % (diagn.coil_current[3])
        self.txtCoil3.setText(stringa)
        stringa = "Coil #4: %d" % (diagn.coil_current[4])
        self.txtCoil4.setText(stringa)
        stringa = "Coil #5: %d" % (diagn.coil_current[5])
        self.txtCoil5.setText(stringa)
        stringa = "Coil #6: %d" % (diagn.coil_current[6])
        self.txtCoil6.setText(stringa)
        stringa = "Coil #7: %d" % (diagn.coil_current[7])
        self.txtCoil7.setText(stringa)

#
# Generic class Board, inherited by crate, bcu, siggen, dsp
#
class Board(Tree):
    def __init__(self, label='', parent=None):
        Tree.__init__(self,parent)
        self.label = label
        self.type         = 'Generic'
        self.widget       = QWidget(None, self.label+'_BoardWidget')

        
    def __str__(self):
        return self.label

class crate(Board):
    def __init__(self, label,parent=None):
        Board.__init__(self, label, parent)
        self.type='crate'
        self.widget=form_crate()

class bcu(Board):
    def __init__(self, label,parent=None):
        Board.__init__(self, label, parent)
        self.type='bcu'
        self.widget=form_bcu()

class dsp(Board):
    def __init__(self, label,parent=None):
        Board.__init__(self, label, parent)
        self.type='dsp'
        self.widget=form_dsp()

class siggen(dsp):
    def __init__(self, label,parent=None):
        dsp.__init__(self, label, parent)
        self.type='siggen'

# create tree in reverse order, because of listView's inserting method 
# in this way the list will be properly sorted
class slow_prova(QObject):
    def __init__(self):
        self.t     = Board('root')
        crate1  = crate('crate_1',self.t)
        crate2  = crate('crate_0',self.t)
        siggen1 = siggen('siggen', crate2)
        self.bcu1   = bcu('seconda_bcu', crate2)
        abcuo  = bcu('prima_bcu', crate2)
        cdsp    = dsp('hl_dsp', crate1)
        bdsp    = dsp('el_dsp', crate1)
        adsp    = dsp('il_dsp', crate1)
        asiggen = siggen('3_siggen', crate1)
        abcu1   = bcu('bcu_2', crate1)
        abcuo   = bcu('1_bcu', crate1)
        for b in self.t:
            print b
        print repr(self.t)
       
        self.timer    = QTimer()
        self.gui = tree_gui(self.t)
        a = self.connect(self.timer, SIGNAL("timeout()"), self.timerEvento ) 

    def timerEvento(self):
        stringa = "Frame: "+str(datetime.now().second)
        print stringa
        self.bcu1.widget.txtFrmCntr.setText(stringa)



if __name__ == "__main__":
    import sys
    app = QApplication(sys.argv)
    f = slow_prova()
    f.gui.show()
    app.setMainWidget(f.gui)
    f.timer.start(2000)
    app.exec_loop()


