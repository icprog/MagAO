//@File: SimpleMotor.cpp
//
// Class definition for a simple generic linear or rotary motor,
//
//@

extern "C" {

#include "base/errlib.h"
#include "hwlib/netseriallib.h"
}

#include <string>
#include <errno.h>

#include "SimpleMotor.h"
#include "SimpleMotorCtrl.h"


//@Function: SimpleMotor
//
// Standard constructor
//
//@
SimpleMotor::SimpleMotor( AOApp *app, Config_File &cfg)
{
	int stat;

   Logger::get()->log( Logger::LOG_LEV_TRACE, "SimpleMotor constructor");
	pthread_mutex_init( &mutex, NULL);

	this->app = app;

	LoadConfig( cfg);

 	network_ok = 0;
  	errmsg = "";
   
   	stat = SetupVars();
   	if (IS_ERROR(stat)) throw AOException("Setting up RTDB vars", stat, __FILE__, __LINE__);


   	ERRMSG_LEN = 20;

   
} 

//@Function: ~SimpleMotor
//
// Complete destructor
//
//@
SimpleMotor::~SimpleMotor()
{
// 	TurnOff();
   	ShutdownNetwork();
}
        
//@Function: LoadConfig
//
// Loads operating parameters from a configuration file
//@
int SimpleMotor::LoadConfig( Config_File &cfg)
{
   try {
	   MVaddr  = (char *) ((string)cfg["IPaddr"]).c_str();
	   MVport = cfg["IPport"];
	   MVmax = cfg["Max"];
	   MVmin = cfg["Min"];
	   steps_per_pos = cfg["Ratio"];
      _homingOffset = cfg["HomingOffset"];
   } catch (Config_File_Exception &e)
   {
      Logger::get()->log( Logger::LOG_LEV_FATAL, e.what().c_str());
     throw(e);
   }

	return NO_ERROR;
}

double SimpleMotor::getHomingOffset()
{
   return _homingOffset;
}

void SimpleMotor::setHomingOffset( double offset)
{
   _homingOffset = offset;
}

//@Function: SetupNetwork
//
// Setups the network/serial communication with the motor,
// based on the informations contained in MVaddr and MVport
//
// Can be called multiple times without resetting the communication
// (the subsequent calls will have no effect)
//
// Passing a value of <force> different from zero will force the
// communication to be reset and initialized again. This parameter
// is set to zero by default
//@
int SimpleMotor::SetupNetwork( int /* force */)
{
    int stat;
 
    // Close the previous connection if requested
    /*
    if ( (network_ok ==1) && force)
    {
       stat = ShutdownNetwork();
       CHECK_SUCCESS(stat);
    }
    */

    //Always close previous connection
    ShutdownNetwork();
            
    Logger::get()->log( Logger::LOG_LEV_INFO, "Connecting to %s:%d", MVaddr, MVport);

    // Setup serial/network interface
    pthread_mutex_lock(&mutex);
    stat = SerialInit( MVaddr, MVport );	// This locks if network is down or host unreachable
    pthread_mutex_unlock(&mutex);
    if (stat != NO_ERROR)
        Logger::get()->log( Logger::LOG_LEV_ERROR, "Connect result: %d - Errno: %s", stat, strerror(errno));
    CHECK_SUCCESS(stat);
                                                                                                                                      
    // Set OK flag
    network_ok =1;
    errmsg = "Connected";

    Logger::get()->log( Logger::LOG_LEV_INFO, "Network connect OK");

    return NO_ERROR;
}

//@Function: SetupVars
//
// Setups the necessary MsgD-RTDB vars to control the motor
//
// Multiple calls have no effect, apart from resetting the variables
// to the values corresponding to the current motor status (which should
// not be any significant change).
//@
int SimpleMotor::SetupVars()
{
   try {
      var_pos_cur = RTDBvar( app->MyFullName(), "POS", CUR_VAR, REAL_VARIABLE, 1);
      var_pos_req = RTDBvar( app->MyFullName(), "POS", REQ_VAR, REAL_VARIABLE, 1);
      var_pos_name = RTDBvar( app->MyFullName(), "POSNAME", CUR_VAR, CHAR_VARIABLE, 50);
      var_loend = RTDBvar( app->MyFullName(), "LOEND", NO_DIR, REAL_VARIABLE, 1);
      var_hiend = RTDBvar( app->MyFullName(), "HIEND", NO_DIR, REAL_VARIABLE, 1);
      var_sw_status = RTDBvar( app->MyFullName(), "SW_STATUS", NO_DIR, INT_VARIABLE, 1);
   } catch (AOVarException &e)  {
        Logger::get()->log(Logger::LOG_LEV_FATAL, "%s:%s: %s", __FILE__, __LINE__, e.what().c_str());
        throw AOException("Error creating RTDB variables");
   }

   var_loend.Set( MVmin);
   var_hiend.Set( MVmax);

   app->Notify( var_pos_req, ((SimpleMotorCtrl *)app)->PosReqChanged);

   return NO_ERROR;
}



//@Function: ShutdownNetwork
//
// Closes the network interface
//
//@
int SimpleMotor::ShutdownNetwork()
{
	int stat = NO_ERROR;

	if ( network_ok)
	{
		pthread_mutex_lock(&mutex);
      Logger::get()->log( Logger::LOG_LEV_DEBUG, "SimpleMotor: closing communication");
		stat = SerialClose();
		pthread_mutex_unlock(&mutex);
		CHECK_SUCCESS(stat);

		network_ok = 0;
		errmsg = "Not connected";
	}
   
	return stat;
}

//@Function: PosToSteps
//
// Convert a commanded position into motor steps
//
//@

double SimpleMotor::PosToSteps( double position)
{
   Logger::get()->log( Logger::LOG_LEV_TRACE, "PosToSteps(): %f * %f = %f", position, steps_per_pos, (position * steps_per_pos));
	return position * steps_per_pos + _homingOffset;
}

//@Function: StepsToPos
//
// Convert motor steps to a commanded position
//
//@

double SimpleMotor::StepsToPos( double position)
{
	if (steps_per_pos != 0)
		return (position - _homingOffset) / steps_per_pos;
	else
		return 0;
}


const char *SimpleMotor::GetTargetVarname()
{
   return var_pos_req.complete_name().c_str();
}
   


int SimpleMotor::EndHoming()
{
	return NO_ERROR;
}

int SimpleMotor::Disable()
{
	return NO_ERROR;
}

int SimpleMotor::GetSwitchStatus()
{
   return 0;
}
