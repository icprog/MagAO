#include "Logger.h"

using namespace Arcetri;

int main() {

	try {
		Logger* logger = Logger::get(Logger::LOG_LEV_TRACE);
		
		Logger::setLogFile("startLogFile", ".");
		
		logger->log(Logger::LOG_LEV_TRACE, "Loggo ergo sum");
		
		Logger::setLogFile("endLogFile", ".");
		
		logger->log(Logger::LOG_LEV_TRACE, "Loggo ancora ergo sum di+");
	}
	catch(LoggerFatalException& e) {
		printf("%s\n", e.what().c_str());
	}

	return 0;
}
