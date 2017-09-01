
#include "AOApp.h"
#include "Commons.h"
#include "BcuMaps.h"


class BcuReadyTester: public AOApp {
	
	public:
	
		BcuReadyTester(string firstBcu, string lastBcu, double timeout_s, int delay_ms):
		               AOApp("BcuReadyTester", "L", "127.0.0.1", 0, Logger::LOG_LEV_INFO) {
	    	BcuIdMap bcuIds;
			
			_firstBcu = bcuIds[firstBcu];
			_lastBcu = bcuIds[lastBcu];
			_timeout_s = timeout_s;
			_delay_ms = delay_ms;
			
			_logger->log(Logger::LOG_LEV_INFO, "Created tester for BCUs %d-%d ready with timeout %2.1f s and delay %d ms...", _firstBcu, _lastBcu, _timeout_s, _delay_ms);
	    }
	    
	    void Run() {
	    	Bcu::isBcuReady(_firstBcu, _lastBcu, _timeout_s, _delay_ms);
	    }
    
    private:
    
    	int _firstBcu, _lastBcu;
    	double _timeout_s;
    	int _delay_ms;
	
};

using namespace Arcetri;

int main(int argC, char* argv[]) {
	
	if(argC != 5) {
		printf("Usage: TestIsBcuReady firstBcu lastBcu timeout_s delay_ms\n");
		return 0; 
	}
	else {
		BcuReadyTester bcuReadyTester(string(argv[1]), string(argv[2]), atof(argv[3]), atoi(argv[4]));
		bcuReadyTester.Exec();
	}
};
