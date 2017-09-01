#include "BcuLib/BcuCommon.h"
using namespace Arcetri;

#include "Watchdog.h"
#include "Commons/BcuPacket.h"
#include "Commons/TimeToDie.h"

extern "C" {
#include "base/timelib.h"
}



uint32 Watchdog::_ENABLE_WATCHDOG[2];
uint32 Watchdog::_CLEAR_WATCHDOG[2];
uint32 Watchdog::_DISABLE_WATCHDOG[2];

Watchdog::Watchdog(int id, int bcuStart, int bcuNum, int interval_ms, int logLevel):AbstractBcuRequestManager("WATCHDOG", id, Priority::HIGH_PRIORITY, logLevel) {
	
	_interval_us = interval_ms*1000;
	_enabled = false;
	
	_ENABLE_WATCHDOG[0]  = 0x100000;
	_ENABLE_WATCHDOG[1]  = 0x000000;
	_CLEAR_WATCHDOG[0]   = 0x300000;
	_CLEAR_WATCHDOG[1]   = 0x000000;
	_DISABLE_WATCHDOG[0] = 0x200000;
	_DISABLE_WATCHDOG[1] = 0x000000;
	
	_enableRequest = new BcuRequest(bcuStart, bcuStart+bcuNum-1, 
	                            	255, 255, 
	                            	Bcu::OpCodes::MGP_OP_RESET_DEVICES,
	                            	Constants::WANTREPLY_FLAG,
	                            	0,
	                      			sizeof(_ENABLE_WATCHDOG), 
	                      			-1);
	_clearRequest = new BcuRequest(bcuStart, bcuStart+bcuNum-1, 
	                               255, 255, 
	                               Bcu::OpCodes::MGP_OP_RESET_DEVICES,
	                               Constants::WANTREPLY_FLAG,
		                           0,
		                      	   sizeof(_CLEAR_WATCHDOG), 
		                      	   -1);
	_disableRequest = new BcuRequest(bcuStart, bcuStart+bcuNum-1, 
	                             	 255, 255, 
	                             	 Bcu::OpCodes::MGP_OP_RESET_DEVICES,
	                             	 Constants::WANTREPLY_FLAG,
		                             0,
		                      		 sizeof(_DISABLE_WATCHDOG), 
		                      		 -1);
	
	
	_logger->log(Logger::LOG_LEV_DEBUG, "Watchdog period: %d ms", interval_ms);
	
	initBcuPool(bcuStart, bcuNum);
}

Watchdog::~Watchdog() {
	
}

pthread_t Watchdog::start() {
	pthread_t watchDog;
	pthread_create(&watchDog, NULL, &execute, (void*)this);
    return watchDog;
}

void* Watchdog::execute(void* pthis) {
	Watchdog* me = (Watchdog*)pthis;
	try {
		me->run();
	}
	catch(MirrorControllerShutdownException& e) {
		me->getLogger()->log(Logger::LOG_LEV_WARNING, "Watchdog shutting down...");
	}
	return NULL;
}

bool Watchdog::enable(bool enable) {
	
	// Enable/disable the watchdog on BCUs
	if(enable) {
		setBcuRequestInfo(_enableRequest, (BYTE*)_ENABLE_WATCHDOG);
	}
	else {
		setBcuRequestInfo(_disableRequest, (BYTE*)_DISABLE_WATCHDOG);
	}
	sendBcuRequest();
	waitBcuReply();
	if(!_currentRequestInfo->isFailed()) {
		_enabled = enable;
		if(_enabled) {
			_logger->log(Logger::LOG_LEV_INFO, "WATCHDOG: succesfully enabled !");
		}
		else {
			_logger->log(Logger::LOG_LEV_INFO, "WATCHDOG: succesfully disabled !");
		}
	}	
	else {
		_logger->log(Logger::LOG_LEV_WARNING, "WATCHDOG: enabling/disabling failed (one or more BCUs may be offline)");
	}
	return _enabled;
}



void Watchdog::run() throw (MirrorControllerShutdownException) {

	_enabled = false;
	bool re_enabling = true;
	
	while((_enabled || re_enabling) && !TimeToDie::check()) {
		
		// Check if enabled
		if(!_enabled) {
			_logger->log(Logger::LOG_LEV_INFO, "------------------------------------------------------------------------------");
			_logger->log(Logger::LOG_LEV_INFO, "WATCHDOG: BCUs are offline, trying to enable watchdog...");
			re_enabling = true;
			// Try to enable
			if(enable(true)) {
				re_enabling = false;
			}
		}
		else {
			// Send the "clear" request
			setBcuRequestInfo(_enableRequest, (BYTE*)_CLEAR_WATCHDOG);
			sendBcuRequest();
			waitBcuReply();
			if(!_currentRequestInfo->isFailed()) {
				_logger->log(Logger::LOG_LEV_INFO, "WATCHDOG: I'm still alive, yeh-eh-eh !!!");
			}
			// If request failed means that ate least one BCU is offline, so try
			// to re-enable the watchdog
			else {
				_enabled = false;
				re_enabling = true;
			}
		}
		nusleep(_interval_us);
	}
	_logger->log(Logger::LOG_LEV_WARNING, "WATCHDOG: TERMINATED !!!");
}
