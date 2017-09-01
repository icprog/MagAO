#ifndef TIMETODIE_H_INCLUDED
#define TIMETODIE_H_INCLUDED

#include "MirrorCtrlExceptions.h"

class TimeToDie {
	
	public:
		static void init(bool* timeToDie) { _timeToDie = timeToDie; }
		static bool check() throw (MirrorControllerShutdownException);
		
	private:
		static bool* _timeToDie;
};

#endif
