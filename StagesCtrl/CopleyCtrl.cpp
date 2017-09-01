//@File: CopleyCtrl.cpp
//
// CopleyCtrl implementation
//

#include "CopleyCtrl.h"
#include "Logger.h"
#include "AOStates.h"

using namespace Arcetri;

extern "C" {

#include "hwlib/netseriallib.h"
#include "hwlib/copleylib.h"

}


#ifdef _WIN32
#include "winsock2.h"
typedef unsigned int socklen_t;
#undef NO_ERROR
#endif

#include <stdio.h>
#include <string.h>		// strncmp()
#include <stdlib.h>		// atoi()
#include <stdarg.h>
#include <time.h>		// time(), localtime()
//#include <unistd.h>		// sleep()
#include <pthread.h>
#include <errno.h>


// Program identification

int VersMajor = 1;
int VersMinor = 0;
const char *Date = "Dec 2004";

int debug=0;

#define ERRMSG_LEN		256 			// Length of a (char) error message


//+Entry  help
//
// help prints an usage message
//

void help()
{
	printf("\nStagesCtrl - Vers. %d.%d     A. Puglisi, %s\n\n", VersMajor, VersMinor, Date);
	printf("Usage: StagesCtrl [-v] [<server>] [-f configfile] \n\n");
	printf("       StagesCtrl -h  print an help mesage\n\n");
	printf("       -i	interactive mode (no server connection)\n");
	printf("       -v verbose mode\n");
   printf("       -f configuration file (defaults to \"config\")\n");
	printf("       <server> Server numeric address (default: \"127.0.0.1\")\n");
	printf("       -h prints this message\n\n");
}

  

CopleyCtrl::CopleyCtrl( int argc, char **argv) throw (AOException): AOApp( argc, argv)
{
  Create();
} 

void CopleyCtrl::Create() throw (AOException)
{

   try {
      _stepsRatio   = ConfigDictionary()["stepsRatio"];
      _driveName    = (std::string) ConfigDictionary()["Name"];
      _mvLowEnd     = ConfigDictionary()["mvLowEnd"];
      _mvHighEnd    = ConfigDictionary()["mvHighEnd"];
      _mvSpeed      = ConfigDictionary()["mvSpeed"];
      _driveNetAddr = (std::string) ConfigDictionary()["iDriveNetAddr"];
      _driveNetPort = ConfigDictionary()["iDriveNetPort"];

      _poslimit_pin = ConfigDictionary()["positiveLimitSwitch"];
      _neglimit_pin = ConfigDictionary()["negativeLimitSwitch"];
      _home_pin = ConfigDictionary()["homeSwitch"];
      _homingMethod = (std::string) ConfigDictionary()["homingMethod"];

      _homingPos    = ConfigDictionary()["HomingPosition"];
      _abortPos     = ConfigDictionary()["AbortPosition"];

   } catch (Config_File_Exception &e) {
      _logger->log( Logger::LOG_LEV_FATAL, "Missing configuration data: %s", e.what().c_str());
      throw AOException("Fatal: Missing configuration data");
   }

   try {
      _pGain = ConfigDictionary()["proportionalGain"];
   } catch (Config_File_Exception &e) {
      _pGain = 4500;
   }

   _logger->log( Logger::LOG_LEV_INFO, "Proportional gain set to %d", _pGain);



   _logger->log(Logger::LOG_LEV_DEBUG, "CopleyCtrl for stage %s created", _driveName.c_str());
   _logger->log(Logger::LOG_LEV_DEBUG, "MV limits: %f %f", _mvLowEnd, _mvHighEnd);
}

void CopleyCtrl::PostInit()
{

	// ----------- Initial status
   setCurState(STATE_NOCONNECTION);
   var_errmsg.Set("Starting up");
}

void CopleyCtrl::Run()
{

	while(TimeToDie() == false)
		{
      try {
		   // Do iterative step of the controller
		   DoFSM();	

		   // Sends the new positions to the MsgD-RTDB
		   SetPositionVars(); 
         } catch (AOException &e) {
          _logger->log( Logger::LOG_LEV_ERROR, "Caugth exception (at %s:%d): %s", __FILE__, __LINE__, e.what().c_str());
	   	}

      }

}

//@Function
//
// DoFSM          Manages controller states
//
// Switches the controller from one state to another. States can be changed asynchronously
// from this thread or the listening thread, and this function will properly react to a new state.
//
// Many states include a msleep() function to slow down the thread when immediate action is not necessary
//@

int CopleyCtrl::DoFSM(void)
{
	int status,enabled;
	int stat = NO_ERROR;
   static int stop_counter=0;

   bool stopMoving = false;
   double curPos;
   static double oldPos = 1e-9;

   status = getCurState();

	if ((status != STATE_OPERATING) && (status != STATE_HOMING))
		msleep(1000);
//	else
//		usleep(100*1000);

	// Always check if we can reach the stage
	if (status != STATE_NOCONNECTION)
		if (copleyCommTest() != NO_ERROR)
         {
         copleyShutdown();
			setCurState(STATE_NOCONNECTION);
         }

   status = getCurState();
	var_enable_cur.Get(&enabled);

   // Always check for errors
   if (status != STATE_NOCONNECTION)
      {
      char desc[256];
      desc[0]=0;
      int code = copleyCurStatus( desc, 256 );
	   //_logger->log( Logger::LOG_LEV_TRACE, "Current error code: %d", code);
      if (strlen(desc)==0)
         strcpy(desc,"No error");
      var_errmsg.Set(desc);
      
      // In case of error, switch off immediately
      if ((code != 0) && (status != STATE_READY))
         {
	      _logger->log( Logger::LOG_LEV_DEBUG, "Error code detected: %d - Immediate switch off!", code);
	      _logger->log( Logger::LOG_LEV_DEBUG, "Error description: %s", desc);
         SwitchOff();
         }
      }

      var_poslimit_cur.Set( copleyPositiveLimit());
      var_neglimit_cur.Set( copleyNegativeLimit());

	switch(status)
		{
		// At first, try to start network
		case STATE_NOCONNECTION:
		stat = SetupNetwork();
		if (stat == NO_ERROR)
         stat = copleyCommTest();
		_logger->log(  Logger::LOG_LEV_DEBUG, "Result: %d (%s)", stat, lao_strerror(stat));
		if (stat == NO_ERROR)
			setCurState(STATE_CONNECTED);
		else
			msleep(1000);
		break;

		// After network start, configure stage
		case STATE_CONNECTED:
		_logger->log( Logger::LOG_LEV_DEBUG, "Starting configuration");
      Startup();
      setCurState(STATE_OFF);
		break;

		// When stage is ready, do nothing
      case STATE_OFF:
		case STATE_READY:
		msleep(1000);
		break;

		// If moving for whatever reason, check for moving acknowledgement req.
		case STATE_OPERATING:

      oldPos = curPos;

      _logger->log( Logger::LOG_LEV_DEBUG, "Check for end of movement");
		if (copleyIsMoving() == 0)
            {
            if (stop_counter++ >2)
               stopMoving = true;
            }
      else
         stop_counter=0;

      if (stopMoving)
				{
				if (enabled != ENABLE_ALWAYS)
					SwitchOff();
				else
					{
					_logger->log ( Logger::LOG_LEV_DEBUG, "Switching to READY state");
					setCurState(STATE_READY);
					}
					
				}
		break;

		case STATE_HOMING:
		//msleep(1000);
		_logger->log ( Logger::LOG_LEV_DEBUG, "Check for end of homing");
      if (copleyIsHomed())
         SwitchOff(); // Magellan: never turn off the stages
		break;

		// Unknown states should not exist
		default:
		msleep(1000);
		break;
		}

   // Always set the state to reset external watchdogs
   setCurState( getCurState());

	// Return the generated error code, if any
	return stat;
}

//@Function
//
// SwitchOff                        change to "switched off" state
//@

int CopleyCtrl::SwitchOff(void)
{
	int stat;

	_logger->log ( Logger::LOG_LEV_DEBUG, "Switching to OFF state");

   stat = copleyDisable();
	if (stat == NO_ERROR)
		setCurState(STATE_OFF);

	return stat;
}

//@Function
//
// SwitchOn				change from "switched off" to "ready"
//@

int CopleyCtrl::SwitchOn()
{
	int stat=NO_ERROR;
   printf("SwitchOn() called ***************\n");
	_logger->log ( Logger::LOG_LEV_DEBUG, "Switching to READY state");


   copleySetMoveSpeed( (int)(_mvSpeed * _stepsRatio));		// Unit: count/sec = 0.1 micron/sec
   copleySetAcceleration( (int)(_mvSpeed * _stepsRatio/3));	// Unit: count/sec^2
   copleySetDeceleration( (int)(_mvSpeed * _stepsRatio/3));	// Unit: count/sec^2
   copleySetAbortDeceleration( (int)(_mvSpeed * _stepsRatio *10));	// Unit: count/sec^2
   copleyEnableInPositioningMode();

   
	setCurState(STATE_READY);

	return stat;
}

//@Function
//
// Startup       Puts the Copley controller in a default startup configuration
//
//@

// Do nothing extra - will be expanded for error checking
#define CHECK(a)  (a)

int CopleyCtrl::Startup()
{
   CHECK(copleyDisable());

   if (_home_pin >0)
      {
      CHECK(copleySetHomeSwitchInput( _home_pin -1));
      CHECK(copleyEnableHomeSwitch());
      }
   else
      CHECK(copleyDisableHomeSwitch());

   if (_poslimit_pin >0)
      {
      CHECK(copleySetPosLimitSwitchInput( _poslimit_pin -1));
      CHECK(copleyEnablePosLimitSwitch());
      }
   else
      CHECK(copleyDisablePosLimitSwitch());

   if (_neglimit_pin >0)
      {
      CHECK(copleySetNegLimitSwitchInput( _neglimit_pin -1));
      CHECK(copleyEnableNegLimitSwitch());
      }
   else
      CHECK(copleyDisableNegLimitSwitch());

   CHECK(copleySetBrakeDelay(0));
   CHECK(copleySetHandBrake(2));

   CHECK(copleySetPositionGains(_pGain,20));

   var_switches_cur.Set(0x03);

   return NO_ERROR;
}

//@Function
//
// SetPositionVars()			sets the position variables on the MsgD-RTDB
//
// Sets the position variables on the MsgD-RTDB. Since this function is intended to be called very often
// (to always have an up-to-date position display), a message will be sent to the MsgD-RTDB only if the position
// is changed from the last update.
// 
// Also sends to the MsgD-RTDB the output current value.
//@


int CopleyCtrl::SetPositionVars()
{
	int stat, status;
	double current;
	double pos, loadpos, oldpos;

	status = getCurState();

	// States earlier than HOMING do not have valid positions
	//
	// Homing has positions that will be invalid after the homing procedure
	// will be complete, but they are useful anyway to check the progress.
	// User interfaces should clearly mark the difference in this case.
	//if (status < STATE_HOMING)
//		return NO_ERROR;

	if ((stat = copleyLoadPosition( &loadpos)) != NO_ERROR)
		return stat;

	pos = StepsToUnits( loadpos );

	// Update position only if changed from before
	var_pos_cur.Get(&oldpos);
   if (debug) printf("Old pos: %f new pos: %f\n", oldpos, pos); 
	if (pos != oldpos)
		var_pos_cur.Set(pos, 0, FORCE_SEND);


	double motorpos;
	// Read motor position
	if ((stat = copleyMotorPosition(&motorpos)) != NO_ERROR)
		return stat;
	motorpos = StepsToUnits( motorpos);
	var_motorpos_cur.Set( motorpos);

	// Read output current
	if ((stat = copleyActualCurrent( &current)) != NO_ERROR)
		return stat;

	// Set the output current value, in Ampere
	var_current.Set(current);

        _logger->log( Logger::LOG_LEV_INFO, "Stage pos (mm): %5.3f, current (A): %5.3f", pos, current);

	return NO_ERROR;
}


//+Entry
//
// SetupVars        creates the MsgD-RTDB variables to control one stage
//
// Return value: zero or a negative error code

void CopleyCtrl::SetupVars()
{
   try {
      var_name = RTDBvar( MyFullName(), "NAME", NO_DIR, CHAR_VARIABLE, strlen( _driveName.c_str())+1);
      var_errmsg = RTDBvar( MyFullName(), "ERRMSG", NO_DIR, CHAR_VARIABLE, ERRMSG_LEN);
      var_enable_cur = RTDBvar( MyFullName() , "ENABLE", CUR_VAR);
      var_enable_req = RTDBvar( MyFullName() , "ENABLE", REQ_VAR);
      var_loend = RTDBvar( MyFullName() , "LOEND", NO_DIR, REAL_VARIABLE);
      var_hiend = RTDBvar( MyFullName() , "HIEND", NO_DIR, REAL_VARIABLE);
      var_errcode = RTDBvar( MyFullName() , "ERRCODE");
      var_current = RTDBvar( MyFullName() , "CURRENT", NO_DIR, REAL_VARIABLE);

      var_pos_cur = RTDBvar( MyFullName() , "POS", CUR_VAR, REAL_VARIABLE);
      var_pos_req = RTDBvar( MyFullName() , "POS", REQ_VAR, REAL_VARIABLE);

      var_motorpos_cur = RTDBvar( MyFullName() , "MOTOR.POS", CUR_VAR, REAL_VARIABLE);

      var_switches_cur = RTDBvar( MyFullName() , "LIMIT", CUR_VAR);
      var_switches_req = RTDBvar( MyFullName() , "LIMIT", REQ_VAR);

      var_poslimit_cur = RTDBvar( MyFullName() , "POSLIMIT", CUR_VAR);
      var_neglimit_cur = RTDBvar( MyFullName() , "NEGLIMIT", CUR_VAR);

      var_posname_cur = RTDBvar( MyFullName(), "POSNAME", CUR_VAR, CHAR_VARIABLE, 50);
      var_posname_cur.Set(" ");

      var_brake_req = RTDBvar( MyFullName() , "BRAKE", REQ_VAR);

      // Status variables
      var_brake_cur = RTDBvar( MyFullName() , "BRAKE", CUR_VAR);
      
      var_special = RTDBvar( MyFullName() , "SPECIAL", NO_DIR);

	   var_name.Set( _driveName);
	   var_loend.Set(_mvLowEnd);
	   var_hiend.Set(_mvHighEnd);

	   Notify( var_enable_req, EnableReqChanged);
	   Notify( var_pos_req, PosReqChanged);
	   Notify( var_switches_req, LimitReqChanged);
	   Notify( var_brake_req, BrakeReqChanged);
      Notify( var_special, SpecialReqChanged);

   } catch (AOVarException &e) {
      _logger->log(Logger::LOG_LEV_FATAL, "%s:%s: %s", __FILE__, __LINE__, e.what().c_str());
      throw AOException("Error creating RTDB variables");
   }
}


int CopleyCtrl::EnableReqChanged( void *pt, Variable *var)
{
   int enable_req, enable_cur, status, stat;
   CopleyCtrl *ctrl = (CopleyCtrl *)pt;

   ctrl->var_enable_req.Set(var);

   // Enable can have three values:
	//
	// ENABLE_OFF: disable (refuse any movement requests)
	// ENABLE_OFFAFTERMOVE: move and disable motor (accept movements and disable after each movement)
	// ENABLE_ALWAYS: move and keep motor enabled (accept movements and don't disable afterwards)

	ctrl->var_enable_cur.Get(&enable_cur);	// Current value
	ctrl->var_enable_req.Get(&enable_req);	// New value
	status = ctrl->getCurState();	      // Current status

   if (debug) printf("Enable cur: %d  - Enable req: %d - Status: %d\n", enable_cur, enable_req, status);

	// Ignore invalid values
	if ((enable_req != ENABLE_OFF) && (enable_req != ENABLE_OFFAFTERMOVE) && (enable_req != ENABLE_ALWAYS))
		return NO_ERROR;

	// Switch off if required
	if ((enable_req == ENABLE_OFF) && (enable_cur != ENABLE_OFF))
		if ((stat = ctrl->SwitchOff()) != NO_ERROR)
			return stat;

	if ((enable_req == ENABLE_OFFAFTERMOVE) && (status == STATE_READY))
		if ((stat = ctrl->SwitchOff()) != NO_ERROR)
			return stat;

	// Save new enable state
	ctrl->var_enable_cur.Set(enable_req);

	return NO_ERROR;
}


int CopleyCtrl::PosReqChanged( void *pt, Variable *var)
{
      int cur_state;
      CopleyCtrl *ctrl = (CopleyCtrl *)pt;
	   cur_state = ctrl->getCurState();	      // Current status

      // Ignore requests when we aren't in one of the good states
      if ((cur_state != STATE_OFF) && (cur_state != STATE_READY) && (cur_state = STATE_OPERATING))
      {
         Logger::get()->log( Logger::LOG_LEV_INFO, "Movement request ignored (stage not ready)");
         return NO_ERROR;
      }

		return ctrl->MoveTo( var->Value.Dv[0]);
}

int CopleyCtrl::LimitReqChanged( void *pt, Variable *var)
{
      int action, stat, cur_state;
      CopleyCtrl *ctrl = (CopleyCtrl *)pt;

	   cur_state = ctrl->getCurState();	      // Current status

      // Ignore requests when we aren't in one of the good states
      if ((cur_state != STATE_OFF) && (cur_state != STATE_READY) &&
          (cur_state = STATE_OPERATING) && (cur_state != STATE_BUSY))
      {
         Logger::get()->log( Logger::LOG_LEV_INFO, "Limit request ignored (stage not ready)");
         return NO_ERROR;
      }

   // ---------------------
   // Limit switch enable/disable request (LIMIT.REQ variable)
   //
   // Bit 0: negative limit switch
   // Bit 1: positive limit switch
   // Bit 2: software negative limit switch
   // Bit 3: software positive limit switch

      action = var->Value.Lv[0];

      // negative limit switch
      if (action & 0x01)
         stat = copleyEnableNegLimitSwitch();
      else
         stat = copleyDisableNegLimitSwitch();

      if (stat != NO_ERROR)
         {
         Logger::get()->log( Logger::LOG_LEV_ERROR, "Error in setting limit switch: (%d) %s", stat, lao_strerror(PLAIN_ERROR(stat)));
         return stat;
         }

      // positive limit switch
      if (action & 0x02)
         stat = copleyEnablePosLimitSwitch();
      else
         stat = copleyDisablePosLimitSwitch();

      if (stat != NO_ERROR)
         {
         Logger::get()->log( Logger::LOG_LEV_ERROR, "Error in setting limit switch: (%d) %s", stat, lao_strerror(PLAIN_ERROR(stat)));
         return stat;
         }

      // Software limits are automatically handled in the MoveTo() routine

      ctrl->var_switches_cur.Set(action);
      return NO_ERROR;
      }

int CopleyCtrl::SpecialReqChanged( void *pt, Variable *var)
{
      int value, cur_state;
      CopleyCtrl *ctrl = (CopleyCtrl *)pt;

	   cur_state = ctrl->getCurState();	      // Current status

      // Ignore requests when we aren't in one of the good states
      if ((cur_state != STATE_OFF) && (cur_state != STATE_READY))
      {
         Logger::get()->log( Logger::LOG_LEV_INFO, "Special request ignored (stage not ready)");
         return NO_ERROR;
      }

      value = var->Value.Lv[0];

      Logger::get()->log( Logger::LOG_LEV_INFO, "Special request, value = %d", value);

      if (value == 1)
         return ctrl->StartHoming();

      return NO_ERROR;
}

int CopleyCtrl::BrakeReqChanged( void *pt, Variable *var)
{
   int value, cur_state;
   CopleyCtrl *ctrl = (CopleyCtrl *)pt;

   cur_state = ctrl->getCurState();	      // Current status

   // Ignore requests when we aren't in one of the good states
   if ((cur_state != STATE_OFF) && (cur_state != STATE_READY))
   {
      Logger::get()->log( Logger::LOG_LEV_INFO, "Special request ignored (stage not ready)");
      return NO_ERROR;
   }

   value = var->Value.Lv[0];

   // To avoid warnings
   ctrl = NULL;
   return NO_ERROR;
}

//@Function
//
// MoveTo                       Starts a movement towards the specified position
//
// Starts a stage movements. Checks if the stage is ready, changes the stage state
// if necessary, checks for out_of_range movements and finally moves the stage.
//
//@

int CopleyCtrl::MoveTo( double tgtpos)
{
	int result, status, enabled;

   status = getCurState();	      // Current status
	var_enable_cur.Get(&enabled);

	if (enabled == ENABLE_OFF)
		{
		_logger->log( Logger::LOG_LEV_DEBUG, "Movement refused (drive disabled)");
		return STAGE_NOT_ENABLED_ERROR;
		}

   // Special triggers
   if (tgtpos == _homingPos)
      return StartHoming();

   if (tgtpos == _abortPos)
      return SwitchOff();

	// Check movement range, if software_limits are enabled
   int software_limits;
   var_switches_cur.Get(&software_limits);

	if ( ((software_limits & 0x08) && (tgtpos > _mvHighEnd)) ||
        ((software_limits & 0x04) && (tgtpos < _mvLowEnd)))
		{
		_logger->log( Logger::LOG_LEV_ERROR, "Error: movement out of range");
      _logger->log( Logger::LOG_LEV_ERROR, "Request: %f   Limits: %f %f", tgtpos, _mvHighEnd, _mvLowEnd);
      throw AOException("Error in MoveTo(): movement out of range", VALUE_OUT_OF_RANGE_ERROR);
		}

	// Convert everything in stage steps
	double curpos_steps;
	result = copleyLoadPosition( &curpos_steps);
	if (result)
      throw AOException( "Cannot read current position", result);

	int tgtpos_steps = (int) UnitsToSteps(tgtpos);

   // Switch on if needed
	if (status == STATE_OFF)
      SwitchOn();

	// Re-read status, because it may have changed
   status = getCurState();	

	if (status != STATE_READY)
		{
		_logger->log( Logger::LOG_LEV_ERROR, "Error: stage not ready for movement");
      throw AOException( "Cannot start movement", STAGE_BUSY_ERROR);
		}

	_logger->log( Logger::LOG_LEV_INFO, "Moving to %5.2f", tgtpos);

	result = copleySetAbsolutePositioning();
	if (result != NO_ERROR)
		{
		_logger->log( Logger::LOG_LEV_ERROR, "copleySetAbsolutePositioning(): (%d) %s", result, lao_strerror(result));
     		 throw AOException( "Error in MoveTo()", result);
		}


	result = copleyMoveTo( tgtpos_steps);

	if (result == NO_ERROR)
		setCurState(STATE_OPERATING);
   else
		{
		_logger->log( Logger::LOG_LEV_ERROR, "Error in MoveTo(): (%d) %s", result, lao_strerror(result));
      throw AOException( "Error in MoveTo()", result);
		}

	return NO_ERROR;
}

//@Function
//
// SetupNetwork()		Sets up network link with stage
//@

int CopleyCtrl::SetupNetwork()
{
	int stat;

	// Close any previous connection
	var_errmsg.Set("Not connected");

   _logger->log( Logger::LOG_LEV_INFO, "Connecting to %s, %d", (char *) _driveNetAddr.c_str(), _driveNetPort);
   
	// Setup serial/network interface
   if (( stat = copleyInit((char *)_driveNetAddr.c_str(), _driveNetPort)) != NO_ERROR)
		{
      _logger->log( Logger::LOG_LEV_ERROR, "Error configuring network: (%d) %s - Errno: %s", stat, lao_strerror(stat), strerror(errno));
		return stat;
		}

	// Set OK flag
	var_errmsg.Set("Connected");

	return NO_ERROR;
}


//@Function
//
// StartHoming                start the homing procedure
//@

int CopleyCtrl::StartHoming(void)
{
	int stat=NO_ERROR;

	_logger->log( Logger::LOG_LEV_DEBUG, "Starting homing sequence");

   if (_homingMethod == "POS") 
      copleySetHomingMethod( COPLEY_HOMING_LIMIT_SWITCH_POS);
   else
      copleySetHomingMethod( COPLEY_HOMING_LIMIT_SWITCH_NEG);
   copleySetFastHomingSpeed( (int)(_mvSpeed * _stepsRatio));		// Unit: count/sec = 0.1 micron/sec
   copleySetSlowHomingSpeed( (int)(_mvSpeed * _stepsRatio));		// Unit: count/sec
   copleySetHomingAcceleration( (int)(_mvSpeed * _stepsRatio) /3);		// Unit: count/sec^2
   copleySetHomingOffset(0);
   copleyEnableInPositioningMode();

   copleyHome();

   setCurState(STATE_HOMING);

	return stat;
}

//@Function
//
// UnitsToSteps              units -> steps conversion
//
// Convert from userland coordinates (units = millimeters) to stage steps
//@

double CopleyCtrl::UnitsToSteps( double units)
{
	return units * _stepsRatio;
}

//@Function
//
// StepsToUnits              units -> steps conversion
//
// Convert from stage steps to userland coordinates (units = millimeters)
//@

double CopleyCtrl::StepsToUnits( double steps)
{
	if (_stepsRatio == 0)
		return 0;
	else
		return steps / _stepsRatio;
}

// Main

int main( int argc, char **argv) {

   SetVersion(VersMajor,VersMinor);

   try {
      CopleyCtrl *c;
      c = new CopleyCtrl( argc, argv);

      c->Exec();

      delete c;
   } catch (AOException &e) {
      Logger::get()->log( Logger::LOG_LEV_FATAL, "%s:%d: %s", __FILE__, __LINE__, e.what().c_str());
   }
}
