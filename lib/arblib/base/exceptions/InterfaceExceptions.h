#ifndef INTERFACEEXCEPTIONS_H_INCLUDE
#define INTERFACEEXCEPTIONS_H_INCLUDE

#include "AOExcept.h"
#include "base/errordb.h"

namespace Arcetri {
namespace Arbitrator {

	/*
	 * Base class for all the arbitrator interface exceptions.
	 */
	class ArbitratorInterfaceException: public AOException {
		public:
			ArbitratorInterfaceException(string message, 
			                    		 int errcode = ARB_INTF_GENERIC_ERROR, 
			                    		 string file = "", 
			                    		 int line = 0): AOException(message, errcode, file, line) {}
	
	};

}
}

#endif /*INTERFACEEXCEPTIONS_H_INCLUDE*/
