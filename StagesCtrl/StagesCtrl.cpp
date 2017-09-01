//@File: StageCtrl.cpp
//
// StageCtrl implementation
//

#include "StagesCtrl.h"
#include "Logger.h"
#include "AOStates.h"

using namespace Arcetri;

extern "C" {

// i-Drive related libs

#include "hwlib/netseriallib.h"
#include "hwlib/idrivelib.h"

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
#include <math.h>       // fabs()


// Program identification

int VersMajor = 1;
int VersMinor = 0;
const char *Date = "Dec 2004";

int debug=0;

#define ERRMSG_LEN		32 			// Length of a (char) error message

extern int idrive_homing;

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

  

StageCtrl::StageCtrl( int argc, char **argv) throw (AOException): AOApp( argc, argv)
{
  Create();
} 

void StageCtrl::Create() throw (AOException)
{

   try {
      _stepsRatio   = ConfigDictionary()["stepsRatio"];
      _driveName    = (std::string) ConfigDictionary()["Name"];
      _driveHwAddr  = ConfigDictionary()["iDriveHwAddr"];
      _mvLowEnd     = ConfigDictionary()["mvLowEnd"];
      _mvHighEnd    = ConfigDictionary()["mvHighEnd"];
      _driveNetAddr = (std::string) ConfigDictionary()["iDriveNetAddr"];
      _driveNetPort = ConfigDictionary()["iDriveNetPort"];
      _goodWindow   = ConfigDictionary()["GoodWindow"];
   } catch (Config_File_Exception &e) {
      _logger->log( Logger::LOG_LEV_FATAL, "Missing configuration data: %s", e.what().c_str());
      throw AOException("Fatal: Missing configuration data");
   }

   _logger->log(Logger::LOG_LEV_DEBUG, "StageCtrl for stage %s created", _driveName.c_str());
   _logger->log(Logger::LOG_LEV_DEBUG, "MV limits: %f %f", _mvLowEnd, _mvHighEnd);
}

void StageCtrl::PostInit()
{

	// ----------- Initial status
   setCurState(STATE_NOCONNECTION);
   var_errmsg.Set("Starting up");
}

void StageCtrl::Run()
{

	while(TimeToDie() == false)
		{
      try {
		   // Do iterative step of the controller
		   DoFSM();	

         int polef;
         GetValue(  _driveHwAddr, 0x2000+106, 0x00, &polef);

         if (polef > 32000)
            polef -= 65536;

         printf("Pole findind angle: %d\n", polef);


		   // Sends the new positions to the MsgD-RTDB
		   SetPositionVars(); 
         } catch (AOException &e) {
          _logger->log( Logger::LOG_LEV_ERROR, "Run(): caught exception %s. Carrying on", e.what().c_str());
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

int StageCtrl::DoFSM(void)
{
	int status,enabled;
	int stat = NO_ERROR;
	static int check_counter=0;
   static int stuck_counter=0;

   bool stopMoving = false;
   double curPos, oldPos;

   status = getCurState();

	if ((status != STATE_OPERATING) && (status != STATE_HOMING))
		msleep(1000);
//	else
//		usleep(100*1000);

	// Always check if we can reach the iDrive
	if (status != STATE_NOCONNECTION)
		if (TestiDriveComm( _driveHwAddr) != NO_ERROR)
			setCurState(STATE_NOCONNECTION);

   status = getCurState();
	var_enable_cur.Get(&enabled);

   // Always check for errors
   if (status != STATE_NOCONNECTION)
      {
      int code = GetCurrentError( _driveHwAddr);
	   _logger->log( Logger::LOG_LEV_TRACE, "Current error code: %d", code);
      var_errcode.Set(code);
      
      // In case of error, switch off immediately
      if ((code != 0) && (status != STATE_READY))
         {
	      _logger->log( Logger::LOG_LEV_DEBUG, "Error code detected: %d - Immediate switch off!", code);
         SwitchOff();
         }
      }


	switch(status)
		{
		// At first, try to start network
		case STATE_NOCONNECTION:
		stat = SetupNetwork();
		if (stat == NO_ERROR)
         stat = TestiDriveComm( _driveHwAddr);
		_logger->log(  Logger::LOG_LEV_DEBUG, "Result: %d (%s)", stat, lao_strerror(stat));
		if (stat == NO_ERROR)
			setCurState(STATE_CONNECTED);
		else
			msleep(1000);
		break;

		// After network start, configure iDrive
		case STATE_CONNECTED:
		_logger->log( Logger::LOG_LEV_DEBUG, "Starting configuration");
		stat = LoadParameterFile( (std::string) ConfigDictionary()["setupfile"]);
		if (stat == NO_ERROR)
         {
			setCurState(STATE_CONFIGURED);
         var_switches_cur.Set(0x0F);
         }
		break;

		// When iDrive is successfully configured, start polefinding and homing
		case STATE_CONFIGURED:
      //stat = StartPoleFinding();
      //msleep(10000);
      //if (stat == NO_ERROR)
      //   stat = StartHoming();
      setCurState(STATE_OFF);
      break;


      // This happens only in case of some error
      case STATE_POLEFINDING:
      msleep(10000);
      setCurState(STATE_OFF);
      break;


      case -1:
		// For now we skip directly to the SWITCHEDOFF state
		SwitchOff();
		break;	

		// When stage is ready, do nothing
      case STATE_OFF:
		case STATE_READY:
      stuck_counter=0;
		msleep(1000);
		break;

		// If moving for whatever reason, check for moving acknowledgement req.
		//
		// Calling the EndMoving() function is NECESSARY before another movement can be attempted,
		// (this is dictated by the i-Drive controller)
		// so this code MUST be executed from time to time, either with a heartbeat message or
		// putting it in some kind of loop

		case STATE_OPERATING:

      // iDrive can remain stuck in OPERATING mode after the position has been reached. If we stay
      // here for more than three seconds without moving, stop the drive.


      oldPos = curPos;
      var_pos_cur.Get(&curPos);
      if (fabs(curPos-oldPos)< _goodWindow)
         {
       	_logger->log( Logger::LOG_LEV_DEBUG, "Stuck counter at %d", stuck_counter);
         if (stuck_counter++ >3)
            {
       	   _logger->log( Logger::LOG_LEV_DEBUG, "Detected stuck stage, terminating movement");
            stopMoving = true;
            }
         }
      else
         stuck_counter=0;

      // ... and we must check the target reached flag from iDrive every now and then..
		if (check_counter++ >3)
			{
       	_logger->log( Logger::LOG_LEV_DEBUG, "Check for end of movement");
			check_counter = 0;
			if (EndMoving(  _driveHwAddr, 1) >0)
            stopMoving = true;
         }

      if (stopMoving)
				{
				if (enabled == ENABLE_OFFAFTERMOVE)
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
		if (EndMoving(  _driveHwAddr, 1) >0)
			SwitchOff();
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

int StageCtrl::SwitchOff(void)
{
	int stat;

	_logger->log ( Logger::LOG_LEV_DEBUG, "Switching to OFF state");

	stat = StopiDrive( _driveHwAddr);
	if (stat == NO_ERROR)
		setCurState(STATE_OFF);

	return stat;
}

//@Function
//
// SwitchOn				change from "switched off" to "ready"
//@

int StageCtrl::SwitchOn()
{
	int stat;
	_logger->log ( Logger::LOG_LEV_DEBUG, "Switching to READY state");

	stat = LoadParameterFile( (std::string) ConfigDictionary()["startfile"]);
	if (stat == NO_ERROR)
		setCurState(STATE_READY);

	return stat;
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


int StageCtrl::SetPositionVars()
{
	int stat, pos_int, status;
	int current;
	double pos,oldpos;

	status = getCurState();

   // We need a connection up
   if (status < STATE_CONNECTED)
      return NO_ERROR;

	// States earlier than HOMING do not have valid positions
	//
	// Homing has positions that will be invalid after the homing procedure
	// will be complete, but they are useful anyway to check the progress.
	// User interfaces should clearly mark the difference in this case.
	//if (status < STATE_HOMING)
//		return NO_ERROR;


	if ((stat = GetCurrentPosition( _driveHwAddr, &pos_int)) != NO_ERROR)
		return stat;

	pos = StepsToUnits( (double)pos_int );

	// Update position only if changed from before
	var_pos_cur.Get(&oldpos);
   if (debug) printf("Old pos: %f new pos: %f\n", oldpos, pos); 
	if (pos != oldpos)
		var_pos_cur.Set(pos);

	// Read output current
	if ((stat = GetValue(  _driveHwAddr, 0x2407, 0x0C, &current)) != NO_ERROR)
		return stat;

	// Set the output current value, in Ampere
	var_current.Set((float)current/100.0);

	return NO_ERROR;
}


//+Entry
//
// SetupVars        creates the MsgD-RTDB variables to control one stage
//
// Return value: zero or a negative error code

void StageCtrl::SetupVars()
{
   try {
      var_name = RTDBvar( MyFullName(), string("NAME"), NO_DIR, CHAR_VARIABLE, strlen( _driveName.c_str())+1);
      var_errmsg = RTDBvar( MyFullName(), string("ERRMSG"), NO_DIR, CHAR_VARIABLE, ERRMSG_LEN);
      var_enable_cur = RTDBvar( MyFullName() , string("ENABLE"), CUR_VAR);
      var_enable_req = RTDBvar( MyFullName() , string("ENABLE"), REQ_VAR);
      var_loend = RTDBvar( MyFullName() , string("LOEND"), NO_DIR, REAL_VARIABLE);
      var_hiend = RTDBvar( MyFullName() , string("HIEND"), NO_DIR, REAL_VARIABLE);
      var_errcode = RTDBvar( MyFullName() , string("ERRCODE"));
      var_current = RTDBvar( MyFullName() , string("CURRENT"), NO_DIR, REAL_VARIABLE);

      var_pos_cur = RTDBvar( MyFullName() , string("POS"), CUR_VAR, REAL_VARIABLE);
      var_pos_req = RTDBvar( MyFullName() , string("POS"), REQ_VAR, REAL_VARIABLE);

      var_switches_cur = RTDBvar( MyFullName() , string("LIMIT"), CUR_VAR);

      var_switches_req = RTDBvar( MyFullName() , string("LIMIT"), REQ_VAR);

      var_brake_cur = RTDBvar( MyFullName() , string("BRAKE"), CUR_VAR);
      var_brake_req = RTDBvar( MyFullName() , string("BRAKE"), REQ_VAR);
      
      var_special = RTDBvar( MyFullName() , string("SPECIAL"), NO_DIR);

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


int StageCtrl::EnableReqChanged( void *pt, Variable *var)
{
   int enable_req, enable_cur, status, stat;
   StageCtrl *ctrl = (StageCtrl *)pt;

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


int StageCtrl::PosReqChanged( void *pt, Variable *var)
{
      int cur_state;
      StageCtrl *ctrl = (StageCtrl *)pt;
	   cur_state = ctrl->getCurState();	      // Current status

      // Ignore requests when we aren't in one of the good states
      if ((cur_state != STATE_OFF) && (cur_state != STATE_READY) && (cur_state = STATE_OPERATING))
      {
         Logger::get()->log( Logger::LOG_LEV_INFO, "Movement request ignored (stage not ready)");
         return NO_ERROR;
      }

		return ctrl->MoveTo( var->Value.Dv[0]);
}

int StageCtrl::LimitReqChanged( void *pt, Variable *var)
{
      int action, stat, cur_state;
      StageCtrl *ctrl = (StageCtrl *)pt;

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
         stat = SetValue( ctrl->_driveHwAddr, 0x202A, 0, 19);
      else
         stat = SetValue( ctrl->_driveHwAddr, 0x202A, 0, 0);

      if (stat != NO_ERROR)
         {
         Logger::get()->log( Logger::LOG_LEV_ERROR, "Error in setting limit switch: (%d) %s", stat, lao_strerror(PLAIN_ERROR(stat)));
         return stat;
         }

      // positive limit switch
      if (action & 0x02)
         stat = SetValue( ctrl->_driveHwAddr, 0x2029, 0, 20);
      else
         stat = SetValue( ctrl->_driveHwAddr, 0x2029, 0, 0);

      if (stat != NO_ERROR)
         {
         Logger::get()->log( Logger::LOG_LEV_ERROR, "Error in setting limit switch: (%d) %s", stat, lao_strerror(PLAIN_ERROR(stat)));
         return stat;
         }

      // Software limits are automatically handled in the MoveTo() routine

      ctrl->var_switches_cur.Set(action);
      return NO_ERROR;
      }

int StageCtrl::SpecialReqChanged( void *pt, Variable *var)
{
      int value, cur_state;
      StageCtrl *ctrl = (StageCtrl *)pt;

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
      if (value == 2)
         return ctrl->StartPoleFinding();

      return NO_ERROR;
}

int StageCtrl::BrakeReqChanged( void *pt, Variable *var)
{
   int value, cur_state;
   StageCtrl *ctrl = (StageCtrl *)pt;

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

int StageCtrl::MoveTo( double tgtpos)
{
	int result, status, enabled;

   status = getCurState();	      // Current status
	var_enable_cur.Get(&enabled);

	if (enabled == ENABLE_OFF)
		{
		_logger->log( Logger::LOG_LEV_DEBUG, "Movement refused (drive disabled)");
		return STAGE_NOT_ENABLED_ERROR;
		}

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

	// Convert everything in iDrive steps
	int curpos_steps;
	result = GetCurrentPosition(  _driveHwAddr, &curpos_steps);
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

	_logger->log( Logger::LOG_LEV_DEBUG, "Moving to %f", tgtpos);
	result = MoveiDrive(  _driveHwAddr, tgtpos_steps, 1, 1);

	if (result == NO_ERROR)
		setCurState(STATE_OPERATING);
   else
		{
		_logger->log( Logger::LOG_LEV_ERROR, "Error in MoveiDrive(): (%d) %s", result, lao_strerror(result));
      throw AOException( "Error in MoveiDrive()", result);
		}

	return NO_ERROR;
}

//@Function
//
// SetupNetwork()		Sets up network link with iDrive
//@

int StageCtrl::SetupNetwork()
{
	int result;

	// Close any previous connection
	SerialClose();
	var_errmsg.Set("Not connected");

   _logger->log( Logger::LOG_LEV_INFO, "Connecting to %s, %d", (char *) _driveNetAddr.c_str(), _driveNetPort);

	// Setup serial/network interface
	if (( result = SerialInit( (char *)_driveNetAddr.c_str(), _driveNetPort)) != NO_ERROR)
		{
      _logger->log( Logger::LOG_LEV_ERROR, "Error configuring network: (%d) %s", result, lao_strerror(result));

		return NETWORK_ERROR;
		}

	// Set OK flag
	var_errmsg.Set("Connected");

	return NO_ERROR;
}


//+Entry
//
// LoadParameterFile           loads a parameter file
//
// Read a parameter file and sends the commands to the i-Drive

int StageCtrl::LoadParameterFile( std::string filename)
{
	int result;


	_logger->log( Logger::LOG_LEV_DEBUG, "Configuring with file %s", filename.c_str());
	if (( result = SetupiDrive(  _driveHwAddr , (char *)filename.c_str())) != NO_ERROR)
		{
		_logger->log( Logger::LOG_LEV_ERROR, "Error during i-Drive setup: (%d) %s", PLAIN_ERROR(result), lao_strerror(PLAIN_ERROR(result)));
		return result;
		}

	_logger->log( Logger::LOG_LEV_DEBUG, "Configuration successful");
	
	return NO_ERROR;
}



//@Function
//
// StartHoming                start the homing procedure
//@

int StageCtrl::StartHoming(void)
{
	int stat;

	_logger->log( Logger::LOG_LEV_DEBUG, "Starting homing sequence");

   idrive_homing=1;
	stat = LoadParameterFile( (std::string) ConfigDictionary()["homingfile"]);
	if (stat == NO_ERROR)
      setCurState(STATE_HOMING);

	return stat;
}

//@Function
//
// StartPoleFinding             start the pole finding procedure
//@

int StageCtrl::StartPoleFinding(void)
{
   int stat;

	_logger->log( Logger::LOG_LEV_DEBUG, "Starting pole finding sequence");

   stat = LoadParameterFile( (std::string) ConfigDictionary()["polefindingfile"]);
   if (stat == NO_ERROR)
		setCurState(STATE_POLEFINDING);

   return stat;
}

//@Function
//
// UnitsToSteps              units -> steps conversion
//
// Convert from userland coordinates (units = millimeters) to iDrive steps
//@

double StageCtrl::UnitsToSteps( double units)
{
	return units * _stepsRatio;
}

//@Function
//
// StepsToUnits              units -> steps conversion
//
// Convert from iDrive steps to userland coordinates (units = millimeters)
//@

double StageCtrl::StepsToUnits( double steps)
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
      StageCtrl *c;
      c = new StageCtrl( argc, argv);

      c->Exec();

      delete c;
   } catch (AOException &e) {
      Logger::get()->log( Logger::LOG_LEV_FATAL, "%s:%d: %s", __FILE__, __LINE__, e.what().c_str());
   }
}
