//@File: Func.cpp
//
// DiagnApp alarms and warnings generic handlers 
//@

#include "mastdiagnlib.h"
#include "BcuLib/bcucommand.h"
#include "AdSecConstants.h"
#include "Logger.h"
#include "DiagnApp.h"


#include "arblib/base/Alerts.h" // Boost initialization done once in housekeeper.cpp
//#include "arblib/base/Command.h" // Boost initialization done once in housekeeper.cpp
//#include "arblib/adSecArb/AdSecCommands.h"

extern "C" {
#include "aotypes.h"        // float32
#include <pthread.h>
#include "aoslib/aoslib.h" // aoslib needed to close hexapod brakes
}

#include "Funct.h"

// FunctAction
//
FunctAction::FunctAction(): _called_in_this_loop(false)
{

}

void FunctAction::BeginLoop()
{
    _called_in_this_loop = false;
}

void FunctAction::react(const std::string &y, double f)
{
    if (_called_in_this_loop == false) {
        doit(y,f);
        _called_in_this_loop = true;
    }
}


// FunctWarning
void FunctWarning::doit(const std::string &y, double f)
{
    Logger::get("FUNCTWARNING")->log(Logger::LOG_LEV_WARNING, "FunctWarning %s %g", y.c_str(), f);
}

// FunctAlarm
void FunctAlarm::doit(const std::string &y, double f)
{
    Logger::get("FUNCTALARM")->log(Logger::LOG_LEV_ERROR, "FunctAlarm %s %g", y.c_str(), f);
}



// FunctEmergencyStop
FunctEmergencyStop::FunctEmergencyStop(std::string , bool shutdown) : FunctAction(),
                                                                      isSirenOn(false),
                                                                      mustShutdown(shutdown)
{
    string basedir = AOApp::ConfigDictionary().getDir()+"/";

    logger = Logger::get("FUNCTEMERGENCYSTOP");
    adSecArbNotifier = new AlertNotifier(((string)AOApp::ConfigDictionary()["AdSecArbName"])+"."+AOApp::Side());
    
    adam = new AdamModbus("ADAM-MODBUS", false);
    
/*           No need to initialize the connection with AOS.
             Because we do not operate the hexapod any longer
    int ret = aos_init( (char*) ( AOApp::Side().c_str())  ); 
    if (!IS_ERROR(ret)) {
        logger->log(Logger::LOG_LEV_ERROR, "Unable to initialize aos library: error %d [%s:%d]", ret, __FILE__, __LINE__);
    }
*/
}

FunctEmergencyStop::~FunctEmergencyStop()
{
    delete adam;
}

void FunctEmergencyStop::doit(double *f)
{
    logger->log(Logger::LOG_LEV_WARNING, "FunctEmergencyStop doit(double*) %p should not be called ",f);
}


void FunctEmergencyStop::doit(const std::string &y, double f)
{
    logger->log(Logger::LOG_LEV_ERROR, "%s = %g  [%s:%d]",y.c_str(),f,__FILE__,__LINE__);

    // Ask ADAM to enable TSS and disable all coils
    //adam.enableTss();	// Armando don't trust TSS!!
	
    try {
		if ( !adam->disableCoils() )  {
		    logger->log(Logger::LOG_LEV_ERROR, "ADAM CAN'T DISABLE COILS [%s:%d]",__FILE__,__LINE__);
		    //mustShutdown=true;
		}
    } catch (AOException &e) {
		logger->log(Logger::LOG_LEV_ERROR, "ADAM CAN'T DISABLE COILS: %s [%s:%d]", e.what().c_str(), __FILE__,__LINE__);
    }

    try{
		if ( (mustShutdown) && !adam->disableMainPower() ) {
		    logger->log(Logger::LOG_LEV_ERROR, "ADAM CAN'T DISABLE MAIN POWER [%s:%d]",__FILE__,__LINE__);
		}
    } catch (AOException &e) {
		logger->log(Logger::LOG_LEV_ERROR, "ADAM CAN'T DISABLE MAIN POWER: %s [%s:%d]", e.what().c_str(), __FILE__,__LINE__);
    }
    //mustShutdown = old_mustShutDown;

    // want to immediately stop the hexapod: send command directly to AOS, then notify Arbitrator
//         logger->log(Logger::LOG_LEV_INFO, "Sending hexapod_close_brakes command [%s:%d]",__FILE__,__LINE__);
//         if ((ret=aos_hxpbrake(brakeclose)) != 0)  
//         logger->log(Logger::LOG_LEV_ERROR, "Couldn't close hexapod brakes. Error %d [%s:%d]", ret, __FILE__,__LINE__);

    // send a "panic" to AdSecArbitrator
    logger->log(Logger::LOG_LEV_INFO, "[FunctEmergencyStop] try to send Alert to AdSec Arbitrator  [%s:%d]", __FILE__,__LINE__);
    
    try {
		if (mustShutdown) {
		    Panic panic("SYSTEM POWERED OFF");
		    adSecArbNotifier->notifyAlert((Alert*)(&panic));
		    logger->log(Logger::LOG_LEV_INFO, "[FunctEmergencyStop] Alert to AdSec Arbitrator succesfully sent! [%s:%d]", __FILE__,__LINE__);
		} else {
		    Error myerror("COILS DISABLED", false);
		    adSecArbNotifier->notifyAlert((Alert*)(&myerror));
		    logger->log(Logger::LOG_LEV_INFO, "[FunctEmergencyStop] Alert to AdSec Arbitrator succesfully sent! [%s:%d]", __FILE__,__LINE__);
		}
        
    } catch(ArbitratorInterfaceException& e) {
		// Nothing to do!
		logger->log(Logger::LOG_LEV_ERROR, "[FunctEmergencyStop] Alert to AdSec Arbitrator not sent! [%s:%d]", __FILE__,__LINE__);
    }
    

    // TODO  DELETE BELOW AND DELETE INCLUDES AND CHANGE MAKEFILE
    // DEBUG  send a comamnd to AdSecArbitrator
	    /*try {
	      ArbitratorInterface* adSecArbCommandNotifier = new ArbitratorInterface(AOApp::ConfigDictionary()["AdSecArbName"]);
	      Command * cmd = new Arcetri::AdSec_Arbitrator::RecoverFailure((unsigned int)1000);
	      adSecArbCommandNotifier->requestCommand(cmd);
	      logger->log(Logger::LOG_LEV_INFO, "[FunctEmergencyStop] Command to AdSec Arbitrator succesfully sent!");
	      delete adSecArbCommandNotifier;
	      delete cmd;
	      }
	      catch(ArbitratorInterfaceException& e) {
	      // Nothing to do!
	      logger->log(Logger::LOG_LEV_ERROR, "[FunctEmergencyStop] Command to AdSec Arbitrator not sent!");
	      }*/
            // DELETE ABOVE
            
	    // play siren
//			if(!isSirenOn) {
//				isSirenOn = true;
//				spawn_siren();
//			}

}


void FunctEmergencyStop::spawn_siren() 
{
	pthread_attr_t attribute;
	pthread_t thread_id;

	pthread_attr_init(&attribute);
	pthread_attr_setdetachstate(&attribute, PTHREAD_CREATE_DETACHED); // dies at end
	logger->log(Logger::LOG_LEV_TRACE, "Spawning thread [%s:%d]",__FILE__,__LINE__);
	if(pthread_create(&thread_id,&attribute, PlaySiren,this)!=0) {
	    logger->log(Logger::LOG_LEV_WARNING, "CAN'T START PLAYING SIREN [%s:%d]",__FILE__,__LINE__);
	}
}

void *FunctEmergencyStop::PlaySiren(void* argp)
{
	FunctEmergencyStop *pt = (FunctEmergencyStop*)argp;

	std::string command = "play " + pt->_siren_complete_filename + " > /dev/null  2> /dev/null";
	system(command.c_str());
	pt->isSirenOn=false;

    return NULL;
}


// FunctAlarmAndDump
void FunctAlarmAndDump::doit(const std::string &y, double f)
{
    FunctAlarm::doit(y,f);
    
    if (_diagapp != NULL)
        _diagapp->setDumpAlarm(y);
}


// FunctEmergencyStopAndDump
void FunctEmergencyStopAndDump::doit(const std::string &y, double f)
{
    FunctEmergencyStop::doit(y,f);
    
    if (_diagapp != NULL)
        _diagapp->setDumpAlarm(y);
}
