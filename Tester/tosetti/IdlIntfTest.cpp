
#include "IdlCtrlInterface.h"
using namespace Arcetri;

int main() {
	
	Logger::setParentName("IDLCTRL-INTF");
	Logger::setLogFile("IDLCTRL-INTF", Utils::getAdoptLog());
	
	// Init and start the thrdLib
	thInit("IDLCTRL-INTF");
	thDefTTL(1);
	thStart("127.0.0.1", 5);
	thSendMsg(0, "", SETREADY, 0, NULL);
	
	IdlCtrlInterface idlIntf("conf/left/idlctrl.conf", Logger::LOG_LEV_TRACE);
	
	Logger* logger = Logger::get("IDL-INTERFACE");
	logger->printStatus();
	
	int loops = 500000;
	for(int i=0; i<loops; i++) {
		IdlCommandReply cmdReply = idlIntf.sendCommand("print, 5");
		logger->log(Logger::LOG_LEV_DEBUG, "Idl output: %s", cmdReply.getOutput().c_str());
		logger->log(Logger::LOG_LEV_DEBUG, "Idl error: %s", cmdReply.getError().c_str());
		//usleep(100000);
	}
	logger->log(Logger::LOG_LEV_DEBUG, "Terminated");
}
