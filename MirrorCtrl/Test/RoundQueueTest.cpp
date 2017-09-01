#include "RoundQueue.h"
#include "Logger.h"
using namespace Arcetri;

int main() {
	
	UdpConnection conn(10001, "127.0.0.2", 10002, 10);
	
	RoundQueue myRoundQueue(8, &conn, 10);
	
	Logger* _logger = Logger::get(Logger::LOG_LEV_DEBUG);
	
	BcuPacket* p = new BcuPacket();
	int count = 0;
	while(myRoundQueue.canInsert()) {
		myRoundQueue.insert(p);
		_logger->log(Logger::LOG_LEV_DEBUG, "Inserted packet n. %d", count);
		count ++;
	}
	_logger->log(Logger::LOG_LEV_DEBUG, "RoundQueue - incoming full");
	
	count = 0;
	while(myRoundQueue.canSend()) {
		myRoundQueue.send();
		_logger->log(Logger::LOG_LEV_DEBUG, "Sent packet n. %d", count);
		count ++;
	}
	_logger->log(Logger::LOG_LEV_DEBUG, "RoundQueue - outgoing empty");
	
	count = 0;
	while(myRoundQueue.canReceive()) {
		myRoundQueue.receive();
		_logger->log(Logger::LOG_LEV_DEBUG, "Received packet n. %d", count);
		count ++;
	}
	_logger->log(Logger::LOG_LEV_DEBUG, "RoundQueue - receiving empty");
	
	
	return 0;
}
