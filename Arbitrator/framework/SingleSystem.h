#ifndef SINGLESYSTEM_H_INCLUDE
#define SINGLESYSTEM_H_INCLUDE

#include "Logger.h"

#include "arblib/base/Alerts.h"
#include "arblib/base/Command.h"
#include "arblib/base/exceptions/CommandExceptions.h"
#include "AbstractSystem.h"

#include <string>

using namespace std;

namespace Arcetri {
namespace Arbitrator {

/*
 * @Class: SingleSystem
 *
 * System that can be instantiated only once.
 * Trying to call the constructor more than once will result in a fatal exception.
 */
class SingleSystem : public AbstractSystem {

	public:

		/*
		 * Create an abstract system.
		 */
		SingleSystem(string systemName);

		/*
		 *
		 */
		virtual ~SingleSystem();


  private:
      static int _counter;

};

}
}

#endif /*SINGLESYSTEM_H_INCLUDE*/
