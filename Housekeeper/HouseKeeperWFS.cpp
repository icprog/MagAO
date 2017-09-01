//@File: HouseKeeperWFS.cpp
//
// Implementation of the HouseKeeperWFS class
//@

#include <cstdio>
#include <cstring>
#include <sstream>


extern "C" {
#include "base/thrdlib.h"    
#include "iolib.h" //ReadBinaryFile
#include "commlib.h"
}

#include "HouseKeeperWFS.h"
#include "HouseKeeperGain.h"
#include "Timing.h"
#include "RTDBvar.h"



// Initialize static members
//int HouseKeeperWFS::use_common =0;
//        

//@Member HouseKeeperWFS
//
//Standard constructor
//@

HouseKeeperWFS::HouseKeeperWFS(int argc, char **argv) throw (AOException) : 
    DiagnApp(argc, argv) 
{
   CreateFD();
}



//HouseKeeperWFS::HouseKeeperWFS(const std::string& conffile, int verb )
//    : DiagnApp::DiagnApp(conffile, verb)
void HouseKeeperWFS::CreateFD()
{
    ReadConfig(); 
    //_unprotected = true;
    _bcuRequestTimeout_ms=(int)ConfigDictionary()["timeout_sec"]*1000; 
}

//@Member $\sim$ HouseKeeperWFS
//
//Destructor
//@
HouseKeeperWFS::~HouseKeeperWFS()
{
}

//@Member: ReadConfig
//
//@
void HouseKeeperWFS::ReadConfig()
{
    _dummy = (bool) ( (int)(ConfigDictionary()["DummyMode"]));
    if(_dummy) _logger->log(Logger::LOG_LEV_WARNING, "PROGRAM IS RUNNING IN DUMMY MODE [%s:%d]",__FILE__,__LINE__);

    // insert stuff here
}


//@Member: SetupVars
//
//@
void HouseKeeperWFS::SetupVars()
{
    try{
        DiagnApp::SetupVars();

        string bcu39stratix = "Bcu39 Stratix";
        string bcu39power   = "Bcu39 Power";

        varBcu39stratix_name  = RTDBvar( MyFullName(), "TEMP1.NAME", NO_DIR, CHAR_VARIABLE, bcu39stratix.size()+1);
        varBcu39stratix_value = RTDBvar( MyFullName(), "TEMP1.VALUE", NO_DIR, REAL_VARIABLE);
        varBcu39power_name    = RTDBvar( MyFullName(), "TEMP2.NAME", NO_DIR, CHAR_VARIABLE, bcu39power.size()+1);
        varBcu39power_value   = RTDBvar( MyFullName(), "TEMP2.VALUE", NO_DIR, REAL_VARIABLE);

        varBcu39stratix_name.Set( bcu39stratix);
        varBcu39power_name.Set( bcu39power);

        string HVtemperature = "HV board";
        varHVtemperature_name    = RTDBvar( MyFullName(), "TEMP3.NAME", NO_DIR, CHAR_VARIABLE, HVtemperature.size()+1);
        varHVtemperature_value   = RTDBvar( MyFullName(), "TEMP3.VALUE", NO_DIR, REAL_VARIABLE);
        varHVtemperature_name.Set(HVtemperature);

    } catch(AOException &e){
        _logger->log(Logger::LOG_LEV_ERROR, "%s [%s:%d]",e.what().c_str(),__FILE__,__LINE__);
        // TODO FATAL or ERROR. Eventually die
    }
}

//@Member: InstallHandlers
//
//@
void HouseKeeperWFS::InstallHandlers()
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
void HouseKeeperWFS::CreateDiagnVars()
{


    std::string          groupname;
    //DiagnRange<double>   rng(-2, -1 , 1, 2);
    //size_type            run_mean_len=100;
    //unsigned int         cons_allow_faults=0;
    FunctActionPtr       hAlarm   (new FunctAlarm());                   
    FunctActionPtr       hWarning (new FunctWarning());
//    FunctActionPtr       hEmergencyStop   (new FunctEmergencyStop(ConfigDictionary()["SirenFile"]));
//    FunctActionPtr       hEmergencyStopShutdown   (new FunctEmergencyStop(ConfigDictionary()["SirenFile"], true));
    FunctActionPtr       hEmergencyStop   (new FunctWarning());
    FunctActionPtr       hEmergencyStopShutdown   (new FunctWarning());

    ///////////////////////////// WGN Nios Fixed ////////////////////////////
    // WGN nios fixed area are only read at start-up.
    //

    groupname = "WGNWhoAmI";
    for (int i=0; i<Bcu47::N_WGN; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctUint16(&(wgn_nios[i].who_ami)));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStop));
        _vars.Add(theVar);
    }

//@C
// DSPLogicRelease 
//@
    // (raw->logic_release >> 8) & 0xFF);                // PIO versione logica Major
    // (raw->logic_release & 0xFF);                      // PIO versione logica Minor
    // print, ishft(x,-8) AND 255, ishft(x,0) AND 255
    groupname = "WGNLogicRelease";
    for (int i=0; i<Bcu47::N_WGN; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctUint16(&(wgn_nios[i].logic_release)));
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
    groupname = "WGNSoftwareRelease";
    for (int i=0; i<Bcu47::N_WGN; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctUint32(&(wgn_nios[i].software_release)));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStop));
        _vars.Add(theVar);
    } 

//@C
// DSPSerialNumber
//@
    groupname = "WGNSerialNumber";
    for (int i=0; i<Bcu47::N_WGN; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctUint16(&(wgn_nios[i].serial_number)));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStop));
        _vars.Add(theVar);
    }

    ///////////////////////////// BCU Nios Fixed ////////////////////////////

    /////////////// BCUCrateID ///////////////////
    groupname = "Bcu39CrateID";
    for (int i=0; i<Bcu39::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctUint16(&(raw.bcu39[i].crateID)));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStop));
        _vars.Add(theVar);
    } 
    groupname = "Bcu47CrateID";
    for (int i=0; i<Bcu47::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctUint16(&(raw.bcu47[i].crateID)));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStop));
        _vars.Add(theVar);
    } 

    /////////////// BCUWhoAmI ///////////////////
    // 255 = BCU
    // 252 = SIGGEN
    // 0-x = DSP
    groupname = "Bcu39WhoAmI";
    for (int i=0; i<Bcu39::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctUint16(&(raw.bcu39[i].who_ami)));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStop));
        _vars.Add(theVar);
    } 
    groupname = "Bcu47WhoAmI";
    for (int i=0; i<Bcu47::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctUint16(&(raw.bcu47[i].who_ami)));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStop));
        _vars.Add(theVar);
    } 

    /////////////// BCUSoftwareRelease ///////////////////
    // (raw->software_release >> 24) & 0xFF);                // versione software del nios Major
    // (raw->software_release >> 16) & 0xFF);                // versione software del nios Minor
    // (raw->software_release  & 0xFFFF);                    // versione software del nios #define_flags
    // 
    // x=ulong64(bb.last) & printhex,ishft(x,-24) AND 255, ishft(x,-16) AND 255, ishft(x,0) AND 65535
    groupname = "Bcu39SoftwareRelease";
    for (int i=0; i<Bcu39::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctUint32(&(raw.bcu39[i].software_release)));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStop));
        _vars.Add(theVar);
    } 
    groupname = "Bcu47SoftwareRelease";
    for (int i=0; i<Bcu47::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctUint32(&(raw.bcu47[i].software_release)));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStop));
        _vars.Add(theVar);
    } 

    /////////////// BCULogicRelease ///////////////////
    // (raw->logic_release >> 8) & 0xFF);                // PIO versione logica Major
    // (raw->logic_release & 0xFF);                      // PIO versione logica Minor
    //
    // x=ulong64(bb.last) & printhex, ishft(x,-8) AND 255, ishft(x,0) AND 25
    groupname = "Bcu39LogicRelease";
    for (int i=0; i<Bcu39::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctUint16(&(raw.bcu39[i].logic_release)));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStop));
        _vars.Add(theVar);
    } 
    groupname = "Bcu47LogicRelease";
    for (int i=0; i<Bcu47::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctUint16(&(raw.bcu47[i].logic_release)));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStop));
        _vars.Add(theVar);
    } 

    /////////////// BCUIPAddress ///////////////////
    groupname = "Bcu39Address";
    for (int i=0; i<Bcu39::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctIPAddress(&(raw.bcu39[i].ip_address)));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStop));
        _vars.Add(theVar);
    } 
    groupname = "Bcu47Address";
    for (int i=0; i<Bcu47::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctIPAddress(&(raw.bcu47[i].ip_address)));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStop));
        _vars.Add(theVar);
    }

    ////////////// BCUFramesCounter ///////////////////
    groupname = "Bcu39FramesCounter";
    for (int i=0; i<Bcu39::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctUint32(&(raw.bcu39[i].frames_counter)));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStop));
        _vars.Add(theVar);
    } 
    groupname = "Bcu47FramesCounter";
    for (int i=0; i<Bcu47::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctUint32(&(raw.bcu47[i].frames_counter)));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStop));
        _vars.Add(theVar);
    } 
    
    /////////////// BCUSerialNumber ///////////////////
    groupname = "Bcu39SerialNumber";
    for (int i=0; i<Bcu39::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctUint16(&(raw.bcu39[i].serial_number)));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStop));
        _vars.Add(theVar);
    } 
    groupname = "Bcu47SerialNumber";
    for (int i=0; i<Bcu47::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctUint16(&(raw.bcu47[i].serial_number)));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStop));
        _vars.Add(theVar);
    } 

    /////////////// BCUEnableMasterDiagnostic ///////////////////
    groupname = "Bcu39EnableMasterDiagnostic";
    for (int i=0; i<Bcu39::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctUint16(&(raw.bcu39[i].enable_master_diag)));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStop));
        _vars.Add(theVar);
    } 
    groupname = "Bcu47EnableMasterDiagnostic";
    for (int i=0; i<Bcu47::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctUint16(&(raw.bcu47[i].enable_master_diag)));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStop));
        _vars.Add(theVar);
    } 
    
    /////////////// BCUDecimationFactor ///////////////////
    groupname = "Bcu39DecimationFactor";
    for (int i=0; i<Bcu39::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctUint16(&(raw.bcu39[i].decimation_factor)));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStop));
        _vars.Add(theVar);
    } 
    groupname = "Bcu47DecimationFactor";
    for (int i=0; i<Bcu47::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctUint16(&(raw.bcu47[i].decimation_factor)));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStop));
        _vars.Add(theVar);
    } 
    
    /////////////// BCURemoteIPAddress ///////////////////
    groupname = "Bcu39RemoteIPAddress";
    for (int i=0; i<Bcu39::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctIPAddress(&(raw.bcu39[i].remote_ip_address)));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStop));
        _vars.Add(theVar);
    } 
    groupname = "Bcu47RemoteIPAddress";
    for (int i=0; i<Bcu47::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctIPAddress(&(raw.bcu47[i].remote_ip_address)));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStop));
        _vars.Add(theVar);
    } 


    /////////////// Bcu39StratixTemp ///////////////////
    groupname = "Bcu39StratixTemp";
    for (int i=0; i<Bcu39::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctBCUStratixTemp(&(raw.bcu39[i].BCU_diagnostic.stratix_temp)));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStopShutdown));
        _vars.Add(theVar);
    } 

    /////////////// HVtemperature ///////////////////
    groupname = "HVTemp";
    for (int i=0; i<Bcu39::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctBCUStratixTemp(&(raw.bcu39[i].DSP_diagnostic[0].driver_temp)));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStopShutdown));
        _vars.Add(theVar);
    } 


    /////////////// Bcu39PowerTemp ///////////////////
    groupname = "Bcu39PowerTemp";
    for (int i=0; i<Bcu39::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctBCUStratixTemp(&(raw.bcu39[i].BCU_diagnostic.power_temp)));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStopShutdown));
        _vars.Add(theVar);
    }


    /////////////// BCUStratixTemp ///////////////////
    groupname = "Bcu47StratixTemp";
    for (int i=0; i<Bcu47::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctBCUStratixTemp(&(raw.bcu47[i].BCU_diagnostic.stratix_temp)));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStopShutdown));
        _vars.Add(theVar);
    } 

    /////////////// BCUPowerTemp ///////////////////
    groupname = "Bcu47PowerTemp";
    for (int i=0; i<Bcu47::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctBCUStratixTemp(&(raw.bcu47[i].BCU_diagnostic.power_temp)));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStopShutdown));
        _vars.Add(theVar);
    } 


    ////////////////////////////////////////// bits of na_bcu_diagnostic_struct.reset_status  //////////////////////
    
//@C
// /verb+reset_status+ has status of reset signals for the BCU
//@

//@C
// /verb+BCUResetStatusFPGAReset+ is 1 when FPGA reset line is on
//@
    groupname = "Bcu39ResetStatusFPGAReset"; 
    for (int i=0; i<Bcu39::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctUint32Bit(&(raw.bcu39[i].BCU_diagnostic.reset_status), 0)); 
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStop));
        _vars.Add(theVar);
    } 

//@C
// /verb+BCUResetStatusBUSReset+ is 1 when BUS reset line is on
//@
    groupname = "Bcu39ResetStatusBUSReset"; 
    for (int i=0; i<Bcu39::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctUint32Bit(&(raw.bcu39[i].BCU_diagnostic.reset_status), 1)); 
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStop));
        _vars.Add(theVar);
    } 

//@C
// /verb+BCUResetStatusDSPReset+ is 1 when local DSP reset line is on
//@
    groupname = "Bcu39ResetStatusDSPReset"; 
    for (int i=0; i<Bcu39::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctUint32Bit(&(raw.bcu39[i].BCU_diagnostic.reset_status), 2)); 
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStop));
        _vars.Add(theVar);
    } 

//@C
// /verb+BCUResetStatusFLASHReset+ is 1 when FLASH reset line is on
//@
    groupname = "Bcu39ResetStatusFLASHReset"; 
    for (int i=0; i<Bcu39::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctUint32Bit(&(raw.bcu39[i].BCU_diagnostic.reset_status), 3)); 
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStop));
        _vars.Add(theVar);
    } 

//@C
// /verb+BCUResetStatusPCIReset+ is 1 when PCI reset line is on
//@
    groupname = "Bcu39ResetStatusPCIReset"; 
    for (int i=0; i<Bcu39::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctUint32Bit(&(raw.bcu39[i].BCU_diagnostic.reset_status), 4)); 
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStop));
        _vars.Add(theVar);
    } 


//@C
// /verb+reset_status+ has status of reset signals for the BCU
//@

//@C
// /verb+BCUResetStatusFPGAReset+ is 1 when FPGA reset line is on
//@
    groupname = "Bcu47ResetStatusFPGAReset"; 
    for (int i=0; i<Bcu47::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctUint32Bit(&(raw.bcu47[i].BCU_diagnostic.reset_status), 0)); 
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStop));
        _vars.Add(theVar);
    } 

//@C
// /verb+BCUResetStatusBUSReset+ is 1 when BUS reset line is on
//@
    groupname = "Bcu47ResetStatusBUSReset"; 
    for (int i=0; i<Bcu47::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctUint32Bit(&(raw.bcu47[i].BCU_diagnostic.reset_status), 1)); 
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStop));
        _vars.Add(theVar);
    } 

//@C
// /verb+BCUResetStatusDSPReset+ is 1 when local DSP reset line is on
//@
    groupname = "Bcu47ResetStatusDSPReset"; 
    for (int i=0; i<Bcu47::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctUint32Bit(&(raw.bcu47[i].BCU_diagnostic.reset_status), 2)); 
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStop));
        _vars.Add(theVar);
    } 

//@C
// /verb+BCUResetStatusFLASHReset+ is 1 when FLASH reset line is on
//@
    groupname = "Bcu47ResetStatusFLASHReset"; 
    for (int i=0; i<Bcu47::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctUint32Bit(&(raw.bcu47[i].BCU_diagnostic.reset_status), 3)); 
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStop));
        _vars.Add(theVar);
    } 

//@C
// /verb+BCUResetStatusPCIReset+ is 1 when PCI reset line is on
//@
    groupname = "Bcu47ResetStatusPCIReset"; 
    for (int i=0; i<Bcu47::N_CRATES; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv (new FunctUint32Bit(&(raw.bcu47[i].BCU_diagnostic.reset_status), 4)); 
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStop));
        _vars.Add(theVar);
    } 



    ///////////////////////////////////////////////////// WAVEGEN ////////////////////////////////////////

    /////////////// SGNPowerTemp ///////////////////
    groupname = "WGNPowerTemp";
    for (int i=0; i<Bcu47::N_CRATES; i++)
        for (int j=0; j<Bcu47::N_WGN_CRATE; j++)
        {
            _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i*Bcu47::N_WGN_CRATE+j);
            FunctPtr fConv (new FunctBCUStratixTemp(&(raw.bcu47[i].DSP_diagnostic[j].power_temp))); 
            DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hEmergencyStopShutdown));
            _vars.Add(theVar);
        } 

}

#include  "BcuLib/BcuCommon.h"
using namespace Arcetri::Bcu;

//@Member: GetRawData
//
//@
void HouseKeeperWFS::GetRawData()
{
    int firstDSP, lastDSP;
    bool loop_again=true;

    while(loop_again && TimeToDie() == false){
 
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
             firstDSP=255, lastDSP=255; //TODO c'e' un define per le BCU da qualche parte?

             _logger->log(Logger::LOG_LEV_TRACE, "Reading DSP [%d-%d] from BCU_39 [%s:%d]",
                firstDSP, lastDSP, __FILE__, __LINE__);
             cmdSender->sendMultiBcuCommand(Priority::LOW_PRIORITY, myMap["BCU_39"],
                firstDSP, lastDSP, OpCodes::MGP_OP_RDSEQ_SRAM, AddressMap::BCU_NIOS_FIXED_AREA, 
                sizeof(na_bcu_nios_fixed_area_struct), 
                (unsigned char*)(&(raw.bcu39)),
                _bcuRequestTimeout_ms, Constants::WANTREPLY_FLAG);

             loop_again = false;
             delete cmdSender;

          } catch (CommandSenderException &e) {
             ToggleUnprotected(true);
             _logger->log(Logger::LOG_LEV_ERROR, "Error reading BCUs:  %s [%s:%d]",
                e.what().c_str(), __FILE__, __LINE__);
             delete cmdSender;
             msleep(1000); 
             continue;
          }
       } else { //dummy mode
          for(int crate_n=0; crate_n<Bcu39::N_CRATES; crate_n++) {
             bcu_dummy(&raw.bcu39[crate_n], crate_n);
          }
          for(int crate_n=0; crate_n<Bcu47::N_CRATES; crate_n++) {
             bcu_dummy(&raw.bcu47[crate_n], crate_n);
          }
         loop_again = false;
       }
       ToggleUnprotected(false);
    }	                
}


//@Member: GetFrameCounter
// 
//@
unsigned int HouseKeeperWFS::GetFrameCounter()
{
    //static unsigned int fc=0;
    MatchDiagnVar fc = MatchDiagnVar("Bcu39FramesCounter", 0);
    DiagnVarPtr var=_vars.Find(fc);
    return (unsigned int)var->Last();
}

//@Member: PostInit
//
//@
void HouseKeeperWFS::PostInit()
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
void HouseKeeperWFS::ReadNios()
{
	
    //@C
    // Read (once and for all?) the nios fixed ares of the DSP boards.
    //@
    
    bool loop_again=true;

    while(loop_again && TimeToDie() == false){

        _logger->log(Logger::LOG_LEV_TRACE, "[Housekeeper::ReadNios] [%s:%d]", __FILE__, __LINE__);

        // read raw memory
        //
        

        if(!_dummy){
    /*
 
            // TODO qui non c'e' bisogno di creare il CommandSender tutte le volte,
            // ma bisogna ricordarsi di distruggerlo e ricrearlo nel caso di timeout-exception
            CommandSender* cmdSender = new CommandSender();
            BcuIdMap myMap;
            try{


                // TODO read of WAVEGEN board to be implemented!
                
                firstDSP=250, lastDSP=250;  //TODO c'e' un define per la wavegrn da qualche parte?

                _logger->log(Logger::LOG_LEV_TRACE, "Reading WGN [%d-%d] address %p from BCU_MIRROR_ALL [%s:%d]",
                    firstDSP, lastDSP, dsp_nios_fixed_area, __FILE__, __LINE__);
               
                int bcu = myMap["BCU_47"]; 
                cmdSender->sendMultiBcuCommand(Priority::LOW_PRIORITY, bcu, bcu, firstDSP, lastDSP, 
                                                    OpCodes::MGP_OP_RDSEQ_SRAM, dsp_nios_fixed_area, sizeof(na_bcu_nios_fixed_area_struct),
                                                    (unsigned char *)wgn_nios, 
                                                    _bcuRequestTimeout_ms, Constants::WANTREPLY_FLAG);
                loop_again = false;
                delete cmdSender;

            } catch (CommandSenderException &e) {
                ToggleUnprotected(true);
                _logger->log(Logger::LOG_LEV_ERROR, "Error reading NIOS WGN from BCU_47:  %s [%s:%d]",
                    e.what().c_str(), __FILE__, __LINE__);
                delete cmdSender;
                msleep(1000); 
                continue;
            }

*/
        loop_again=false;

        } else { //dummy mode
            for(int wgn_n=0; wgn_n<Bcu47::N_WGN; wgn_n++) {
             dsp_nios_dummy(&wgn_nios[wgn_n], wgn_n);
            }
            loop_again = false;
        }
        ToggleUnprotected(true);
    }
}


//@Member: InSlowLoop
//
//@
void HouseKeeperWFS::InSlowLoop()
{
}

//@Member: InFastLoop
//
//@
void HouseKeeperWFS::InFastLoop()
{
//@C
// Sleep for a while as set in the configuration file
//@
    float pp = ConfigDictionary()["PollingPeriod"];
    nusleep( (unsigned int)(pp*1e6));

}    

void HouseKeeperWFS::Periodic()
{
    try {
        _logger->log(Logger::LOG_LEV_DEBUG, "%s",(*(_vars.Find(MatchDiagnVar("Bcu39StratixTemp",0)))).str().c_str() ) ;
        _logger->log(Logger::LOG_LEV_DEBUG, "%s",(*(_vars.Find(MatchDiagnVar("Bcu47StratixTemp",0)))).str().c_str() ) ;
        _logger->log(Logger::LOG_LEV_DEBUG, "%s",(*(_vars.Find(MatchDiagnVar("Bcu39PowerTemp",0)))).str().c_str() ) ;
        _logger->log(Logger::LOG_LEV_DEBUG, "%s",(*(_vars.Find(MatchDiagnVar("Bcu47PowerTemp",0)))).str().c_str() ) ;
        _logger->log(Logger::LOG_LEV_DEBUG, "%s",(*(_vars.Find(MatchDiagnVar("Bcu39FramesCounter",0)))).str().c_str() ) ;
        _logger->log(Logger::LOG_LEV_DEBUG, "%s",(*(_vars.Find(MatchDiagnVar("Bcu47FramesCounter",0)))).str().c_str() ) ;

    // Lo sbobinamento di questi la scia un po' a desiderare...
        _logger->log(Logger::LOG_LEV_DEBUG, "%s",(*(_vars.Find(MatchDiagnVar("WGNPowerTemp",0)))).str().c_str() ) ;
        _logger->log(Logger::LOG_LEV_DEBUG, "%s",(*(_vars.Find(MatchDiagnVar("Bcu39CrateID",0)))).str().c_str() ) ;
        _logger->log(Logger::LOG_LEV_DEBUG, "%s",(*(_vars.Find(MatchDiagnVar("Bcu47CrateID",0)))).str().c_str() ) ;
        _logger->log(Logger::LOG_LEV_DEBUG, "%s",(*(_vars.Find(MatchDiagnVar("Bcu39WhoAmI",0)))).str().c_str() ) ;
        _logger->log(Logger::LOG_LEV_DEBUG, "%s",(*(_vars.Find(MatchDiagnVar("Bcu47WhoAmI",0)))).str().c_str() ) ;
        _logger->log(Logger::LOG_LEV_DEBUG, "%s",(*(_vars.Find(MatchDiagnVar("WGNWhoAmI",0)))).str().c_str() ) ;
        _logger->log(Logger::LOG_LEV_DEBUG, "%s",(*(_vars.Find(MatchDiagnVar("Bcu39LogicRelease",0)))).str().c_str() ) ;
        _logger->log(Logger::LOG_LEV_DEBUG, "%s",(*(_vars.Find(MatchDiagnVar("Bcu47LogicRelease",0)))).str().c_str() ) ;
        _logger->log(Logger::LOG_LEV_DEBUG, "%s",(*(_vars.Find(MatchDiagnVar("WGNLogicRelease",0)))).str().c_str() ) ;
        _logger->log(Logger::LOG_LEV_DEBUG, "%s",(*(_vars.Find(MatchDiagnVar("Bcu39SoftwareRelease",0)))).str().c_str() ) ;
        _logger->log(Logger::LOG_LEV_DEBUG, "%s",(*(_vars.Find(MatchDiagnVar("Bcu47SoftwareRelease",0)))).str().c_str() ) ;
        _logger->log(Logger::LOG_LEV_DEBUG, "%s",(*(_vars.Find(MatchDiagnVar("WGNSoftwareRelease",0)))).str().c_str() ) ;

        _logger->log(Logger::LOG_LEV_DEBUG, "%s",(*(_vars.Find(MatchDiagnVar("Bcu39SerialNumber",0)))).str().c_str() ) ;
        _logger->log(Logger::LOG_LEV_DEBUG, "%s",(*(_vars.Find(MatchDiagnVar("Bcu47SerialNumber",0)))).str().c_str() ) ;

    // No IP type in DiagnApp...
        _logger->log(Logger::LOG_LEV_DEBUG, "%s",(*(_vars.Find(MatchDiagnVar("Bcu39Address",0)))).str().c_str() ) ;
        _logger->log(Logger::LOG_LEV_DEBUG, "%s",(*(_vars.Find(MatchDiagnVar("Bcu47Address",0)))).str().c_str() ) ;



        varBcu39stratix_value.Set( (*(_vars.Find(MatchDiagnVar("Bcu39StratixTemp",0)))).Last());
        varBcu39power_value.Set( (*(_vars.Find(MatchDiagnVar("Bcu39PowerTemp",0)))).Last());
        varHVtemperature_value.Set( (*(_vars.Find(MatchDiagnVar("HVTemp",0)))).Last());

        } catch (DiagnSetException &e) {
            _logger->log(Logger::LOG_LEV_DEBUG, "%s", e.what().c_str());
        }
}

//@Member: 
//
//@
#include <stdlib.h>
//static 
double HouseKeeperWFS::randi(double a, double b)
{
    return a + ( (b-a)*rand()/(RAND_MAX+1.0));
}

#define SetBit(dato,n) (dato=((1<<n)|dato))     /* Set bit n high in word dato                     */

//@Member:
//
//@
uint16 HouseKeeperWFS::invConvertGain(double value, const float *coeff){
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
void HouseKeeperWFS::bcu_dummy(na_bcu_nios_fixed_area_struct  *raw, int crate_n)
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
void HouseKeeperWFS::dsp_nios_dummy(na_dsp_nios_fixed_area_struct  *raw, int dsb_n)
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

