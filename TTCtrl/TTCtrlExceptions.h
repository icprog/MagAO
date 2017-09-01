#ifndef TTCTRLEXCEPTIONS_
#define TTCTRLEXCEPTIONS_

#include "AOExcept.h"

/*
 * WavesException
 */
class WavesException: public AOException {
	
	public:
		WavesException(string m):AOException(m) { exception_id = "WavesException"; }
};

#endif /*TTCTRLEXCEPTIONS_*/
