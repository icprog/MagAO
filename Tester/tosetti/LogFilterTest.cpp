#include "LogFilter.h"
#include "Logger.h"

using namespace Arcetri;

int main() {
	
	Logger::setParentName("LOGFILTER_TEST");

	try {
		string logFileAbsPath = "/tmp/logfilter.log";
	
		LogFilter lf(logFileAbsPath, Logger::LOG_LEV_DEBUG);	
		FilterCondition cond("?", "2007-09-26 15:50:32.427526", "2007-09-26 15:50:32.428002", "?", FilterCondition::OR);
		lf.setCondition(cond);
		lf.execute();
	}
	catch(LogFilterException& e) {
		Logger::get(Logger::LOG_LEV_DEBUG)->log(Logger::LOG_LEV_DEBUG, "%s", e.what().c_str());
	}

	return 0;
}
