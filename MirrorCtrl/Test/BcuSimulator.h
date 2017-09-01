
#include "UdpConnection.h"
#include "Utils.h"
#include <pthread.h>

class BcuSimulator {

	// All public !!!
	public:
		
		BcuSimulator(int localPort, string bcuCommIpAddress, int bcuCommPort);
		~BcuSimulator();
	
		UdpConnection* _conn;

		Logger* _logger;
		
		pthread_t start();
		static void* execute(void* pthis);
		void simulate();
		
		const string info() { return ("[Local port: " + Utils::itoa(_conn->getLocalPort()) + " Remote Ip:" + _conn->getRemoteIpAsString().c_str() + ":"+Utils::itoa( _conn->getRemotePort())+"]"); }  
        
};
