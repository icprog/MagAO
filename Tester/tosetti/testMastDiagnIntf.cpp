#include "Utils.h"
#include "mastdiagnlib.h"

using namespace Arcetri;


int main() {

    string masterdiag = "masterdiagnostic." + Utils::getAdoptSide();
 
	Logger::setParentName("MASTDIAGNINTF");
	
	Logger* logger = Logger::get(Logger::LOG_LEV_INFO);
	
	// Init and start the thrdLib
	thInit((char*)"MASTDIAGNINTF");
	thStart((char*)"127.0.0.1", 5);
	thSendMsg(0, (char*)"", SETREADY, 0, NULL);
	
	// Create the interface...
	MastDiagnInterface* mdIntf = new MastDiagnInterface(masterdiag, Logger::LOG_LEV_INFO);
	
	///...and use it!
	try {
		mdIntf->dumpAdSecDiagnHistory();
		//mdIntf->dumpOptLoopDiagnHistory();
	}
	catch(MastDiagnInterfaceException& e) {
		logger->log(Logger::LOG_LEV_ERROR, "%s", e.what().c_str());
	}
    
    msleep(2000);

	// Remember to clean up
	thClose();
	Logger::destroy();
	
	return 0;
}
