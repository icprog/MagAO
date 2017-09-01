#include "IntfLib/InterferometerInterface.h"

using namespace Arcetri;

int main() {
	
	
	Logger::setParentName("INTFINTERFACE-TEST");
	Logger::setLogFile("INTFINTERFACE-TEST", Utils::getAdoptLog());
	
	// Init and start the thrdLib
	thInit("INTFINTERFACE-TEST");
	thDefTTL(1);
	thStart("127.0.0.1", 5);
	thSendMsg(0, "", SETREADY, 0, NULL);
	
	InterferometerInterface interface = InterferometerInterface(Logger::LOG_LEV_TRACE);
	
	interface.setPostProcessing("HDF5");
	
	return 0;
}