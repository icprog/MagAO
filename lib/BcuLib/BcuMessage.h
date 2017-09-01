#ifndef BCUMESSAGE_H_INCLUDE
#define BCUMESSAGE_H_INCLUDE

#include <string>

using namespace std;

namespace Arcetri {

	namespace Bcu {
		
		/*
		 * @Struct: BcuMessage
		 * Raw structure hosting a BcuMessage
		 */
		struct BcuMessage {
			int _firstBcu;				// First BCU to consider
			int _lastBcu;				// Last BCU to consider
			int _firstDsp;				// First DSP to be addressed
			int _lastDsp;				// Last DSP to be addressed
			int _opCode;				// Command (WR_SAME, WR_SEQ, RD_SEQ). Note that "SEQ" or "SAME"
										// is interpreted (by MirrorCtrl) for ***both*** DSP and BCU 
										// addressed range !!!
			int _status;				// Message status (SUCCESS or FAULT), set for the reply
			int _dataAddress;			// (DWORDS) BCU memory address to read/write 
			int _flags;					// Flags to add to BCU command (es, FLAG_ASQUADWORD)
			int _dataSize;				// (BYTES) Length of data to read/write from/to address;
										// also "size of _shmId" divided by (lastDSP-firstDSP+1)*(lastBcu-firstBcu-1) 
			int _shmId;          		// Shared memory buffer id, source/dest for a write/read.
										// Use -1 if not specified.
		};
		
		/*
		 * @Class{API}: Priority
		 * Priorities for a BcuMessage.
		 * @
		 */
		class Priority {
	
			public:
			
				// Ranges
				static const int MIN_ALLOWED_PRIORITY = 0;
				static const int MAX_ALLOWED_PRIORITY = 9;
			
				// Availables priorities
				static const int LOW_PRIORITY 	 = 3;
				static const int MEDIUM_PRIORITY = 6;	// This should be used only for diagnostic and not for a sendMultiBcuCommand
				static const int HIGH_PRIORITY 	 = 9;	// This must be the MAX value: never define an higher value !
		    	
		    	static string getPriorityDescription(int priority) {
		    		switch(priority) {
		    			case 3: return "LOW-PRIORITY";
		    					break;
		    			case 6: return "MEDIUM-PRIORITY";
		    					break;
		    			case 9: return "HIGH-PRIORITY";
		    					break;
		    			default: return "UNKNOWN-PRIORITY";
		    		}
		    	}
		    	
		};
		
	}
}

#endif /*BCUMESSAGE_H_INCLUDE*/
