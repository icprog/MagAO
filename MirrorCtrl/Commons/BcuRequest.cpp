#include "BcuRequest.h"

BcuRequest::BcuRequest(BcuMessage* bcuMessage) {
	memcpy(&_bcuMessage, bcuMessage, sizeof(BcuMessage));
}

BcuRequest::BcuRequest(int firstBcu, int lastBcu, int firstDsp, int lastDsp,
				   	   int opCode, int flags, 
				   	   int dataAddressDw, int dataSizeBytes, int shmId) {
				   	   	
		setFirstBcu(firstBcu);
		setLastBcu(lastBcu);
		setFirstDsp(firstDsp);
		setLastDsp(lastDsp);
		setOpCode(opCode);
		setFlags(flags);
		setStatus(0);
		setDataAddressDw(dataAddressDw);
		setDataSizeBytes(dataSizeBytes);
		setShmId(shmId);
}

BcuRequest::BcuRequest(const BcuRequest& request) {
 	memcpy(&_bcuMessage, request.getBcuMessage(), sizeof(BcuMessage));
}


