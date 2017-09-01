#ifndef BASICOPCODES_H_INCLUDE
#define BASICOPCODES_H_INCLUDE

namespace Arcetri {
namespace Arbitrator {

typedef int MsgCode;	// Type for codes defined in msgcodes.h (see AOARB_* codes)
typedef int OpCode;

/*
 * Defines some basic opcodes for Alerts and Commands.
 */
class BasicOpCodes {

	public:

		// --- UTILITY COMMANDS e REQUESTS --- //
		static const OpCode PING_CMD  	     = 1000;
		static const OpCode REQUEST_FEEDBACK = 1001;
		static const OpCode REQUEST_STATUS   = 1002;

		// --- COMMANDS --- //
		static const OpCode FORCE_FAILURE   = 2000;	// Not a real command: used only to force FSM
		static const OpCode FORCE_PANIC     = 2001;	// Not a real command: used only to force FSM
		static const OpCode RECOVER_FAILURE = 2002;
		static const OpCode RECOVER_PANIC   = 2003;

		// --- ALERTS --- //
		static const OpCode WARNING 	= 3001;
		static const OpCode ERROR 	= 3002;
		static const OpCode PANIC	= 3003;
		static const OpCode HOOFFLOAD	= 3004;
		static const OpCode DUMPSAVED	= 3005;


};


}
}

#endif
