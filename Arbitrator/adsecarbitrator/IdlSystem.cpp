//@File: IdlSystem.h

#include <math.h>

#include "adsecarbitrator/IdlSystem.h"
#include "adsecarbitrator/AdSecArbitrator.h"
#include "idllib/IdlCtrlInterface.h"
#include "NaNutils.h"
#include "NaNutils.h"
#include "base/errordb.h"

using namespace Arcetri::AdSec_Arbitrator;


// ******************** TEST ************************** //
// To force all commands to set cmdRes = true
//#define TEST_ONLY


/********************** PUBLIC *********************/

IdlSystem::IdlSystem(bool simulation): AbstractSystem("IDL") {
	_arbitrator = AbstractArbitrator::getInstance();
   _gainZero=true;
   _loopClosed=false;
   _simulation=simulation;
}

IdlSystem::~IdlSystem() {

}


AbstractSystemCommandResult IdlSystem::sendIdlCommand(string idlCmd, int timeout_ms, bool checkStderr) {

	static IdlCtrlInterface idlCtrlIntf(((std::string)AdSecArbitrator::ConfigDictionary()["IdlCtrlProcess"])+"."+AOApp::Side(), Logger::stringToLevel(AdSecArbitrator::ConfigDictionary()["IDLLogLev"]));
   AbstractSystemCommandResult res;
   string errstr;

	try {
		_logger->log(Logger::LOG_LEV_INFO, "Command (timeout ms): %s (%d)", idlCmd.c_str(), timeout_ms);
		IdlCommandReply reply = idlCtrlIntf.sendCommand(idlCmd, timeout_ms);
		string idlErr = reply.getError();
		string idlOut = reply.getOutput();
		if(idlErr != "") {
         if (!checkStderr) {
            // Output on stderr is ignored because IDL has the habit of writing there when compiling modules, etc.
			   _logger->log(Logger::LOG_LEV_WARNING, "Parsing IDLRPC stderr");
			   _logger->log(Logger::LOG_LEV_WARNING, "%s", idlErr.c_str());

           if (strstr(idlErr.c_str(),"% Execution halted at:") == NULL) {
              _logger->log(Logger::LOG_LEV_DEBUG, "No syntax issue");
           }
           else {
              _logger->log(Logger::LOG_LEV_WARNING, "Syntax error found. Catching...");
              IdlCommandReply reply = idlCtrlIntf.sendCommand("print, errcode", timeout_ms);
              idlErr = reply.getError();
              idlOut = reply.getOutput();
              _logger->log(Logger::LOG_LEV_WARNING, "Syntax error successfully catched");
           }

         } else {
			   _logger->log(Logger::LOG_LEV_ERROR, "%s", idlErr.c_str());
            res = SysCmdError;
            errstr = "Error in IDL command. Execution halted.";
            setCommandResult( res, errstr);     // Force command result
            return res;
         }
		}

      string token;
      stringstream ss(idlOut);
      while (ss >> token);     // Split into token and get last token
      int errcode = atoi(token.c_str());
      errstr = lao_strerror(errcode);
      setErrorString(lao_strerror(errcode));

      if (    (errcode == IDL_GENERIC_ERROR)     || (errcode == IDL_MISSING_INPUT) 
           || (errcode == IDL_SC_SENDING_SLOPES) || (errcode == IDL_INVALID_FILE) 
           || (errcode == IDL_INPUT_TYPE)        || (errcode == IDL_STROKE_REQUIRED)
           || (errcode == IDL_FORCE_REQUIRED)    || (errcode == IDL_UNEXP_ADAM_ANS)
           || (errcode == IDL_DIST_ACTIVE)       || (errcode == IDL_FLAT_NOT_FOUND)
         )
         res = SysCmdRetry;
      else if (errcode <0)
         res = SysCmdError;
      else 
		 res = SysCmdSuccess;
	}
	catch(IdlCtrlInterfaceException& e) {
		_logger->log(Logger::LOG_LEV_ERROR, "%s", e.what().c_str());
      errstr = "Exception in IdlCtrlInterface: " + e.what();
      res = SysCmdError;
	}

   setCommandResult( res, errstr);

   return res;
}

AbstractSystemCommandResult IdlSystem::simpleCommand( string cmdName, int timeout_ms, string idlFunc) {

	initCommandResult();
	_logger->log(Logger::LOG_LEV_INFO, "Executing cmd: %s (%s) - timeout %d ms...", 
                 cmdName.c_str(), idlFunc.c_str(), timeout_ms);
   AbstractSystemCommandResult cmdRes;

   string theCommand="errcode=adsec_error.idl_syntax_error & errcode= " + idlFunc + " & print, errcode";

   if(_simulation) {
		_logger->log(Logger::LOG_LEV_INFO, "SIMULATION: IDL cmd: "+theCommand);
      setCommandResult(SysCmdSuccess);
		return SysCmdSuccess;
   }

   cmdRes = sendIdlCommand(theCommand, timeout_ms, false);

	if(cmdRes == SysCmdRetry) {
		Warning warning( cmdName + " not executed (retry)");
		_arbitrator->notifyClients(&warning);
	}
	else if(cmdRes == SysCmdError) {
		//Error error( cmdName + " failed");
      // TO BE VERIFIED IF THE CODE HAS NO SYNTAX ERROR AND RESTORE TO ERROR EVENT
		Warning warning( cmdName + " failed or syntax error occured");
		_arbitrator->notifyClients(&warning);
	}

   return cmdRes;
}

void IdlSystem::powerOn(int timeout_ms) {
   simpleCommand ("powerOn", timeout_ms, "fsm_power_on()");
}

void IdlSystem::powerOff(int timeout_ms) {
   simpleCommand("powerOff", timeout_ms, "fsm_power_off()");
}

void IdlSystem::loadProgram(int timeout_ms) {
   simpleCommand("loadProgram", timeout_ms, "fsm_load_program(/AUTO)");
}

void IdlSystem::setFlatFF(int timeout_ms) {
   simpleCommand("setFlatFF", timeout_ms, "fsm_set_flat_ff()");
}

void IdlSystem::ifAcquire(int timeout_ms) {
   simpleCommand("ifAcquire", timeout_ms, "fsm_if_acquire()");
}

void IdlSystem::ifReduce(int timeout_ms) {
   simpleCommand("ifReduce", timeout_ms, "fsm_if_reduce()");
}

void IdlSystem::dustTest(int timeout_ms) {
   simpleCommand("dustTest", timeout_ms, "fsm_dust_test()");
}

void IdlSystem::coilTest(int timeout_ms) {
   simpleCommand("coilTest", timeout_ms, "fsm_coil_test()");
}

void IdlSystem::capsensTest(int timeout_ms) {
   simpleCommand("capsensTest", timeout_ms, "fsm_capsens_test()");
}

void IdlSystem::ffAcquire(int timeout_ms) {
   simpleCommand("ffAcquire", timeout_ms, "fsm_ff_acquire()");
}

void IdlSystem::ffReduce(int timeout_ms) {
   simpleCommand("ffReduce", timeout_ms, "fsm_ff_reduce()");
}

void IdlSystem::wdTest(int timeout_ms) {
   simpleCommand("wdTest", timeout_ms, "fsm_wd_test()");
}

void IdlSystem::flTest(int timeout_ms) {
   simpleCommand("flTest", timeout_ms, "fsm_fl_test()");
}

void IdlSystem::psTest(int timeout_ms) {
   simpleCommand("psTest", timeout_ms, "fsm_ps_test()");
}

void IdlSystem::recoverSkipFrame(int timeout_ms) {
   simpleCommand("recoverSkipFrame", timeout_ms, "fsm_skip_recovery()");
}

void IdlSystem::reset(int timeout_ms) {
   simpleCommand("reset", timeout_ms, "fsm_reset()");
}

void IdlSystem::setFlatChop(int timeout_ms, flatParams /* params */) {
   simpleCommand("setFlatChop", timeout_ms, "fsm_set_flat()");
}

void IdlSystem::setFlatAo( float elevation, int timeout_ms) {
   ostringstream cmd;
   cmd << "fsm_set_flat( elevation=" << elevation << ")";
   simpleCommand("setFlatAo", timeout_ms, cmd.str());
}

/*
void IdlSystem::setFlatSl(int timeout_ms, flatParams params) {
   simpleCommand("setFlatSl", timeout_ms, "fsm_set_flat()");
}
*/

void IdlSystem::rest(int timeout_ms) {
   simpleCommand("rest", timeout_ms, "fsm_rip()");
}

void IdlSystem::applyCommands(int timeout_ms, applyCommandParams params) {
   string cmd = "fsm_apply_cmd('"+params.commandsFile+"')"; 
   simpleCommand("applyCommands", timeout_ms, cmd);
}

void IdlSystem::HOOffload(int timeout_ms, applyCommandParams params) {
   string cmd = "fsm_ho_offload('"+params.commandsFile+"')"; 
   simpleCommand("applyCommands", timeout_ms, cmd);
}

/*
void IdlSystem::chopToSl(int timeout_ms) {
   simpleCommand("chopToSl", timeout_ms, "switch_state_sl_cmd");
}

void IdlSystem::aoToSl(int timeout_ms) {
   simpleCommand("aoToSl", timeout_ms, "switch_state_sl_cmd");
}

void IdlSystem::slToAo(int timeout_ms) {
   simpleCommand("slToAo", timeout_ms, "switch_state_ao_cmd");
}
*/

void IdlSystem::chopToAo(int timeout_ms) {
   simpleCommand("chopToAo", timeout_ms, "switch_state_ao_cmd");
}

void IdlSystem::aoToChop(int timeout_ms) {
   simpleCommand("aoToChop", timeout_ms, "switch_state_chop_cmd");
}

/*
void IdlSystem::slToChop(int timeout_ms) {
   simpleCommand("slToChop", timeout_ms, "switch_state_chop_cmd");
}
*/

void IdlSystem::runChop(int timeout_ms) {
   simpleCommand("runChop", timeout_ms, "run_chop_cmd");
}

void IdlSystem::stopChop(int timeout_ms) {
   simpleCommand("stopChop", timeout_ms, "stop_chop_cmd");
}

void IdlSystem::runAo(int timeout_ms, runAoParams params) {

   // Use the oversampling period instead of the frequency
   float ovsPeriod;
   if (params._ovsFreq == 0)
      ovsPeriod = 0.02; 
   else
      ovsPeriod = 1.0/params._ovsFreq;

   ostringstream cmd;
   cmd << "fsm_run_ao(" << params._loopFreq << ", " << params._decimation << ", " << ovsPeriod << ")";

   simpleCommand("runAo", timeout_ms, cmd.str());

   _loopClosed = (getCommandResult() == SysCmdSuccess);
}

void IdlSystem::setZernikes(int timeout_ms, setZernikesParams params) {
	_logger->log(Logger::LOG_LEV_INFO, "Executing setZernikes with timeout %d ms...", timeout_ms);
    ostringstream cmd;
    ostringstream params_oss;
    cmd << "fsm_correct_modes([" << params.deltaM[0];
    for (int i=1 ; i<=21; i++) cmd << "," << params.deltaM[i] ;
    cmd << "],/PASSED, /DELTA)";
    AbstractSystemCommandResult res = simpleCommand("setZernikes", timeout_ms, cmd.str());
    if (res != SysCmdSuccess){ 
        Warning warn("Set zernikes request failed. No delta shape has been applied");
        _arbitrator->notifyClients(&warn);
    }

 
   /// T.B.I.
}

void IdlSystem::stopAo(int timeout_ms, bool restoreShape) {
   _loopClosed = false;
   string cmd;
   if (restoreShape) 
        cmd = "fsm_stop_ao()";
   else
        cmd = "fsm_stop_ao(/HOLD)";

   simpleCommand("stopAo", timeout_ms, cmd);
   _loopClosed = !(getCommandResult() == SysCmdSuccess);
}

void IdlSystem::pauseAo(int timeout_ms) {
   simpleCommand("pauseAo", timeout_ms, "fsm_pause_ao()");
   _loopClosed = !(getCommandResult() == SysCmdSuccess);
}

void IdlSystem::resumeAo(int timeout_ms) {
   simpleCommand("resumeAo", timeout_ms, "fsm_resume_ao()");
   _loopClosed = (getCommandResult() == SysCmdSuccess);
}

void IdlSystem::setGain(int timeout_ms, gainParams params) {
	_logger->log(Logger::LOG_LEV_INFO, "Executing setGain with timeout %d ms...", timeout_ms);
	_logger->log(Logger::LOG_LEV_INFO, "	> gainFile:  %s", params._gainFile.c_str());

   if (params._gainFile.find("gain0.fits") != string::npos)
      _gainZero=true;
   else
      _gainZero=false;

   string cmd = "fsm_set_gain('"+ params._gainFile+"')";

   simpleCommand("setGain", timeout_ms, cmd);
}

bool IdlSystem::isGainZero() {
   return _gainZero;
}

bool IdlSystem::isLoopClosed() {
   return _loopClosed;
}


void IdlSystem::saveSlopes(int timeout_ms, saveSlopesParams params) {
	_logger->log(Logger::LOG_LEV_INFO, "Executing saveSlopes with timeout %d ms...", timeout_ms);
	_logger->log(Logger::LOG_LEV_INFO, "	> filename:  %s", params._filename.c_str());
	_logger->log(Logger::LOG_LEV_INFO, "	> n frames:  %d", params._nFrames);

   ostringstream cmd;
   cmd << "fsm_get_slopes('" << params._filename << "', " << params._nFrames << ")";

   simpleCommand("saveSlopes", timeout_ms, cmd.str());
}

void IdlSystem::saveStatus(int timeout_ms, saveStatusParams params) {
	_logger->log(Logger::LOG_LEV_INFO, "Executing saveStatus with timeout %d ms...", timeout_ms);
	_logger->log(Logger::LOG_LEV_INFO, "	> filename:  %s", params._filename.c_str());

   ostringstream cmd;
   cmd << "fsm_savestatus('" << params._filename << "', /SAVE_ACCEL)";

   simpleCommand("saveStatus", timeout_ms, cmd.str());
}

void IdlSystem::setDisturb(int timeout_ms, disturbParams params) {
	_logger->log(Logger::LOG_LEV_INFO, "Executing setDisturb with timeout %d ms...", timeout_ms);
	_logger->log(Logger::LOG_LEV_INFO, "	> disturbFile:  %s", params._disturbFile.c_str());
	_logger->log(Logger::LOG_LEV_INFO, "	> disturbActive:  %s", params._on ? "ON": "OFF");

   AbstractSystemCommandResult cmdRes;

   if (!params._on) {
      simpleCommand("setDisturb", timeout_ms, "fsm_disable_disturb()");
   }
   else {

      string cmd = "fsm_load_disturb('"+ params._disturbFile+"')";
      cmdRes = simpleCommand("setDisturb", timeout_ms, cmd);

      if (cmdRes == SysCmdSuccess)
         simpleCommand("setDisturb", timeout_ms, "fsm_enable_disturb()");

   }

}


void IdlSystem::setRecMat(int timeout_ms, recMatParams params) {
	_logger->log(Logger::LOG_LEV_INFO, "Executing setRecMat with timeout %d ms...", timeout_ms);
	_logger->log(Logger::LOG_LEV_INFO, "	> m2cFile:  %s", params._m2cFile.c_str());
	_logger->log(Logger::LOG_LEV_INFO, "	> recMatFile:  %s", params._recMatFile.c_str());
	_logger->log(Logger::LOG_LEV_INFO, "	> aDelayFile:  %s", params._aDelayFile.c_str());
	_logger->log(Logger::LOG_LEV_INFO, "	> bDelayFile:  %s", params._bDelayFile.c_str());

   string cmd = "fsm_load_rec( M2C_F='"+ params._m2cFile+"', REC_MAT_F='" + params._recMatFile + "', A_DELAY_F='" + params._aDelayFile + "', B_DELAY_F='" + params._bDelayFile + "', /NOBLOCKCHECK)";

   simpleCommand("setRecMat", timeout_ms, cmd);
}


void IdlSystem::loadShape(int timeout_ms, shapeParams params) {
	_logger->log(Logger::LOG_LEV_INFO, "Executing loadShape with timeout %d ms...", timeout_ms);
	_logger->log(Logger::LOG_LEV_INFO, "	> shapeFile:  %s", params._shapeFile.c_str());

   string cmd = "fsm_load_shape('"+ params._shapeFile+"')";

   simpleCommand("loadShape", timeout_ms, cmd);
}


void IdlSystem::saveShape(int timeout_ms, shapeParams params) {
	_logger->log(Logger::LOG_LEV_INFO, "Executing saveShape with timeout %d ms...", timeout_ms);
	_logger->log(Logger::LOG_LEV_INFO, "	> shapeFile:  %s", params._shapeFile.c_str());

   string cmd = "fsm_save_shape('"+ params._shapeFile+"', /USER)";

   simpleCommand("saveShape", timeout_ms, cmd);

}

void IdlSystem::recoverFailure(int timeout_ms) {
   simpleCommand("recoverFailure", timeout_ms, "fsm_fault_recovery(/FORCE)");
}

bool IdlSystem::isDemoMode() {
    ostringstream cmd;
    bool ret=false;
	
    _logger->log(Logger::LOG_LEV_DEBUG, "Checking IDL demo mode");
    
    cmd << "test_demo_mode()";
    
    AbstractSystemCommandResult res = simpleCommand("checkDemoMode", 5000, cmd.str());
    
    if (res != SysCmdSuccess){ 
       _logger->log(Logger::LOG_LEV_FATAL, "IDL is in demo mode");
      ret=true;
    }
    return ret;
}

void IdlSystem::setPistonCurrent( double elevation, int timeout_ms) {
    ostringstream cmd;
	
    _logger->log(Logger::LOG_LEV_INFO, "Executing setPistonCurrentThreshold with timeout %d ms...", timeout_ms);
    
    cmd << "setPistonCurrent( " << elevation << ")";
    _logger->log(Logger::LOG_LEV_INFO, "	> elevation:  %f", elevation);
    
    AbstractSystemCommandResult res = simpleCommand("setPistonCurrent", timeout_ms, cmd.str());
    
    // want to RIP the mirror in case of error 
    if (res != SysCmdSuccess){ 
		Error error("setPistonCurrent failed", true);
		_arbitrator->notifyClients(&error);
    }
}


void IdlSystem::selectWfsPort( int timeout_ms, int input) {
    ostringstream cmd;
	
    _logger->log(Logger::LOG_LEV_INFO, "Executing selectWfsPort with timeout %d ms...", timeout_ms);
    
    cmd << "select_wfs_port( " << input << ")";
    _logger->log(Logger::LOG_LEV_INFO, "	> input:  %d", input);
    
    AbstractSystemCommandResult res = simpleCommand("selectWfsPort", timeout_ms, cmd.str());
    
    if (res != SysCmdSuccess){ 
		Error error("selectWfsPort failed", false);
		_arbitrator->notifyClients(&error);
    }
}





//void IdlSystem::setTSS( bool ontss, int timeout_ms) {
//
//    string cmd="TSS_DISABLE()";
//    if(ontss) cmd="TSS_ENABLE()";
//    _logger->log(Logger::LOG_LEV_INFO, "Executing setTSS with timeout %d ms...", timeout_ms);
//
//    simpleCommand(cmd, timeout_ms, cmd);
//}


void IdlSystem::processDump(int timeout_ms, string filename) {

   string cmd = "process_dump('" +filename + "')";
   simpleCommand("process_dump", timeout_ms, cmd);
}

