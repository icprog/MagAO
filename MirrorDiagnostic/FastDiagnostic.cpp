//@File: FastDiagnostic.cpp
//
// Implementation of the FastDiagnostic class
//@

#include <cstdio>
#include <cstring>
#include <sstream>


extern "C" {
#include "base/thrdlib.h"
#include "iolib.h" //ReadBinaryFile
#include "aoslib/aoslib.h" 
}

#include "FastDiagnostic.h"
#include "FastDiagnostic_actions.h"
#include "Timing.h"
#include "DiagnVarAccumulator.h"

#include "arblib/base/Alerts.h" // Needed for Alert serialization
#include "arblib/base/ArbitratorInterface.h" 

//#include "AdsecDiagnLib.h"




// Initialize static members
//int FastDiagnostic::use_common =0;

//@Member FastDiagnostic
//
//Standard constructor
//@

FastDiagnostic::FastDiagnostic(int argc, char **argv) throw (AOException) :
    DiagnApp(argc, argv)
{
   CreateFD();
}



//FastDiagnostic::FastDiagnostic(const std::string& conffile, int verb )
//    : DiagnApp::DiagnApp(conffile, verb)
void FastDiagnostic::CreateFD()
{
    ReadConfig();
    Logger *logger;

    // Initialize some loggers
    try {
    	logger = Logger::get("GET_RAW_DATA", Logger::stringToLevel(_cfg["GET_RAW_DATA_LOG_LEV"]));
    }
    catch(Config_File_Exception& e) {
    	Logger::get()->log(Logger::LOG_LEV_INFO, "Unable to find log level for GET_RAW_DATA logger. Using default value (%d)", (int)_cfg["LogLevel"]);
    	logger = Logger::get("GET_RAW_DATA", Logger::stringToLevel(_cfg["LogLevel"]));
    }

    _info = NULL;
    _basedir = ConfigDictionary().getDir()+"/";


    //_adam = NULL;
    //if(!isOffline()) {
    //	_adam = new Adam(Side());
    //}
    // Creates an object able to react in case of emergency
    if(!isOffline()) {
    	_hEmergencyStop = FunctActionPtr(new FunctEmergencyStopAndDump(this, _basedir+(string)(ConfigDictionary()["SirenFile"])));
    }
    else {
      _hEmergencyStop = FunctActionPtr(new FunctAlarm());
    }

    // Creates the map "short-name" -> "full name" for the RTDB
    // variebles interesting for both Supervisor and AdSec.
    //_rtdbSharedVarsMap = new SharedVarsMap();

    // Init shm bufname
    _bufname = (string)_cfg["RawSharedBase"] + "." + Utils::getAdoptSide() + ":" + (string)_cfg["RawSharedName"];

    _logger->log(Logger::LOG_LEV_INFO, " sizeof(switch_bcu)      = %d  [%s:%d]",  sizeof(switch_bcu), __FILE__, __LINE__ );
    _logger->log(Logger::LOG_LEV_INFO, " sizeof(crate_bcu)       = %d  [%s:%d]",  sizeof(crate_bcu), __FILE__, __LINE__ );
    _logger->log(Logger::LOG_LEV_INFO, " sizeof(AdsecDiagnRaw)   = %d  [%s:%d]",  sizeof(AdsecDiagnRaw), __FILE__, __LINE__ );

    //Flat mirror positions and forces
    _ref_pos = new vector<double>(BcuMirror::N_CH, 0.0);
    _ref_for = new vector<double>(BcuMirror::N_CH, 0.0);
    _ref_for_mean = 0.;
    _high_order_offload_matrix =  NULL;
    _currAveNullMean60 = new double[BcuMirror::N_CH];
    _offloadCommandVector = new double[BcuMirror::N_CH];
    _highOrderOffloadMatrixName = "";

    _mastdiagninterface = new MastDiagnInterface(((std::string)AOApp::ConfigDictionary()["MasterDiagnosticIdentity"])+"."+Side(), Logger::LOG_LEV_ERROR);

    _highOrderOffloadLoopCounter = 0;

    // Initialize AOS communication for low-order offload
    aos_init( (char*) Side().c_str());
}

//@Member $\sim$ FastDiagnostic
//
//Destructor
//@
FastDiagnostic::~FastDiagnostic()
{
    //if(_info) free(_info);
    int stat;
    emergencyReact();
    if( IS_ERROR(stat = bufRelease( (char*)MyFullName().c_str(), _info) ) ) {
        Logger::get("GET_RAW_DATA")->log(Logger::LOG_LEV_WARNING, "Detaching from shared buffer - %s - Perror(%s)  [%s:%d]",
            lao_strerror(stat), strerror(errno), __FILE__, __LINE__ );
    }
    //delete _adam;
    if (_mastdiagninterface)
        delete _mastdiagninterface;
    delete _info;
}

//@Member: ReadConfig
//
//@
void FastDiagnostic::ReadConfig()
{
    _cfg=ConfigDictionary();

    // insert stuff here
}


//@Member: SetupVars
//
//@
void FastDiagnostic::SetupVars()
{
    try{
        DiagnApp::SetupVars();

        _VarTimeoutCur = RTDBvar(MyFullName(), "TIMEOUT_MS", CUR_VAR, INT_VARIABLE, 1);
        _VarTimeoutReq = RTDBvar(MyFullName(), "TIMEOUT_MS", REQ_VAR, INT_VARIABLE, 1);
        _VarTimeoutReq.Set( (int)_cfg["TIMEOUT_MS"]);
        _VarTimeoutCur.Set( (int)_cfg["TIMEOUT_MS"]);

        // Check if AdSec OVS_P variable exists, if not creates it.
        Logger::get("OVS_P-HANDLER", Logger::LOG_LEV_INFO);
        string opvsVarRtdbVarName = Side()+"."+(string)(_cfg["OVS_P_VARNAME"]);   //"C.ADSEC.OVS_P";
        _VarAdsecOvsp = RTDBvar(opvsVarRtdbVarName, INT_VARIABLE, 1);
        int ovs_p;
        _VarAdsecOvsp.Get(&ovs_p);
        Logger::get("OVS_P-HANDLER")->log(Logger::LOG_LEV_INFO, "Retrieved OVS_P variable (%s=%d)", opvsVarRtdbVarName.c_str(), ovs_p);

        // Reference position variable (flat)
        string varprefix = (string)_cfg["IdlIdentity"]+"."+Utils::getAdoptSide()+".";
        string RefPosRtdbVarName = varprefix+(string)(_cfg["REF_POS_VARNAME"]);
        string RefForRtdbVarName = varprefix+(string)(_cfg["REF_FOR_VARNAME"]);
        string HighOrderOffloadMatrixVarname = varprefix+(string)(_cfg["HO_OFFLOAD_VARNAME"]);

        _VarRefPos = RTDBvar(RefPosRtdbVarName, REAL_VARIABLE, BcuMirror::N_CH);
        _VarRefFor = RTDBvar(RefForRtdbVarName, REAL_VARIABLE, BcuMirror::N_CH);
        _VarHighOrderOffloadMatrixName = RTDBvar(HighOrderOffloadMatrixVarname, CHAR_VARIABLE,  256);
        
        _VarDoOffloadModesCur = RTDBvar( MyFullName(), "DO_OFFLOAD_MODES", CUR_VAR, INT_VARIABLE, 1);
        _VarDoOffloadModesReq = RTDBvar( MyFullName(), "DO_OFFLOAD_MODES", REQ_VAR, INT_VARIABLE, 1);
        _VarDoOffloadModesReq.Set( (int)false);
        _VarDoOffloadModesCur.Set( (int)false);

        _VarOffloadTime = RTDBvar( MyFullName()+".DO_OFFLOAD_LASTTIME", CHAR_VARIABLE, 20);
        
        // Ask for notifies
        Notify(_VarTimeoutReq, TimeoutReqChanged);
        Notify(_VarAdsecOvsp, AdsecOvspChanged);
        Notify(_VarRefPos,  RefPosChanged);
        Notify(_VarRefFor,  RefForChanged);
        Notify(_VarDoOffloadModesReq, DoOffloadModesReqChanged);
        Notify(_VarHighOrderOffloadMatrixName,  HighOrderOffloadMatrixChanged);

    } catch(AOException &e){
        Logger::get()->log(Logger::LOG_LEV_ERROR, "%s [%s:%d]",e.what().c_str(),__FILE__,__LINE__);
        // TODO FATAL or ERROR. Eventually die
    }
}


//@Member: InstallHandlers
//
//@
void FastDiagnostic::InstallHandlers()
{
    DiagnApp::InstallHandlers();
}


//@Member: TimeoutReqChanged
//
//@
int FastDiagnostic::TimeoutReqChanged(void *pt, Variable *var) {
        FastDiagnostic *FD = (FastDiagnostic*)pt;

        long int tmout_req = var->Value.Lv[0];

        long int tmout_cur;
        
        FD->_VarTimeoutCur.Get(0, (int *)&tmout_cur);

        Logger::get()->log(Logger::LOG_LEV_TRACE, "modifying timeout:  req %d ms cur %d ms [%s:%d]", tmout_req, tmout_cur,
                    __FILE__, __LINE__);

        // TODO qui c'e' eventualmente da fare una MatchAndSet sulla VarTimeoutReq per tenere aggiornata la copia locale. E' cosi'?
        // qui eventualmente fai qualcosa
        FD->_VarTimeoutCur.Set((int)tmout_req);

        FD->_VarTimeoutCur.Get(0,(int*) &tmout_cur);

        Logger::get()->log(Logger::LOG_LEV_TRACE, "timeout modified:   req %d ms cur %d ms [%s:%d]", tmout_req, tmout_cur,
                    __FILE__, __LINE__);

        return NO_ERROR;
}

//@Member: OvpChanged
//
//@
int FastDiagnostic::AdsecOvspChanged(void *pt, Variable *var) {

	FastDiagnostic *FD = (FastDiagnostic*)pt;

   long int adsecOvs_p = var->Value.Lv[0];
    FD->_VarAdsecOvsp.Set((int)&adsecOvs_p, NO_SEND);

    Logger::get()->log(Logger::LOG_LEV_INFO, "Changed OVS_P: %d  [%s:%d]", adsecOvs_p, __FILE__, __LINE__);

    return NO_ERROR;
}

//@Member: RefPosChanged
//
//@
int FastDiagnostic::RefPosChanged(void *pt, Variable *var) {
    FastDiagnostic *FD = (FastDiagnostic*)pt;
    FD->_VarRefPos.Set(var);
    double* tmp_ref_pos = new double[BcuMirror::N_CH];
    FD->_VarRefPos.Get(&tmp_ref_pos);
    for (int i=0; i<BcuMirror::N_CH; i++) FD->_ref_pos->at(i)=tmp_ref_pos[i];
    //FD->_ref_pos = new vector<double>(tmp_ref_pos, tmp_ref_pos + sizeof(tmp_ref_pos)/sizeof(double));
    FD->_logger->log(Logger::LOG_LEV_INFO, "Changed reference positions [%s:%d]",  __FILE__, __LINE__);
    // reset loop counter of high order offload mechanism
    FD->_highOrderOffloadLoopCounter = 0;
    return NO_ERROR;
}

//@Member: RefForChanged
//
//@
int FastDiagnostic::RefForChanged(void *pt, Variable *var) {
    FastDiagnostic *FD = (FastDiagnostic*)pt;
    FD->_VarRefFor.Set(var);
    double* tmp_ref_for = new double[BcuMirror::N_CH];
    FD->_VarRefFor.Get(&tmp_ref_for);
    
    FD->_ref_for_mean=0;
    for (int i=0; i<BcuMirror::N_CH; i++) {
	   FD->_ref_for->at(i)=tmp_ref_for[i];
	   FD->_ref_for_mean += tmp_ref_for[i]; 
    }
    FD->_ref_for_mean /= BcuMirror::N_CH;
    FD->_logger->log(Logger::LOG_LEV_INFO, "Changed reference forces: Averaged reference force is %g [%s:%d]", FD->_ref_for_mean,  __FILE__, __LINE__);
    return NO_ERROR;
}

//@Member: HighOrderOffloadMatrixChanged
//
//@
int FastDiagnostic::HighOrderOffloadMatrixChanged(void *pt, Variable *var) {
    FastDiagnostic *FD = (FastDiagnostic*)pt;
    FD->_VarHighOrderOffloadMatrixName.Set(var);
    string highOrderOffloadMatrixName = FD->_VarHighOrderOffloadMatrixName.Get();
        
    FD->_logger->log(Logger::LOG_LEV_INFO, "Received HighOrderOffloadMatrix change %s [%s:%d]",
          highOrderOffloadMatrixName.c_str(),__FILE__,__LINE__);

    if (FD->_highOrderOffloadMatrixName.compare(highOrderOffloadMatrixName) == 0 ){
      return NO_ERROR;
    } else {
      FD->_highOrderOffloadMatrixName = highOrderOffloadMatrixName;
    }

    // import matrix from fits file
    int   ndims_p2m;
    long *dims_p2m;
    int   datatype_p2m;
    int stat = PeekFitsFile( (char*)highOrderOffloadMatrixName.c_str(), &datatype_p2m, &ndims_p2m, &dims_p2m);
    if (stat != NO_ERROR) {
        FD->_logger->log(Logger::LOG_LEV_ERROR, "Cannot open %s. Reason is %s [%s:%d]",highOrderOffloadMatrixName.c_str(), lao_strerror(stat), __FILE__,__LINE__);
        throw AOException("Cannot read header from highOrderOffloadMatrix file", stat, __FILE__, __LINE__);
    }
    if (ndims_p2m != 2) {
        FD->_logger->log(Logger::LOG_LEV_ERROR, "Wrong data in highOrderOffloadMatrix %s. It should be a 2D matrix instead of %dD [%s:%d]", highOrderOffloadMatrixName.c_str(), ndims_p2m, __FILE__,__LINE__);
        throw AOException("Wrong ndim in highOrderOffload matrix", FILE_ERROR, __FILE__, __LINE__);
    }
    unsigned int npos_p2m   = dims_p2m[0];
    unsigned int nmodes_p2m = dims_p2m[1];

    if (datatype_p2m != DOUBLE_IMG){
        FD->_logger->log(Logger::LOG_LEV_ERROR, "Wrong data type in highOrderOffload %s. It should be double (-64) instead of %d [%s:%d]",highOrderOffloadMatrixName.c_str(), datatype_p2m, __FILE__,__LINE__);
        throw AOException("Wrong data type in highOrderOffload matrix", FILE_ERROR, __FILE__, __LINE__);
    }
    
    if (npos_p2m != (unsigned int)BcuMirror::N_CH){
        FD->_logger->log(Logger::LOG_LEV_ERROR, "highOrderOffload %s has %d columns, command vector has %d elements [%s:%d]",highOrderOffloadMatrixName.c_str(), npos_p2m, BcuMirror::N_CH, __FILE__,__LINE__);
        throw AOException("Wrong column number in highOrderOffload matrix", FILE_ERROR, __FILE__, __LINE__);
    }
    
    FD->_high_order_offload_matrix  = (double*) ReadFitsFile((char*)highOrderOffloadMatrixName.c_str(), &stat); //ReadBinaryFile(mplusfile, &stat);
    if (FD->_high_order_offload_matrix == NULL){
        FD->_logger->log(Logger::LOG_LEV_ERROR, "Cannot open %s. Reason is %s [%s:%d]",highOrderOffloadMatrixName.c_str(), lao_strerror(stat), __FILE__,__LINE__);
        throw AOException("Cannot read highOrderOffload matrix file", FILE_ERROR, __FILE__, __LINE__);
    }
    // TODO do checks?
    if (stat != (int)(nmodes_p2m * BcuMirror::N_CH * sizeof(double)) ){
        FD->_logger->log(Logger::LOG_LEV_ERROR, "%s highOrderOffload matrix buffer has wrong size. Expected %d bytes got %d [%s:%d]",
                highOrderOffloadMatrixName.c_str(), nmodes_p2m * BcuMirror::N_CH * sizeof(float), stat,__FILE__,__LINE__);
        throw AOException("Matrix has wrong size", FILE_ERROR, __FILE__, __LINE__);
    }

    Logger::get()->log(Logger::LOG_LEV_INFO, "Changed HighOrderOffloadMatrix [%s:%d]",  __FILE__, __LINE__);
    
    // reset loop counter
    FD->_highOrderOffloadLoopCounter = 0;
    return NO_ERROR;
}

//@Member: OvpChanged
//
//@
int FastDiagnostic::DoOffloadModesReqChanged(void *pt, Variable *var) {
    FastDiagnostic *FD = (FastDiagnostic*)pt;

    long int doOffloadmodes_req = var->Value.Lv[0];
        
    FD->_VarDoOffloadModesCur.Set((int)doOffloadmodes_req);

    FD->_logger->log(Logger::LOG_LEV_TRACE, "doOffloadModes modified: %d [%s:%d]", doOffloadmodes_req, __FILE__, __LINE__);

    return NO_ERROR;
}



//@Member: emergencyReact
// Appropriately react in case of danger for the AdSec.
//@
void FastDiagnostic::emergencyReact() {
   //FunctActionPtr             hEmergencyStop;
   //if(!isOffline()) {
   // 	hEmergencyStop = FunctActionPtr(new FunctEmergencyStop(_basedir+ConfigDictionary()["SirenFile"]));
  // }
  // else {
  //  	hEmergencyStop = FunctActionPtr(new FunctAlarm());
  // }
   _hEmergencyStop->react("emergencyReact", 1.0);
   _hEmergencyStop->BeginLoop();
   //_logger->log(Logger::LOG_LEV_WARNING, "emergencyReact: going to disable coils [%s:%d]", __FILE__, __LINE__);
   //;if(_adam) {
   //		_adam->disableCoils();
   //}
}


/*
//@Function: diagndump_handler
//
// handler of a DIAGNDUMP command. Serialize data in a
// file called YYMMDDhhmss.dump
//@
#include <ctime>
#include <fstream>
int FastDiagnostic::diagndump_handler(MsgBuf *, void *argp)
{
    FastDiagnostic *pt = (FastDiagnostic*)argp;
    char outstr[200];
    time_t t;
    struct tm *tmp;

    t = time(NULL);
    tmp = localtime(&t);
    strftime(outstr, sizeof(outstr), "%y%m%d%H%M%S.dump", tmp);

    // make an archive
    std::ofstream ofs(outstr);
    boost::archive::binary_oarchive oa(ofs);
    oa << pt->_vars;

    return NO_ERROR;
}*/


//@Member: CreateDiagnVars
//
//@
void FastDiagnostic::CreateDiagnVars()
{

    std::string          groupname;
    //DiagnRange<double>   rng(-2, -1 , 1, 2);
    //size_type            run_mean_len=100;
    //unsigned int         cons_allow_faults=0;
    FunctActionPtr             hAlarm   (new FunctAlarm());
    FunctActionPtr             hAlarmDump (new FunctAlarmAndDump(this));
    FunctActionPtr             hWarning (new FunctWarning());

    FunctActionPtr             hEmergencyStop;
    FunctActionPtr             hEmergencyStopDump;

    if(!isOffline()) {
    	hEmergencyStop = FunctActionPtr(new FunctEmergencyStop(_basedir+(string)ConfigDictionary()["SirenFile"]));
    	hEmergencyStopDump = FunctActionPtr(new FunctEmergencyStopAndDump(this, ConfigDictionary()["SirenFile"]));
    }
    else {
    	hEmergencyStop = FunctActionPtr(new FunctAlarm());
    	hEmergencyStopDump = FunctActionPtr(new FunctAlarmAndDump(this));
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////// HEADER/FOOTER ///////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//@C
// /verb+WFSFrameCounter+: counter of frames analyzed by the WFS
//@
    groupname = "WFSFrameCounter";
    //for (int cr=0; cr<BcuMirror::N_CRATES; cr++)
    for (int cr=0; cr<1; cr++) // header-footer checks are done by the raw frame downloader
    {
        Logger::get()->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), cr);
        FunctPtr fConv (new FunctFramesCounter( &(_raw.crate[cr].header.WFSFrameCounter), &(_raw.crate[cr].footer.WFSFrameCounter) ));
        DiagnVarPtr theVar (new DiagnVar(groupname, cr, fConv, hWarning, hAlarm));
        _vars.Add(theVar);
    }

//@C
// /verb+MirrorFrameCounter+: counter of frames arrived to the secondary unit (both from the WFS and from internal trigger)
//@
    groupname = "MirrorFrameCounter";
    for (int cr=0; cr<1; cr++) // header-footer checks are done by the raw frame downloader
    {
        Logger::get()->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), cr);
        FunctPtr fConv (new FunctFramesCounter( &(_raw.crate[cr].header.MirrorFrameCounter), &(_raw.crate[cr].footer.MirrorFrameCounter) ));
        DiagnVarPtr theVar (new DiagnVar(groupname, cr, fConv, hWarning, hAlarm));
        _vars.Add(theVar);
    }

//@C
// /verb+ParamBlockBlockSelection+: Block Selection 0 = block #0, 1 = block #1
//@
    groupname = "ParamBlockBlockSelection";
    for (int cr=0; cr<1; cr++) // header-footer checks are done by the raw frame downloader
    {
        Logger::get()->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), cr);
        FunctPtr fConv (new FunctUint32Bit( &(_raw.crate[cr].header.ParamBlockSelector), 0) );
        DiagnVarPtr theVar (new DiagnVar(groupname, cr, fConv, hWarning, hAlarm));
        _vars.Add(theVar);
    }

//@C
// /verb+ParamBlockSlopeLinearizeMethod+ (relevant for SC):
// 1 = the slopes are normalized through the sum of the 4 pixel of each sub-aperture (relevant for SC)
// 2 = the slopes are normalized through the sum of all pixel of previous step (relevant for SC)
// 3 = the slopes normalized through the SlopeConstant variable (relevant for SC)
//@
    groupname = "ParamBlockSlopeLinearizeMethod";
    for (int cr=0; cr<1; cr++) // header-footer checks are done by the raw frame downloader
    {
        Logger::get()->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), cr);
        FunctPtr fConv (new FunctSlopeNorm( &(_raw.crate[cr].header.ParamBlockSelector), 1) );
        DiagnVarPtr theVar (new DiagnVar(groupname, cr, fConv, hWarning, hAlarm));
        _vars.Add(theVar);
    }

//@C
// /verb+ParamBlockSlopeLinearize+:  slope linearization procedure  0 = disabled, 1 = enabled
//@
    groupname = "ParamBlockSlopeLinearize";
    for (int cr=0; cr<1; cr++) // header-footer checks are done by the raw frame downloader
    {
        Logger::get()->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), cr);
        FunctPtr fConv (new FunctUint32Bit( &(_raw.crate[cr].header.ParamBlockSelector), 4) );
        DiagnVarPtr theVar (new DiagnVar(groupname, cr, fConv, hWarning, hAlarm));
        _vars.Add(theVar);
    }

//@C
// /verb+ParamBlockDeltaCommand+: enable (1) or disable (0) the delta command calculation
// using the actual position of the mirror respect to the old command (relevant for RTR)
//@
    groupname = "ParamBlockDeltaCommand";
    for (int cr=0; cr<1; cr++) // header-footer checks are done by the raw frame downloader
    {
        Logger::get()->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), cr);
        FunctPtr fConv (new FunctUint32Bit( &(_raw.crate[cr].header.ParamBlockSelector), 5) );
        DiagnVarPtr theVar (new DiagnVar(groupname, cr, fConv, hWarning, hAlarm));
        _vars.Add(theVar);
    }

//@C
// /verb+ParamBlockDiagnStorage+: enable (1) or disable (0) the diagnostic data storage to the SDRAM (relevant for SC & RTR)
//@
    groupname = "ParamBlockDiagnStorage";
    for (int cr=0; cr<1; cr++) // header-footer checks are done by the raw frame downloader
    {
        Logger::get()->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), cr);
        FunctPtr fConv (new FunctUint32Bit( &(_raw.crate[cr].header.ParamBlockSelector), 6) );
        DiagnVarPtr theVar (new DiagnVar(groupname, cr, fConv, hWarning, hAlarm));
        _vars.Add(theVar);
    }

//@C
// /verb+ParamBlockFastLink+: enable (1) or disable (0) the fast-link commands used to send the slope vector to the switchBCU (relevant for SC)
//@
    groupname = "ParamBlockFastLink";
    for (int cr=0; cr<1; cr++) // header-footer checks are done by the raw frame downloader
    {
        Logger::get()->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), cr);
        FunctPtr fConv (new FunctUint32Bit( &(_raw.crate[cr].header.ParamBlockSelector), 7) );
        DiagnVarPtr theVar (new DiagnVar(groupname, cr, fConv, hWarning, hAlarm));
        _vars.Add(theVar);
    }

//@C
// /verb+ParamBlockAccelerometers+: enable (1) or disable (0) the DM accelerometers acquisition (relevant for RTR)
//@
    groupname = "ParamBlockAccelerometers";
    for (int cr=0; cr<1; cr++) // header-footer checks are done by the raw frame downloader
    {
        Logger::get()->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), cr);
        FunctPtr fConv (new FunctUint32Bit( &(_raw.crate[cr].header.ParamBlockSelector), 11) );
        DiagnVarPtr theVar (new DiagnVar(groupname, cr, fConv, hWarning, hAlarm));
        _vars.Add(theVar);
    }

//@C
// /verb+ParamBlockDisturbEnabled+: indicates the state of disturb on mirror commands (0=disabled, 1=enabled)
//@
    groupname = "ParamBlockDisturbEnabled";
    for (int cr=0; cr<1; cr++) // header-footer checks are done by the raw frame downloader
    {
        Logger::get()->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), cr);
        FunctPtr fConv (new FunctUint32Bit( &(_raw.crate[cr].header.ParamBlockSelector), 12) );
        DiagnVarPtr theVar (new DiagnVar(groupname, cr, fConv, hWarning, hAlarm));
        _vars.Add(theVar);
    }

//@C
// /verb+ParamBlockOffLoadBlock+:  off-loading mode block selection 0 = block #0, 1 = block #1
//@
    groupname = "ParamBlockOffLoadBlock";
    for (int cr=0; cr<1; cr++) // header-footer checks are done by the raw frame downloader
    {
        Logger::get()->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), cr);
        FunctPtr fConv (new FunctUint32Bit( &(_raw.crate[cr].header.ParamBlockSelector), 13) );
        DiagnVarPtr theVar (new DiagnVar(groupname, cr, fConv, hWarning, hAlarm));
        _vars.Add(theVar);
    }

//@C
// /verb+CommandHistoryPtr+: ???
//@
    groupname = "CommandHistoryPtr";
    for (int cr=0; cr<1; cr++) // header-footer checks are done by the raw frame downloader
    {
        Logger::get()->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), cr);
        FunctPtr fConv (new FunctUint32( &(_raw.crate[cr].header.CommandHistoryPtr) ));
        DiagnVarPtr theVar (new DiagnVar(groupname, cr, fConv, hWarning, hAlarm));
        _vars.Add(theVar);
    }


    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////// SWITCH BCU //////////////////////////////////////////////////////////
    /////////////////////////// ASSERT : THERE IS ONLY ONE SWITCH BCU IN THE SYSTEM /////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//@C
// /verb+Slopes+: slopes (computed by the WFS)
//@
    groupname = "Slopes";
    for (int sl=0; sl<LB_N_SLOPES; sl++)
    {
        Logger::get()->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), sl);
        FunctPtr fConv (new FunctFloat32( &(_raw.switc[0].slopes[sl]) ) );
        DiagnVarPtr theVar (new DiagnVar(groupname, sl, fConv, hWarning, hAlarm));
        _vars.Add(theVar);
    }

//@C
// /verb+SwitchSafeSkipCounter+: number of skipped frames because safety checks on modes, commands or currents failed
//@
    {
        groupname = "SwitchSafeSkipCounter";
        Logger::get()->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), 0);
        FunctPtr fConv (new FunctUint32( &(_raw.switc[0].swb_SafeSkipFrameCnt) ) );
        DiagnVarPtr theVar (new DiagnVar(groupname, 0, fConv, hWarning, hAlarm));
        _vars.Add(theVar);
    }

//TODO Add SafeSkipRate

//@C
// /verb+PendingSkipCounter+: number of skipped frames because of "command pending" faults
//@
    {
        groupname = "SwitchPendingSkipCounter";
        Logger::get()->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), 0);
        FunctPtr fConv (new FunctUint32( &(_raw.switc[0].swb_SafeSkipFrameCnt) ) );
        DiagnVarPtr theVar (new DiagnVar(groupname, 0, fConv, hWarning, hAlarm));
        _vars.Add(theVar);
    }
//TODO Add PendingSkipRate

//@C
// /verb+SwitchWFSGlobalTimeout+: number of timeouts caused by the "not_received_WFS_frame_for_a_while" condition
//@
    {
        groupname = "SwitchWFSGlobalTimeout";
        Logger::get()->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), 0);
        FunctPtr fConv (new FunctUint32( &(_raw.switc[0].swb_SafeSkipFrameCnt) ) );
        DiagnVarPtr theVar (new DiagnVar(groupname, 0, fConv, hWarning, hAlarm));
        _vars.Add(theVar);
    }
//TODO Add corresponding rate

//@C
// /verb+SwitchNumFLTimeout+: number of Fast Link timeouts
//@
    {
        groupname = "SwitchNumFLTimeout";
        Logger::get()->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), 0);
        FunctPtr fConv (new FunctUint32( &(_raw.switc[0].swb_NumFLTimeout) ) );
        DiagnVarPtr theVar (new DiagnVar(groupname, 0, fConv, hWarning, hAlarm));
        _vars.Add(theVar);
    }
//TODO Add corresponding rate

//@C
// /verb+SwitchNumFLCrcErr+: number of Fast Link Crc errors
//@
    {
        groupname = "SwitchNumFLCrcErr";
        Logger::get()->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), 0);
        FunctPtr fConv (new FunctUint32( &(_raw.switc[0].swb_NumFLCrcErr) ) );
        DiagnVarPtr theVar (new DiagnVar(groupname, 0, fConv, hWarning, hAlarm));
        _vars.Add(theVar);
    }
//TODO Add corresponding rate

//@C
// /verb+SwitchAccelerometerCoeff+: Accelerometers coefficients
//@
    groupname = "SwitchAccelerometerCoeff";
    for (int acc=0; acc<3; acc++)
    {
        Logger::get()->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), acc);
        FunctPtr fConv (new FunctFloat32( &(_raw.switc[0].accelerometer_coeffs[acc]) ) );
        DiagnVarPtr theVar (new DiagnVar(groupname, acc, fConv, hWarning, hAlarmDump));
        _vars.Add(theVar);
    }
//@C
// /verb+SwitchTimeStamp+: time stamp (counter increased at  59.29 kHz - 16.87us )
//@
    {
        groupname = "SwitchTimeStamp";
        Logger::get()->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), 0);
        FunctPtr fConv (new FunctUint32( &(_raw.switc[0].TimeStamp) ) );
        DiagnVarPtr theVar (new DiagnVar(groupname, 0, fConv, hWarning, hAlarm));
        _vars.Add(theVar);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////// CRATE BCUS /////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//@C
// /verb+ChDistAverage+: gap on act[i] measured by capacitive sensors [m]
//@
    groupname = "ChDistAverage";
    for (int cr=0; cr<BcuMirror::N_CRATES; cr++)
        for (int i=0; i<BcuMirror::N_DSP_CRATE; i++)
            for (int j=0; j<BcuMirror::N_CH_DSP_; j++)
            {
                int num = cr*BcuMirror::N_CH_CRATE  + i*BcuMirror::N_CH_DSP_ + j;
                Logger::get()->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), num);
                FunctPtr fConv (new FunctFloat32(&(_raw.crate[cr].dsp[i].DistAverage[j])));
                DiagnVarPtr theVar (new DiagnVar(groupname, num, fConv, hWarning, hEmergencyStopDump));
                _vars.Add(theVar);
            }

    /////////////// DistAverageAll ////////////////
    /*groupname = "DistAverageAll";
    rng = DiagnRange<double>(-2, -1 , 1, 2);
    std::cout << "Creating Var " << groupname  << std::endl;
    FunctPtr fConv (new ConvAll(vDistAve));
    DiagnVarPtr theVar (new DiagnVar(fConv, run_mean_len, hWarning, hAlarm, rng, groupname, 0 ));
    _vars.Add(theVar);*/

//@C
// /verb+ChCurrAverage+: force on act[i]  [N] TODO or Ampere?
//@
    groupname = "ChCurrAverage";
    for (int cr=0; cr<BcuMirror::N_CRATES; cr++)
        for (int i=0; i<BcuMirror::N_DSP_CRATE; i++)
            for (int j=0; j<BcuMirror::N_CH_DSP_; j++)
            {
                int num = cr*BcuMirror::N_CH_CRATE  + i*BcuMirror::N_CH_DSP_ + j;
                Logger::get()->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), num);
                FunctPtr    fConv  (new FunctFloat32(&(_raw.crate[cr].dsp[i].CurrAverage[j])));
                DiagnVarPtr theVar (new DiagnVar(groupname, num, fConv, hWarning, hEmergencyStopDump));
                _vars.Add(theVar);
            }

//@C
// /verb+Modes+: modal coefficients (computed by DSP reconstructors)
//@
    groupname = "Modes";
    for (int cr=0; cr<BcuMirror::N_CRATES; cr++)
        for (int i=0; i<BcuMirror::N_DSP_CRATE; i++)
            for (int j=0; j<BcuMirror::N_CH_DSP_; j++)
            {
                int num = cr*BcuMirror::N_CH_CRATE  + i*BcuMirror::N_CH_DSP_ + j;
                Logger::get()->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), num);
                FunctPtr fConv (new FunctModes(&(_raw.crate[cr].dsp[i].Modes[j])));
                DiagnVarPtr theVar (new DiagnVar(groupname, num, fConv, hWarning, hAlarm));
                _vars.Add(theVar);
            }

//@C
// /verb+ChNewDeltaCommand+: delta command to apply modal
//@
    groupname = "ChNewDeltaCommand";
    for (int cr=0; cr<BcuMirror::N_CRATES; cr++)
        for (int i=0; i<BcuMirror::N_DSP_CRATE; i++)
            for (int j=0; j<BcuMirror::N_CH_DSP_; j++)
            {
                int num = cr*BcuMirror::N_CH_CRATE  + i*BcuMirror::N_CH_DSP_ + j;
                Logger::get()->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), num);
                FunctPtr fConv (new FunctFloat32(&(_raw.crate[cr].dsp[i].NewDeltaCommand[j])));
                DiagnVarPtr theVar (new DiagnVar(groupname, num, fConv, hWarning, hAlarm));
                _vars.Add(theVar);
            }

//@C
// /verb+ChFFCommand+:  Delta Feed-Forward Command
//@
    groupname = "ChFFCommand";
    for (int cr=0; cr<BcuMirror::N_CRATES; cr++)
        for (int i=0; i<BcuMirror::N_DSP_CRATE; i++)
            for (int j=0; j<BcuMirror::N_CH_DSP_; j++)
            {
                int num = cr*BcuMirror::N_CH_CRATE  + i*BcuMirror::N_CH_DSP_ + j;
                Logger::get()->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), num);
                FunctPtr fConv (new FunctFloat32(&(_raw.crate[cr].dsp[i].FFCommand[j])));
                DiagnVarPtr theVar (new DiagnVar(groupname, num, fConv, hWarning, hAlarm));
                _vars.Add(theVar);
            }

//@C
// /verb+ChIntControlCurrent+:  Integrated Control Current
//@
    groupname = "ChIntControlCurrent";
    for (int cr=0; cr<BcuMirror::N_CRATES; cr++)
        for (int i=0; i<BcuMirror::N_DSP_CRATE; i++)
            for (int j=0; j<BcuMirror::N_CH_DSP_; j++)
            {
                int num = cr*BcuMirror::N_CH_CRATE  + i*BcuMirror::N_CH_DSP_ + j;
                Logger::get()->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), num);
                FunctPtr fConv (new FunctFloat32(&(_raw.crate[cr].dsp[i].IntControlCurrent[j])));
                DiagnVarPtr theVar (new DiagnVar(groupname, num, fConv, hWarning, hAlarm));
                _vars.Add(theVar);
            }

//@C
// /verb+ChFFPureCurrent+:  Integrated Feed-Forward Current
//@
    groupname = "ChFFPureCurrent";
    for (int cr=0; cr<BcuMirror::N_CRATES; cr++)
        for (int i=0; i<BcuMirror::N_DSP_CRATE; i++)
            for (int j=0; j<BcuMirror::N_CH_DSP_; j++)
            {
                int num = cr*BcuMirror::N_CH_CRATE  + i*BcuMirror::N_CH_DSP_ + j;
                Logger::get()->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), num);
                FunctPtr fConv (new FunctFloat32(&(_raw.crate[cr].dsp[i].FFPureCurrent[j])));
                DiagnVarPtr theVar (new DiagnVar(groupname, num, fConv, hWarning, hAlarm));
                _vars.Add(theVar);
            }

//@C
// /verb+ChDistRMS+: RMS of the values in the accumulator buffer of the distances for a capacitive sensor
//@

    {
    double pos_fixed_point  = 1.0e-11; // TODO ATTENZIONE e questo da dove viene ?????
    uint32 mask[2] = {0x00FF, 0xFF00};
    int idx_patch[8]={0,1,4,5,2,3,6,7}; // riordino per baco? di Mario (ch2 e ch3) sono invertiti
    //TODO attenzione!! se gli accumulatori sono troppo lunghi il giochino d idetreminare inv_num_samples
    // partendo dall DistAverage non funziona PIU

    groupname = "ChDistRMS";
    for (int cr=0; cr<BcuMirror::N_CRATES; cr++)
        for (int i=0; i<BcuMirror::N_DSP_CRATE; i++)
            for (int j=0; j<BcuMirror::N_CH_DSP_; j++)
            {
                int num = cr*BcuMirror::N_CH_CRATE  + i*BcuMirror::N_CH_DSP_ + j;
                Logger::get()->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), num);

                //TODO questo e' pericoloso: se _vars reordinasse gli elementi quando li aggiunge al container
                // allora varAve non sarebbe piu' valido. Con lo std::vector non dovrebbe succedere.
                MatchDiagnVar fc = MatchDiagnVar("ChDistAverage", num);
                DiagnVarPtr varAverage = _vars.Find(fc);

                FunctParseAccumulatorPtr fConv    (new FunctParseAccumulator(
                        &( _raw.crate[cr].dsp[i].DistAccumulator [idx_patch[2*j]] ),
                        &( _raw.crate[cr].dsp[i].DistAccumulator [idx_patch[2*j+1]] ),
                        &( _raw.crate[cr].dsp[i].DistAccumulator2[idx_patch[2*j]] ),
                        &( _raw.crate[cr].dsp[i].DistAccumulator2[idx_patch[2*j+1]] ),
                        &( _raw.crate[cr].dsp[i].DistAccumulator2[8+j/2] ),
                        mask[(unsigned int)(j%2)],
                        pos_fixed_point,
                        varAverage
                        ));
                DiagnVarAccumulatorPtr theVar (new DiagnVarAccumulator(groupname, num, fConv, hWarning, hAlarmDump));
                _vars.Add(theVar);
            }
    }
   //@C
   // /verb+ChCurrRMS+: RMS of the values in the accumulator buffer of the coil currents
   //@
    {
    double curr_fixed_point  = 1.0e-08; // TODO ATTENZIONE e questo da dove viene ?????
    uint32 mask[2] = {0x00FF, 0xFF00};
    int idx_patch[8]={0,1,4,5,2,3,6,7}; // riordino per baco? di Mario (ch2 e ch3) sono invertiti

       groupname = "ChCurrRMS";
       for (int cr=0; cr<BcuMirror::N_CRATES; cr++)
           for (int i=0; i<BcuMirror::N_DSP_CRATE; i++)
               for (int j=0; j<BcuMirror::N_CH_DSP_; j++)
               {
                   int num = cr*BcuMirror::N_CH_CRATE  + i*BcuMirror::N_CH_DSP_ + j;
                   Logger::get()->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), num);

                   //TODO questo e' pericoloso: se _vars reordinasse gli elementi quando li aggiunge al container
                   // allora varAve non sarebbe piu' valido. Con lo std::vector non dovrebbe succedere.
                   MatchDiagnVar fc = MatchDiagnVar("ChCurrAverage", num);
                   DiagnVarPtr varAverage = _vars.Find(fc);
                   
                   FunctParseAccumulatorPtr fConv    (new FunctParseAccumulator(
                        &( _raw.crate[cr].dsp[i].CurrAccumulator [idx_patch[2*j]] ),
                        &( _raw.crate[cr].dsp[i].CurrAccumulator [idx_patch[2*j+1]] ),
                        &( _raw.crate[cr].dsp[i].CurrAccumulator2[idx_patch[2*j]] ),
                        &( _raw.crate[cr].dsp[i].CurrAccumulator2[idx_patch[2*j+1]] ),
                        &( _raw.crate[cr].dsp[i].CurrAccumulator2[8+j/2] ),
                        mask[(unsigned int)(j%2)],
                        curr_fixed_point,
                        varAverage
                        ));
                   DiagnVarAccumulatorPtr theVar (new DiagnVarAccumulator(groupname, num, fConv, hWarning, hAlarmDump));
                   _vars.Add(theVar);


                   /* LB commented out in 25/11/08 to disentangle Dist vars and Curr vars
                   //
                   // We use ChDistRMS to estimate the number of samples in the accumulator lines,
                   // for both positions and currents because the share the same inv_num_samples register.
                   //DiagnVarAccumulatorPtr varAverage = boost::shared_ptr<DiagnVarAccumulator>(
                   //        static_cast<DiagnVarAccumulator*>(boost::get_pointer(_vars.Find(MatchDiagnVar("ChDistRMS", num)))));
                   DiagnVarAccumulatorPtr varAverage = boost::static_pointer_cast<DiagnVarAccumulator, DiagnVar>
                       (_vars.Find(MatchDiagnVar("ChDistRMS", num)));

                   FunctParseAccumulatorCurrPtr fConv    (new FunctParseAccumulatorCurr(
                           &( _raw.crate[cr].dsp[i].CurrAccumulator [idx_patch[2*j]] ),
                           &( _raw.crate[cr].dsp[i].CurrAccumulator [idx_patch[2*j+1]] ),
                           &( _raw.crate[cr].dsp[i].CurrAccumulator2[idx_patch[2*j]] ),
                           &( _raw.crate[cr].dsp[i].CurrAccumulator2[idx_patch[2*j+1]] ),
                           &( _raw.crate[cr].dsp[i].CurrAccumulator2[8+j/2] ),
                           mask[(unsigned int)(j%2)],
                           curr_fixed_point,
                           varAverage
                           ));
                   DiagnVarPtr theVar (new DiagnVar(groupname, num, fConv, hWarning, hAlarm));
                   //fConv->SetTargetvar(theVar);
                   _vars.Add(theVar);
                   END LB 25/11/08  */ 
               }
    }

    /////////////// Offload (modal) ///////////////////
    groupname = "OffloadModes";
    DiagnSetPtr positions = _vars.Filter(DiagnWhich("ChDistAverage",DiagnWhich::All,DiagnWhich::All,DiagnWhich::Fast));
    unsigned int npos = positions->size(); 

    string pos2modfile = ConfigDictionary().getDir()+ "/" + (string)_cfg["Pos2ModFitsFile"];
    _logger->log(Logger::LOG_LEV_INFO, "Using pos2mod file %s [%s:%d]", pos2modfile.c_str(), __FILE__, __LINE__);

    int   ndims_p2m;
    long *dims_p2m;
    int  datatype_p2m;
    int stat = PeekFitsFile( (char*)pos2modfile.c_str(), &datatype_p2m, &ndims_p2m, &dims_p2m);
    if (stat != NO_ERROR) {
        _logger->log(Logger::LOG_LEV_ERROR, "Cannot open %s. Reason is %s [%s:%d]",pos2modfile.c_str(), lao_strerror(stat), __FILE__,__LINE__);
        throw AOException("Cannot read header from pos2mod file", stat, __FILE__, __LINE__);
    }
    if (ndims_p2m != 2) {
        _logger->log(Logger::LOG_LEV_ERROR, "Wrong data in pos2mod %s. It should be a 2D matrix instead of %dD [%s:%d]",pos2modfile.c_str(), ndims_p2m, __FILE__,__LINE__);
        throw AOException("Wrong ndim in pos2mod matrix", FILE_ERROR, __FILE__, __LINE__);
    }
    unsigned int npos_p2m   = dims_p2m[0];
    unsigned int nmodes_p2m = dims_p2m[1];

    if (datatype_p2m != DOUBLE_IMG){
        _logger->log(Logger::LOG_LEV_ERROR, "Wrong data type in pos2mod %s. It should be double (-64) instead of %d [%s:%d]",pos2modfile.c_str(), datatype_p2m, __FILE__,__LINE__);
        throw AOException("Wrong data type in pos2mod matrix", FILE_ERROR, __FILE__, __LINE__);
    }
    
    if (npos_p2m != npos){
        _logger->log(Logger::LOG_LEV_ERROR, "pos2mod %s has %d columns, position vector has %d elements [%s:%d]",pos2modfile.c_str(), npos_p2m, npos, __FILE__,__LINE__);
        throw AOException("Wrong column number in pos2mod matrix", FILE_ERROR, __FILE__, __LINE__);
    }
    
    double* matrix = (double*) ReadFitsFile((char*)pos2modfile.c_str(), &stat); //ReadBinaryFile(mplusfile, &stat);
    if (matrix == NULL){
        _logger->log(Logger::LOG_LEV_ERROR, "Cannot open %s. Reason is %s [%s:%d]",pos2modfile.c_str(), lao_strerror(stat), __FILE__,__LINE__);
        throw AOException("Cannot read pos2mod matrix file", FILE_ERROR, __FILE__, __LINE__);
    }
    // TODO do checks?
    if (stat != (int)(nmodes_p2m * npos * sizeof(double)) ){
        _logger->log(Logger::LOG_LEV_ERROR, "%s pos2mod matrix buffer has wrong size. Expected %d bytes got %d [%s:%d]",
                pos2modfile.c_str(), nmodes_p2m * npos * sizeof(float), stat,__FILE__,__LINE__);
        throw AOException("M matrix has wrong size", FILE_ERROR, __FILE__, __LINE__);
    }

    
    if (_ref_pos->size() != npos) { 
        _logger->log(Logger::LOG_LEV_ERROR, "Flat positions vector size error. Expected %d elements, got %d [%s:%d]", npos, _ref_pos->size(), __FILE__,__LINE__);
        throw AOException("Flat postions vector size error.", FILE_ERROR, __FILE__, __LINE__);
    }

    _logger->log(Logger::LOG_LEV_INFO, "RTDBvar [%s:%d]", __FILE__, __LINE__);
    string oflvar = (string)_cfg["IdlIdentity"] + "." + Utils::getAdoptSide() + "." + (string)_cfg["OffloadValuesVarName"];
    _offloadvars = RTDBvar(oflvar,  REAL_VARIABLE, nmodes_p2m);
    _logger->log(Logger::LOG_LEV_INFO, "RTDBvar2 [%s:%d]", __FILE__, __LINE__);
    //_offloadprio = RTDBvar((string)_cfg["OffloadPriorityVarName"], NO_DIR , false, INT_VARIABLE, nmodes_p2m);
    
    for (unsigned int i=0; i < nmodes_p2m; i++)
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), i);
        FunctPtr fConv    (new FunctModalProj(
                    positions,
                    i,
                    nmodes_p2m,
                    _ref_pos,
                    matrix
                    ));
        DiagnVarPtr theVar (new DiagnVar(groupname, i, fConv, hWarning, hAlarm));
        _vars.Add(theVar);
    }
    

    //@C
   // /verb+SkipFrameRate+: rate of skipped frames (ratio between skipframecounter and mirrorframecounter (WFS and OVS) )
   //@
    groupname = "SkipFrameRate";
    {
        Logger::get()->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), 0);
        MatchDiagnVar fc = MatchDiagnVar("MirrorFrameCounter", 0);
        DiagnVarPtr varFC = _vars.Find(fc);
        MatchDiagnVar skip = MatchDiagnVar("SwitchSafeSkipCounter", 0);
        DiagnVarPtr varSkip = _vars.Find(skip);
 
        FunctPtr fConv (new FunctSkipFrameRate( varSkip, varFC  ));
        DiagnVarPtr theVar (new DiagnVar(groupname, 0, fConv, hWarning, hAlarmDump));
        _vars.Add(theVar);
    }

     //@C
     // POSPISTON average position 
     //
    #include "DiagnApp_convert.h"
    groupname = "PosPiston";
    {
        Logger::get()->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), 0);
    	DiagnSetPtr positions = _vars.Filter(DiagnWhich("ChDistAverage",DiagnWhich::All,DiagnWhich::All,DiagnWhich::Fast));
 
        FunctPtr fConv (new FunctAverageDiagnSet(positions));
        DiagnVarPtr theVar (new DiagnVar(groupname, 0, fConv, hWarning, hAlarm));
        _vars.Add(theVar);
    }

     //@C
     // CURRPISTON average current 
     //
    groupname = "CurrPiston";
    {
      Logger::get()->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), 0);
      DiagnSetPtr currents = _vars.Filter(DiagnWhich("ChCurrAverage",DiagnWhich::All,DiagnWhich::All,DiagnWhich::Fast));
 
      FunctPtr fConv (new FunctAverageDiagnSet(currents));
      DiagnVarPtr theVar (new DiagnVar(groupname, 0, fConv, hWarning, hAlarm));
      _vars.Add(theVar);
    }

     //@C
     // MAXABSCURRENT max absolute current
     //
    groupname = "MaxAbsCurrent";
    {
      Logger::get()->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), 0);
      DiagnSetPtr currents = _vars.Filter(DiagnWhich("ChCurrAverage",DiagnWhich::All,DiagnWhich::All,DiagnWhich::Fast));
 
      FunctPtr fConv (new FunctMaxAbsDiagnSet(currents));
      DiagnVarPtr theVar (new DiagnVar(groupname, 0, fConv, hWarning, hAlarm));
      _vars.Add(theVar);
    }


}

//@Member: StoreOfflineFrame
//
//@
void FastDiagnostic::StoreOfflineFrame(BYTE* frame, uint32 frameSize) {
	static Logger* logger = Logger::get("STORE_O_FRAME", Logger::LOG_LEV_DEBUG);
	if(frameSize == sizeof(_raw)) {

		// --- LOW LEVEL DEBUG ---
		char str[81];
		str[80] = '\0';
		for(int b=0; b<16; b++) {
			sprintf(&(str[b*5]), "%04x ", frame[b*4]);
		}
		logger->log(Logger::LOG_LEV_DEBUG, "Offline frame: %s", str);
		// -----------------------

		memcpy((void*)(&_raw), frame, sizeof(_raw));
		logger->log(Logger::LOG_LEV_DEBUG, "Offline frame stored to raw buffer!");
	}
	else {
		logger->log(Logger::LOG_LEV_ERROR, "Offline frame size doesn't match raw buffer (not stored to raw buffer)");
	}
}

//@Member: GetRawData
//
//@
void FastDiagnostic::GetRawData()
{
	// ------------- TEST ---------------
	// Automatically load history from file
//	if(isOffline) {
//		static int i = 0;
//		logger->log(Logger::LOG_LEV_DEBUG, "FastDiagnostic::GetRawData: Loading offline frame... (%d/%d)", i, getHistorySize());
//		if(Loadofflineframe(i)) {
//			logger->log(Logger::LOG_LEV_DEBUG, "FastDiagnostic::GetRawData: Offline frame %d/%d succesfully loaded!", i, getHistorySize());
//			i = (i+1) % getHistorySize();
//		}
//		else {
//			logger->log(Logger::LOG_LEV_WARNING, "FastDiagnostic::GetRawData: Offline frame loading failed!");
//			exit(0);
//		}
//	}
	// ----------------------------------

    int  stat;
    int currentTimeout;
    bool nextShutdown = false;
    long int adsecOvs_p;
    bool loop_again=true;

    while(loop_again && TimeToDie() == false) {
        _VarAdsecOvsp.Get((int*)&adsecOvs_p);
        if(adsecOvs_p == 0) {
        	 _VarTimeoutCur.Get(&currentTimeout); // Set the current timeout with FASTDGN00.TIMEOUT_MS.CUR
        }
        else {
        	currentTimeout = adsecOvs_p;
        }

        // If offline the _raw is filled by the handler of DIAGNLOADOFFLINEFRAME message
        if(isOffline()) {
        	loop_again = false;
        	nextShutdown =  false;
        }
        else {

         stat = getFastDiagnosticRawData( MyFullName(), _bufname, &_raw, currentTimeout*2, false, &_info);

			if (IS_ERROR(stat)) {
				ToggleUnprotected(true);
				if(adsecOvs_p != 0) {
					if(nextShutdown) {
			         _logger->log(Logger::LOG_LEV_WARNING, "Timeout - Will emergencyReact now [%s:%d]", __FILE__, __LINE__);
						emergencyReact();
                        loop_again = false;
					}
                    else {
			            _logger->log(Logger::LOG_LEV_WARNING, "Timeout - Will emergencyReact next loop [%s:%d]", __FILE__, __LINE__);
					    nextShutdown = true;
				        loop_again = true;
                    }
				}
			}
			else {
				 loop_again = false;
				 nextShutdown =  false;
			}
        }
    }
    ToggleUnprotected(false);
}

//@Member: GetTimeStamp
//
// Return timestamp in seconds of the current frame
//
// A SwitchTimeStamp variable is defined but this function is called
// before updating variables hence the value of the SwitchTimeStamp DiagnVar
// refers to the previous diagnostic frame.
//
// The time stamp is in _raw.switc[0].TimeStamp and is given in unit
// of ADSEC_TIMESTAMP_PERIOD since the powering on of the electronics.
//
// Since this function is called in the fast cycle after GetRawData() (which is the only function that
// writes \verb+_raw+), it is safe to suppose that \verb+_raw_ready+ is true without
// checking it (and save some mutex-related stuff).
// that
//
// Unit must be seconds
//
//@
double FastDiagnostic::GetTimeStamp()
{
    double ts;
    //MatchDiagnVar fc = MatchDiagnVar("SwitchTimeStamp", 0);
    //DiagnVarPtr var=_vars.Find(fc);
    //ts = var->Last() * ADSEC_TIMESTAMP_PERIOD ;
    ts = (double)(_raw.switc[0].TimeStamp * ADSEC_TIMESTAMP_PERIOD) ;
    Logger::get()->log(Logger::LOG_LEV_TRACE, "TimeStamp %g [%s:%d]",ts, __FILE__, __LINE__);
    return ts;
}

//@Member: GetFrameCounter
//
// Return the frame counter of the current frame
//
// Frame Counter must monotonically increase and 2 successive frames MUST have different FrameCounter
//@
unsigned int FastDiagnostic::GetFrameCounter()
{
    MatchDiagnVar fc = MatchDiagnVar("MirrorFrameCounter", 0);
    DiagnVarPtr var=_vars.Find(fc);
    return (unsigned int)var->Last();
}

//@Member: PostInit
//
//@
void FastDiagnostic::PostInit()
{
    //AttachRawShMem();
}

//@Member: InSlowLoop
//
//@
void FastDiagnostic::InSlowLoop()
{
}

//@Member: Periodic
//
//@
void FastDiagnostic::Periodic()
{
        try {
            string family = "OFFLOADMODES";
            if (_varsOffLoadModes == NULL) {
                _logger->log(Logger::LOG_LEV_INFO, "Filtering for vars  %s  [%s:%d]", family.c_str(), __FILE__, __LINE__);
                _varsOffLoadModes  = _vars.Filter(DiagnWhich(family, DiagnWhich::All, DiagnWhich::All, DiagnWhich::Fast, DiagnWhich::Enabled)) ;
            }
            if ( _varsOffLoadModes->empty() == false ) {
                // get values
                _logger->log(Logger::LOG_LEV_TRACE, "GETTO %s  [%s:%d]",family.c_str(),__FILE__, __LINE__);
                vector<double> vvv = _varsOffLoadModes->GetValues() ;
                
                // write to rtdb
                _logger->log(Logger::LOG_LEV_DEBUG, "Writing RTDBvars for %s  [%s:%d]", family.c_str(), __FILE__, __LINE__);
                _offloadvars.Set(vvv, FORCE_SEND);
                _logger->log(Logger::LOG_LEV_TRACE, "SCRITTO %s  [%s:%d]",family.c_str(),__FILE__, __LINE__);
                
                // call AOS to offload modes
                int doOffloadModes;
                _VarDoOffloadModesCur.Get(&doOffloadModes); 

                ostringstream s;
                s << "Offload modes:";
                for (size_type i=0; i<vvv.size(); i++)
                    s << "  " << vvv[i];
                _logger->log(Logger::LOG_LEV_TRACE, s.str().c_str());
                if (doOffloadModes) {
                    _logger->log(Logger::LOG_LEV_INFO, "Calling aos_offload [%s:%d]", __FILE__, __LINE__);
                    if (vvv.size() < OFL_ZERN_NUMB)  {
                        _logger->log(Logger::LOG_LEV_WARNING, "N_elements of offloadmodes variable doesnot match OFL_ZERN_NUMB [%s:%d]", __FILE__, __LINE__);
                    } else {
                        _VarOffloadTime.Set(Utils::timeAsIsoString()); // set timestamp on rtdb
                        int ret = aos_offload(&(vvv[0]));
                        if (IS_ERROR(ret))  _logger->log(Logger::LOG_LEV_WARNING, "Error sending offloadmodes command to aos: %s [%s:%d]", lao_strerror(ret), __FILE__, __LINE__);
                    }
                }
                
            }
        } catch (AOException &e) {
            _logger->log(Logger::LOG_LEV_ERROR, "%s", e.what().c_str());
        }
        
	/// High Order Offload
        try { 
	    //static int n_loop = 0;
	    int average_interval = 20; // period between offloads [s]
	    
	    _logger->log(Logger::LOG_LEV_DEBUG, "HighOrder offload averaging %d  [%s:%d]", _highOrderOffloadLoopCounter % average_interval,  __FILE__, __LINE__);
	    
	    // read CurrPiston
	    DiagnVarPtr varCurrPiston = _vars.Find( MatchDiagnVar("CurrPiston", 0) );
	    double currPiston  = varCurrPiston->Value();


	    // reset every average_interval loops
	    if (_highOrderOffloadLoopCounter % average_interval == 0) 
		for (int i=0; i<BcuMirror::N_CH; i++) _currAveNullMean60[i]=0;

	    // accumulate values CurrAveNullMean = Mean ( ChCurrAverage - CurrPiston - (ref_for - ref_for_mean), 60s ) 
	    DiagnSetPtr varCurr  = _vars.Filter(DiagnWhich("ChCurrAverage", DiagnWhich::All, DiagnWhich::All)) ;
	    int ctr=0;
	    for (DiagnSet::iterator it = varCurr->begin();   it != varCurr->end();    it++) {
		   _currAveNullMean60[ctr] += (**it).isEnabled() ? ( (**it).Value() -currPiston - _ref_for->at(ctr) + _ref_for_mean) / average_interval : 0.0;  // remove NaN val ;
	    	if (ctr==0) _logger->log(Logger::LOG_LEV_DEBUG, "CurrPiston %g Ref Force %g Ref Force Mean %g [%s:%d]", 
            currPiston, _ref_for->at(ctr), _ref_for_mean, 
               __FILE__, __LINE__);
		   ctr++;
	    }

	    // increment counter
	    _highOrderOffloadLoopCounter++;

	    // average done. Compute HighOrderOffloadVector = HOOffloadMatrix ## (CurrAveNullMean60-RefForceNullMean)
	   if (_high_order_offload_matrix != NULL &&  _highOrderOffloadLoopCounter % average_interval == 0) {
	    	_logger->log(Logger::LOG_LEV_DEBUG, "HighOrder offload matrix %g %g %g %g [%s:%d]", 
            _high_order_offload_matrix[0], _high_order_offload_matrix[1], _high_order_offload_matrix[2], _high_order_offload_matrix[3], 
               __FILE__, __LINE__);
	    	_logger->log(Logger::LOG_LEV_DEBUG, "CurrAveNullMean %g %g %g %g [%s:%d]", 
            _currAveNullMean60[0], _currAveNullMean60[1], _currAveNullMean60[2], _currAveNullMean60[3], 
               __FILE__, __LINE__);
		   
         Utils::matrixVect(_high_order_offload_matrix,  _currAveNullMean60, _offloadCommandVector, BcuMirror::N_CH, BcuMirror::N_CH);
		
	    	_logger->log(Logger::LOG_LEV_DEBUG, "OffloadCommandVector %g %g %g %g [%s:%d]", 
            _offloadCommandVector[0], _offloadCommandVector[1], _offloadCommandVector[2], _offloadCommandVector[3], 
               __FILE__, __LINE__);
		   
         // WriteFitsFile to OffloadCommandVectorFitsFile
		   long dims[] = {BcuMirror::N_CH};
		   string HOOffloadCommandFile = (string)(ConfigDictionary()["HighOrderOffloadCommandFile"]);
         unlink((char*)HOOffloadCommandFile.c_str()); // cfitsio does not overwrite files!! 
         int stat = WriteFitsFile( (char*)HOOffloadCommandFile.c_str(), (unsigned char *)_offloadCommandVector, TDOUBLE, dims, 1);
         if (stat != NO_ERROR) {
		      _logger->log(Logger::LOG_LEV_ERROR, "highOrderOffloadVector fits file not written [%s:%d]", __FILE__, __LINE__);
         }

		// Log to telemetry
		   try {
		      string telelogfile  = (string)(ConfigDictionary()["TelemetryLogFile"]);
		      Logger::get("HighOrderOffload", Logger::LOG_LEV_INFO, telelogfile)->log_telemetry(Logger::LOG_LEV_INFO,  _offloadCommandVector, BcuMirror::N_CH) ;
		   } catch (...) {
		      _logger->log(Logger::LOG_LEV_ERROR, "highOrderOffloadVector not logged in telemetry  [%s:%d]", __FILE__, __LINE__);
		   }
		   // Send Notify to Arbitrator
		   AlertNotifier* adSecArbNotifier = new AlertNotifier((string)(AOApp::ConfigDictionary()["AdSecArbName"])+"."+Side());
		   HighOrderOffload highOrderOffload( (string)(ConfigDictionary()["HighOrderOffloadCommandFile"])   );
		   adSecArbNotifier->notifyAlert((Alert*)(&highOrderOffload));
       }
		
/*        
*/
	} catch (AOException &e) {
            _logger->log(Logger::LOG_LEV_ERROR, "highOrderOffload failed: %s", e.what().c_str());
	}
}

/* Check whether any variable got an alarm, and dump the masterdiagnostic in case.
 * Dump again only after a good frame
 */ 

void FastDiagnostic::InFastLoop() {

    static long prevdumpAlarm = 0;
    timeval     prevdumpTime;

    long dumpalarm = getDumpAlarmAndClear();

     if ((dumpalarm) && (dumpalarm != prevdumpAlarm)) {
        try {
            _mastdiagninterface->dumpAdSecDiagnHistory();
            _logger->log(Logger::LOG_LEV_INFO, "dumpadsecdiagnhistory succesfully sent to masterdiagnostic [%s:%d]", __FILE__, __LINE__);
            gettimeofday(&prevdumpTime, NULL);
        } catch (MastDiagnInterfaceException& e) {
            // Nothing to do!
            _logger->log(Logger::LOG_LEV_ERROR, "dumpadsecdiagnhistory not sent! [%s:%d]", __FILE__, __LINE__);
        } catch (...) {
            _logger->log(Logger::LOG_LEV_ERROR, "unexpected exception:  [%s:%d]",  __FILE__, __LINE__);
        }
        
    }
    else if (dumpalarm) {
        _logger->log(Logger::LOG_LEV_INFO, "dumpadsecdiagnhistory already sent. [%s:%d]",  __FILE__, __LINE__);
    }
    
    prevdumpAlarm = dumpalarm;
}



