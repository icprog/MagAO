
#include "UdpConnection.h"
#include "Logger.h"
#include "AdamLib.h"
using namespace Arcetri;


int main() {
	
	Logger::setParentName("ADAM-TEST");
	Logger::setLogFile("ADAM-TEST", Utils::getAdoptLog());

	AdamWatchdog* adamWD = NULL;
	try {
		adamWD = new AdamWatchdog("L");
		Logger::get("TCPCONNECTION", Logger::LOG_LEV_TRACE);
		adamWD->start();
		
		sleep(5);
	}
	catch(AdamException& e) {
		Logger::get()->log(Logger::LOG_LEV_FATAL, "%s", e.what().c_str());
	}
	delete adamWD; // This will call adamWD->stop();
	
	Logger::destroy();
}
