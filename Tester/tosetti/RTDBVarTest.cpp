extern "C" {
	#include "base/thrdlib.h"
}
	
#include "Logger.h"
#include "RTDBvar.h"

using namespace Arcetri;

int main() {
	
	printf("Initializing thrdlib...\n");
	if(!IS_ERROR(thInit("RTDBVarTest"))) {
		printf("Starting thrdlib...\n");
		thStart("127.0.0.1", 1);
	}
	
	Logger::setParentName("RTDBVarTest");
	Logger::setLogFile("RTDBVarTest", Utils::getAdoptLog());
	Logger* logger = Logger::get(Logger::LOG_LEV_TRACE);
 	Logger::printPoolStatus();
	
	logger->log(Logger::LOG_LEV_DEBUG, "--- RTDB var test ---");
	
	int val = 10;
	RTDBvar aVar = RTDBvar("L.CTRL1.VAR_A", INT_VARIABLE, 1);
	aVar.Set(val);
	aVar.Get(&val);
	logger->log(Logger::LOG_LEV_DEBUG, "Var value = %d",  val);
	
	sleep(15);
	
	aVar.Update();
	aVar.Get(&val);
	logger->log(Logger::LOG_LEV_DEBUG, "Var value = %d", val);
	
	sleep(15);
	
	aVar.Update();
	aVar.Get(&val);
	logger->log(Logger::LOG_LEV_DEBUG, "Var value = %d", val);
	
	return 0;
	
}