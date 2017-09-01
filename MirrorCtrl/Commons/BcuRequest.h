#ifndef BCUREQUEST_H_INCLUDED
#define BCUREQUEST_H_INCLUDED

// Lib
#include "aotypes.h"
#include "BcuLib/BcuCommon.h"
using namespace Arcetri::Bcu;


/*
 * A Bcu request received from a MirrorCtrl client,
 * by way of MsgD service.
 * This is actually a wrapper a around a BcuMessage (see BcuCommon.h)
 */
class BcuRequest { 
	
	public:
	
		BcuRequest() {}
	
		/*
		 * Constructor: create a BcuRequest using an existing BcuMessage.
		 * The bcuMessage is copied !
		 */
		BcuRequest(BcuMessage* bcuMessage);
	
		/*
		 * Constructor (for test purposes only)
		 */
		BcuRequest(int firstBcu, int lastBcu, int firstDSP, int lastDSP,
				   int opCode, int flags,
				   int dataAddressDw, int dataSizeBytes,
				   int shmId);
				   
		/*
		 * Copy constructor
		 */
		 BcuRequest(const BcuRequest& request);
		 
				   
		/*
		 * Get the BcuMessage 
		 */
		BcuMessage* getBcuMessage() const { return (BcuMessage*)(&_bcuMessage); }
				   
		int getFirstBcu() 	  	  { return _bcuMessage._firstBcu; }	
		int getLastBcu()  	  	  { return _bcuMessage._lastBcu; }	
		int getFirstDsp() 	  	  { return _bcuMessage._firstDsp; }
		int getLastDsp()  	  	  { return _bcuMessage._lastDsp; }
		int getOpCode()	  	  	  { return _bcuMessage._opCode; }
		int getFlags()	  	  	  { return _bcuMessage._flags; }
		int getStatus()	  	  	  { return _bcuMessage._status; }
		int getDataAddressDw()	  { return _bcuMessage._dataAddress; }
		int getDataAddressBytes() { return _bcuMessage._dataAddress*sizeof(DWORD); }
		int getDataSizeDw()   	  { return _bcuMessage._dataSize/sizeof(DWORD); }
		int getDataSizeBytes() 	  { return _bcuMessage._dataSize; }
		int getShmId()		  	  { return _bcuMessage._shmId; }
		
		void setFirstBcu(int firstBcu) 	  	 	 { _bcuMessage._firstBcu = firstBcu; }	
		void setLastBcu(int lastBcu)  	  	 	 { _bcuMessage._lastBcu = lastBcu; }								
		void setFirstDsp(int firstDsp) 	  	 	 { _bcuMessage._firstDsp = firstDsp; }
		void setLastDsp(int lastDsp)  	  	 	 { _bcuMessage._lastDsp = lastDsp; }
		void setOpCode(int opCode)	  	  	 	 { _bcuMessage._opCode = opCode; }
		void setFlags(int flags)	  	  		 { _bcuMessage._flags = flags; }
		void setStatus(int status)	  	  	 	 { _bcuMessage._status = status; }
		void setDataAddressDw(int dataAddressDw) { _bcuMessage._dataAddress = dataAddressDw; }
		void setDataSizeBytes(int dataSize) 	 { _bcuMessage._dataSize = dataSize; }
		void setShmId(int shmId)		  		 { _bcuMessage._shmId = shmId; }
		
		
		void increaseDataAddressDw(int dWords){ _bcuMessage._dataAddress += dWords; }
		
		bool isReadOpcode()	 { return OpCodes::isRead(getOpCode()); }
		bool isWriteOpcode() { return OpCodes::isWrite(getOpCode()); };
		bool isSameOpcode()	 { return OpCodes::isSame(getOpCode()); };
		bool isSeqOpcode()	 { return OpCodes::isSeq(getOpCode()); };
		int getDspFactor()	 { return OpCodes::getDspFactor(getOpCode()); };
	
	
	private:
		// The raw content of the BcuRequest
		BcuMessage _bcuMessage;
};

#endif

