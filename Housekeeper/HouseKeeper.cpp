//@File: HouseKeeper.cpp
//
// Implementation of the HouseKeeper class
//@

#include <cstdio>
#include <cstring>
#include <sstream>


extern "C" {
#include "base/thrdlib.h"    
#include "iolib.h" //ReadBinaryFile
#include "commlib.h"
}

#include "HouseKeeper.h"
#include "HouseKeeperGain.h"
#include "Timing.h"


// Initialize static members
//int HouseKeeper::use_common =0;
//        

//@Member HouseKeeper
//
//Standard constructor
//@

HouseKeeper::HouseKeeper(int argc, char **argv) throw (AOException) : 
    DiagnApp(argc, argv),
    _sample_counter(0)
{
   CreateFD();
}



//HouseKeeper::HouseKeeper(const std::string& conffile, int verb )
//    : DiagnApp::DiagnApp(conffile, verb)
void HouseKeeper::CreateFD()
{
    ReadConfig(); 
    //_unprotected = true;
    _bcuRequestTimeout_ms=(int)ConfigDictionary()["timeout_sec"]*1000; 
    
    // Creates an object able to react in case of emergency
    _adam = NULL;
    if(!_dummy) {
    	_adam = new AdamModbus("ADAM-MODBUS", false);
    }

    _mastdiagninterface = new MastDiagnInterface(string(HOUSEKEEPER_MASTDIAG_STR)+"."+Side(), Logger::LOG_LEV_ERROR);

}

//@Member $\sim$ HouseKeeper
//
//Destructor
//@
HouseKeeper::~HouseKeeper()
{
    _logger->log(Logger::LOG_LEV_FATAL, "Housekeeper is exiting. Will shut down the mirror soon  [%s:%d]",__FILE__,__LINE__);
	if(_adam) {
	  _adam->disableMainPower();
     _logger->log(Logger::LOG_LEV_FATAL, "Adam main power disabled  [%s:%d]",__FILE__,__LINE__);
	}
    delete _comSender;
    if (_mastdiagninterface)
        delete _mastdiagninterface;
    delete _adam;
}

//@Member: ReadConfig
//
//@
void HouseKeeper::ReadConfig()
{
    _dummy = (bool) ( (int)(ConfigDictionary()["DummyMode"]));
    if(_dummy) _logger->log(Logger::LOG_LEV_WARNING, "PROGRAM IS RUNNING IN DUMMY MODE [%s:%d]",__FILE__,__LINE__);

    // insert stuff here
}


//@Member: SetupVars
//
//@
void HouseKeeper::SetupVars()
{
    try{
        DiagnApp::SetupVars();
    } catch(AOException &e){
        _logger->log(Logger::LOG_LEV_ERROR, "%s [%s:%d]",e.what().c_str(),__FILE__,__LINE__);
        // TODO FATAL or ERROR. Eventually die
    }
}

//@Member: InstallHandlers
//
//@
void HouseKeeper::InstallHandlers()
{
    DiagnApp::InstallHandlers();
}


//@Member: CreateDiagnVars
//
// 
// NOTES:
// Flags that in the electronic are negated, here are positive (i.e. BCUDigitalIOBusSysFault=1 means fault)
//
//@
void HouseKeeper::CreateDiagnVars()
{

    std::string          groupname;
    FunctActionPtr       hAlarm   (new FunctAlarm());                   
    FunctActionPtr       hWarning (new FunctWarning());
    FunctActionPtr       hEmergencyStop   (new FunctEmergencyStop(ConfigDictionary()["SirenFile"]));
    FunctActionPtr       hEmergencyStopShutdown   (new FunctEmergencyStop(ConfigDictionary()["SirenFile"], true));

    FunctActionPtr       hAlarmDump   (new FunctAlarmAndDump(this));                   
    FunctActionPtr       hEmergencyStopDump   (new FunctEmergencyStopAndDump(this, ConfigDictionary()["SirenFile"]));
    FunctActionPtr       hEmergencyStopShutdownDump   (new FunctEmergencyStopAndDump(this, ConfigDictionary()["SirenFile"], true));

    ///////////////////////////// DSP Nios Fixed ////////////////////////////
    // DSP nios fixed area are only read at start-up.
    //

//@C
// DSPWhoAmI
//@
    groupname = "DSPWhoAmI";
    for (int i=0; i<BcuMirror::N_DSB; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctUint16(&(dsp_nios[i].who_ami)));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStop));
        _vars.Add(theVar);
    }
    groupname = "SGNWhoAmI";
    for (int i=0; i<BcuMirror::N_SGN; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctUint16(&(sgn_nios[i].who_ami)));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStop));
        _vars.Add(theVar);
    }

//@C
// DSPLogicRelease 
//@
    // (raw->logic_release >> 8) & 0xFF);                // PIO versione logica Major
    // (raw->logic_release & 0xFF);                      // PIO versione logica Minor
    // print, ishft(x,-8) AND 255, ishft(x,0) AND 255
    groupname = "DSPLogicRelease";
    for (int i=0; i<BcuMirror::N_DSB; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctUint16(&(dsp_nios[i].logic_release)));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStop));
        _vars.Add(theVar);
    } 
    groupname = "SGNLogicRelease";
    for (int i=0; i<BcuMirror::N_SGN; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctUint16(&(sgn_nios[i].logic_release)));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStop));
        _vars.Add(theVar);
    } 

//@C    
// DSPSoftwareRelease 
//@
    // (raw->software_release >> 24) & 0xFF);                // versione software del nios Major
    // (raw->software_release >> 16) & 0xFF);                // versione software del nios Minor
    // (raw->software_release  & 0xFFFF);                    // versione software del nios #define_flags
    // 
    //  print, ishft(x,-24) AND 255, ishft(x,-16) AND 255, ishft(x,0) AND 65535
    groupname = "DSPSoftwareRelease";
    for (int i=0; i<BcuMirror::N_DSB; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctUint32(&(dsp_nios[i].software_release)));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStop));
        _vars.Add(theVar);
    } 
    groupname = "SGNSoftwareRelease";
    for (int i=0; i<BcuMirror::N_SGN; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctUint32(&(sgn_nios[i].software_release)));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStop));
        _vars.Add(theVar);
    } 

//@C
// DSPSerialNumber
//@
    groupname = "DSPSerialNumber";
    for (int i=0; i<BcuMirror::N_DSB; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctUint16(&(dsp_nios[i].serial_number)));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStop));
        _vars.Add(theVar);
    }
    groupname = "SGNSerialNumber";
    for (int i=0; i<BcuMirror::N_SGN; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctUint16(&(sgn_nios[i].serial_number)));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStop));
        _vars.Add(theVar);
    }



    for (int i=0; i<BcuMirror::N_DSB; i++)
        for (int j=0; j<BcuMirror::N_DSP_DSB*BcuMirror::N_CH_DSP_ ; j++)
        {
            int index = i * BcuMirror::N_DSP_DSB*BcuMirror::N_CH_DSP_ + j;

//@C
// DSPADCspiOffset
//@
            {
            groupname = "DSPADCspiOffset";
            _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), index);
            FunctPtr fConv (new FunctFloat32(&(dsp_nios[i].ADC_spi_offset[j])));
            DiagnVarPtr theVar (new DiagnVar(groupname, index, fConv, hWarning, hEmergencyStop));
            _vars.Add(theVar);
            }

//@C
// DSPADCspiGain
//@
            {
            groupname = "DSPADCspiGain";
            _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), index);
            FunctPtr fConv (new FunctFloat32(&(dsp_nios[i].ADC_spi_gain[j])));
            DiagnVarPtr theVar (new DiagnVar(groupname, index, fConv, hWarning, hEmergencyStop));
            _vars.Add(theVar);
            }

//@C
// DSPADCOffset
//@
            {
            groupname = "DSPADCOffset";
            _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), index);
            FunctPtr fConv (new FunctFloat32(&(dsp_nios[i].ADC_offset[j])));
            DiagnVarPtr theVar (new DiagnVar(groupname, index, fConv, hWarning, hEmergencyStop));
            _vars.Add(theVar);
            }

//@C
// DSPADCGain
//@
            {
            groupname = "DSPADCGain";
            _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), index);
            FunctPtr fConv (new FunctFloat32(&(dsp_nios[i].ADC_gain[j])));
            DiagnVarPtr theVar (new DiagnVar(groupname, index, fConv, hWarning, hEmergencyStop));
            _vars.Add(theVar);
            }

//@C
// DSPDACOffset
//@
            {
            groupname = "DSPDACOffset";
            _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), index);
            FunctPtr fConv (new FunctFloat32(&(dsp_nios[i].DAC_offset[j])));
            DiagnVarPtr theVar (new DiagnVar(groupname, index, fConv, hWarning, hEmergencyStop));
            _vars.Add(theVar);
            }

//@C
// DSPDACGain
//@
            {
            groupname = "DSPDACGain";
            _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), index);
            FunctPtr fConv (new FunctFloat32(&(dsp_nios[i].DAC_gain[j])));
            DiagnVarPtr theVar (new DiagnVar(groupname, index, fConv, hWarning, hEmergencyStop));
            _vars.Add(theVar);
            }

        }

//@C
// DSPDiagnosticRecordPtr
//@
    groupname = "DSPDiagnosticRecordPtr";
    for (int i=0; i<BcuMirror::N_DSB; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctUint32(&(dsp_nios[i].diagnostic_record_ptr)));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStop));
        _vars.Add(theVar);
    }

//@C
// DSPDiagnosticRecordLen
//@
    groupname = "DSPDiagnosticRecordLen";
    for (int i=0; i<BcuMirror::N_DSB; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctUint32(&(dsp_nios[i].diagnostic_record_len)));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStop));
        _vars.Add(theVar);
    }

//@C
// DSPRdDiagnosticRecordPtr
//@
    groupname = "DSPRdDiagnosticRecordPtr";
    for (int i=0; i<BcuMirror::N_DSB; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctUint32(&(dsp_nios[i].rd_diagnostic_record_ptr)));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStop));
        _vars.Add(theVar);
    }

//@C
// DSPWrDiagnosticRecordPtr
//@
    groupname = "DSPWrDiagnosticRecordPtr";
    for (int i=0; i<BcuMirror::N_DSB; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctUint32(&(dsp_nios[i].wr_diagnostic_record_ptr)));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStop));
        _vars.Add(theVar);
    }


    ///////////////////////////// BCU Nios Fixed ////////////////////////////

    /////////////// BCUCrateID ///////////////////
    groupname = "SwitchCrateID";
    for (int i=0; i<BcuSwitch::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctUint16(&(raw.switc[i].crateID)));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStop));
        _vars.Add(theVar);
    } 
    groupname = "BCUCrateID";
    for (int i=0; i<BcuMirror::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctUint16(&(raw.crate[i].crateID)));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStop));
        _vars.Add(theVar);
    } 

    /////////////// BCUWhoAmI ///////////////////
    // 255 = BCU
    // 252 = SIGGEN
    // 0-x = DSP
    groupname = "SwitchWhoAmI";
    for (int i=0; i<BcuSwitch::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctUint16(&(raw.switc[i].who_ami)));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStop));
        _vars.Add(theVar);
    } 
    groupname = "BCUWhoAmI";
    for (int i=0; i<BcuMirror::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctUint16(&(raw.crate[i].who_ami)));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStop));
        _vars.Add(theVar);
    } 

    /////////////// BCUSoftwareRelease ///////////////////
    // (raw->software_release >> 24) & 0xFF);                // versione software del nios Major
    // (raw->software_release >> 16) & 0xFF);                // versione software del nios Minor
    // (raw->software_release  & 0xFFFF);                    // versione software del nios #define_flags
    // 
    // x=ulong64(bb.last) & printhex,ishft(x,-24) AND 255, ishft(x,-16) AND 255, ishft(x,0) AND 65535
    groupname = "SwitchSoftwareRelease";
    for (int i=0; i<BcuSwitch::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctUint32(&(raw.switc[i].software_release)));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStop));
        _vars.Add(theVar);
    } 
    groupname = "BCUSoftwareRelease";
    for (int i=0; i<BcuMirror::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctUint32(&(raw.crate[i].software_release)));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStop));
        _vars.Add(theVar);
    } 

    /////////////// BCULogicRelease ///////////////////
    // (raw->logic_release >> 8) & 0xFF);                // PIO versione logica Major
    // (raw->logic_release & 0xFF);                      // PIO versione logica Minor
    //
    // x=ulong64(bb.last) & printhex, ishft(x,-8) AND 255, ishft(x,0) AND 25
    groupname = "SwitchLogicRelease";
    for (int i=0; i<BcuSwitch::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctUint16(&(raw.switc[i].logic_release)));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStop));
        _vars.Add(theVar);
    } 
    groupname = "BCULogicRelease";
    for (int i=0; i<BcuMirror::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctUint16(&(raw.crate[i].logic_release)));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStop));
        _vars.Add(theVar);
    } 

    /////////////// BCUIPAddress ///////////////////
    groupname = "SwitchIPAddress";
    for (int i=0; i<BcuSwitch::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctIPAddress(&(raw.switc[i].ip_address)));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStop));
        _vars.Add(theVar);
    } 
    groupname = "BCUIPAddress";
    for (int i=0; i<BcuMirror::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctIPAddress(&(raw.crate[i].ip_address)));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStop));
        _vars.Add(theVar);
    }

    /////////////// BCUCrateConfiguration ///////////////////
	//TODO

    /////////////// BCULocalCurrentThreshold // livello di corrente locale di thresholds (positiva e negativa) //////////////////
    groupname = "BCULocalCurrentThreshold";
    for (int i=0; i<BcuMirror::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctConvertGain(&(raw.crate[i].local_current_threshold),
                        AdSec::gain_local_current_threshold, &(raw.crate[i].pb_serial_number)));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStop));
        _vars.Add(theVar);
    }

    /////////////// BCUVPSet /// impostazione tensione rami power (positivo e negativo) //////////////////
    groupname = "BCUVPSet";
    for (int i=0; i<BcuMirror::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctConvertGain(&(raw.crate[i].vp_set),
                        AdSec::gain_vp_set, &(raw.crate[i].pb_serial_number)));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStop));
        _vars.Add(theVar);
    }

    /////////////// BCUTotalCurrentThresholdPos /// livello di corrente totale di thresholds (positiva) //////////////////
    groupname = "BCUTotalCurrentThresholdPos";
    for (int i=0; i<BcuMirror::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctConvertGain(&(raw.crate[i].total_current_thres_pos),
                        AdSec::gain_total_current_thres_pos, &(raw.crate[i].pb_serial_number)));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStop));
        _vars.Add(theVar);
    }

    /////////////// BCUTotalCurrentThresholdNeg /// livello di corrente totale di thresholds (positiva) //////////////////
    groupname = "BCUTotalCurrentThresholdNeg";
    for (int i=0; i<BcuMirror::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctConvertGain(&(raw.crate[i].total_current_thres_neg),
                        AdSec::gain_total_current_thres_neg, &(raw.crate[i].pb_serial_number)));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStop));
        _vars.Add(theVar);
    }

    /////////////// BCUFramesCounter ///////////////////
    groupname = "SwitchFramesCounter";
    for (int i=0; i<BcuSwitch::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctUint32(&(raw.switc[i].frames_counter)));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStop));
        _vars.Add(theVar);
    } 
    groupname = "BCUFramesCounter";
    for (int i=0; i<BcuMirror::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctUint32(&(raw.crate[i].frames_counter)));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStop));
        _vars.Add(theVar);
    } 
    
    /////////////// BCUSerialNumber ///////////////////
    groupname = "SwitchSerialNumber";
    for (int i=0; i<BcuSwitch::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctUint16(&(raw.switc[i].serial_number)));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStop));
        _vars.Add(theVar);
    } 
    groupname = "BCUSerialNumber";
    for (int i=0; i<BcuMirror::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctUint16(&(raw.crate[i].serial_number)));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStop));
        _vars.Add(theVar);
    } 

    /////////////// BCUPowerBackplaneSerial ///////////////////
    // Power backplane not present in the SwitchBCU
    //groupname = "SwitchPowerBackplaneSerial";
    //for (int i=0; i<BcuSwitch::N_CRATES; i++)
    //{
    //    _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
    //    FunctPtr fConv (new FunctUint16(&(raw.switc[i].pb_serial_number)));
    //    DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStop));
    //    _vars.Add(theVar);
    //} 
    groupname = "BCUPowerBackplaneSerial";
    for (int i=0; i<BcuMirror::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctUint16(&(raw.crate[i].pb_serial_number)));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStop));
        _vars.Add(theVar);
    } 
    
    /////////////// BCUEnableMasterDiagnostic ///////////////////
    groupname = "SwitchEnableMasterDiagnostic";
    for (int i=0; i<BcuSwitch::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctUint16(&(raw.switc[i].enable_master_diag)));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStop));
        _vars.Add(theVar);
    } 
    groupname = "BCUEnableMasterDiagnostic";
    for (int i=0; i<BcuMirror::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctUint16(&(raw.crate[i].enable_master_diag)));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStop));
        _vars.Add(theVar);
    } 
    
    /////////////// BCUDecimationFactor ///////////////////
    groupname = "SwitchDecimationFactor";
    for (int i=0; i<BcuSwitch::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctUint16(&(raw.switc[i].decimation_factor)));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStop));
        _vars.Add(theVar);
    } 
    groupname = "BCUDecimationFactor";
    for (int i=0; i<BcuMirror::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctUint16(&(raw.crate[i].decimation_factor)));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStop));
        _vars.Add(theVar);
    } 
    
    /////////////// BCURemoteIPAddress ///////////////////
    groupname = "SwitchRemoteIPAddress";
    for (int i=0; i<BcuSwitch::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctIPAddress(&(raw.switc[i].remote_ip_address)));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStop));
        _vars.Add(theVar);
    } 
    groupname = "BCURemoteIPAddress";
    for (int i=0; i<BcuMirror::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctIPAddress(&(raw.crate[i].remote_ip_address)));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStop));
        _vars.Add(theVar);
    } 


    ///////////////////////////////////////////////// SWITCH BCU /////////////////////////////////////////////////////
    // SwitchBCU has no powerback plane. The only meaningful diagnostic data are StratixTemp and PowerTemp

    /////////////// SwitchStratixTemp ///////////////////
    groupname = "SwitchStratixTemp";
    for (int i=0; i<BcuSwitch::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctBCUStratixTemp(&(raw.switc[i].BCU_diagnostic.stratix_temp)));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStopShutdownDump));
        _vars.Add(theVar);
    } 

    /////////////// SwitchPowerTemp ///////////////////
    groupname = "SwitchPowerTemp";
    for (int i=0; i<BcuSwitch::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctBCUStratixTemp(&(raw.switc[i].BCU_diagnostic.power_temp)));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStopShutdownDump));
        _vars.Add(theVar);
    }



    ///////////////////////////////////////////////// BCU /////////////////////////////////////////////////////

    /////////////// BCUStratixTemp ///////////////////
    groupname = "BCUStratixTemp";
    for (int i=0; i<BcuMirror::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctBCUStratixTemp(&(raw.crate[i].BCU_diagnostic.stratix_temp)));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStopShutdownDump));
        _vars.Add(theVar);
    } 

    /////////////// BCUPowerTemp ///////////////////
    groupname = "BCUPowerTemp";
    for (int i=0; i<BcuMirror::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctBCUStratixTemp(&(raw.crate[i].BCU_diagnostic.power_temp)));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStopShutdownDump));
        _vars.Add(theVar);
    } 

    /////////////// BCUVoltageVCCL ///////////////////
    groupname = "BCUVoltageVCCL";
    for (int i=0; i<BcuMirror::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctConvertGain(&(raw.crate[i].BCU_diagnostic.voltage_vccl), 
                        AdSec::gain_voltage_vccl, &(raw.crate[i].pb_serial_number)));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStopDump));
        _vars.Add(theVar);
    } 
    /////////////// BCUVoltageVCCA ///////////////////
    groupname = "BCUVoltageVCCA";
    for (int i=0; i<BcuMirror::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctConvertGain(&(raw.crate[i].BCU_diagnostic.voltage_vcca), 
                        AdSec::gain_voltage_vcca, &(raw.crate[i].pb_serial_number)));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStopDump));
        _vars.Add(theVar);
    } 
    /////////////// BCUVoltageVSSA ///////////////////
    groupname = "BCUVoltageVSSA";
    for (int i=0; i<BcuMirror::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctConvertGain(&(raw.crate[i].BCU_diagnostic.voltage_vssa), 
                         AdSec::gain_voltage_vssa, &(raw.crate[i].pb_serial_number)));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStopDump));
        _vars.Add(theVar);
    } 
    /////////////// BCUVoltageVCCP ///////////////////
    groupname = "BCUVoltageVCCP";
    for (int i=0; i<BcuMirror::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctConvertGain(&(raw.crate[i].BCU_diagnostic.voltage_vccp), 
                                             AdSec::gain_voltage_vccp, &(raw.crate[i].pb_serial_number)));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStopDump));
        _vars.Add(theVar);
    } 
    /////////////// BCUVoltageVSSP ///////////////////
    groupname = "BCUVoltageVSSP";
    for (int i=0; i<BcuMirror::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctConvertGain(&(raw.crate[i].BCU_diagnostic.voltage_vssp), 
                                             AdSec::gain_voltage_vssp, &(raw.crate[i].pb_serial_number)));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStopDump));
        _vars.Add(theVar);
    } 


    /////////////// BCUCurrentVCCL ///////////////////
    groupname = "BCUCurrentVCCL";
    for (int i=0; i<BcuMirror::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctConvertGain(&(raw.crate[i].BCU_diagnostic.current_vccl), 
                                             AdSec::gain_current_vccl, &(raw.crate[i].pb_serial_number)));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStopDump));
        _vars.Add(theVar);
    } 
    /////////////// BCUCurrentVCCA ///////////////////
    groupname = "BCUCurrentVCCA";
    for (int i=0; i<BcuMirror::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctConvertGain(&(raw.crate[i].BCU_diagnostic.current_vcca), 
                                             AdSec::gain_current_vcca, &(raw.crate[i].pb_serial_number)));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStopDump));
        _vars.Add(theVar);
    } 
    /////////////// BCUCurrentVSSA ///////////////////
    groupname = "BCUCurrentVSSA";
    for (int i=0; i<BcuMirror::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctConvertGain(&(raw.crate[i].BCU_diagnostic.current_vssa), 
                                             AdSec::gain_current_vssa, &(raw.crate[i].pb_serial_number)));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStopDump));
        _vars.Add(theVar);
    } 
    /////////////// BCUCurrentVCCP ///////////////////
    groupname = "BCUCurrentVCCP";
    for (int i=0; i<BcuMirror::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctConvertGain(&(raw.crate[i].BCU_diagnostic.current_vccp), 
                                             AdSec::gain_current_vccp, &(raw.crate[i].pb_serial_number)));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStopDump));
        _vars.Add(theVar);
    } 
    /////////////// BCUCurrentVSSP ///////////////////
    groupname = "BCUCurrentVSSP";
    for (int i=0; i<BcuMirror::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctConvertGain(&(raw.crate[i].BCU_diagnostic.current_vssp), 
                                             AdSec::gain_current_vssp, &(raw.crate[i].pb_serial_number)));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStopDump));
        _vars.Add(theVar);
    }

  
//@C
// \verb+TotalCurrentVCCP+ is the total current on the positive branch of "power" power-supply (all 6 crates). Only on master BCU.
// TODO mettere solo sul crate master
//@
    groupname = "TotalCurrentVCCP";
    for (int i=0; i<BcuMirror::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctConvertGain(&(raw.crate[i].BCU_diagnostic.tot_current_vccp), 
                                             AdSec::gain_total_current_vccp, &(raw.crate[i].pb_serial_number)));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStopDump));
        _vars.Add(theVar);
    } 

//@C
// \verb+TotalCurrentVSSP+ is the total current on the negative branch of "power" power-supply (all 6 crates). Only on master BCU.
// TODO mettere solo sul crate master
//@
    groupname = "TotalCurrentVSSP";
    for (int i=0; i<BcuMirror::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctConvertGain(&(raw.crate[i].BCU_diagnostic.tot_current_vssp), 
                                             AdSec::gain_total_current_vssp, &(raw.crate[i].pb_serial_number)));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStopDump));
        _vars.Add(theVar);
    }

//@C
// \verb+TotalCurrentVP+ is total current (sum of + and - branches) of "Power" power-supply (all 6 crates). Only on master BCU.
// TODO mettere solo sul crate master
//@
    groupname = "TotalCurrentVP";
    for (int i=0; i<BcuMirror::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctConvertGain(&(raw.crate[i].BCU_diagnostic.tot_current_vp), 
                                             AdSec::gain_total_current_vp, &(raw.crate[i].pb_serial_number)));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStopDump));
        _vars.Add(theVar);
    }


    /////////////// BCUCoolerIn0Temp ///////////////////
    groupname = "BCUCoolerIn0Temp";
    for (int i=0; i<BcuMirror::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctConvertGain(&(raw.crate[i].BCU_diagnostic.in0_temp), 
                                             AdSec::gain_in0_temp, &(raw.crate[i].pb_serial_number)));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStopShutdownDump));
        _vars.Add(theVar);
    } 
    /////////////// BCUCoolerOut0Temp ///////////////////
    groupname = "BCUCoolerOut0Temp";
    for (int i=0; i<BcuMirror::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctConvertGain(&(raw.crate[i].BCU_diagnostic.out0_temp), 
                                             AdSec::gain_out0_temp, &(raw.crate[i].pb_serial_number)));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStopShutdownDump));
        _vars.Add(theVar);
    } 
    /////////////// BCUCoolerIn1Temp ///////////////////
    groupname = "BCUCoolerIn1Temp";
    for (int i=0; i<BcuMirror::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctConvertGain(&(raw.crate[i].BCU_diagnostic.in1_temp), 
                                             AdSec::gain_in1_temp, &(raw.crate[i].pb_serial_number)));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStopShutdownDump));
        _vars.Add(theVar);
    } 
    /////////////// BCUCoolerOut1Temp ///////////////////
    groupname = "BCUCoolerOut1Temp";
    for (int i=0; i<BcuMirror::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctConvertGain(&(raw.crate[i].BCU_diagnostic.out1_temp), 
                                             AdSec::gain_out1_temp, &(raw.crate[i].pb_serial_number)));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStopShutdownDump));
        _vars.Add(theVar);
    } 
    /////////////// BCUCoolerPressure ///////////////////
    groupname = "BCUCoolerPressure";
    for (int i=0; i<BcuMirror::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctConvertGain(&(raw.crate[i].BCU_diagnostic.pressure), 
                                             AdSec::gain_pressure, &(raw.crate[i].pb_serial_number)));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStopDump));
        _vars.Add(theVar);
    } 
    /////////////// BCUHumidity ///////////////////
    groupname = "BCUHumidity";
    for (int i=0; i<BcuMirror::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctConvertGain(&(raw.crate[i].BCU_diagnostic.ext_umidity), 
                                             AdSec::gain_ext_umidity, &(raw.crate[i].pb_serial_number)));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStopDump));
        _vars.Add(theVar);
    } 



    ////////////////////////////////////////// bits of bcu.BCU_diagnostic.bck_digitalIO //////////////////////
    
//@C
// /verb+bck_digitalIO+ has digital inputs coming from power backplane
//@

//@C
// /verb+BCUDigitalIODriverEnabled+ is 0 when drivers are disabled and 1 when drivers are disabled.
//@
    groupname = "BCUDigitalIODriverEnabled"; // "0=driver disabled, 1=driver enabled" //beware: in raw_struct is negated: n_driver_enabled 
    for (int i=0; i<BcuMirror::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctUint32Bit(&(raw.crate[i].BCU_diagnostic.bck_digitalIO), 0, true)); 
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStop));
        _vars.Add(theVar);
    } 
//@C
// /verb+BCUDigitalIOCrateIsMaster+ is 0 for slave crates and 1 for the master crate
//@
    groupname = "BCUDigitalIOCrateIsMaster"; // "0=crate slave, 1=crate master"  
    for (int i=0; i<BcuMirror::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctUint32Bit(&(raw.crate[i].BCU_diagnostic.bck_digitalIO), 3, false)); 
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStop));
        _vars.Add(theVar);
    } 
//@C
// /verb+BCUDigitalIOBusSysFault+ is 1 when a board in the crate (BCU, DSP, SIGGEN or POWER_BCK) has generated a
// power fault due to a malfunctioning
//@
    groupname = "BCUDigitalIOBusSysFault"; // "0=OK, 1=fault"  //beware: in raw_struct is negated: bus_sys_faults_n 
    for (int i=0; i<BcuMirror::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctUint32Bit(&(raw.crate[i].BCU_diagnostic.bck_digitalIO), 4, true)); 
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStopDump));
        _vars.Add(theVar);
    } 
//@C
// /verb+BCUDigitalIOVccFault+ is 1 in case of fault of power supplies
//@
    groupname = "BCUDigitalIOVccFault"; // "0=OK, 1=fault"  
    for (int i=0; i<BcuMirror::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctUint32Bit(&(raw.crate[i].BCU_diagnostic.bck_digitalIO), 6, false)); 
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStopDump));
        _vars.Add(theVar);
    } 
//@C
// /verb+ACPowerFault0+ is 1 in case of fault of 1st 48V power-supply
//@
    groupname = "BCUDigitalIOACPowerFault0"; // "0=OK, 1=fault"  
    for (int i=0; i<BcuMirror::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctUint32Bit(&(raw.crate[i].BCU_diagnostic.bck_digitalIO), 9)); 
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStopDump));
        _vars.Add(theVar);
    } 
//@C
// /verb+ACPowerFault1+ is 1 in case of fault of 2nd 48V power-supply
//@
    groupname = "BCUDigitalIOACPowerFault1"; // "0=OK, 1=fault"  
    for (int i=0; i<BcuMirror::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctUint32Bit(&(raw.crate[i].BCU_diagnostic.bck_digitalIO), 10)); 
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStopDump));
        _vars.Add(theVar);
    } 
//@C
// /verb+ACPowerFault2+ is 1 in case of fault of 3rd 48V power-supply
//@
    groupname = "BCUDigitalIOACPowerFault2"; // "0=OK, 1=fault"  
    for (int i=0; i<BcuMirror::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctUint32Bit(&(raw.crate[i].BCU_diagnostic.bck_digitalIO), 11)); 
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStopDump));
        _vars.Add(theVar);
    } 
//@C
// /verb+ICDisconnected+ is 1 when the cable chaining the crates is disconnected
//@
    groupname = "BCUDigitalIOICDisconnected"; // "0=OK, 1=disconnected"   //beware: in raw_struct is negated: ic_disconnected_n 
    for (int i=0; i<BcuMirror::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctUint32Bit(&(raw.crate[i].BCU_diagnostic.bck_digitalIO), 12, true)); 
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStop));
        _vars.Add(theVar);
    } 

//@C
// /verb+Overcurrent+ is 1 when HW protection on coil currents triggers
//@
    groupname = "BCUDigitalIOOvercurrent"; // "0=OK, 1=overcurrent"  
    for (int i=0; i<BcuMirror::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctUint32Bit(&(raw.crate[i].BCU_diagnostic.bck_digitalIO), 13)); 
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStopDump));
        _vars.Add(theVar);
    } 

//@C
// /verb+CrateID+ gives the crate number between 0 and 7
//@
    groupname = "BCUDigitalIOCrateID"; // 0-7 
    for (int i=0; i<BcuMirror::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctCrateID(&(raw.crate[i].BCU_diagnostic.bck_digitalIO), 14)); 
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStop));
        _vars.Add(theVar);
    } 


    ////////////////////////////////////////// bits of na_bcu_diagnostic_struct.reset_status  //////////////////////
    
//@C
// /verb+reset_status+ has status of reset signals for the BCU
//@

//@C
// /verb+BCUResetStatusFPGAReset+ is 1 when FPGA reset line is on
//@
    groupname = "BCUResetStatusFPGAReset"; 
    for (int i=0; i<BcuMirror::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctUint32Bit(&(raw.crate[i].BCU_diagnostic.reset_status), 0)); 
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStop));
        _vars.Add(theVar);
    } 

//@C
// /verb+BCUResetStatusBUSReset+ is 1 when BUS reset line is on
//@
    groupname = "BCUResetStatusBUSReset"; 
    for (int i=0; i<BcuMirror::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctUint32Bit(&(raw.crate[i].BCU_diagnostic.reset_status), 1)); 
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStop));
        _vars.Add(theVar);
    } 

//@C
// /verb+BCUResetStatusDSPReset+ is 1 when local DSP reset line is on
//@
    groupname = "BCUResetStatusDSPReset"; 
    for (int i=0; i<BcuMirror::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctUint32Bit(&(raw.crate[i].BCU_diagnostic.reset_status), 2)); 
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStop));
        _vars.Add(theVar);
    } 

//@C
// /verb+BCUResetStatusFLASHReset+ is 1 when FLASH reset line is on
//@
    groupname = "BCUResetStatusFLASHReset"; 
    for (int i=0; i<BcuMirror::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctUint32Bit(&(raw.crate[i].BCU_diagnostic.reset_status), 3)); 
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStop));
        _vars.Add(theVar);
    } 

//@C
// /verb+BCUResetStatusPCIReset+ is 1 when PCI reset line is on
//@
    groupname = "BCUResetStatusPCIReset"; 
    for (int i=0; i<BcuMirror::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctUint32Bit(&(raw.crate[i].BCU_diagnostic.reset_status), 4)); 
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStop));
        _vars.Add(theVar);
    } 

//@C
// /verb+BCUResetStatusBusDriverEnable+: status of local signal to enable coils (dafault: enabled)
//@
    groupname = "BCUResetStatusBusDriverEnable"; 
    for (int i=0; i<BcuMirror::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctUint32Bit(&(raw.crate[i].BCU_diagnostic.reset_status), 5)); 
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStop));
        _vars.Add(theVar);
    } 

//@C
// /verb+BCUResetStatusBusDriverEnableStatus+: status of global signal to enable coils as read by the bus
//@
    groupname = "BCUResetStatusBusDriverEnableStatus"; 
    for (int i=0; i<BcuMirror::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctUint32Bit(&(raw.crate[i].BCU_diagnostic.reset_status), 6)); 
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStop));
        _vars.Add(theVar);
    } 

//@C
// /verb+BCUResetStatusBusPowerFaultCtrl+: status of fault signal internally generated by the BCU in case of malfunctioning
//@
    groupname = "BCUResetStatusBusPowerFaultCtrl"; 
    for (int i=0; i<BcuMirror::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctUint32Bit(&(raw.crate[i].BCU_diagnostic.reset_status), 7)); 
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStopDump));
        _vars.Add(theVar);
    } 

//@C
// /verb+BCUResetStatusBusPowerFault+: status of the gloabal fault signal as read by the bus
//@
    groupname = "BCUResetStatusBusPowerFault"; 
    for (int i=0; i<BcuMirror::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctUint32Bit(&(raw.crate[i].BCU_diagnostic.reset_status), 8)); 
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStopDump));
        _vars.Add(theVar);
    } 

//@C
// /verb+BCUResetStatusSystemWatchdog+: status of BCU internal watchdog on ethernet commands (1=enabled, 0=disabled)
//@
    groupname = "BCUResetStatusSystemWatchdog"; 
    for (int i=0; i<BcuMirror::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctUint32Bit(&(raw.crate[i].BCU_diagnostic.reset_status), 9)); 
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStop));
        _vars.Add(theVar);
    } 


//////////////////////////////////////////////////////////////////////////////////////////////////   
///////////////////////////////////////////////////// DSP ////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
//
//
    /////////////// DSPStratixTemp ///////////////////
    groupname = "DSPStratixTemp";
    for (int i=0; i<BcuMirror::N_CRATES; i++)
        for (int j=0; j<BcuMirror::N_DSB_CRATE; j++)
        {
            _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i*BcuMirror::N_DSB_CRATE+j);
            FunctPtr fConv (new FunctBCUStratixTemp(&(raw.crate[i].DSP_diagnostic[j].stratix_temp))); 
            DiagnVarPtr theVar (new DiagnVar(groupname, i*BcuMirror::N_DSB_CRATE+j, fConv, hWarning, hEmergencyStopShutdownDump));
            _vars.Add(theVar);
        } 
    /////////////// DSPPowerTemp ///////////////////
    groupname = "DSPPowerTemp";
    for (int i=0; i<BcuMirror::N_CRATES; i++)
        for (int j=0; j<BcuMirror::N_DSB_CRATE; j++)
        {
            _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i*BcuMirror::N_DSB_CRATE+j);
            FunctPtr fConv (new FunctBCUStratixTemp(&(raw.crate[i].DSP_diagnostic[j].power_temp)));
            DiagnVarPtr theVar (new DiagnVar(groupname, i*BcuMirror::N_DSB_CRATE+j, fConv, hWarning, hEmergencyStopShutdownDump));
            _vars.Add(theVar);
        }  
    /////////////// DSPDspsTemp ///////////////////
    groupname = "DSPDspsTemp";
    for (int i=0; i<BcuMirror::N_CRATES; i++)
        for (int j=0; j<BcuMirror::N_DSB_CRATE; j++)
        {
            _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i*BcuMirror::N_DSB_CRATE+j);
            FunctPtr fConv (new FunctBCUStratixTemp(&(raw.crate[i].DSP_diagnostic[j].dsps_temp)));
            DiagnVarPtr theVar (new DiagnVar(groupname, i*BcuMirror::N_DSB_CRATE+j, fConv, hWarning, hEmergencyStopShutdownDump));
            _vars.Add(theVar);
        } 
    /////////////// DSPDriverTemp ///////////////////
    groupname = "DSPDriverTemp";
    for (int i=0; i<BcuMirror::N_CRATES; i++)
        for (int j=0; j<BcuMirror::N_DSB_CRATE; j++)
        {
            _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i*BcuMirror::N_DSB_CRATE+j);
            FunctPtr fConv (new FunctBCUStratixTemp(&(raw.crate[i].DSP_diagnostic[j].driver_temp)));
            DiagnVarPtr theVar (new DiagnVar(groupname, i*BcuMirror::N_DSB_CRATE+j, fConv, hWarning, hEmergencyStopShutdownDump));
            _vars.Add(theVar);
        } 

    /////////////// DSPCoilCurrent ///////////////////
    groupname = "DSPCoilCurrent";
    for (int i=0; i<BcuMirror::N_CRATES; i++)
        for (int j=0; j<BcuMirror::N_DSB_CRATE; j++)
            for (int k=0; k<BcuMirror::N_DSP_DSB*BcuMirror::N_CH_DSP_; k++)
            {
                int num = (i*BcuMirror::N_DSB_CRATE + j) * BcuMirror::N_DSP_DSB*BcuMirror::N_CH_DSP_ + k;
                _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), num);
                FunctPtr fConv (new FunctDSPCoilCurrent(
                            &(raw.crate[i].DSP_diagnostic[j].coil_current[k]),
                            dsp_nios[(i*BcuMirror::N_DSB_CRATE+j) ].ADC_spi_gain[k],
                            dsp_nios[(i*BcuMirror::N_DSB_CRATE+j) ].ADC_spi_offset[k]
                            ));
                DiagnVarPtr theVar (new DiagnVar(groupname, num, 
                            fConv, hWarning, hEmergencyStopDump));
                _vars.Add(theVar);
            } 


////////////////////////////////////// DSP driver_status //////////////////////////////


//@C
// /verb+DSPDriverStatusFPGAReset+ status of FPGA reset line
//@
    groupname = "DSPDriverStatusFPGAReset";
    for (int i=0; i<BcuMirror::N_CRATES; i++)
        for (int j=0; j<BcuMirror::N_DSB_CRATE; j++)
        {
            _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i*BcuMirror::N_DSB_CRATE+j);
            FunctPtr fConv (new FunctUint32Bit(&(raw.crate[i].DSP_diagnostic[j].driver_status), 0));
            DiagnVarPtr theVar (new DiagnVar(groupname, i*BcuMirror::N_DSB_CRATE+j, fConv, hWarning, hEmergencyStop));
            _vars.Add(theVar);
        } 

//@C
// /verb+DSPDriverStatusDSP0Reset+ status of DSP0 reset line
//@
    groupname = "DSPDriverStatusDSP0Reset";
    for (int i=0; i<BcuMirror::N_CRATES; i++)
        for (int j=0; j<BcuMirror::N_DSB_CRATE; j++)
        {
            _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i*BcuMirror::N_DSB_CRATE+j);
            FunctPtr fConv (new FunctUint32Bit(&(raw.crate[i].DSP_diagnostic[j].driver_status), 1));
            DiagnVarPtr theVar (new DiagnVar(groupname, i*BcuMirror::N_DSB_CRATE+j, fConv, hWarning, hEmergencyStop));
            _vars.Add(theVar);
        } 

//@C
// /verb+DSPDriverStatusDSP1Reset+ status of DSP1 reset line
//@
    groupname = "DSPDriverStatusDSP1Reset";
    for (int i=0; i<BcuMirror::N_CRATES; i++)
        for (int j=0; j<BcuMirror::N_DSB_CRATE; j++)
        {
            _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i*BcuMirror::N_DSB_CRATE+j);
            FunctPtr fConv (new FunctUint32Bit(&(raw.crate[i].DSP_diagnostic[j].driver_status), 2));
            DiagnVarPtr theVar (new DiagnVar(groupname, i*BcuMirror::N_DSB_CRATE+j, fConv, hWarning, hEmergencyStop));
            _vars.Add(theVar);
        } 

//@C
// /verb+DSPDriverStatusFLASHReset+ status of FLASH reset line
//@
    groupname = "DSPDriverStatusFLASHReset";
    for (int i=0; i<BcuMirror::N_CRATES; i++)
        for (int j=0; j<BcuMirror::N_DSB_CRATE; j++)
        {
            _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i*BcuMirror::N_DSB_CRATE+j);
            FunctPtr fConv (new FunctUint32Bit(&(raw.crate[i].DSP_diagnostic[j].driver_status), 3));
            DiagnVarPtr theVar (new DiagnVar(groupname, i*BcuMirror::N_DSB_CRATE+j, fConv, hWarning, hEmergencyStop));
            _vars.Add(theVar);
        } 


//@C
// /verb+DSPDriverStatusDriverEnable+ status of coil enable line
//@
    groupname = "DSPDriverStatusDriverEnable";
    for (int i=0; i<BcuMirror::N_CRATES; i++)
        for (int j=0; j<BcuMirror::N_DSB_CRATE; j++)
            for (int k=0; k<BcuMirror::N_DSP_DSB*BcuMirror::N_CH_DSP_; k++)
            {
                int num = (i*BcuMirror::N_DSB_CRATE+j)* BcuMirror::N_DSP_DSB*BcuMirror::N_CH_DSP_+k;
                _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), num);
                FunctPtr fConv (new FunctUint32Bit(&(raw.crate[i].DSP_diagnostic[j].driver_status), 4+k));
                DiagnVarPtr theVar (new DiagnVar(groupname, num, 
                            fConv, hWarning, hEmergencyStop));
                _vars.Add(theVar);
            } 

//@C
// /verb+DSPDriverStatusBusPowerFaultCtrl+: status of the fault signal internally generated by the DSP in case of malfunctioning
//@
    groupname = "DSPDriverStatusBusPowerFaultCtrl"; 
    for (int i=0; i<BcuMirror::N_CRATES; i++)
        for (int j=0; j<BcuMirror::N_DSB_CRATE; j++)
        {
            _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i*BcuMirror::N_DSB_CRATE+j);
            FunctPtr fConv (new FunctUint32Bit(&(raw.crate[i].DSP_diagnostic[j].driver_status), 12));
            DiagnVarPtr theVar (new DiagnVar(groupname, i*BcuMirror::N_DSB_CRATE+j, fConv, hWarning, hEmergencyStopDump));
            _vars.Add(theVar);
        } 


//@C
// /verb+DSPDriverStatusBusPowerFault+: status of the global fault signal as read by the bus
//@
    groupname = "DSPDriverStatusBusPowerFault"; 
    for (int i=0; i<BcuMirror::N_CRATES; i++)
        for (int j=0; j<BcuMirror::N_DSB_CRATE; j++)
        {
            _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i*BcuMirror::N_DSB_CRATE+j);
            FunctPtr fConv (new FunctUint32Bit(&(raw.crate[i].DSP_diagnostic[j].driver_status), 13));
            DiagnVarPtr theVar (new DiagnVar(groupname, i*BcuMirror::N_DSB_CRATE+j, fConv, hWarning, hEmergencyStopDump));
            _vars.Add(theVar);
        } 

//@C
// /verb+DSPDriverStatusBusDriverEnable+ status of global signal of coils enabled as read by the bus
//@
    groupname = "DSPDriverStatusBusDriverEnable";
    for (int i=0; i<BcuMirror::N_CRATES; i++)
        for (int j=0; j<BcuMirror::N_DSB_CRATE; j++)
        {
            _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i*BcuMirror::N_DSB_CRATE+j);
            FunctPtr fConv (new FunctUint32Bit(&(raw.crate[i].DSP_diagnostic[j].driver_status), 14));
            DiagnVarPtr theVar (new DiagnVar(groupname, i*BcuMirror::N_DSB_CRATE+j, fConv, hWarning, hEmergencyStopDump));
            _vars.Add(theVar);
        } 

//@C
// /verb+DSPDriverStatusEnableDSPWatchdog+ 
//@
    groupname = "DSPDriverStatusEnableDSPWatchdog";
    for (int i=0; i<BcuMirror::N_CRATES; i++)
        for (int j=0; j<BcuMirror::N_DSB_CRATE; j++)
        {
            _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i*BcuMirror::N_DSB_CRATE+j);
            FunctPtr fConv (new FunctUint32Bit(&(raw.crate[i].DSP_diagnostic[j].driver_status), 15));
            DiagnVarPtr theVar (new DiagnVar(groupname, i*BcuMirror::N_DSB_CRATE+j, fConv, hWarning, hEmergencyStop));
            _vars.Add(theVar);
        } 

//@C
// /verb+DSPDriverStatusDSP0WatchdogExpired+ 
//@
    groupname = "DSPDriverStatusDSP0WatchdogExpired";
    for (int i=0; i<BcuMirror::N_CRATES; i++)
        for (int j=0; j<BcuMirror::N_DSB_CRATE; j++)
        {
            _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i*BcuMirror::N_DSB_CRATE+j);
            FunctPtr fConv (new FunctUint32Bit(&(raw.crate[i].DSP_diagnostic[j].driver_status), 16));
            DiagnVarPtr theVar (new DiagnVar(groupname, i*BcuMirror::N_DSB_CRATE+j, fConv, hWarning, hEmergencyStopDump));
            _vars.Add(theVar);
        } 

//@C
// /verb+DSPDriverStatusDSP1WatchdogExpired+ 
//@
    groupname = "DSPDriverStatusDSP1WatchdogExpired";
    for (int i=0; i<BcuMirror::N_CRATES; i++)
        for (int j=0; j<BcuMirror::N_DSB_CRATE; j++)
        {
            _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i*BcuMirror::N_DSB_CRATE+j);
            FunctPtr fConv (new FunctUint32Bit(&(raw.crate[i].DSP_diagnostic[j].driver_status), 17));
            DiagnVarPtr theVar (new DiagnVar(groupname, i*BcuMirror::N_DSB_CRATE+j, fConv, hWarning, hEmergencyStopDump));
            _vars.Add(theVar);
        } 



    /////////////////////////////////////////////////////SIGGEN////////////////////////////////////////

    /////////////// SGNStratixTemp ///////////////////
    groupname = "SGNStratixTemp";
    for (int i=0; i<BcuMirror::N_CRATES; i++)
        for (int j=0; j<BcuMirror::N_SGN_CRATE; j++)
        {
            _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i*BcuMirror::N_SGN_CRATE+j);
            FunctPtr fConv (new FunctBCUStratixTemp(&(raw.crate[i].DSP_diagnostic[BcuMirror::N_DSB_CRATE+j].stratix_temp))); 
            DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStopShutdownDump));
            _vars.Add(theVar);
        } 
    /////////////// SGNPowerTemp ///////////////////
    groupname = "SGNPowerTemp";
    for (int i=0; i<BcuMirror::N_CRATES; i++)
        for (int j=0; j<BcuMirror::N_SGN_CRATE; j++)
        {
            _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i*BcuMirror::N_SGN_CRATE+j);
            FunctPtr fConv (new FunctBCUStratixTemp(&(raw.crate[i].DSP_diagnostic[BcuMirror::N_DSB_CRATE+j].power_temp))); 
            DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStopShutdownDump));
            _vars.Add(theVar);
        } 
    /////////////// SGNDspsTemp ///////////////////
    groupname = "SGNDspsTemp";
    for (int i=0; i<BcuMirror::N_CRATES; i++)
        for (int j=0; j<BcuMirror::N_SGN_CRATE; j++)
        {
            _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i*BcuMirror::N_SGN_CRATE+j);
            FunctPtr fConv (new FunctBCUStratixTemp(&(raw.crate[i].DSP_diagnostic[BcuMirror::N_DSB_CRATE+j].dsps_temp))); 
            DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStopShutdownDump));
            _vars.Add(theVar);
        } 

    //////////////////////////////////////////////// OTHERS ///////////////////////////////////////////

 //@C
// /verb+ExternalTemperature+  external temperature in degree celsius
//@
    groupname = "ExternalTemperature";
    {
        DiagnSetPtr Set0 = _vars.Filter(DiagnWhich("BCUCoolerOut1Temp",2,2));
        DiagnSetPtr Set1 = _vars.Filter(DiagnWhich("BCUCoolerOut1Temp",5,5));
        DiagnSetPtr ExternalTempSensors = Set0 | Set1;

        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), 0);
        FunctPtr fConv (new FunctAverageDiagnSet( ExternalTempSensors )); 
        DiagnVarPtr theVar (new DiagnVar(groupname, 0, fConv, hWarning, hEmergencyStopDump));
        _vars.Add(theVar);
    } 

 //@C
// /verb+FluxRateIn+ Flux rate in
//@
    groupname = "FluxRateIn";
    {
        DiagnSetPtr FluxRateIn;
        if (ConfigDictionary().has_key(groupname+"BCU"))  {

            string value = ConfigDictionary()[groupname+"BCU"];
            string bcuname = value.substr(0, value.find(":"));
            int index = atoi(value.substr(value.find(":")+1).c_str());

            _logger->log(Logger::LOG_LEV_DEBUG, "Custom config found: %s %s:%d\n", groupname.c_str(), bcuname.c_str(), index);
            FluxRateIn = _vars.Filter(DiagnWhich(bcuname, index, index));
        }
        else {
            FluxRateIn = _vars.Filter(DiagnWhich("BCUHumidity",1,1));
        }

        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), 0);
        FunctPtr fConv (new FunctAverageDiagnSet( FluxRateIn )); 
        DiagnVarPtr theVar (new DiagnVar(groupname, 0, fConv, hWarning, hEmergencyStopDump));
        _vars.Add(theVar);
    }



//@C
// /verb+WaterMainInlet+ Temperature of main water inlet
//@
    groupname = "WaterMainInlet";
    {
        DiagnSetPtr WaterMainInlet = _vars.Filter(DiagnWhich("BCUCoolerOut0Temp",2,2));

        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), 0);
        FunctPtr fConv (new FunctAverageDiagnSet( WaterMainInlet )); 
        DiagnVarPtr theVar (new DiagnVar(groupname, 0, fConv, hWarning, hEmergencyStopDump));
        _vars.Add(theVar);
    }

//@C
// /verb+WaterMainOutlet+ Temperature of main water outlet
//@
    groupname = "WaterMainOutlet";
    {
        DiagnSetPtr WaterMainOutlet = _vars.Filter(DiagnWhich("BCUCoolerOut0Temp",4,4));

        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), 0);
        FunctPtr fConv (new FunctAverageDiagnSet( WaterMainOutlet )); 
        DiagnVarPtr theVar (new DiagnVar(groupname, 0, fConv, hWarning, hEmergencyStopDump));
        _vars.Add(theVar);
    }



//@C
// /verb+WaterColdplateInlet+ Temperature of water inlets on coldplate
//    In1 crate0, Out0 crate0, Out1 crate0-1
//@
    groupname = "WaterColdplateInlet";
    {
        DiagnSetPtr Set0 = _vars.Filter(DiagnWhich("BCUCoolerIn1Temp",0,0));
        // DiagnSetPtr Set1 = _vars.Filter(DiagnWhich("BCUCoolerOut0Temp",0,0));
        DiagnSetPtr Set2 = _vars.Filter(DiagnWhich("BCUCoolerOut0Temp",1,1));
        DiagnSetPtr WaterColdplateInlet = Set0 | Set2; //| Set1;

        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), 0);
        FunctPtr fConv (new FunctAverageDiagnSet( WaterColdplateInlet )); 
        DiagnVarPtr theVar (new DiagnVar(groupname, 0, fConv, hWarning, hEmergencyStopDump));
        _vars.Add(theVar);
    }

//@C
// /verb+WaterColdplateOutlet+ Temperature of water outlets on coldplate
//    In1 crate1, Out0 crate1
//@
    groupname = "WaterColdplateOutlet";
    {
        //Original code starts here... and is commented out 
	/*
		DiagnSetPtr Set0 = _vars.Filter(DiagnWhich("BCUCoolerIn1Temp",1,1));
        DiagnSetPtr Set1 = _vars.Filter(DiagnWhich("BCUCoolerOut1Temp",1,1));
        DiagnSetPtr WaterColdplateOutlet = Set0 | Set1;
	*/
		//Original code ends here...

		//Code modification, to eliminate BCUCoolerOut1Temp to be part of WaterColdplateOutlet computation
		//Modification made by RB @LCO: 20121204
		DiagnSetPtr Set0 = _vars.Filter(DiagnWhich("BCUCoolerIn1Temp",1,1));
       // DiagnSetPtr Set1 = _vars.Filter(DiagnWhich("BCUCoolerOut1Temp",1,1));
        DiagnSetPtr WaterColdplateOutlet = Set0 ;    // | Set1;
		//End of modified code

        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), 0);
        FunctPtr fConv (new FunctAverageDiagnSet( WaterColdplateOutlet )); 
        DiagnVarPtr theVar (new DiagnVar(groupname, 0, fConv, hWarning, hEmergencyStopDump));
        _vars.Add(theVar);
    }




//@C
// /verb+ExternalHumidity+  external humidity in percentage (0-100)
//@
    groupname = "ExternalHumidity";
    {
        DiagnSetPtr Set0 = _vars.Filter(DiagnWhich("BCUHumidity",2,2));
        DiagnSetPtr Set1 = _vars.Filter(DiagnWhich("BCUHumidity",5,5));
        DiagnSetPtr BCUHumids = Set0 | Set1;
        
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), 0);
        FunctPtr fConv (new FunctAverageDiagnSet( BCUHumids )); 
        DiagnVarPtr theVar (new DiagnVar(groupname, 0, fConv, hWarning, hEmergencyStopDump));
        _vars.Add(theVar);
    } 


//@C
// /verb+DewPoint+ dew point in celsius degree
//@
    groupname = "DewPoint";
    {
        DiagnVarPtr ExtTemp = _vars.Find(MatchDiagnVar("ExternalTemperature", 0));
        DiagnVarPtr ExtH    = _vars.Find(MatchDiagnVar("ExternalHumidity", 0));
        
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), 0);
        FunctPtr fConv (new FunctDewPoint(ExtTemp, ExtH)); 
        DiagnVarPtr theVar (new DiagnVar(groupname, 0, fConv, hWarning, hAlarmDump));
        _vars.Add(theVar);
    } 
   
//@C
// /verb+CheckDewPoint+ check that dew point temperature is lower than WaterMainInlet and WaterColdplateInlet
//@
    groupname = "CheckDewPoint";
    {
        DiagnSetPtr Set0 = _vars.Filter(DiagnWhich("WaterMainInlet",0,0));
        DiagnSetPtr Set1 = _vars.Filter(DiagnWhich("WaterColdplateInlet",0,0));
        DiagnSetPtr WaterInlets = Set0 | Set1;
        DiagnVarPtr DewP = _vars.Find(MatchDiagnVar("DewPoint", 0));
        
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), 0);
        FunctPtr fConv (new FunctCheckDewPoint(DewP, WaterInlets)); 
        DiagnVarPtr theVar (new DiagnVar(groupname, 0, fConv, hWarning, hEmergencyStopDump));
        _vars.Add(theVar);
    } 
  
/////////////////////////////////////// UPDATE STATUS ////////////////////////////////////
//@C
// /verb+global_counter+ 
//@
    groupname = "DSPGlobalCounter";
    for (int i=0; i<BcuMirror::N_DSP; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctUint32(&(updatestatus.global_counter[i])));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hAlarm));
        _vars.Add(theVar);
    } 

//@C
// /verb+ff_ud_current+ 
//@
    groupname = "DSPFFUDCurrent";
    for (int i=0; i<BcuMirror::N_CH; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctFloat32(&(updatestatus.ff_ud_current[i])));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hAlarm));
        _vars.Add(theVar);
    } 

//@C
// /verb+pos_command+ 
//@
    groupname = "DSPPosCommand";
    for (int i=0; i<BcuMirror::N_CH; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctFloat32(&(updatestatus.pos_command[i])));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hAlarm));
        _vars.Add(theVar);
    } 

//@C
// /verb+bias_current+ 
//@
    groupname = "DSPBiasCurrent";
    for (int i=0; i<BcuMirror::N_CH; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctFloat32(&(updatestatus.bias_current[i])));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hAlarm));
        _vars.Add(theVar);
    } 

//@C
// /verb+fullbias_curr+ 
//@
    groupname = "DSPFullBiasCurr";
    for (int i=0; i<BcuMirror::N_CH; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctFloat32(&(updatestatus.fullbias_curr[i])));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hAlarm));
        _vars.Add(theVar);
    } 

//@C
// /verb+step_ptr_preshaper_cmd+ 
//@
    groupname = "DSPStepPtrPreshaperCmd";
    for (int i=0; i<BcuMirror::N_CH; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctFloat32(&(updatestatus.step_ptr_preshaper_cmd[i])));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hAlarm));
        _vars.Add(theVar);
    } 

//@C
// /verb+step_ptr_preshaper_curr+ 
//@
    groupname = "DSPStepPtrPreshaperCurr";
    for (int i=0; i<BcuMirror::N_CH; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctFloat32(&(updatestatus.step_ptr_preshaper_curr[i])));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hAlarm));
        _vars.Add(theVar);
    } 

//@C
// /verb+float_adc_value+ 
//@
    groupname = "DSPFloatADCValue";
    for (int i=0; i<BcuMirror::N_CH; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctFloat32(&(updatestatus.float_adc_value[i])));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hAlarm));
        _vars.Add(theVar);
    } 

//@C
// /verb+dist_threshold+ 
//@
    groupname = "DSPDistThreshold";
    for (int i=0; i<BcuMirror::N_CH; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctFloat32(&(updatestatus.dist_threshold[i])));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hAlarm));
        _vars.Add(theVar);
    } 

//@C
// /verb+distance+ 
//@
    groupname = "DSPDistance";
    for (int i=0; i<BcuMirror::N_CH; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctFloat32(&(updatestatus.distance[i])));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hAlarm));
        _vars.Add(theVar);
    } 

//@C
// /verb+control_enable+ 
//@
    groupname = "DSPControlEnable";
    for (int i=0; i<BcuMirror::N_CH; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctFloat32(&(updatestatus.control_enable[i])));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hAlarm));
        _vars.Add(theVar);
    } 

//@C
// /verb+float_DAC_value+ 
//@
    groupname = "DSPFloatDACValue";
    for (int i=0; i<BcuMirror::N_CH; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctFloat32(&(updatestatus.float_DAC_value[i])));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hAlarm));
        _vars.Add(theVar);
    } 

//@C
// /verb+err_loop_gain+ 
//@
    groupname = "DSPErrLoopGain";
    for (int i=0; i<BcuMirror::N_CH; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctFloat32(&(updatestatus.err_loop_gain[i])));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hAlarm));
        _vars.Add(theVar);
    } 

//@C
// /verb+speed_loop_gain+ 
//@
    groupname = "DSPSpeedLoopGain";
    for (int i=0; i<BcuMirror::N_CH; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctFloat32(&(updatestatus.speed_loop_gain[i])));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hAlarm));
        _vars.Add(theVar);
    } 

//@C
// /verb+bias_command+ 
//@
    groupname = "DSPBiasCommand";
    for (int i=0; i<BcuMirror::N_CH; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctFloat32(&(updatestatus.bias_command[i])));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hAlarm));
        _vars.Add(theVar);
    } 

//@C
// /verb+fullbias_cmd+ 
//@
    groupname = "DSPFullBiasCmd";
    for (int i=0; i<BcuMirror::N_CH; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctFloat32(&(updatestatus.fullbias_cmd[i])));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hAlarm));
        _vars.Add(theVar);
    } 

//@C
// /verb+cmol_a+ 
//@
    groupname = "DSPCmolA";
    for (int i=0; i<BcuMirror::N_CH; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctFloat32(&(updatestatus.cmol_a[i])));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hAlarm));
        _vars.Add(theVar);
    } 

//@C
// /verb+cmol_b+ 
//@
    groupname = "DSPCmolB";
    for (int i=0; i<BcuMirror::N_CH; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctFloat32(&(updatestatus.cmol_b[i])));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hAlarm));
        _vars.Add(theVar);
    } 

//@C
// /verb+fmol_a+ 
//@
    groupname = "DSPFmolA";
    for (int i=0; i<BcuMirror::N_CH; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctFloat32(&(updatestatus.fmol_a[i])));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hAlarm));
        _vars.Add(theVar);
    } 

//@C
// /verb+fmol_b+ 
//@
    groupname = "DSPFmolB";
    for (int i=0; i<BcuMirror::N_CH; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctFloat32(&(updatestatus.fmol_b[i])));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hAlarm));
        _vars.Add(theVar);
    } 

//@C
// /verb+pos_check_internal_cnt+ 
//@
    groupname = "DSPPosCheckCnt";
    for (int i=0; i<BcuMirror::N_CH; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctUint32(&(updatestatus.pos_check_cnt[i])));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hAlarm));
        _vars.Add(theVar);
    } 


//@C
// /verb+modal skip frame+ 
//
// Flag indicating which DSP is signaling a modal skip frame
// Volatile.
//@
    groupname = "DSPMODALSKIPFRAME";
    for (int i=0; i<BcuMirror::N_DSP; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctFloat32(&(updatestatus.modal_skipframe[i])));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hAlarmDump));
        _vars.Add(theVar);
    } 

//@C
// /verb+dsp command skip frame+ 
//
// Flag indicating which DSP is signaling a command skip frame
// Volatile.
//@
    groupname = "DSPCOMMANDSKIPFRAME";
    for (int i=0; i<BcuMirror::N_DSP; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctFloat32(&(updatestatus.command_skipframe[i])));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hAlarmDump));
        _vars.Add(theVar);
    } 

//@C
// /verb+dsp force skip frame+ 
//
// Flag indicating which DSP is signaling a force skip frame
// Volatile.
//@
    groupname = "DSPFORCESKIPFRAME";
    for (int i=0; i<BcuMirror::N_DSP; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctFloat32(&(updatestatus.force_skipframe[i])));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hAlarmDump));
        _vars.Add(theVar);
    } 

//@C
// /verb+DSPControlCurrent+ obtained as  DAC_value - ff_ud_current - bias_current
//@
    groupname = "DSPControlCurrent";
    for (int i=0; i<BcuMirror::N_CH; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        
        DiagnVarPtr floatDACvalue    = _vars.Find(MatchDiagnVar("DSPFloatDACValue", i));
        DiagnVarPtr ff_ud_current    = _vars.Find(MatchDiagnVar("DSPFFUDCurrent", i));
        DiagnVarPtr bias_current     = _vars.Find(MatchDiagnVar("DSPBiasCurrent", i));
        
        FunctPtr fConv (new FunctDSPControlCurrent(floatDACvalue, ff_ud_current, bias_current));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hAlarmDump));
        _vars.Add(theVar);
    } 

//@C
// /verb+PSBlock+ 
//@
    groupname = "PSBlock";
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), 0);
        FunctPtr fConv (new FunctUint32Bit(&(updatestatus.switch_param_selector), 0));
        DiagnVarPtr theVar (new DiagnVar(groupname, 0, fConv, hWarning, hAlarm));
        _vars.Add(theVar); 
    }

//@C
// /verb+PSNorm4+ 
//@
    groupname = "PSNorm4";
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), 0);
        FunctPtr fConv (new FunctUint32Bit(&(updatestatus.switch_param_selector), 1));
        DiagnVarPtr theVar (new DiagnVar(groupname, 0, fConv, hWarning, hAlarm));
        _vars.Add(theVar); 
    }

//@C
// /verb+PSNormAll+ 
//@
    groupname = "PSNormAll";
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), 0);
        FunctPtr fConv (new FunctUint32Bit(&(updatestatus.switch_param_selector), 2));
        DiagnVarPtr theVar (new DiagnVar(groupname, 0, fConv, hWarning, hAlarm));
        _vars.Add(theVar); 
    }

//@C
// /verb+PSNormConst+ 
//@
    groupname = "PSNormConst";
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), 0);
        FunctPtr fConv (new FunctUint32Bit(&(updatestatus.switch_param_selector), 3));
        DiagnVarPtr theVar (new DiagnVar(groupname, 0, fConv, hWarning, hAlarm));
        _vars.Add(theVar); 
    }

//@C
// /verb+PSLinearizeSlopes+ 
//@
    groupname = "PSLinearizeSlopes";
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), 0);
        FunctPtr fConv (new FunctUint32Bit(&(updatestatus.switch_param_selector), 4));
        DiagnVarPtr theVar (new DiagnVar(groupname, 0, fConv, hWarning, hAlarm));
        _vars.Add(theVar); 
    }

//@C
// /verb+PSUseActualPos+ 
//@
    groupname = "PSUseActualPos";
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), 0);
        FunctPtr fConv (new FunctUint32Bit(&(updatestatus.switch_param_selector), 5));
        DiagnVarPtr theVar (new DiagnVar(groupname, 0, fConv, hWarning, hAlarm));
        _vars.Add(theVar); 
    }

//@C
// /verb+PSDiagnStoreSDRAM+ 
//@
    groupname = "PSDiagnStoreSDRAM";
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), 0);
        FunctPtr fConv (new FunctUint32Bit(&(updatestatus.switch_param_selector), 6));
        DiagnVarPtr theVar (new DiagnVar(groupname, 0, fConv, hWarning, hAlarm));
        _vars.Add(theVar); 
    }

//@C
// /verb+PSFLEnabled+ 
//@
    groupname = "PSFLEnabled";
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), 0);
        FunctPtr fConv (new FunctUint32Bit(&(updatestatus.switch_param_selector), 7));
        DiagnVarPtr theVar (new DiagnVar(groupname, 0, fConv, hWarning, hAlarm));
        _vars.Add(theVar); 
    }

//@C
// /verb+PSOVSFrame+ 
//@
    groupname = "PSOVSFrame";
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), 0);
        FunctPtr fConv (new FunctUint32Bit(&(updatestatus.switch_param_selector), 8));
        DiagnVarPtr theVar (new DiagnVar(groupname, 0, fConv, hWarning, hAlarm));
        _vars.Add(theVar); 
    }


//@C
// /verb+PSSkipFrame+ 
//
// mask is 0x600 (bit 9 and 10)
// Return value is:
// 0 = no skip frame
// 1 = mode skip frame
// 2 = command skip frame
// 3 = current skip frame
//@
    groupname = "PSSkipFrame";
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), 0);
        FunctPtr fConv (new FunctUint32Mask(&(updatestatus.switch_param_selector), 0x600, 9));
        DiagnVarPtr theVar (new DiagnVar(groupname, 0, fConv, hWarning, hAlarm));
        _vars.Add(theVar); 
    }

//@C
// /verb+PSAccelEna+ 
//@
    groupname = "PSAccelEna";
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), 0);
        FunctPtr fConv (new FunctUint32Bit(&(updatestatus.switch_param_selector), 11));
        DiagnVarPtr theVar (new DiagnVar(groupname, 0, fConv, hWarning, hAlarm));
        _vars.Add(theVar); 
    }

//@C
// /verb+PSDSPStoreSDRAM+ 
//@
    groupname = "PSDSPStoreSDRAM";
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), 0);
        FunctPtr fConv (new FunctUint32Bit(&(updatestatus.switch_param_selector), 14));
        DiagnVarPtr theVar (new DiagnVar(groupname, 0, fConv, hWarning, hAlarm));
        _vars.Add(theVar); 
    }

//@C
// /verb+PSDisturb+ 
//
// mask is 0x9000 (bit 12 and 15)
// Return value is :
// 0 = 
// 1 = disturb applied and incremented on WFS frames
// 8 = disturb applied and incremented on OVS frames
// 9 = disturb applied and incremented on all frames
//@
    groupname = "PSDisturb";
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), 0);
        FunctPtr fConv (new FunctUint32Mask(&(updatestatus.switch_param_selector), 0x9000, 12));
        DiagnVarPtr theVar (new DiagnVar(groupname, 0, fConv, hWarning, hAlarm));
        _vars.Add(theVar); 
    }


//@C
// /verb+SwitchOffloadSelector+ 
//@
    groupname = "SwitchOffloadSelector";
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), 0);
        FunctPtr fConv (new FunctUint32(&(updatestatus.switch_offload_selector)));
        DiagnVarPtr theVar (new DiagnVar(groupname, 0, fConv, hWarning, hAlarm));
        _vars.Add(theVar); 
    }
//@C
// /verb+SwitchMirrFramesCounter+ 
//@
    groupname = "SwitchMirrFramesCounter";
    {   
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), 0);
        FunctPtr fConv (new FunctUint32(&(updatestatus.switch_mirr_frames_counter)));
        DiagnVarPtr theVar (new DiagnVar(groupname, 0, fConv, hWarning, hAlarm));
        _vars.Add(theVar); 
    }
//@C
// /verb+SwitchSafeSkipFrameCnt+
//
// Counter of frames skipped by AdSec for safety reason (modes, commands, forces)
// Monotonically increasing.
//@
    groupname = "SwitchSafeSkipFrameCnt";
    {   
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), 0);
        FunctPtr fConv (new FunctUint32(&(updatestatus.switch_safe_skip_frame_cnt)));
        DiagnVarPtr theVar (new DiagnVar(groupname, 0, fConv, hWarning, hAlarm));
        _vars.Add(theVar); 
    }
//@C
// /verb+SwitchPendingSkipFrameCnt+ 
//
// Counter of WFS frames that were discarded by AdSec because it was busy in an oversampling loop
// Monotonically increasing.
//@
    groupname = "SwitchPendingSkipFrameCnt";
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), 0);
        FunctPtr fConv (new FunctUint32(&(updatestatus.switch_pending_skip_frame_cnt)));
        DiagnVarPtr theVar (new DiagnVar(groupname, 0, fConv, hWarning, hAlarm));
        _vars.Add(theVar); 
    }
//@C
// /verb+SwitchWFSGlobalTimeout+ 
//@
    groupname = "SwitchWFSGlobalTimeout";
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), 0);
        FunctPtr fConv (new FunctUint32(&(updatestatus.switch_wfs_global_timeout)));
        DiagnVarPtr theVar (new DiagnVar(groupname, 0, fConv, hWarning, hAlarm));
        _vars.Add(theVar); 
    }
//@C
// /verb+SwitchWFSGlobalTimeoutCnt+ 
//@
    groupname = "SwitchWFSGlobalTimeoutCnt";
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), 0);
        FunctPtr fConv (new FunctUint32(&(updatestatus.switch_wfs_global_timeout_cnt)));
        DiagnVarPtr theVar (new DiagnVar(groupname, 0, fConv, hWarning, hAlarm));
        _vars.Add(theVar); 
    }
//@C
// /verb+SwitchNumFLTimeout+ 
//@
    groupname = "SwitchNumFLTimeout";
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), 0);
        FunctPtr fConv (new FunctUint32(&(updatestatus.switch_num_fl_timeout)));
        DiagnVarPtr theVar (new DiagnVar(groupname, 0, fConv, hWarning, hAlarm));
        _vars.Add(theVar); 
    }
//@C
// /verb+SwitchNumCrcErr+ 
//@
    groupname = "SwitchNumCrcErr";
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), 0);
        FunctPtr fConv (new FunctUint32(&(updatestatus.switch_num_crc_err)));
        DiagnVarPtr theVar (new DiagnVar(groupname, 0, fConv, hWarning, hAlarm));
        _vars.Add(theVar); 
    }
//@C
// /verb+SwitchWFSFrameCounter+ 
//@
    groupname = "SwitchWFSFrameCounter";
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), 0);
        FunctPtr fConv (new FunctUint32(&(updatestatus.switch_scstartrtr)));
        DiagnVarPtr theVar (new DiagnVar(groupname, 0, fConv, hWarning, hAlarm));
        _vars.Add(theVar); 
    }

//@C
// /verb+MeanDSPCoilCurrent+ 
//@
    groupname = "MeanDSPCoilCurrent";
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), 0);
    	DiagnSetPtr dspcoilcurrent = _vars.Filter(DiagnWhich("DSPCoilCurrent",DiagnWhich::All,DiagnWhich::All,DiagnWhich::Fast));
        FunctPtr fConv (new FunctAverageDiagnSet(dspcoilcurrent));
        DiagnVarPtr theVar (new DiagnVar(groupname, 0, fConv, hWarning, hAlarm));
        _vars.Add(theVar); 
    }

//@C
// /verb+HUBTEMP+ 
//@
    groupname = "HUBTEMP";
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), 0);
	DiagnSetPtr Set0 = _vars.Filter(DiagnWhich("BCUCoolerOut0Temp",0,0));
        FunctPtr fConv (new FunctAverageDiagnSet(Set0));
        DiagnVarPtr theVar (new DiagnVar(groupname, 0, fConv, hWarning, hAlarmDump));
        _vars.Add(theVar); 
    }
   
//@C
// /verb+PBPLANETEMP+ 
//@
    groupname = "PBPLANETEMP";
    for (int i=0; i<BcuMirror::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
	DiagnSetPtr Set0 = _vars.Filter(DiagnWhich("BCUCoolerIn0Temp",i,i));
        FunctPtr fConv (new FunctAverageDiagnSet(Set0));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hAlarmDump));
        _vars.Add(theVar);
    } 

//@C
// /verb+CPTEMP+ 
//@
    groupname = "CPTEMP";
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), 0);
        DiagnSetPtr Set0 = _vars.Filter(DiagnWhich("BCUCoolerIn1Temp",3,3));
        DiagnSetPtr Set1 = _vars.Filter(DiagnWhich("BCUCoolerIn1Temp",4,4));
        DiagnSetPtr Set2 = _vars.Filter(DiagnWhich("BCUCoolerIn1Temp",5,5));
        DiagnSetPtr Set  =  (Set0 | Set1) | Set2;

        FunctPtr fConv (new FunctAverageDiagnSet(Set));
        DiagnVarPtr theVar (new DiagnVar(groupname, 0, fConv, hWarning, hAlarmDump));
        _vars.Add(theVar);
    } 

//@C
// /verb+RBODYTEMP+ 
//@
    groupname = "RBODYTEMP";
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), 0);
        DiagnSetPtr Set0 = _vars.Filter(DiagnWhich("BCUCoolerIn1Temp",2,2));
        DiagnSetPtr Set1 = _vars.Filter(DiagnWhich("BCUCoolerOut0Temp",3,3));
        DiagnSetPtr Set2 = _vars.Filter(DiagnWhich("BCUCoolerOut0Temp",5,5));
        DiagnSetPtr Set  =  (Set0 | Set1) | Set2;

        FunctPtr fConv (new FunctAverageDiagnSet(Set));
        DiagnVarPtr theVar (new DiagnVar(groupname, 0, fConv, hWarning, hAlarmDump));
        _vars.Add(theVar);
    } 

//@C
// /verb+INNERSTRTEMP+ 
//@
    groupname = "INNERSTRTEMP";
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), 0);
        DiagnSetPtr Set = _vars.Filter(DiagnWhich("BCUCoolerOut1Temp",5,5));
        FunctPtr fConv (new FunctAverageDiagnSet(Set));
        DiagnVarPtr theVar (new DiagnVar(groupname, 0, fConv, hWarning, hAlarmDump));
        _vars.Add(theVar);
    } 




/*
//@C
// /verb+AdamMainPower+ 
//@
    groupname = "AdamMainPower";
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), 0);
        FunctPtr fConv (new FunctInt32((int32 *)&_adamvec[15]));
        DiagnVarPtr theVar (new DiagnVar(groupname, 0, fConv, hWarning, hAlarm));
       _vars.Add(theVar); 
    }
//@C
// /verb+AdamWatchdogExp+ 
//@
    groupname = "AdamWatchdogExp";
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), 0);
        FunctPtr fConv (new FunctInt32((int32 *)&_adamvec[14]));
        DiagnVarPtr theVar (new DiagnVar(groupname, 0, fConv, hWarning, hAlarm));
       _vars.Add(theVar); 
    }
//@C
// /verb+AdamTSSDisable+ 
//@
    groupname = "AdamTSSDisable";
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), 0);
        FunctPtr fConv (new FunctInt32((int32 *)&_adamvec[12]));
        DiagnVarPtr theVar (new DiagnVar(groupname, 0, fConv, hWarning, hAlarm));
       _vars.Add(theVar); 
    }
//@C
// /verb+AdamDriverEnable+ 
//@
    groupname = "AdamDriverEnable";
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), 0);
        FunctPtr fConv (new FunctInt32((int32 *)&_adamvec[11]));
        DiagnVarPtr theVar (new DiagnVar(groupname, 0, fConv, hWarning, hAlarm));
       _vars.Add(theVar); 
    }
//@C
// /verb+AdamFPGAClearN+ 
//@
    groupname = "AdamFPGAClearN";
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), 0);
        FunctPtr fConv (new FunctInt32((int32 *)&_adamvec[10]));
        DiagnVarPtr theVar (new DiagnVar(groupname, 0, fConv, hWarning, hAlarm));
       _vars.Add(theVar); 
    }
//@C
// /verb+AdamBootSelectN+ 
//@
    groupname = "AdamBootSelectN";
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), 0);
        FunctPtr fConv (new FunctInt32((int32 *)&_adamvec[9]));
        DiagnVarPtr theVar (new DiagnVar(groupname, 0, fConv, hWarning, hAlarm));
       _vars.Add(theVar); 
    }
//@C
// /verb+AdamSysResetN+ 
//@
    groupname = "AdamSysResetN";
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), 0);
        FunctPtr fConv (new FunctInt32((int32 *)&_adamvec[8]));
        DiagnVarPtr theVar (new DiagnVar(groupname, 0, fConv, hWarning, hAlarm));
       _vars.Add(theVar); 
    }
//@C
// /verb+AdamTSSPowerFaultN0+ 
//@
    groupname = "AdamTSSPowerFaultN0";
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), 0);
        FunctPtr fConv (new FunctInt32((int32 *)&_adamvec[0]));
        DiagnVarPtr theVar (new DiagnVar(groupname, 0, fConv, hWarning, hAlarm));
       _vars.Add(theVar); 
    }
//@C
// /verb+AdamTSSPowerFaultN1+ 
//@
    groupname = "AdamTSSPowerFaultN1";
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), 0);
        FunctPtr fConv (new FunctInt32((int32 *)&_adamvec[1]));
        DiagnVarPtr theVar (new DiagnVar(groupname, 0, fConv, hWarning, hAlarm));
       _vars.Add(theVar); 
    }
//@C
// /verb+AdamTSSFaultN+ 
//@
    groupname = "AdamTSSFaultN";
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), 0);
        FunctPtr fConv (new FunctInt32((int32 *)&_adamvec[3]));
        DiagnVarPtr theVar (new DiagnVar(groupname, 0, fConv, hWarning, hAlarm));
       _vars.Add(theVar); 
    }
//@C
// /verb+AdamTCSSysFault+ 
//@
    groupname = "AdamTCSSysFault";
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), 0);
        FunctPtr fConv (new FunctInt32((int32 *)&_adamvec[4]));
        DiagnVarPtr theVar (new DiagnVar(groupname, 0, fConv, hWarning, hAlarm));
       _vars.Add(theVar); 
    }
//@C
// /verb+AdamTCSPowerFaultN0+ 
//@
    groupname = "AdamTCSPowerFaultN0";
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), 0);
        FunctPtr fConv (new FunctInt32((int32 *)&_adamvec[5]));
        DiagnVarPtr theVar (new DiagnVar(groupname, 0, fConv, hWarning, hAlarm));
       _vars.Add(theVar); 
    }
//@C
// /verb+AdamTCSPowerFaultN1+ 
//@
    groupname = "AdamTCSPowerFaultN1";
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), 0);
        FunctPtr fConv (new FunctInt32((int32 *)&_adamvec[6]));
        DiagnVarPtr theVar (new DiagnVar(groupname, 0, fConv, hWarning, hAlarm));
       _vars.Add(theVar); 
    }
//@C
// /verb+AdamTCSPowerFaultN2+ 
//@
    groupname = "AdamTCSPowerFaultN2";
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), 0);
        FunctPtr fConv (new FunctInt32((int32 *)&_adamvec[7]));
        DiagnVarPtr theVar (new DiagnVar(groupname, 0, fConv, hWarning, hAlarm));
       _vars.Add(theVar); 
    }

*/
}


//@Member: GetRawUpdateStatus
// Variables added in Aug09 to replace update_status IDL routine.
//
//@
void HouseKeeper::GetRawUpdateStatus()
{
    int firstDSP, lastDSP;
    bool loop_again=true;

    while(loop_again && TimeToDie() == false){
        if(!_dummy){
            BcuIdMap myMap;
            try{	
                if(_comSender == NULL) _comSender = new CommandSender();

                // read all DSP boards on every crate
                firstDSP=0, lastDSP=BcuMirror::N_DSP_CRATE-1;

                _logger->log(Logger::LOG_LEV_DEBUG, "Reading DSP [%d-%d] from all BCUs [%s:%d]", firstDSP, lastDSP, __FILE__, __LINE__);
                _comSender->sendMultiBcuCommand(Priority::LOW_PRIORITY, myMap["BCU_MIRROR_ALL"],
                    firstDSP, lastDSP, OpCodes::MGP_OP_RDSEQ_DSP, AddressMap::DSP_GLOBAL_COUNTER, 
                    sizeof(uint32), 
                    (unsigned char*)(&(updatestatus.global_counter)),
                    _bcuRequestTimeout_ms, Constants::WANTREPLY_FLAG);

                _comSender->sendMultiBcuCommand(Priority::LOW_PRIORITY, myMap["BCU_MIRROR_ALL"],
                    firstDSP, lastDSP, OpCodes::MGP_OP_RDSEQ_DSP, AddressMap::DSP_FF_UD_CURRENT, 
                    BcuMirror::N_CH_DSP_ *  sizeof(float32), 
                    (unsigned char*)(&(updatestatus.ff_ud_current)),
                    _bcuRequestTimeout_ms, Constants::WANTREPLY_FLAG);
            
                _comSender->sendMultiBcuCommand(Priority::LOW_PRIORITY, myMap["BCU_MIRROR_ALL"],
                    firstDSP, lastDSP, OpCodes::MGP_OP_RDSEQ_DSP, AddressMap::DSP_POS_COMMAND, 
                    BcuMirror::N_CH_DSP_ *  sizeof(float32), 
                    (unsigned char*)(&(updatestatus.pos_command)),
                    _bcuRequestTimeout_ms, Constants::WANTREPLY_FLAG);
            
                _comSender->sendMultiBcuCommand(Priority::LOW_PRIORITY, myMap["BCU_MIRROR_ALL"],
                    firstDSP, lastDSP, OpCodes::MGP_OP_RDSEQ_DSP, AddressMap::DSP_BIAS_CURRENT, 
                    BcuMirror::N_CH_DSP_ *  sizeof(float32), 
                    (unsigned char*)(&(updatestatus.bias_current)),
                    _bcuRequestTimeout_ms, Constants::WANTREPLY_FLAG);
            
                _comSender->sendMultiBcuCommand(Priority::LOW_PRIORITY, myMap["BCU_MIRROR_ALL"],
                    firstDSP, lastDSP, OpCodes::MGP_OP_RDSEQ_DSP, AddressMap::DSP_FULLBIAS_CURR,
                    BcuMirror::N_CH_DSP_ *  sizeof(float32), 
                    (unsigned char*)(&(updatestatus.fullbias_curr)),
                    _bcuRequestTimeout_ms, Constants::WANTREPLY_FLAG);
            
                _comSender->sendMultiBcuCommand(Priority::LOW_PRIORITY, myMap["BCU_MIRROR_ALL"],
                    firstDSP, lastDSP, OpCodes::MGP_OP_RDSEQ_DSP, AddressMap::DSP_STEP_PTR_PRESHAPER_CMD,
                    BcuMirror::N_CH_DSP_ *  sizeof(float32), 
                    (unsigned char*)(&(updatestatus.step_ptr_preshaper_cmd)),
                    _bcuRequestTimeout_ms, Constants::WANTREPLY_FLAG);
            
                _comSender->sendMultiBcuCommand(Priority::LOW_PRIORITY, myMap["BCU_MIRROR_ALL"],
                    firstDSP, lastDSP, OpCodes::MGP_OP_RDSEQ_DSP, AddressMap::DSP_STEP_PTR_PRESHAPER_CURR,
                    BcuMirror::N_CH_DSP_ *  sizeof(float32), 
                    (unsigned char*)(&(updatestatus.step_ptr_preshaper_curr)),
                    _bcuRequestTimeout_ms, Constants::WANTREPLY_FLAG);
            
                _comSender->sendMultiBcuCommand(Priority::LOW_PRIORITY, myMap["BCU_MIRROR_ALL"],
                    firstDSP, lastDSP, OpCodes::MGP_OP_RDSEQ_DSP, AddressMap::DSP_FLOAT_ADC_VALUE,
                    BcuMirror::N_CH_DSP_ *  sizeof(float32), 
                    (unsigned char*)(&(updatestatus.float_adc_value)),
                    _bcuRequestTimeout_ms, Constants::WANTREPLY_FLAG);
                
                _comSender->sendMultiBcuCommand(Priority::LOW_PRIORITY, myMap["BCU_MIRROR_ALL"],
                    firstDSP, lastDSP, OpCodes::MGP_OP_RDSEQ_DSP, AddressMap::DSP_DIST_THRESHOLD,
                    BcuMirror::N_CH_DSP_ *  sizeof(float32), 
                    (unsigned char*)(&(updatestatus.dist_threshold)),
                    _bcuRequestTimeout_ms, Constants::WANTREPLY_FLAG);
            
                _comSender->sendMultiBcuCommand(Priority::LOW_PRIORITY, myMap["BCU_MIRROR_ALL"],
                    firstDSP, lastDSP, OpCodes::MGP_OP_RDSEQ_DSP, AddressMap::DSP_DISTANCE,
                    BcuMirror::N_CH_DSP_ *  sizeof(float32), 
                    (unsigned char*)(&(updatestatus.distance)),
                    _bcuRequestTimeout_ms, Constants::WANTREPLY_FLAG);
            
                _comSender->sendMultiBcuCommand(Priority::LOW_PRIORITY, myMap["BCU_MIRROR_ALL"],
                    firstDSP, lastDSP, OpCodes::MGP_OP_RDSEQ_DSP, AddressMap::DSP_CONTROL_ENABLE,
                    BcuMirror::N_CH_DSP_ *  sizeof(float32), 
                    (unsigned char*)(&(updatestatus.control_enable)),
                    _bcuRequestTimeout_ms, Constants::WANTREPLY_FLAG);
            
                _comSender->sendMultiBcuCommand(Priority::LOW_PRIORITY, myMap["BCU_MIRROR_ALL"],
                    firstDSP, lastDSP, OpCodes::MGP_OP_RDSEQ_DSP, AddressMap::DSP_FLOAT_DAC_VALUE,
                    BcuMirror::N_CH_DSP_ *  sizeof(float32), 
                    (unsigned char*)(&(updatestatus.float_DAC_value)),
                    _bcuRequestTimeout_ms, Constants::WANTREPLY_FLAG);
            
                _comSender->sendMultiBcuCommand(Priority::LOW_PRIORITY, myMap["BCU_MIRROR_ALL"],
                    firstDSP, lastDSP, OpCodes::MGP_OP_RDSEQ_DSP, AddressMap::DSP_ERR_LOOP_GAIN,
                    BcuMirror::N_CH_DSP_ *  sizeof(float32), 
                    (unsigned char*)(&(updatestatus.err_loop_gain)),
                    _bcuRequestTimeout_ms, Constants::WANTREPLY_FLAG);
                
                _comSender->sendMultiBcuCommand(Priority::LOW_PRIORITY, myMap["BCU_MIRROR_ALL"],
                    firstDSP, lastDSP, OpCodes::MGP_OP_RDSEQ_DSP, AddressMap::DSP_SPEED_LOOP_GAIN,
                    BcuMirror::N_CH_DSP_ *  sizeof(float32), 
                    (unsigned char*)(&(updatestatus.speed_loop_gain)),
                    _bcuRequestTimeout_ms, Constants::WANTREPLY_FLAG);
            
                _comSender->sendMultiBcuCommand(Priority::LOW_PRIORITY, myMap["BCU_MIRROR_ALL"],
                    firstDSP, lastDSP, OpCodes::MGP_OP_RDSEQ_DSP, AddressMap::DSP_BIAS_COMMAND,
                    BcuMirror::N_CH_DSP_ *  sizeof(float32), 
                    (unsigned char*)(&(updatestatus.bias_command)),
                    _bcuRequestTimeout_ms, Constants::WANTREPLY_FLAG);
            
                _comSender->sendMultiBcuCommand(Priority::LOW_PRIORITY, myMap["BCU_MIRROR_ALL"],
                    firstDSP, lastDSP, OpCodes::MGP_OP_RDSEQ_DSP, AddressMap::DSP_FULLBIAS_CMD,
                    BcuMirror::N_CH_DSP_ *  sizeof(float32), 
                    (unsigned char*)(&(updatestatus.fullbias_cmd)),
                    _bcuRequestTimeout_ms, Constants::WANTREPLY_FLAG);
            
                _comSender->sendMultiBcuCommand(Priority::LOW_PRIORITY, myMap["BCU_MIRROR_ALL"],
                    firstDSP, lastDSP, OpCodes::MGP_OP_RDSEQ_DSP, AddressMap::DSP_CMOL_A,
                    BcuMirror::N_CH_DSP_ *  sizeof(float32), 
                    (unsigned char*)(&(updatestatus.cmol_a)),
                    _bcuRequestTimeout_ms, Constants::WANTREPLY_FLAG);
                
                _comSender->sendMultiBcuCommand(Priority::LOW_PRIORITY, myMap["BCU_MIRROR_ALL"],
                    firstDSP, lastDSP, OpCodes::MGP_OP_RDSEQ_DSP, AddressMap::DSP_CMOL_B,
                    BcuMirror::N_CH_DSP_ *  sizeof(float32), 
                    (unsigned char*)(&(updatestatus.cmol_b)),
                    _bcuRequestTimeout_ms, Constants::WANTREPLY_FLAG);
            
                _comSender->sendMultiBcuCommand(Priority::LOW_PRIORITY, myMap["BCU_MIRROR_ALL"],
                    firstDSP, lastDSP, OpCodes::MGP_OP_RDSEQ_DSP, AddressMap::DSP_FMOL_A,
                    BcuMirror::N_CH_DSP_ *  sizeof(float32), 
                    (unsigned char*)(&(updatestatus.fmol_a)),
                    _bcuRequestTimeout_ms, Constants::WANTREPLY_FLAG);
                
                _comSender->sendMultiBcuCommand(Priority::LOW_PRIORITY, myMap["BCU_MIRROR_ALL"],
                    firstDSP, lastDSP, OpCodes::MGP_OP_RDSEQ_DSP, AddressMap::DSP_FMOL_B,
                    BcuMirror::N_CH_DSP_ *  sizeof(float32), 
                    (unsigned char*)(&(updatestatus.fmol_b)),
                    _bcuRequestTimeout_ms, Constants::WANTREPLY_FLAG);
            
                _comSender->sendMultiBcuCommand(Priority::LOW_PRIORITY, myMap["BCU_MIRROR_ALL"],
                    firstDSP, lastDSP, OpCodes::MGP_OP_RDSEQ_DSP, AddressMap::DSP_POS_CHECK_CNT,
                    BcuMirror::N_CH_DSP_ *  sizeof(uint32), 
                    (unsigned char*)(&(updatestatus.pos_check_cnt)),
                    _bcuRequestTimeout_ms, Constants::WANTREPLY_FLAG);
            
                _comSender->sendMultiBcuCommand(Priority::LOW_PRIORITY, myMap["BCU_MIRROR_ALL"],
                    firstDSP, lastDSP, OpCodes::MGP_OP_RDSEQ_DSP, AddressMap::DSP_MODAL_SKIPFRAME,
                    sizeof(float32), 
                    (unsigned char*)(&(updatestatus.modal_skipframe)),
                    _bcuRequestTimeout_ms, Constants::WANTREPLY_FLAG);
                
                _comSender->sendMultiBcuCommand(Priority::LOW_PRIORITY, myMap["BCU_MIRROR_ALL"],
                    firstDSP, lastDSP, OpCodes::MGP_OP_RDSEQ_DSP, AddressMap::DSP_COMMAND_SKIPFRAME,
                    sizeof(float32), 
                    (unsigned char*)(&(updatestatus.command_skipframe)),
                    _bcuRequestTimeout_ms, Constants::WANTREPLY_FLAG);
                
                _comSender->sendMultiBcuCommand(Priority::LOW_PRIORITY, myMap["BCU_MIRROR_ALL"],
                    firstDSP, lastDSP, OpCodes::MGP_OP_RDSEQ_DSP, AddressMap::DSP_FORCE_SKIPFRAME,
                    sizeof(float32), 
                    (unsigned char*)(&(updatestatus.force_skipframe)),
                    _bcuRequestTimeout_ms, Constants::WANTREPLY_FLAG);
            
                _comSender->sendMultiBcuCommand(Priority::LOW_PRIORITY, myMap["BCU_SWITCH"],
                    255, 255, OpCodes::MGP_OP_RDSEQ_DSP, AddressMap::SWITCH_PARAM_SELECTOR,
                    sizeof(uint32), 
                    (unsigned char*)(&(updatestatus.switch_param_selector)),
                    _bcuRequestTimeout_ms, Constants::WANTREPLY_FLAG);

                _comSender->sendMultiBcuCommand(Priority::LOW_PRIORITY, myMap["BCU_SWITCH"],
                    255, 255, OpCodes::MGP_OP_RDSEQ_DSP, AddressMap::SWITCH_OFFLOAD_SELECTOR,
                    sizeof(uint32), 
                    (unsigned char*)(&(updatestatus.switch_offload_selector)),
                    _bcuRequestTimeout_ms, Constants::WANTREPLY_FLAG);

                _comSender->sendMultiBcuCommand(Priority::LOW_PRIORITY, myMap["BCU_SWITCH"],
                    255, 255, OpCodes::MGP_OP_RDSEQ_DSP, AddressMap::SWITCH_MIRR_FRAMES_COUNTER,
                    sizeof(uint32), 
                    (unsigned char*)(&(updatestatus.switch_mirr_frames_counter)),
                    _bcuRequestTimeout_ms, Constants::WANTREPLY_FLAG);

                _comSender->sendMultiBcuCommand(Priority::LOW_PRIORITY, myMap["BCU_SWITCH"],
                    255, 255, OpCodes::MGP_OP_RDSEQ_DSP, AddressMap::SWITCH_SAFE_SKIP_FRAME_CNT,
                    sizeof(uint32), 
                    (unsigned char*)(&(updatestatus.switch_safe_skip_frame_cnt)),
                    _bcuRequestTimeout_ms, Constants::WANTREPLY_FLAG);

                _comSender->sendMultiBcuCommand(Priority::LOW_PRIORITY, myMap["BCU_SWITCH"],
                    255, 255, OpCodes::MGP_OP_RDSEQ_DSP, AddressMap::SWITCH_PENDING_SKIP_FRAME_CNT,
                    sizeof(uint32), 
                    (unsigned char*)(&(updatestatus.switch_pending_skip_frame_cnt)),
                    _bcuRequestTimeout_ms, Constants::WANTREPLY_FLAG);

                _comSender->sendMultiBcuCommand(Priority::LOW_PRIORITY, myMap["BCU_SWITCH"],
                    255, 255, OpCodes::MGP_OP_RDSEQ_DSP, AddressMap::SWITCH_WFS_GLOBAL_TIMEOUT,
                    sizeof(uint32), 
                    (unsigned char*)(&(updatestatus.switch_wfs_global_timeout)),
                    _bcuRequestTimeout_ms, Constants::WANTREPLY_FLAG);

                _comSender->sendMultiBcuCommand(Priority::LOW_PRIORITY, myMap["BCU_SWITCH"],
                    255, 255, OpCodes::MGP_OP_RDSEQ_DSP, AddressMap::SWITCH_WFS_GLOBAL_TIMEOUT_CNT,
                    sizeof(uint32), 
                    (unsigned char*)(&(updatestatus.switch_wfs_global_timeout_cnt)),
                    _bcuRequestTimeout_ms, Constants::WANTREPLY_FLAG);

                _comSender->sendMultiBcuCommand(Priority::LOW_PRIORITY, myMap["BCU_SWITCH"],
                    255, 255, OpCodes::MGP_OP_RDSEQ_DSP, AddressMap::SWITCH_NUM_FL_TIMEOUT,
                    sizeof(uint32), 
                    (unsigned char*)(&(updatestatus.switch_num_fl_timeout)),
                    _bcuRequestTimeout_ms, Constants::WANTREPLY_FLAG);

                _comSender->sendMultiBcuCommand(Priority::LOW_PRIORITY, myMap["BCU_SWITCH"],
                    255, 255, OpCodes::MGP_OP_RDSEQ_DSP, AddressMap::SWITCH_NUM_CRC_ERR,
                    sizeof(uint32), 
                    (unsigned char*)(&(updatestatus.switch_num_crc_err)),
                    _bcuRequestTimeout_ms, Constants::WANTREPLY_FLAG);
                
                _comSender->sendMultiBcuCommand(Priority::LOW_PRIORITY, myMap["BCU_SWITCH"],
                    255, 255, OpCodes::MGP_OP_RDSEQ_DSP, AddressMap::SWITCH_SCSTARTRTR,
                    sizeof(uint32), 
                    (unsigned char*)(&(updatestatus.switch_scstartrtr)),
                    _bcuRequestTimeout_ms, Constants::WANTREPLY_FLAG);

                loop_again = false;
                //delete cmdSender;

            } catch(CommandSenderTimeoutException& e) {
                ToggleUnprotected(true);
                _logger->log(Logger::LOG_LEV_ERROR, "Timeout reading BCUs:  %s [%s:%d]", e.what().c_str(), __FILE__, __LINE__);
		        delete _comSender;	// !!! see Bcu::CommandSender::sendMultiBcuCommand(...) !!!
		        _comSender = NULL;
		        //ErrC = TIMEOUT_ERROR;
                sleep(1); 
                continue;
	        }
	        catch(CommandSenderException& e) {
                ToggleUnprotected(true);
                _logger->log(Logger::LOG_LEV_ERROR, "Error reading BCUs:  %s [%s:%d]", e.what().c_str(), __FILE__, __LINE__);
		        //ErrC = COMMUNICATION_ERROR;
                sleep(1); 
                continue;
	        }

       } else { //dummy mode
          //for(int crate_n=0; crate_n<BcuSwitch::N_CRATES; crate_n++) {
          //   bcu_dummy(&raw.switc[crate_n], crate_n);
         // }
          //for(int crate_n=0; crate_n<BcuMirror::N_CRATES; crate_n++) {
          //   bcu_dummy(&raw.crate[crate_n], crate_n);
         // }
         loop_again = false;
       }
       ToggleUnprotected(false);
    }	           
}


//@Member: GetRawData
//
//@
void HouseKeeper::GetRawData()
{
    int firstDSP, lastDSP;
    bool loop_again=true;

    while(loop_again && TimeToDie() == false){

       try {
 
        _logger->log(Logger::LOG_LEV_TRACE, "[Housekeeper::GetRawData] [%s:%d]", __FILE__, __LINE__);

        // read raw memory
        //
        if(!_dummy){
          // TODO qui non c'e' bisogno di creare il CommandSender tutte le volte,
          // ma bisogna ricordarsi di distruggerlo e ricrearlo nel caso di timeout-exception
          // Io per semplicita' (?) distruggo e ricreo sempre.
          CommandSender* cmdSender = new CommandSender();
          BcuIdMap myMap;
          // read all BCU boards on every crate
          try{

             //ReadAdam(_adamvec);
             firstDSP=255, lastDSP=255; //TODO c'e' un define per le BCU da qualche parte?

             _logger->log(Logger::LOG_LEV_DEBUG, "Reading DSP [%d-%d] from BCU_SWITCH [%s:%d]",
                firstDSP, lastDSP, __FILE__, __LINE__);
             cmdSender->sendMultiBcuCommand(Priority::LOW_PRIORITY, myMap["BCU_SWITCH"],
                firstDSP, lastDSP, OpCodes::MGP_OP_RDSEQ_SRAM, AddressMap::BCU_NIOS_FIXED_AREA, 
                sizeof(na_bcu_nios_fixed_area_struct), 
                (unsigned char*)(&(raw.switc)),
                _bcuRequestTimeout_ms, Constants::WANTREPLY_FLAG);

             _logger->log(Logger::LOG_LEV_DEBUG, "Reading DSP [%d-%d] from BCU_MIRROR_ALL [%s:%d]",
                firstDSP, lastDSP, __FILE__, __LINE__);
             cmdSender->sendMultiBcuCommand(Priority::LOW_PRIORITY, myMap["BCU_MIRROR_ALL"],
                firstDSP, lastDSP, OpCodes::MGP_OP_RDSEQ_SRAM, AddressMap::BCU_NIOS_FIXED_AREA, 
                sizeof(na_bcu_nios_fixed_area_struct), 
                (unsigned char*)(&(raw.crate)),
                _bcuRequestTimeout_ms, Constants::WANTREPLY_FLAG);
            
             delete cmdSender;

             //Get raw data for update status
             GetRawUpdateStatus() ;

             loop_again = false;
             _sample_counter++;

          } catch (CommandSenderException &e) {
             ToggleUnprotected(true);
             _logger->log(Logger::LOG_LEV_ERROR, "Error reading BCUs:  %s [%s:%d]",
                e.what().c_str(), __FILE__, __LINE__);
             delete cmdSender;
             msleep(1000); 
             continue;
          }
       } else { //dummy mode
          for(int crate_n=0; crate_n<BcuSwitch::N_CRATES; crate_n++) {
             bcu_dummy(&raw.switc[crate_n], crate_n);
          }
          for(int crate_n=0; crate_n<BcuMirror::N_CRATES; crate_n++) {
             bcu_dummy(&raw.crate[crate_n], crate_n);
          }
         loop_again = false;
       }
       ToggleUnprotected(false);

       }
      catch(AOException &e) {
         _logger->log( Logger::LOG_LEV_ERROR, "Exception!! %s", e.what().c_str());
         msleep(1000);
         continue;
      }
    }	                
}


/*
void HouseKeeper::ReadAdam(int *array)
{
   //@C
   // Read (every cycle) the current ADAM status in input
   //@

   char cmd[] = "$016\r";
   char ans[16];
   const int cmdLen = 5;
   const int ansLen = 10;

   if(!_dummy) {

      UdpConnection* _adamConn = _adam->getUdpConnection();
      try {
         _logger->log(Logger::LOG_LEV_DEBUG, "Command sent to Adam: %s", cmd);
         _adamConn->send((BYTE*)cmd, cmdLen);
      }
      catch (UdpFatalException& e) {
         _logger->log(Logger::LOG_LEV_DEBUG, "Command sent to Adam: %s",e.what().c_str(),__FILE__,__LINE__);
         throw;
      }
      try {
         _adamConn->receive((BYTE *)ans, ansLen);
         ans[ansLen-1]='\0';
      }
      catch (UdpFatalException& e) {
         _logger->log(Logger::LOG_LEV_DEBUG, "Exception: answer from Adam: %s", ans);
         throw;
      }

      _logger->log(Logger::LOG_LEV_DEBUG, "Answer from Adam: %s", ans);

      //Answer verification
      string answer = string(ans);
      string ver = (answer.substr(0,3));
      if(strncmp((const char*)(ver.c_str()),"!01", 3) != 0) {
         _logger->log(Logger::LOG_LEV_DEBUG, "Verification pattnern: %s", ver.c_str());
         _logger->log(Logger::LOG_LEV_DEBUG, "Bad answer from Adam: %s", ans);
//          throw UdpFatalException("Error answer to ADAM ");
      }

      string values = answer.substr(5, answer.size()-1);

      _logger->log(Logger::LOG_LEV_DEBUG, "Value answer from Adam: %s", (const char*)values.c_str());
      unsigned int x = strtol((const char *)(values.c_str()), NULL, 16);
      _logger->log(Logger::LOG_LEV_INFO, "Integer Adam status: %d", x);

      for (int i=0; i<=15; i++) {
         int bit = ((x >> i) & 1) ;
         array[i]=bit;
      }
      _logger->log(Logger::LOG_LEV_DEBUG, "Adam in status: %d:%d:%d:%d:%d:%d:%d:%d", array[0], array[1], array[2], array[3], array[4], array[5], array[6], array[7]);
      _logger->log(Logger::LOG_LEV_DEBUG, "Adam out status: %d:%d:%d:%d:%d:%d:%d:%d", array[8], array[9], array[10], array[11], array[12], array[13], array[14], array[15]);
   }


}
*/


//@Member: GetFrameCounter
// 
//@
unsigned int HouseKeeper::GetFrameCounter()
{
    return _sample_counter;
}

//@Member: PostInit
//
//@
void HouseKeeper::PostInit()
{
	
	// Set the thrdlib time to leave (TTL) for the incoming queue.
	// Because the TTL is equal to MirrorCtrl timeout, if the MirrorCtrl
	// reply slowly (i.e. because of BCUs off), the thrdlib will clean 
	// unreceived messages and prevent the queue overflow.
	int thMsgTTL_sec = ConfigDictionary()["timeout_sec"];
	thDefTTL(thMsgTTL_sec);

    ReadNios();
}

//@Member: ReadNios
//
//@
void HouseKeeper::ReadNios()
{
	
    //@C
    // Read (once and for all?) the nios fixed ares of the DSP boards.
    //@
    
    int firstDSP, lastDSP;
    bool loop_again=true;

    while(loop_again && TimeToDie() == false){

        _logger->log(Logger::LOG_LEV_TRACE, "[Housekeeper::ReadNios] [%s:%d]", __FILE__, __LINE__);

        // read raw memory
        //
        if(!_dummy){
            // TODO qui non c'e' bisogno di creare il CommandSender tutte le volte,
            // ma bisogna ricordarsi di distruggerlo e ricrearlo nel caso di timeout-exception
            CommandSender* cmdSender = new CommandSender();
            BcuIdMap myMap;
            try{
                // read all DSP boards on every crate
                firstDSP=0, lastDSP=BcuMirror::N_DSP_CRATE-1;

                _logger->log(Logger::LOG_LEV_TRACE, "Reading DSP [%d-%d] address %p from BCU_MIRROR_ALL [%s:%d]",
                    firstDSP, lastDSP, AddressMap::DSP_NIOS_FIXED_AREA, __FILE__, __LINE__);
                cmdSender->sendMultiBcuCommand(Priority::LOW_PRIORITY, myMap["BCU_MIRROR_ALL"],
                    firstDSP, lastDSP, OpCodes::MGP_OP_RDSEQ_SRAM, AddressMap::DSP_NIOS_FIXED_AREA, 
                    sizeof(na_dsp_nios_fixed_area_struct), 
                    (unsigned char*)(dsp_nios),
                    _bcuRequestTimeout_ms, Constants::WANTREPLY_FLAG);
            
                firstDSP=252, lastDSP=252;  //TODO c'e' un define per la siggen da qualche parte?

                _logger->log(Logger::LOG_LEV_TRACE, "Reading SGN [%d-%d] address %p from BCU_MIRROR_ALL [%s:%d]",
                    firstDSP, lastDSP, AddressMap::DSP_NIOS_FIXED_AREA, __FILE__, __LINE__);
                
                ////////////////// BEGIN PATCH FOR ACCELEROMETER/SIGGEN BOARDS EVIL MIXTURE ////////////////
                // Split the multi bcu command in BcuMirror::N_CRATES bcu commands
                int subDataLenBytes = sizeof(na_dsp_nios_fixed_area_struct);// / BcuMirror::N_CRATES;
                int	firstBcu = myMap["BCU_MIRROR_0"];
                int lastBcu  = myMap["BCU_MIRROR_0"] + BcuMirror::N_CRATES - 1;

                for(int crate = firstBcu; crate <= lastBcu; crate++) {
                    int sigGenDsp = BcuMirror::SIGGEN_BOARD_ID; // Default value
                    // Check if is a crate with alternative siggen id
                    if ( (crate-firstBcu) % BcuMirror::ACCELER_CRATE_NEXT == BcuMirror::ACCELER_CRATE_FIRST) sigGenDsp = BcuMirror::SIGGEN_BOARD_ID_ALT; 
                    // Send the single-crate bcu command
                    cmdSender->sendMultiBcuCommand(Priority::LOW_PRIORITY, crate, crate, sigGenDsp, sigGenDsp, 
                                                    OpCodes::MGP_OP_RDSEQ_SRAM, AddressMap::DSP_NIOS_FIXED_AREA, subDataLenBytes, 
                                                    &( ((unsigned char*)(sgn_nios))[(crate-firstBcu)*subDataLenBytes] ),
                                                    _bcuRequestTimeout_ms, Constants::WANTREPLY_FLAG);
                }
                ////////////////// END PATCH FOR ACCELEROMETER/SIGGEN BOARDS EVIL MIXTURE ////////////////
                loop_again = false;
                delete cmdSender;

            } catch (CommandSenderException &e) {
                ToggleUnprotected(true);
                _logger->log(Logger::LOG_LEV_ERROR, "Error reading NIOS DSP and SGN from BCU_MIRROR_ALL:  %s [%s:%d]",
                    e.what().c_str(), __FILE__, __LINE__);
                delete cmdSender;
                msleep(1000); 
                continue;
            }

        } else { //dummy mode
            for(int dsb_n=0; dsb_n<BcuMirror::N_DSB; dsb_n++) {
             dsp_nios_dummy(&dsp_nios[dsb_n], dsb_n);
            }
            for(int sgn_n=0; sgn_n<BcuMirror::N_SGN; sgn_n++) {
             dsp_nios_dummy(&sgn_nios[sgn_n], sgn_n);
            }
            loop_again = false;
        }
        ToggleUnprotected(true);
    }
    for (int dsb_n=0; dsb_n < BcuMirror::N_DSB; dsb_n++){
      _logger->log(Logger::LOG_LEV_TRACE, "DSP NIOS WhoAmI %d [%s:%d]", dsp_nios[dsb_n].who_ami,  __FILE__, __LINE__);
      _logger->log(Logger::LOG_LEV_TRACE, "DSP NIOS Serial %d [%s:%d]", dsp_nios[dsb_n].serial_number,  __FILE__, __LINE__);
    }
}


//@Member: InSlowLoop
//
//@
void HouseKeeper::InSlowLoop()
{
}

//@Member: InFastLoop
//
//@
void HouseKeeper::InFastLoop()
{
//@C
// Sleep for a while as set in the configuration file
//@
    float pp = ConfigDictionary()["PollingPeriod"];
    nusleep( (unsigned int)(pp*1e6));

//@C
// If needed, change thresholds of BCUCoolerIn according to calculated DewPoint 
//@
// double dewpoint = (_vars.Find(MatchDiagnVar("DewPoint",0)))->Value();
//   _logger->log(Logger::LOG_LEV_TRACE, "Setting BCUCoolerIn1Temp0 alarm_min to %g  [%s:%d]", dewpoint, __FILE__, __LINE__);
//   DiagnParam p;
//   p.SetWarningMin(dewpoint+3.0); 
//   p.SetAlarmMin(dewpoint);
//   (_vars.Find(MatchDiagnVar("BCUCoolerIn1Temp",0)))->ImportParams(p);


    // Check whether any variable got an alarm, and dump.
    // Dump again only when the alarm condition changed or MIN_TIME_BTWDUMP has passed.

    static long prevdumpAlarm = 0;
    static timeval prevdumpTime = {0,0};
    
    timeval timenow;
    gettimeofday(&timenow, NULL);

    long dumpalarm = getDumpAlarmAndClear();

    if ((dumpalarm) && (dumpalarm != prevdumpAlarm)) {
        // force telemetry update
        _logger->log(Logger::LOG_LEV_INFO, "forcing telemetry output due to dump signal [%s]", __FILE__);
        _telemetry->LogTelemetry( DiagnApp::GetTimeStamp(), true);
        
        if (timenow.tv_sec - prevdumpTime.tv_sec > HOUSEKEEPER_MIN_TIME_BTWDUMP) {
            try {
                // force telemetry update
                _logger->log(Logger::LOG_LEV_INFO, "forcing telemetry output due to dump signal [%s]", __FILE__);
                _telemetry->LogTelemetry( DiagnApp::GetTimeStamp(), true);
                
                gettimeofday(&prevdumpTime, NULL);
                // if its a different alarm, dump master diagnostic data
                if (_mastdiagninterface != NULL) {
                    _mastdiagninterface->dumpAdSecDiagnHistory();
                    _logger->log(Logger::LOG_LEV_INFO, "dumpadsecdiagnhistory sent to masterdiagnostic [%s:%d]",__FILE__, __LINE__);
                }
            } catch (MastDiagnInterfaceException& e) {
                _logger->log(Logger::LOG_LEV_ERROR, "dumpadsecdiagnhistory not sent! [%s:%d]", __FILE__, __LINE__);
            } catch (...) {
                _logger->log(Logger::LOG_LEV_ERROR, "unexpected exception:  [%s:%d]",  __FILE__, __LINE__);
            }
        }
    }
    prevdumpAlarm = dumpalarm;
}    

void HouseKeeper::Periodic()
{
    /*try {
        _logger->log(Logger::LOG_LEV_DEBUG, "%s",(*(_vars.Find(MatchDiagnVar("BCUFramesCounter",0)))).str().c_str() ) ;
        _logger->log(Logger::LOG_LEV_DEBUG, "%s",(*(_vars.Find(MatchDiagnVar("BCUStratixTemp",0)))).str().c_str() ) ;
        _logger->log(Logger::LOG_LEV_DEBUG, "%s",(*(_vars.Find(MatchDiagnVar("BCUPowerTemp",0)))).str().c_str() ) ;
        _logger->log(Logger::LOG_LEV_DEBUG, "%s",(*(_vars.Find(MatchDiagnVar("SGNStratixTemp",0)))).str().c_str() ) ;
        _logger->log(Logger::LOG_LEV_DEBUG, "%s",(*(_vars.Find(MatchDiagnVar("DSPStratixTemp",0)))).str().c_str() ) ;
        _logger->log(Logger::LOG_LEV_DEBUG, "%s",(*(_vars.Find(MatchDiagnVar("DSPPowerTemp",0)))).str().c_str() ) ;
        _logger->log(Logger::LOG_LEV_DEBUG, "%s",(*(_vars.Find(MatchDiagnVar("DSPDspsTemp",0)))).str().c_str() ) ;
        _logger->log(Logger::LOG_LEV_DEBUG, "%s",(*(_vars.Find(MatchDiagnVar("DSPDriverTemp",0)))).str().c_str() ) ;
        _logger->log(Logger::LOG_LEV_DEBUG, "%s",(*(_vars.Find(MatchDiagnVar("DSPCoilCurrent",0)))).str().c_str() ) ;
        _logger->log(Logger::LOG_LEV_DEBUG, "%s",(*(_vars.Find(MatchDiagnVar("BCUVoltageVCCL",0)))).str().c_str() ) ;
        _logger->log(Logger::LOG_LEV_DEBUG, "%s",(*(_vars.Find(MatchDiagnVar("BCUVoltageVCCA",0)))).str().c_str() ) ;
        _logger->log(Logger::LOG_LEV_DEBUG, "%s",(*(_vars.Find(MatchDiagnVar("BCUVoltageVSSA",0)))).str().c_str() ) ;
        _logger->log(Logger::LOG_LEV_DEBUG, "%s",(*(_vars.Find(MatchDiagnVar("BCUVoltageVCCP",0)))).str().c_str() ) ;
        _logger->log(Logger::LOG_LEV_DEBUG, "%s",(*(_vars.Find(MatchDiagnVar("BCUVoltageVSSP",0)))).str().c_str() ) ;
        _logger->log(Logger::LOG_LEV_DEBUG, "%s",(*(_vars.Find(MatchDiagnVar("BCUCurrentVCCL",0)))).str().c_str() ) ;
        _logger->log(Logger::LOG_LEV_DEBUG, "%s",(*(_vars.Find(MatchDiagnVar("BCUCurrentVCCA",0)))).str().c_str() ) ;
        _logger->log(Logger::LOG_LEV_DEBUG, "%s",(*(_vars.Find(MatchDiagnVar("BCUCurrentVSSA",0)))).str().c_str() ) ;
        _logger->log(Logger::LOG_LEV_DEBUG, "%s",(*(_vars.Find(MatchDiagnVar("BCUCurrentVCCP",0)))).str().c_str() ) ;
        _logger->log(Logger::LOG_LEV_DEBUG, "%s",(*(_vars.Find(MatchDiagnVar("BCUCurrentVSSP",0)))).str().c_str() ) ;
        _logger->log(Logger::LOG_LEV_DEBUG, "%s",(*(_vars.Find(MatchDiagnVar("TotalCurrentVCCP",0)))).str().c_str() ) ;
        _logger->log(Logger::LOG_LEV_DEBUG, "%s",(*(_vars.Find(MatchDiagnVar("TotalCurrentVSSP",0)))).str().c_str() ) ;
        _logger->log(Logger::LOG_LEV_DEBUG, "%s",(*(_vars.Find(MatchDiagnVar("TotalCurrentVP",0)))).str().c_str() ) ;
        _logger->log(Logger::LOG_LEV_DEBUG, "%s",(*(_vars.Find(MatchDiagnVar("BCUCoolerIn0Temp",0)))).str().c_str() ) ;
        _logger->log(Logger::LOG_LEV_DEBUG, "%s",(*(_vars.Find(MatchDiagnVar("BCUCoolerIn1Temp",0)))).str().c_str() ) ;
        _logger->log(Logger::LOG_LEV_DEBUG, "%s",(*(_vars.Find(MatchDiagnVar("BCUCoolerOut0Temp",0)))).str().c_str() ) ;
        _logger->log(Logger::LOG_LEV_DEBUG, "%s",(*(_vars.Find(MatchDiagnVar("BCUCoolerOut1Temp",0)))).str().c_str() ) ;
        _logger->log(Logger::LOG_LEV_DEBUG, "%s",(*(_vars.Find(MatchDiagnVar("BCUDigitalIODriverEnabled",0)))).str().c_str() ) ;
        _logger->log(Logger::LOG_LEV_DEBUG, "%s",(*(_vars.Find(MatchDiagnVar("BCUDigitalIOCrateIsMaster",0)))).str().c_str() ) ;
        _logger->log(Logger::LOG_LEV_DEBUG, "%s",(*(_vars.Find(MatchDiagnVar("BCUDigitalIOBusSysFault",0)))).str().c_str() ) ;
        _logger->log(Logger::LOG_LEV_DEBUG, "%s",(*(_vars.Find(MatchDiagnVar("ExternalTemperature",0)))).str().c_str() ) ;
        _logger->log(Logger::LOG_LEV_DEBUG, "%s",(*(_vars.Find(MatchDiagnVar("ExternalHumidity",0)))).str().c_str() ) ;
        _logger->log(Logger::LOG_LEV_DEBUG, "%s",(*(_vars.Find(MatchDiagnVar("DewPoint",0)))).str().c_str() ) ;
    } catch (DiagnSetException &e) {
            _logger->log(Logger::LOG_LEV_DEBUG, "%s", e.what().c_str());
    }
    try {
        DiagnSetPtr vars2log  = _vars.Filter(DiagnWhich("DSPStratixTemp", -1, -1));
        vector<double> vvv = vars2log->GetValues() ;
        _logger->log_telemetry(Logger::LOG_LEV_DEBUG,  vvv ) ;
    } catch (DiagnSetException &e) {
        _logger->log(Logger::LOG_LEV_DEBUG, "%s", e.what().c_str());
    }*/
}

//@Member: 
//
//@
#include <stdlib.h>
//static 
double HouseKeeper::randi(double a, double b)
{
    return a + ( (b-a)*rand()/(RAND_MAX+1.0));
}

#define SetBit(dato,n) (dato=((1<<n)|dato))     /* Set bit n high in word dato                     */

//@Member:
//
//@
uint16 HouseKeeper::invConvertGain(double value, const float *coeff){
    float y;
    if (coeff[0] == 0) {
        y = (value-coeff[2])/coeff[1];
    } else {
        y = (-coeff[1] + sqrt( coeff[1]*coeff[1] - 4 * coeff[0] * (coeff[2]-value)) ) / (2 * coeff[0]);
    }
    return (uint16)y;
}



//@Entry: bcu_dummy
// Fill randomly a bcu raw structure
//  
//@
#include "HouseKeeperGain.h"
#include "HouseKeeperRawStruct.h"
void HouseKeeper::bcu_dummy(na_bcu_nios_fixed_area_struct  *raw, int crate_n)
{
    static int frames_counter = 0;
    raw->pb_serial_number             =  7; //No real backplane has serial number = 0
    raw->crateID                      =  crate_n;
    raw->who_ami                      =  255;
    raw->software_release             =  0x04100101;
    raw->logic_release                =  0x0102;
    strncpy((char*)raw->mac_address,"ABCDEF",6);
    strncpy((char*)raw->ip_address,"\012\012\001\022",4);
    strncpy((char*)raw->crate_configuration,
           "\377\374\2\4\6\10\12\14\16\20\22\24\26\30\32\34\36\40\42\44",20);
    raw->local_current_threshold            =  invConvertGain(11.0,    AdSec::gain_local_current_threshold[raw->pb_serial_number]);
    raw->vp_set                             =  invConvertGain(5.5,     AdSec::gain_vp_set[raw->pb_serial_number]);
    raw->total_current_thres_pos            =  invConvertGain(11.0,    AdSec::gain_total_current_thres_pos[raw->pb_serial_number]);
    raw->total_current_thres_neg            =  invConvertGain(-11.0,   AdSec::gain_total_current_thres_neg[raw->pb_serial_number]);

    raw->frames_counter               = frames_counter;
    if (raw->crateID == 0) frames_counter++;
    raw->relais_board_out             = 13;
    raw->serial_number                = 1;
    raw->relais_board_out_1           = 19;
    raw->enable_hl_commands           = 0;
    raw->enable_pos_acc               = 1;
    raw->diagnostic_record_ptr        = 0x13718;
    raw->diagnostic_record_len        = 255;
    raw->enable_master_diag           = 1;
    raw->decimation_factor            = 1;

    strncpy((char*)raw->remote_mac_address,"TYURIE",6);
    strncpy((char*)raw->remote_ip_address,"\022\032\004\012",4);
    raw->remote_udp_port              = 63456;
    raw->rd_diagnostic_record_ptr     = 0x80000;
    raw->wr_diagnostic_record_ptr     = 0x90000;
    raw->rd_byte_serialAIA            = 0;     // numero di byte ricevuti dalla seriale AIA
    raw->wr_byte_serialAIA            = 0;     // numero di byte da spedire alla seriale AIA mettere ad uno il bit 31 per partire
    
    raw->BCU_diagnostic.stratix_temp       =  (uint16)(randi(40.0, 50.0) / 0.0078125);
    raw->BCU_diagnostic.power_temp         =  (uint16)(randi(35.0, 40) / 0.0078125);
    
    if (crate_n == 0) SetBit(raw->BCU_diagnostic.bck_digitalIO,3);
    SetBit(raw->BCU_diagnostic.bck_digitalIO,4);
    SetBit(raw->BCU_diagnostic.bck_digitalIO,12);
    raw->BCU_diagnostic.bck_digitalIO = ((crate_n<<14)|raw->BCU_diagnostic.bck_digitalIO); 

    raw->BCU_diagnostic.voltage_vccl       =  invConvertGain(randi(3.4,3.5),       AdSec::gain_voltage_vccl[raw->pb_serial_number]);
    raw->BCU_diagnostic.voltage_vcca       =  invConvertGain(randi(11.5,12.0),       AdSec::gain_voltage_vcca[raw->pb_serial_number]);
    raw->BCU_diagnostic.voltage_vssa       =  invConvertGain(randi(-12.0,-11.5),   AdSec::gain_voltage_vssa[raw->pb_serial_number]);
    raw->BCU_diagnostic.voltage_vccp       =  invConvertGain(randi(10.5,11),       AdSec::gain_voltage_vccp[raw->pb_serial_number]);
    raw->BCU_diagnostic.voltage_vssp       =  invConvertGain(randi(-11,-10.5),     AdSec::gain_voltage_vssp[raw->pb_serial_number]);
    raw->BCU_diagnostic.current_vccl       =  invConvertGain(randi(1.0,1.2),       AdSec::gain_current_vccl[raw->pb_serial_number]);
    raw->BCU_diagnostic.current_vcca       =  invConvertGain(randi(5.0,6.0),       AdSec::gain_current_vcca[raw->pb_serial_number]);
    raw->BCU_diagnostic.current_vssa       =  invConvertGain(randi(-5.0,-6.0),     AdSec::gain_current_vssa[raw->pb_serial_number]);
    raw->BCU_diagnostic.current_vccp       =  invConvertGain(randi(10.0,12.0),     AdSec::gain_current_vccp[raw->pb_serial_number]);
    raw->BCU_diagnostic.current_vssp       =  invConvertGain(randi(-10.0,-12.0),   AdSec::gain_current_vssp[raw->pb_serial_number]);
    raw->BCU_diagnostic.tot_current_vccp   =  invConvertGain(randi(10.0,12.0),     AdSec::gain_total_current_vccp[raw->pb_serial_number]);
    raw->BCU_diagnostic.tot_current_vssp   =  invConvertGain(randi(-10.0,-12.0),   AdSec::gain_total_current_vssp[raw->pb_serial_number]);
    raw->BCU_diagnostic.tot_current_vp     =  invConvertGain(randi(10.0,12.0),     AdSec::gain_total_current_vp[raw->pb_serial_number]);
    raw->BCU_diagnostic.in0_temp           =  invConvertGain(randi(15.0,18.0),     AdSec::gain_in0_temp[raw->pb_serial_number]);
    raw->BCU_diagnostic.in1_temp           =  invConvertGain(randi(15.0,18.0),     AdSec::gain_in1_temp[raw->pb_serial_number]);
    raw->BCU_diagnostic.out0_temp          =  invConvertGain(randi(19.5,19.5),     AdSec::gain_out0_temp[raw->pb_serial_number]);
    raw->BCU_diagnostic.out1_temp          =  invConvertGain(randi(18.0,20.0),     AdSec::gain_out1_temp[raw->pb_serial_number]);
    raw->BCU_diagnostic.ext_umidity        =  invConvertGain(randi(40.0,80.0),     AdSec::gain_ext_umidity[raw->pb_serial_number]);
    raw->BCU_diagnostic.pressure           =  invConvertGain(randi(2.0, 4.0),  AdSec::gain_pressure[raw->pb_serial_number]);
    raw->BCU_diagnostic.reset_status       =  0;
    
    for(int nb=0; nb < NUM_BOARD_ON_CRATE-1; nb++)
    {
        raw->DSP_diagnostic[nb].stratix_temp =  (uint16)(randi(40.0, 50.0) / 0.0078125);
        raw->DSP_diagnostic[nb].power_temp   =  (uint16)(randi(25.0, 30.0) / 0.0078125);
        raw->DSP_diagnostic[nb].dsps_temp    =  (uint16)(randi(25.0, 30.0) / 0.0078125);
        raw->DSP_diagnostic[nb].driver_temp  =  (uint16)(randi(25.0, 30.0) / 0.0078125);
        for (int i=0; i<8; i++){
            raw->DSP_diagnostic[nb].coil_current[i] = (uint16)( (randi(-1, 1) + 1.463) / 0.0012);
        }
    }
}

//@Entry: dsp_nios_dummy
// Fill randomly a dsp raw structure
//  
//@
#include "HouseKeeperGain.h"
#include "HouseKeeperRawStruct.h"
void HouseKeeper::dsp_nios_dummy(na_dsp_nios_fixed_area_struct  *raw, int dsb_n)
{
    int i;
    raw->who_ami                      =  dsb_n%12;
    raw->logic_release                =  0x0402;
    raw->software_release             =  0x08200301;
    raw->serial_number                =  dsb_n;
    for (i=0; i<8; i++){
        raw->ADC_spi_offset[i] = -1.463;
        raw->ADC_spi_gain[i]   = 0.0012;
        raw->ADC_offset[i]     = 0.2;
        raw->ADC_gain[i]       = 4.0;
        raw->DAC_offset[i]     = 0.3;
        raw->DAC_gain[i]       = 6.0;
    }
    raw->diagnostic_record_ptr        = 0x18342;
    raw->diagnostic_record_len        = 255;
    raw->rd_diagnostic_record_ptr     = 0x80000;
    raw->wr_diagnostic_record_ptr     = 0x100000;
}




