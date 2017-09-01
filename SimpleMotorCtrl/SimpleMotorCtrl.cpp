//@File: SimpleMotorCtrl.cpp
//
// {\tt WheelCtrl} : MsgD-RTDB multithreaded client for a SimpleMotor-derived motor
//
//@

extern "C" {
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>		// atof()
#include <math.h>       // fabs()
#include <string.h>
#include <stdarg.h>
#include <errno.h>

#include <pthread.h>

#include "base/thrdlib.h"
#include "hwlib/netseriallib.h"
}

#include "SimpleMotorCtrl.h"
#include "FilterWheel.h"
#include "Rerotator.h"
#include "MCBL2805.h"
#include "ADCWheel.h"
#include "Mercury.h"
#include "RTDBvar.h"

using namespace Arcetri;

#ifndef __VISAO_NO_MAIN
int VersMajor = 1;
int VersMinor = 0;
const char *Date = "September 2005";
const char *Author = "A. Puglisi";

#endif //#ifndef __VISAO_NO_MAIN

// How many samples to average to see if we are in position.
// Setting this to 1 or 0 will disable the averaging.
// Can also be overriden in config. file with the "PosAverage" keyword.
#define POS_AVG (50)

// Mutex to lock out other threads during complex operations
pthread_mutex_t threadMutex;

//
//+Entry help
//
//  help prints an usage message
//
// Rimosso perche' non usato (L.F.)
//
//static void help()
//{
//    printf("\nSimpleMotorCtrl  - Vers. %d.%d.  %s, %s\n\n", VersMajor,VersMinor,Author,Date);
//    printf("Usage: WheelCtrl [-v] [-s <Server Address>]\n\n");
//    printf("   -s    specify server address (defaults to localhost)\n");
//    printf("   -f    specify configuration file (defaults to config)\n");
//    printf("   -v    verbose mode\n");
//    printf("\n");
//}



SimpleMotorCtrl::SimpleMotorCtrl( std::string name, const std::string& conffile) throw (AOException) : AOApp( name, conffile)
{
   Create();
}

SimpleMotorCtrl::SimpleMotorCtrl( int argc, char **argv) throw (AOException) : AOApp( argc, argv)
{
   Create();
}

void SimpleMotorCtrl::Create() throw (AOException)
{

   try {
   _homingPos = ConfigDictionary()["HomingPosition"];
   _abortPos  = ConfigDictionary()["AbortPosition"];
   _goodWindow = ConfigDictionary()["GoodWindow"];
   _startingPos  = ConfigDictionary()["StartingPos"];
   _autoHoming   = ConfigDictionary()["AutoHoming"];
   _autoHomingOffset = ConfigDictionary()["AutoHomingOffset"];

   } catch (Config_File_Exception &e) {
      _logger->log( Logger::LOG_LEV_FATAL, "Config file exception: %s", e.what().c_str());
      throw AOException("Fatal: Missing configuration data");
   }

   // Read custom position list, if present.
   _customPos.clear();

   int numCustomPos=0;
   try {
      numCustomPos = ConfigDictionary()["customPositionNum"];
   } catch (Config_File_Exception &e) {
      _logger->log( Logger::LOG_LEV_INFO, "No custom positions defined in cfg file.");
   }
  
   try {
      _posAverage = ConfigDictionary()["PosAverage"];
      _logger->log( Logger::LOG_LEV_INFO, "Position check set to an average of %d samples.", _posAverage);
   } catch (Config_File_Exception &e) {
      _posAverage = POS_AVG;
      _logger->log( Logger::LOG_LEV_INFO, "No PosAverage keyword defined in cfg file, using default value of %d.", _posAverage);
   }
  
   try 
   {
      NoDisable = (int)ConfigDictionary()["NoDisable"];
   }
   catch (Config_File_Exception &e) 
   {
      NoDisable = false;
   }

   if (numCustomPos >0) 
     for (int i=0; i<numCustomPos; i++) {
         ostringstream namekey, poskey;
         namekey << "pos" << i << "_name";
         poskey << "pos" << i << "_pos";
         string name;
         float pos;
         try {
            name = (std::string) ConfigDictionary()[namekey.str()];
            pos  = (float) ConfigDictionary()[poskey.str()];
            _customPos[name] = pos;
         } catch (Config_File_Exception &e) {
            _logger->log( Logger::LOG_LEV_ERROR, "Custom position %d not found in cfg file.", i);
         }
     }

   //Read whether to use the homing dither, default to not using it.
   try {
      _homingDither = ConfigDictionary()["homingDither"];
   } catch (Config_File_Exception &e) {
      _homingDither = 0;
   }

   try {
      _homingDitherOffset = ConfigDictionary()["homingDitherOffset"];
   } catch (Config_File_Exception &e) {
      _homingDitherOffset = 1;
   }


   try {
      _disabletime = (int)ConfigDictionary()["disableTime"];
   } catch (Config_File_Exception &e) {
      _disabletime = -1;
   }

   pthread_mutex_init(&threadMutex, NULL);

   motor = NULL;
   _savedPos = 0;
   _savedState = false;

   _homingDitherApplied = 1; //So a pre-home move doesn't cause a home

   

}//void SimpleMotorCtrl::Create()

void SimpleMotorCtrl::Run()
{
   _logger->log( Logger::LOG_LEV_INFO, "Running...");

	while(!TimeToDie()) {
        try {

      		DoFSM();
        } catch (AOException &e) {
        	_logger->log( Logger::LOG_LEV_ERROR, "Caught exception (at %s:%d): %s", __FILE__, __LINE__, e.what().c_str());

         // When the exception is thrown, the mutex was held!
          pthread_mutex_unlock(&threadMutex);
        }
    }
}

// ------------- Creates the motor object

SimpleMotor *SimpleMotorCtrl::CreateMotor()
{
	SimpleMotor *motor = NULL;
  

	if ((string)ConfigDictionary()["MotorType"] == string("filterwheel"))
      {
      _logger->log( Logger::LOG_LEV_DEBUG, "This is a filterwheel");
	  motor = new FilterWheel( this, ConfigDictionary());
      }
	if ((string)ConfigDictionary()["MotorType"] == string("adc"))
      {
      _logger->log( Logger::LOG_LEV_DEBUG, "This is an adc");
	  motor = new ADCWheel( this, ConfigDictionary());
      
      }
	else if ((string)ConfigDictionary()["MotorType"] == string("rerotator"))
      {
      _logger->log( Logger::LOG_LEV_DEBUG, "This is a rerotator");
		motor = new Rerotator( this, ConfigDictionary());
      }
	else if ((string)ConfigDictionary()["MotorType"] == string("mcbl2805"))
      {
      _logger->log( Logger::LOG_LEV_DEBUG, "This is an MCBL2805");
	   motor = new MCBL2805( this, ConfigDictionary());
      }
	else if ((string)ConfigDictionary()["MotorType"] == string("mercury"))
      {
      _logger->log( Logger::LOG_LEV_DEBUG, "This is a mercury");
	   motor = new Mercury( this, ConfigDictionary());
      }
   if (!motor)
      _logger->log( Logger::LOG_LEV_ERROR, "Invalid motor type");

   
   return motor;
}


int SimpleMotorCtrl::DoFSM()
{
	// [todo] This contains blocking methods, like TestNetwork(),
	// and doesn't allow the AOApp to die !
   int status, stat,i;
   static float delay=1.0;
   int moving = 0;
   double avg;

   if (!motor) motor = CreateMotor();

   // Lock out everyone else!!
   pthread_mutex_lock(&threadMutex);

   status = getCurState();

   // Always check the network
   if ((motor) && (status != STATE_NOCONNECTION))
   {
      if ((stat =motor->TestNetwork()) != NO_ERROR)
      {
         _logger->log( Logger::LOG_LEV_TRACE, "Test network returned %d", stat);
         motor->ShutdownNetwork();
         status = STATE_NOCONNECTION;
         delay = 1.0;
         setCurState(status);
       } 
   }

   // Start by creating the motor object (it will initialize itself)
   // and do the homing procedure

   if ((status == STATE_OPERATING) || (status == STATE_HOMING)|| (status == STATE_READY)) {
      // Sample average position
      if (_posAverage >0) {
          avg=0;
          for (i=0; i<_posAverage; i++){
             avg +=  motor->GetPosition(-1);
             }
          avg /= _posAverage;
          
      } else {
          avg = motor->GetPosition(-1);
      }
     
      motor->var_pos_cur.Set(floor(avg*1000. + 0.5)/1000., 0, CHECK_SEND);
      
   }

   switch(status)
   {
      case STATE_NOCONNECTION:
      if (motor)
         if (motor->SetupNetwork() == NO_ERROR)
         {
            if (motor->TestNetwork() == NO_ERROR)
            {
               setCurState(STATE_CONNECTED);
            }
            else
            {
               motor->ShutdownNetwork();
            }
         }
     break;

     case STATE_CONNECTED:
     if (motor) {
       motor->TurnOn();

       if (_savedState) {

          // When resuming from a saved state power-off, recalculate offset
          double offset = motor->getHomingOffset(); 
          motor->setHomingOffset( offset -_savedPos);
          setCurState(STATE_READY);
          delay = 1.0;

       } else {

          // Otherwise, perform normal homing

         if (_autoHoming) {
            _logger->log( Logger::LOG_LEV_INFO, "Starting auto homing");
            _homingDitherApplied = 0;
            motor->Home();
            setCurState(STATE_HOMING);
            
         } else { 
            if(!NoDisable) 
            {
               motor->Disable();
            }
            setCurState(STATE_READY);
         }

       //_targetPos = motor->GetPosition(0);

       delay = 1.0;
       }
     }
     break;

   // Wait until the movement is finished (it means that the homing was OK)
   case STATE_HOMING:
      updatePos();
      motor->GetSwitchStatus();

      //For VisAO presets, we update req var in RTDB so it is current.
      if (fabs(avg - motor->StepsToPos(0)) < _goodWindow) 
      {
         motor->EndHoming();
         sleep(1); // have to make sure EndHoming() takes before MoveTo, or it gets missed.
         if(_homingDither && !_homingDitherApplied) 
         {
            _targetPos = avg - _homingDitherOffset; //motor->GetPosition(-1) - _homingDitherOffset;

            motor->MoveTo(_targetPos);
            _starttime = time(NULL);
            
         }
         else 
         {
            
            if (_autoHomingOffset) 
            {
         	   motor->MoveTo( _startingPos);
               _starttime = time(NULL);
                _targetPos = _startingPos;
            } 
            else 
            {
               _targetPos = motor->GetPosition(-1);
            }
         }
         setCurState(STATE_OPERATING);
      }

      delay = 0.1;
      break;

   case STATE_OPERATING:
      updatePos(-1); //don't force the update - the averaging above takes care of that
      motor->GetSwitchStatus();

     
      // Average position is equal to target position -> movement is done
      //std::cout << _targetPos << " " << avg << " " << _goodWindow << "\n";
      if (fabs(_targetPos - avg) < _goodWindow)
      {
         if(_homingDither && !_homingDitherApplied)
         {
             _homingDitherApplied = 1;
             //std::cout << "home: 3\n";
             motor->Home();
             setCurState(STATE_HOMING);
          }
          else
          {
            if(!NoDisable)
            {
               motor->Disable();
            }
            status = STATE_READY;
            setCurState(status);
          }
         delay = 0.1;
         updatePos(-1);
      }

      // Disable drive after a long timeout
      if (_disabletime >0) 
      {
         
           if (time(NULL) - _starttime > _disabletime) {
              std::cout << "Timeout\n";
               _logger->log( Logger::LOG_LEV_WARNING, "Aborting movement after %d seconds", _disabletime);
               stat = motor->Abort();
               _homingDitherApplied = 1; //So we don't home on next move.
               _targetPos = motor->GetPosition(-1);
               setCurState(STATE_READY);
           }
      }
            
      break;
      
   // Periodically update variables
   case STATE_READY:
      updatePos(-1);
      motor->GetSwitchStatus();

      // Update status variables and change position polling speed
      moving = motor->IsMoving();

      // If keeping position, see if we have to refine the position
      if (moving == 0) {
         //double diffpos = fabs(_targetPos - avg);//motor->GetPosition());
        /* if (diffpos > _goodWindow) {
             _logger->log( Logger::LOG_LEV_INFO, "Correcting motor position from %5.3g to %5.3g", avg, _targetPos);//motor->GetPosition(), _targetPos);
             motor->MoveTo(_targetPos);
	          status = STATE_OPERATING;
         }*/
      }
      else {
         delay = 0;
         status = STATE_OPERATING;
         updatePos(-1);		// Force position update
      }

      setCurState(status);
      delay = 1.;
      break;
     }

   pthread_mutex_unlock(&threadMutex);

   // Always set current status (for external watchdog)
   setCurState( getCurState());

   nusleep( (unsigned int)(delay * 1e6));

  return NO_ERROR;
}

int SimpleMotorCtrl::PosReqChanged( void *pt, Variable *var)
{
    SimpleMotorCtrl *ctrl = (SimpleMotorCtrl *)pt;
    int stat;
    double pos = var->Value.Dv[0];

    pthread_mutex_lock(&threadMutex);

    // This one must be checked before!!
    if (pos == ctrl->_abortPos) {
      _logger->log( Logger::LOG_LEV_DEBUG, "Aborting movement");
      stat = ctrl->motor->Abort();
      ctrl->_homingDitherApplied = 1; //So we don't home on next move.
      ctrl->_targetPos = ctrl->motor->GetPosition(0);
      ctrl->setCurState(STATE_READY);
      pthread_mutex_unlock(&threadMutex);
      return stat;
   }

   

    int status = ctrl->getCurState();
   
    
    if (status != STATE_READY) {
        _logger->log( Logger::LOG_LEV_WARNING, "Movement command refused (device not ready)");
       
    	pthread_mutex_unlock(&threadMutex);
        return NO_ERROR;
    }
    _logger->log( Logger::LOG_LEV_INFO, "Received movement command to %6.2f", pos);

    double curPos = ctrl->motor->GetPosition(0);

    
    if (curPos == pos) {
      
      _logger->log( Logger::LOG_LEV_INFO, "Motor already in position");
      stat = NO_ERROR;
    }
    else if (pos == ctrl->_homingPos) {
        _logger->log( Logger::LOG_LEV_INFO, "Starting homing");
        ctrl->_homingDitherApplied = 0;
        stat = ctrl->motor->Home();
        if (stat == NO_ERROR)
           ctrl->setCurState(STATE_HOMING);
    }
    else {
        stat = ctrl->motor->MoveTo(pos);
	if (stat == NO_ERROR) {
	   ctrl->setCurState(STATE_OPERATING);
           ctrl->_targetPos = pos;
           ctrl->_starttime = time(NULL);
        }
    }

    pthread_mutex_unlock(&threadMutex);
    return stat;
}

void SimpleMotorCtrl::SetupVars()
{
   _logger->log(Logger::LOG_LEV_INFO, "Setting up SimpleMotorCtrl RTDB vars...");
   try {
      setCurState(STATE_NOCONNECTION);

       //Publish these for VisAO use
      var_startingpos =  RTDBvar(MyFullName(), "STARTPOS", NO_DIR, REAL_VARIABLE, 1);
      var_startingpos.Set((double)_startingPos, 0, FORCE_SEND);

      var_homepos =  RTDBvar(MyFullName(), "HOMINGPOS", NO_DIR, REAL_VARIABLE, 1);
      var_homepos.Set((double)_homingPos, 0, FORCE_SEND);

      var_abortpos =  RTDBvar(MyFullName(), "ABORTPOS", NO_DIR, REAL_VARIABLE, 1);
      var_abortpos.Set((double)_abortPos, 0, FORCE_SEND);


   } catch (AOVarException &e) {
   	  // [to check] Errors in this logging...
      //_logger->log(Logger::LOG_LEV_FATAL, "%s:%s: %s", __FILE__, __LINE__, e.what().c_str());
      throw AOException("Error creating RTDB variables");
   }
}

void SimpleMotorCtrl::InstallHandlers() {

   int stat;
    _logger->log(Logger::LOG_LEV_INFO, "Installing notification handler for MOTOR_SAVESTATE ...");
    if((stat=thHandler(MOTOR_SAVESTATE, "*", 0, savestate_handler, "savestate", this))<0) {
       _logger->log(Logger::LOG_LEV_FATAL, "Error in installing notification handler for MOTOR_SAVESTATE: %s [%s:%d]",
            lao_strerror(stat),__FILE__, __LINE__);
       throw AOException("thHandler error in installing notification handler for MOTOR_SAVESTATE", stat,__FILE__, __LINE__);
   }
} 

//@Function: savestate_handler
//
// handler of a MOTOR_SAVESTATE command. Saves the current motor position
// and set the _savedState flag.
// At the next restart of the motor, the homing will not be performed,
// and motion will resume from the saved position (using the saved position as an offset
// to the current motor position, which is always reset to zero).
//
// This is intended to prevent too many homings when motors are turned on and off repeatedly.
// 
// After a MOTOR_SAVESTATE command is executed, the motor must be powered off immediately. If another
// movement occurs before powering off, the saved state will not be valid.
//
// If multiple MOTOR_SAVESTATE commands are issued, the last one overrides the others.
//@

int SimpleMotorCtrl::savestate_handler(MsgBuf *msgb, void *argp, int /*hndlrQueueSize*/)
{   
   SimpleMotorCtrl *ctrl = (SimpleMotorCtrl *)argp;
   double pos = ctrl->motor->PosToSteps( ctrl->motor->GetPosition(0));

   ctrl->setSavedState(pos);
   int uno=1;
   thReplyMsg( MOTOR_SAVESTATE, sizeof(int),&uno,msgb);

   return NO_ERROR;
}

void SimpleMotorCtrl::setSavedState( double pos) {
   _savedPos = pos;
   _savedState = true;
}

void SimpleMotorCtrl::updatePos( bool force) {

   double pos = motor->GetPosition( force);     // This will update the POS.CUR value too.

   // Update custom position display, if any.
   string customPos =" ";

   //If we are using custom positions, have the name be intermediate if we are between positions
   if(_customPos.size() > 0) customPos = "intermediate";
   
   map<string, float>::iterator iter;
   for (iter = _customPos.begin(); iter != _customPos.end(); iter++) {
      float myPos = (*iter).second;
      if (fabs(myPos-pos) < _goodWindow)
         customPos = (*iter).first;
   }

   motor->SetPosName(customPos);
}
    
 
#ifndef __VISAO_NO_MAIN

// Main

int debug;

int main( int argc, char **argv) {

   SetVersion(VersMajor,VersMinor);
	
   debug = 0;
	SimpleMotorCtrl *c = NULL;
	try {
      c = new SimpleMotorCtrl( argc, argv);

 		c->Exec();

  	} catch (AOException &e) {
     	Logger::get()->log( Logger::LOG_LEV_FATAL, "%s:%d: %s", __FILE__, __LINE__, e.what().c_str());
  	}
  	
  	// Correctly terminate the AOApp, disconnecting form MsgD
  	delete c;
}

#endif //#ifndef __VISAO_NO_MAIN
