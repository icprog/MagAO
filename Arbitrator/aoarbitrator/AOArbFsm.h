/*
 * This class implements the AO Arbitrator FSM
 *
 * The core FSM is generated automaticall by FSMC based on
 * AOArbFsmCore.fsm
 *
 */

#ifndef AOARBFSM_H_INCLUDE
#define AOARBFSM_H_INCLUDE

#include "framework/AbstractFsm.h"
#include "aoarbitrator/AOCommandImplFactory.h"
#include "aoarbitrator/AOArbFsmCore.h"

#include "arblib/base/BasicOpCodes.h"
#include "arblib/aoArb/AOOpCodes.h"

using namespace Arcetri::Arbitrator;

namespace Arcetri {
namespace Arbitrator {

class AOArbFsm: public AbstractFsm, public AOArbFsmCore {

	public: // METHODS

		//  Constructor
    AOArbFsm(unsigned int commandHistoryMaxSize, string ident);

   		//  Destructor
   		virtual ~AOArbFsm(){};
                bool processCommand() const;

		virtual string currentStateAsString();

	private:
		/*
		 * Implement pure virtual base class methods.
		 */
		void trigger(OpCode code);
		void forceFailure();
		void forcePanic();

// The following map supports the conversion between OpCodes and Events
        private:
                map<OpCode,Event> EvtTable;

};

}
}

#endif // AOFSM_H_INCLUDE
