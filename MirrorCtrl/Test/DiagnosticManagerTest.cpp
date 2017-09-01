#include <string>
using namespace std;

#include "RequestManagers/DiagnosticManager.h"
#include "Logger.h" 
using namespace Arcetri;

int main() {

	Logger::get(Logger::LOG_LEV_TRACE);
	DiagnosticManager* myDm = new DiagnosticManager("DIGNMANTEST", 0, 
													"conf/left/diagnostics/Fast.diagnostic",
													0, 6, 6);
	delete myDm;
	Logger::destroy();
	
	return 0;
}
