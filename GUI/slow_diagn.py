#!/usr/bin/env python
#+File: slow_shmmem.py
#
# Base class for the AdOpt Slow Diagnostic system
#


import sys
import code
import time
import shmmem
import msglib
import struct
from slow_gui import *

# TODO Tutte queste strutture sono create a mano ricalcando le 
# corrispondenti strutture implementate nei programmi Microgate:
# vanno create automaticamente partendo dai .h o dai [nios|diagnostics].txt

# TODO mettere tutto in una classe
TIMEOUT_ERROR = -5001
NO_ERROR      = 0  



class DiagnBlock:
    "Diagnostic block abstract class"
    def __init__(self):
        self.fmt=0

    def calcsize(self):
        return struct.calcsize(self.fmt)

    def fill(self,x):
        self.buffer = struct.unpack(self.fmt,x)

class bcu_diagn( DiagnBlock):
    "MKS_bcu_nios_fixed_area_struct + MKS_bcu_diagn_struct"

    def __init__(self):
        self.fmt           = '7i12B20B4f4i2fi18f'
        self.num_boards    = 20

    def fill(self,x):
        DiagnBlock.fill(self,x)
        ptr = 0
        self.crateID                  = self.buffer[ptr];       ptr += 1
        self.who_ami                  = self.buffer[ptr];       ptr += 1
        self.software_release         = self.buffer[ptr:ptr+3]; ptr += 3
        self.logic_release            = self.buffer[ptr:ptr+2]; ptr += 2
        self.mac_address              = self.buffer[ptr:ptr+8]; ptr += 8
        self.ip_address               = self.buffer[ptr:ptr+4]; ptr += 4
        self.crate_configuration      = self.buffer[ptr:ptr+self.num_boards]; ptr += self.num_boards
        self.local_current_thres_pos  = self.buffer[ptr];       ptr += 1
        self.local_current_thres_neg  = self.buffer[ptr];       ptr += 1
        self.total_current_thres_pos  = self.buffer[ptr];       ptr += 1
        self.total_current_thres_neg  = self.buffer[ptr];       ptr += 1
        self.frames_counter           = self.buffer[ptr];       ptr += 1
        self.relais_board_out         = self.buffer[ptr];       ptr += 1 
        self.serial_number            = self.buffer[ptr];       ptr += 1
        self.relais_board_out_1       = self.buffer[ptr];       ptr += 1

        self.stratix_temp     =  self.buffer[ptr];       ptr += 1
        self.power_temp       =  self.buffer[ptr];       ptr += 1
        self.bck_digitalIO    =  self.buffer[ptr];       ptr += 1

        self.voltage_vccl     =  self.buffer[ptr];       ptr += 1  
        self.voltage_vcca     =  self.buffer[ptr];       ptr += 1
        self.voltage_vssa     =  self.buffer[ptr];       ptr += 1
        self.voltage_vccp     =  self.buffer[ptr];       ptr += 1
        self.voltage_vssp     =  self.buffer[ptr];       ptr += 1
        self.current_vccl     =  self.buffer[ptr];       ptr += 1
        self.current_vcca     =  self.buffer[ptr];       ptr += 1
        self.current_vssa     =  self.buffer[ptr];       ptr += 1
        self.current_vccp     =  self.buffer[ptr];       ptr += 1
        self.current_vssp     =  self.buffer[ptr];       ptr += 1

        self.tot_current_vccp =  self.buffer[ptr];       ptr += 1
        self.tot_current_vssp =  self.buffer[ptr];       ptr += 1
        self.tot_current_vp   =  self.buffer[ptr];       ptr += 1
	
        self.in0_temp         =  self.buffer[ptr];       ptr += 1
        self.in1_temp         =  self.buffer[ptr];       ptr += 1
        self.out0_temp        =  self.buffer[ptr];       ptr += 1
        self.out1_temp        =  self.buffer[ptr];       ptr += 1
        self.ext_umidity      =  self.buffer[ptr];       ptr += 1

    def dump(self):
        print 'CrateID %d   Iam %d    Software v%d.%d(%d)    Logic v%d.%d     Serial %d' %   \
            ( self.crateID, self.who_ami, self.software_release[0],self.software_release[1], \
            self.software_release[2],self.logic_release[0], self.logic_release[1], self.serial_number)
        print 'Crate config',
        for x in range(0, self.num_boards):
            print '%d' % (self.crate_configuration[x]),
        print ''

        print 'Mac_addr %02X:%02X:%02X:%02X:%02X:%02X    IP %d.%d.%d.%d' % \
            (self.mac_address[0],self.mac_address[1],self.mac_address[2],self.mac_address[3],\
             self.mac_address[4],self.mac_address[5],self.ip_address[0],self.ip_address[1],\
             self.ip_address[2],self.ip_address[3])
        print 'Temp (C)      Stratix %.3g       Power %.3g'  % ( self.stratix_temp, self.power_temp) 

class dsp_diagn( DiagnBlock):
    "MKS_dsp_nios_fixed_area_struct + MKS_dsp_diagn_struct"

    def __init__(self):
        self.fmt           = '7i12f'

    def fill(self,x):
        DiagnBlock.fill(self,x)
        ptr = 0
        self.who_ami                  = self.buffer[ptr];       ptr += 1
        self.software_release         = self.buffer[ptr:ptr+3]; ptr += 3
        self.logic_release            = self.buffer[ptr:ptr+2]; ptr += 2
        self.serial_number            = self.buffer[ptr];       ptr += 1
        
        self.stratix_temp     = self.buffer[ptr];       ptr += 1
        self.power_temp       = self.buffer[ptr];       ptr += 1
        self.dsps_temp        = self.buffer[ptr];       ptr += 1
        self.driver_temp      = self.buffer[ptr];       ptr += 1
        self.coil_current     = self.buffer[ptr:ptr+8]; ptr += 8

    def dump(self):
        print 'Iam %d      Software v%d.%d.%d     Logic v%d.%d     Serial %d' % ( self.who_ami, \
            self.software_release[0],self.software_release[1],self.software_release[2], \
            self.logic_release[0], self.logic_release[1], self.serial_number)
        print 'Temp (C)      Stratix %.3g       Power %.3g      Dsps %.3g      Driver %.3g'  % \
            ( self.stratix_temp, self.power_temp, self.dsps_temp, self.driver_temp) 
        print 'Current (mA)  %g  %g  %g  %g  %g  %g  %g  %g'  % (self.coil_current) 
        

class siggen_diagn (dsp_diagn):
    pass


class head_diagn( DiagnBlock):
    "header of the slow diagnostic shmmem"

    def __init__(self):
        self.fmt           = 'i'
    
    def fill(self,x):
        DiagnBlock.fill(self,x)
        self.n_crates  = self.buffer[0]


class crate_diagn( DiagnBlock):
    "crate configuration read from shmmem"

    def __init__(self):
        self.fmt           = '3i'

    def fill(self,x):
        DiagnBlock.fill(self,x)
        self.n_bcus     = self.buffer[0]
        self.n_siggens  = self.buffer[1]
        self.n_dsps     = self.buffer[2]

    def dump(self):
        print 'Crate config is: %d bcu, %d siggen, %d dsp' % \
            (self.n_bcus, self.n_siggens, self.n_dsps)

class SlowDiagn:

    def __init__(self, name):
        self.name   = name
        self.timeout = 2000        # timeout in ms
        self.t       = Board('root')
        self.crate  = []

    def fill(self,x):
        ptr=self.head.calcsize()
        for c in self.crate:
            size = c.diagn.calcsize()
            c.diagn.fill(x[ptr:ptr+size]); ptr += size
            for b in c.boards:
                size = b.diagn.calcsize()
                b.diagn.fill(x[ptr:ptr+size]); ptr += size

    def dump(self):
        for c in self.crate:
            c.dump()
            for b in c.boards:
                b.dump()

    def connectToShared(self, shmName):
        if shmmem.setup( self.name, "127.0.0.1") != NO_ERROR:
            print 'aaa '+str(ServerFD)
            raise GenericError
		
        if shmmem.bufRequest( shmName) != NO_ERROR:
            raise GenericError

    def readShmMem(self):
        self.buf, self.stat = shmmem.bufRead(self.timeout)
        if self.stat == TIMEOUT_ERROR:  # -5001  TODO 
            raise TimeoutError
        self.fill(self.buf)
    
    def Config(self):
        self.buf, self.stat = shmmem.bufRead(self.timeout)
        if self.stat == TIMEOUT_ERROR:  # -5001  TODO 
            raise TimeoutError

        self.head  = head_diagn()
        ptr = 0
        size = self.head.calcsize()
        self.head.fill(self.buf[ptr: ptr+size])
        ptr += size
        self.n_crates = self.head.n_crates
        for i in range(self.n_crates): 
            #self.crate.append(crate_diagn())
            self.crate.append(crate('crate_'+str(i), self.t))
            self.crate[i].diagn = crate_diagn()
            size = self.crate[i].diagn.calcsize()
            self.crate[i].diagn.fill(self.buf[ptr: ptr+size])
            ptr += size
            
            self.crate[i].bcu = []
            for j in range(self.crate[i].diagn.n_bcus):
                #self.crate[i].bcu.append(bcu_diagn())
                self.crate[i].bcu.append(bcu('bcu_'+str(j), self.crate[i]))
                self.crate[i].bcu[j].diagn = bcu_diagn()
                ptr += self.crate[i].bcu[j].diagn.calcsize()
            
            self.crate[i].siggen = []
            for j in range(self.crate[i].diagn.n_siggens):
                self.crate[i].siggen.append(siggen('siggen_'+str(j), self.crate[i]))
                self.crate[i].siggen[j].diagn = siggen_diagn()
                ptr += self.crate[i].siggen[j].diagn.calcsize()

            self.crate[i].dsp = []
            for j in range(self.crate[i].diagn.n_dsps):
                self.crate[i].dsp.append(dsp('dsp_'+str(j),self.crate[i]))
                self.crate[i].dsp[j].diagn = dsp_diagn()
                ptr += self.crate[i].dsp[j].diagn.calcsize()
                
            self.crate[i].boards = self.crate[i].bcu + \
                                   self.crate[i].siggen + \
                                   self.crate[i].dsp
        print repr(self.t) 
        # create gui
        self.gui = tree_gui(self.t)
        # create timer
        self.gui.timer = QTimer()
        self.gui.connect(self.gui.timer, SIGNAL("timeout()"), self.timerEvento )
        
        
    def timerEvento(self):
#        stringa = "Frame: "+str(datetime.now().second)
#        print stringa
#        self.crate[0].bcu[0].widget.txtFrmCntr.setText(stringa)
        try:
            self.readShmMem()
        except TimeoutError:
            print 'timeout occurred'
            sys.exit(0)
        for b in self.t:
            if b.type != 'Generic':
                b.widget.updateDisplay(b.diagn)

class TimeoutError(Exception):
    pass

class GenericError(Exception):
    pass


if __name__ == "__main__":
    import sys
    app = QApplication(sys.argv)
    selfname = 'PYCNSMR'
    x = SlowDiagn(selfname)
    x.timeout = 5000
    
    try:
        x.connectToShared('HOUSEKPR:SLOW')
    except:
        print 'connection to shmmem failed'
        sys.exit(0)
    
    try:
        x.Config()
    except TimeoutError:
        print 'timeout occurred on read config'
        sys.exit(0)
    
    x.gui.show()
    app.setMainWidget(x.gui)
    x.gui.timer.start(2000)
    app.exec_loop()


