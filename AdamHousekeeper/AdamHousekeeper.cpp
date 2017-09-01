//@File: AdamHousekeeper.cpp
//
// Implementation of the AdamHousekeeper class
//@

#include <cstdio>
#include <cstring>
#include <sstream>


extern "C" {
#include "base/thrdlib.h"    
#include "iolib.h" //ReadBinaryFile
#include "commlib.h"
}

#include "AdamHousekeeper.h"
#include "Timing.h"


// Initialize static members
//int AdamHousekeeper::use_common =0;
//        

//@Member AdamHousekeeper
//
//Standard constructor
//@

AdamHousekeeper::AdamHousekeeper(int argc, char **argv) throw (AOException) : 
    DiagnApp(argc, argv),
    _adam(NULL)
{
    ReadConfig(); 
    CreateFD();
}



//AdamHousekeeper::AdamHousekeeper(const std::string& conffile, int verb )
//    : DiagnApp::DiagnApp(conffile, verb)
void AdamHousekeeper::CreateFD()
{
    //_unprotected = true;
    
    // Creates an object able to react in case of emergency
    if ((_adam == NULL) && (!_dummy)) {
        try {
            _adam = new AdamModbus();
        }
        catch(TcpCreationException& e) {
            _logger->log(Logger::LOG_LEV_ERROR, "AdamModbus: unable to create the Tcp connection");
        }
    }
}

//@Member $\sim$ AdamHousekeeper
//
//Destructor
//@
AdamHousekeeper::~AdamHousekeeper()
{
    _logger->log(Logger::LOG_LEV_FATAL, "Housekeeper is exiting. Will shut down the mirror soon  [%s:%d]",__FILE__,__LINE__);
	if(_adam) {
//	  _adam->disableMainPower();
//    _logger->log(Logger::LOG_LEV_FATAL, "Adam main power disabled  [%s:%d]",__FILE__,__LINE__);
        delete _adam;
	}
}

//@Member: ReadConfig
//
//@
void AdamHousekeeper::ReadConfig()
{
    _dummy = (bool) ( (int)(ConfigDictionary()["DummyMode"]));
    if(_dummy) _logger->log(Logger::LOG_LEV_WARNING, "PROGRAM IS RUNNING IN DUMMY MODE [%s:%d]",__FILE__,__LINE__);

    // insert stuff here
}


//@Member: SetupVars
//
//@
void AdamHousekeeper::SetupVars()
{
    try{
        DiagnApp::SetupVars();
        _tssStatus = RTDBvar(MyFullName(), "TSS_STATUS", NO_DIR, INT_VARIABLE, 1);
        _tssStatus.Set(ADAM_TSS_READY);
        _tssEnabled = RTDBvar(MyFullName(), "TSS_ENABLED", NO_DIR, INT_VARIABLE, 1);
        _tssEnabled.Set(true);
        _coilStatus = RTDBvar(MyFullName(), "COIL_STATUS", NO_DIR, INT_VARIABLE, 1);
        _coilStatus.Set(false);
        _mainpowerStatus = RTDBvar(MyFullName(), "MAIN_POWER_STATUS", NO_DIR, INT_VARIABLE, 1);
        _mainpowerStatus.Set(false);
    } catch(AOException &e){
        _logger->log(Logger::LOG_LEV_ERROR, "%s [%s:%d]",e.what().c_str(),__FILE__,__LINE__);
        // TODO FATAL or ERROR. Eventually die
    }
}

//@Member: InstallHandlers
//
//@
void AdamHousekeeper::InstallHandlers()
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
void AdamHousekeeper::CreateDiagnVars()
{

    std::string          groupname;
    FunctActionPtr       hAlarm   (new FunctAlarm());                   
    FunctActionPtr       hWarning (new FunctWarning());

//------------------------------- ADAM VARIABLES -------------------------------------------------------
//
//@C
// /verb+AdamMainPower+ 
//@
    groupname = "AdamFrameCounter";
    {
        _timestamp_tmp = 0; 
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), 0);
        FunctPtr fConv (new FunctInt32((int32 *)&_timestamp_tmp));
        DiagnVarPtr theVar (new DiagnVar(groupname, 0, fConv, hWarning, hAlarm));
       _vars.Add(theVar); 
    }

//
//@C
// /verb+AdamMainPower+ 
//@
    groupname = "AdamMainPower";
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), 0);
        FunctPtr fConv (new FunctInt32((int32 *)&_adamvec[ADAM_MAINPOWER_IDX]));
        DiagnVarPtr theVar (new DiagnVar(groupname, 0, fConv, hWarning, hAlarm));
       _vars.Add(theVar); 
    }
//@C
// /verb+AdamWatchdogExp+ 
//@
    groupname = "AdamWatchdogExp";
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), 0);
        FunctPtr fConv (new FunctInt32((int32 *)&_adamvec[ADAM_WATCHDOG_EXP_IDX]));
        DiagnVarPtr theVar (new DiagnVar(groupname, 0, fConv, hWarning, hAlarm));
       _vars.Add(theVar); 
    }
//@C
// /verb+AdamTSSDisable+ 
//@
    groupname = "AdamTSSDisable";
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), 0);
        FunctPtr fConv (new FunctInt32((int32 *)&_adamvec[ADAM_TSS_DISABLE_IDX]));
        DiagnVarPtr theVar (new DiagnVar(groupname, 0, fConv, hWarning, hAlarm));
       _vars.Add(theVar); 
    }
//@C
// /verb+AdamDriverEnable+ 
//@
    groupname = "AdamDriverEnable";
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), 0);
        FunctPtr fConv (new FunctInt32((int32 *)&_adamvec[ADAM_DRIVER_ENABLE_IDX]));
        DiagnVarPtr theVar (new DiagnVar(groupname, 0, fConv, hWarning, hAlarm));
       _vars.Add(theVar); 
    }
//@C
// /verb+AdamFPGAClearN+ 
//@
    groupname = "AdamFPGAClearN";
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), 0);
        FunctPtr fConv (new FunctInt32((int32 *)&_adamvec[ADAM_FPGA_CLEARN_IDX]));
        DiagnVarPtr theVar (new DiagnVar(groupname, 0, fConv, hWarning, hAlarm));
       _vars.Add(theVar); 
    }
//@C
// /verb+AdamBootSelectN+ 
//@
    groupname = "AdamBootSelectN";
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), 0);
        FunctPtr fConv (new FunctInt32((int32 *)&_adamvec[ADAM_BOOTSELECTN_IDX]));
        DiagnVarPtr theVar (new DiagnVar(groupname, 0, fConv, hWarning, hAlarm));
       _vars.Add(theVar); 
    }
//@C
// /verb+AdamSysResetN+ 
//@
    groupname = "AdamSysResetN";
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), 0);
        FunctPtr fConv (new FunctInt32((int32 *)&_adamvec[ADAM_SYS_RESETN_IDX]));
        DiagnVarPtr theVar (new DiagnVar(groupname, 0, fConv, hWarning, hAlarm));
       _vars.Add(theVar); 
    }
//@C
// /verb+AdamTSSPowerFaultN0+ 
//@
    groupname = "AdamTSSPowerFaultN0";
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), 0);
        FunctPtr fConv (new FunctInt32((int32 *)&_adamvec[ADAM_TSS_POWER_FAULTN0_IDX]));
        DiagnVarPtr theVar (new DiagnVar(groupname, 0, fConv, hWarning, hAlarm));
       _vars.Add(theVar); 
    }
//@C
// /verb+AdamTSSPowerFaultN1+ 
//@
    groupname = "AdamTSSPowerFaultN1";
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), 0);
        FunctPtr fConv (new FunctInt32((int32 *)&_adamvec[ADAM_TSS_POWER_FAULTN1_IDX]));
        DiagnVarPtr theVar (new DiagnVar(groupname, 0, fConv, hWarning, hAlarm));
       _vars.Add(theVar); 
    }
//@C
// /verb+AdamTSSFaultN+ 
//@
    groupname = "AdamTSSFaultN";
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), 0);
        FunctPtr fConv (new FunctInt32((int32 *)&_adamvec[ADAM_TSS_POWER_FAULTN_IDX]));
        DiagnVarPtr theVar (new DiagnVar(groupname, 0, fConv, hWarning, hAlarm));
       _vars.Add(theVar); 
    }
//@C
// /verb+AdamTCSSysFault+ 
//@
    groupname = "AdamTCSSysFault";
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), 0);
        FunctPtr fConv (new FunctInt32((int32 *)&_adamvec[ADAM_TCS_SYS_FAULT_IDX]));
        DiagnVarPtr theVar (new DiagnVar(groupname, 0, fConv, hWarning, hAlarm));
       _vars.Add(theVar); 
    }
//@C
// /verb+AdamTCSPowerFaultN0+ 
//@
    groupname = "AdamTCSPowerFaultN0";
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), 0);
        FunctPtr fConv (new FunctInt32((int32 *)&_adamvec[ADAM_TCS_POWER_FAULT_N0]));
        DiagnVarPtr theVar (new DiagnVar(groupname, 0, fConv, hWarning, hAlarm));
       _vars.Add(theVar); 
    }
//@C
// /verb+AdamTCSPowerFaultN1+ 
//@
    groupname = "AdamTCSPowerFaultN1";
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), 0);
        FunctPtr fConv (new FunctInt32((int32 *)&_adamvec[ADAM_TCS_POWER_FAULT_N1]));
        DiagnVarPtr theVar (new DiagnVar(groupname, 0, fConv, hWarning, hAlarm));
       _vars.Add(theVar); 
    }
//@C
// /verb+AdamTCSPowerFaultN2+ 
//@
    groupname = "AdamTCSPowerFaultN2";
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), 0);
        FunctPtr fConv (new FunctInt32((int32 *)&_adamvec[ADAM_TCS_POWER_FAULT_N2]));
        DiagnVarPtr theVar (new DiagnVar(groupname, 0, fConv, hWarning, hAlarm));
       _vars.Add(theVar); 
    }

}



//@Member: GetRawData
//
//@
void AdamHousekeeper::GetRawData()
{
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
          // read all BCU boards on every crate
          try{
              
              // try connection
              if (_adam == NULL) 
                  CreateFD();
              
              if (_adam != NULL)
                  ReadAdam();
              loop_again = false;

          } catch (CommandSenderException &e) {
             ToggleUnprotected(true);
             _logger->log(Logger::LOG_LEV_ERROR, "Error reading ADAM:  %s [%s:%d]", e.what().c_str(), __FILE__, __LINE__);
             msleep(1000); 
             continue;
          }
       } else { 
            //dummy mode
            adam_dummy();
            loop_again = false;
       }
       ToggleUnprotected(false);

       }
      catch(AOException &e) {
         _logger->log( Logger::LOG_LEV_ERROR, "Exception!! %s", e.what().c_str());
         msleep(1000);
         continue;
      }
      _logger->log(Logger::LOG_LEV_DEBUG, "Adam in status: %d:%d:%d:%d:%d:%d:%d:%d", _adamvec[0], _adamvec[1], _adamvec[2], _adamvec[3], _adamvec[4], _adamvec[5], _adamvec[6], _adamvec[7]);
      _logger->log(Logger::LOG_LEV_DEBUG, "Adam out status: %d:%d:%d:%d:%d:%d:%d:%d", _adamvec[8], _adamvec[9], _adamvec[10], _adamvec[11], _adamvec[12], _adamvec[13], _adamvec[14], _adamvec[15]);
      _logger->log(Logger::LOG_LEV_DEBUG, "Adam frame counter status: %d", _timestamp_tmp) ;
    }	                
}



void AdamHousekeeper::ReadAdam()
{
   //@C
   // Read (every cycle) the current ADAM status in input
   //@

//    char cmd[] = "$016\r";
//    char ans[16];
//    const int cmdLen = 5;
//    const int ansLen = 10;

    int bitmask;

/*
    int adamvec[8];

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

    _timestamp_tmp +=1;
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
       _adamvec[i]=bit;
    }
*/

    bool stat;

        stat = _adam->ReadInputStatus(1, 0, 8, &bitmask);
        if (!stat) {
           _adam->clearTcp();
           _adam->initTcp();
           return;
        }

        for (int i=7; i>=0; i--) {
            int bit = 1 << i;
            _adamvec[i]= (bitmask & bit) ? 1:0;
        }
        _logger->log(Logger::LOG_LEV_INFO, "Adam in status:  %d:%d:%d:%d:%d:%d:%d:%d", _adamvec[0], _adamvec[1], _adamvec[2], _adamvec[3], _adamvec[4], _adamvec[5], _adamvec[6], _adamvec[7]);


        _adam->ReadCoilStatus(1, 16, 8, &bitmask);
        if (!stat) {
           _adam->clearTcp();
           _adam->initTcp();
           return;
        }

        for (int i=7; i>=0; i--) {
            int bit = 1 << i;
            _adamvec[i+8]=(bitmask & bit) ? 1 :0;
        }
        _logger->log(Logger::LOG_LEV_INFO, "Adam out status: %d:%d:%d:%d:%d:%d:%d:%d", _adamvec[8], _adamvec[9], _adamvec[10], _adamvec[11], _adamvec[12], _adamvec[13], _adamvec[14], _adamvec[15]);


   if (_adamvec[ADAM_TCS_SYS_FAULT_IDX] && !_adamvec[ADAM_TSS_DISABLE_IDX]) {
       //TSSActivated
       if ((_adamvec[ADAM_TSS_POWER_FAULTN0_IDX]==0) || (_adamvec[ADAM_TSS_POWER_FAULTN1_IDX]==0)) {
           // TSS malfunctioning
           _tssStatus.Set(ADAM_TSS_MALFUNCTION);
       }
       else {
           _tssStatus.Set(ADAM_TSS_ACTIVATED);
       }
   }
   else if (_adamvec[ADAM_TSS_DISABLE_IDX]) {
       _tssStatus.Set(ADAM_TSS_DISABLED);
   }
   else {
       _tssStatus.Set(ADAM_TSS_READY);
   }

   (_adamvec[ADAM_TSS_DISABLE_IDX] == 0) ? _tssEnabled.Set(true) : _tssEnabled.Set(false);
   (_adamvec[ADAM_DRIVER_ENABLE_IDX] == 1) ? _coilStatus.Set(true) : _coilStatus.Set(false);
   (_adamvec[ADAM_MAINPOWER_IDX] == 1) ? _mainpowerStatus.Set(true) : _mainpowerStatus.Set(false);
}


//@Member: GetFrameCounter
//
//@
unsigned int AdamHousekeeper::GetFrameCounter()
{
    //static unsigned int fc=0;
    MatchDiagnVar fc = MatchDiagnVar("AdamFrameCounter", 0);
    DiagnVarPtr var=_vars.Find(fc);
    return (unsigned int)var->Last();
}


//@Member: PostInit
//
//@
void AdamHousekeeper::PostInit()
{
	
	// Set the thrdlib time to leave (TTL) for the incoming queue.
	// Because the TTL is equal to MirrorCtrl timeout, if the MirrorCtrl
	// reply slowly (i.e. because of BCUs off), the thrdlib will clean 
	// unreceived messages and prevent the queue overflow.
	int thMsgTTL_sec = ConfigDictionary()["timeout_sec"];
	thDefTTL(thMsgTTL_sec);

}


//@Member: InSlowLoop
//
//@
void AdamHousekeeper::InSlowLoop()
{
}

//@Member: InFastLoop
//
//@
void AdamHousekeeper::InFastLoop()
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

}    

void AdamHousekeeper::Periodic()
{
}

//@Member: 
//
//@
#include <stdlib.h>
//static 

#define SetBit(dato,n) (dato=((1<<n)|dato))     /* Set bit n high in word dato                     */

void AdamHousekeeper::adam_dummy()
{
    for (int i=0; i<16; i++){
      _adamvec[i] = (int)round((double)rand()/RAND_MAX);
    }
    _timestamp_tmp +=1;
}

