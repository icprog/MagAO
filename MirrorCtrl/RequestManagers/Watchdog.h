#ifndef WATCHDOG_H_INCLUDED
#define WATCHDOG_H_INCLUDED

#include "Commons/AbstractBcuRequestManager.h"

/*
 * Send a single UDP packet, the "I'm alive" packet, to all BCU
 * with a given period.
 * 
 * Note that this class doesn't use all the services offered by
 * the AbstractBcuRequestManager for efficiency reasons (it have
 * to send always the same BcuPacket)
 */
class Watchdog: public AbstractBcuRequestManager {
	
	public:
		explicit Watchdog(int id, int bcuStart, int bcuNum, int interval_ms, int logLevel);
		virtual ~Watchdog();
		
		int getInterval_ms() { return _interval_us; }
		
		pthread_t start();
		
	private:
		static void* execute(void* pthis);
		void run() throw (MirrorControllerShutdownException);
		
		// Private until isn't necessary to allow external enable/disable
		bool enable(bool enable);
		
	private:
	
		static uint32 _ENABLE_WATCHDOG[2];
		static uint32 _CLEAR_WATCHDOG[2];
		static uint32 _DISABLE_WATCHDOG[2];
		
		bool _enabled;
		
		BcuRequest* _enableRequest;
		BcuRequest* _clearRequest;
		BcuRequest* _disableRequest;
		
		int _interval_us;
};

#endif
