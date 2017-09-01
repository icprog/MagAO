#ifndef ARBITRATOREXCEPTIONS_H_INCLUDE
#define ARBITRATOREXCEPTIONS_H_INCLUDE

#include "AOExcept.h"
#include "base/errordb.h"

namespace Arcetri {
namespace Arbitrator {

	/*
	 * Base class for all the arbitrator exceptions.
	 */
	class ArbitratorException: public AOException {
		public:
			ArbitratorException(string message, 
			                    int errcode = ARB_GENERIC_ERROR, 
			                    string file = "", 
			                    int line = 0): AOException(message, errcode, file, line) {}
	
	};

}
}

#endif /*ARBITRATOREXCEPTIONS_H_INCLUDE*/
