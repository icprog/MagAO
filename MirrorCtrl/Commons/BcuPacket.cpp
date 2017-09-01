// lib
#include "BcuLib/BcuCommon.h"

#include "BcuPacket.h"


BcuPacket::BcuPacket() {
	
	_rawPacket = NULL;
	
	// Allocate header
	_hdr = new UdpHeader();	// Of course I need it !
	
	_data = NULL;			// Initialized in set(...)
	_requestInfo = NULL;	// Initialized in set(...)
}

BcuPacket::BcuPacket(uint8 msgId, uint8 firstDsp, uint8 lastDsp, uint8 opCode,  int memoryAddress, uint8 flags, 
					 uint16 dataLengthDw, BYTE* data, BcuRequestInfo* reqInfo) {
	
	// Allocate header. Doesn't allocate data: it's pre-allocated by BcuPacket creator !!!
	_hdr = new UdpHeader();
	set(msgId, firstDsp, lastDsp, opCode, memoryAddress, flags, dataLengthDw, data, reqInfo);
}

void BcuPacket::set(uint8 msgId, uint8 firstDsp, uint8 lastDsp, uint8 opCode,  int memoryAddress, uint8 flags, 
                     uint16 dataLengthDw, BYTE* data, BcuRequestInfo* reqInfo) {
    
    setFirstDSP(firstDsp);
    setLastDSP(lastDsp);
    setOpCode(opCode);
    setMemoryAddress(memoryAddress);    
    setMsgID(msgId);
    setFlags(flags);
    
    setData(data, dataLengthDw);
    
    setRequestInfo(reqInfo);
}


BcuPacket::~BcuPacket(){
	delete _hdr; 
	//Logger::get()->log(Logger::LOG_LEV_DEBUG, "UDP packet deleted (except data)");
}


int BcuPacket::getFullDataLength() {
	if(Bcu::OpCodes::isSeq(getOpCode())) {
		return getDataLengthDw()*((getLastDSP()-getFirstDSP())/Bcu::OpCodes::getDspFactor(getOpCode())+1);
	}
	else {
		return getDataLengthDw();
	}
}

int BcuPacket::getFullDataLengthInBytes() {
	if(Bcu::OpCodes::isSeq(getOpCode())) {
		return getDataLengthBytes()*((getLastDSP()-getFirstDSP())/Bcu::OpCodes::getDspFactor(getOpCode())+1);
	}
	else {
		return getDataLengthBytes();
	}
}


void BcuPacket::send(UdpConnection* conn) throw (UdpFatalException) {
	
	// Create the buffer to send. This will contain Header[+Body]
	// in a contiguous memory location, so I need a physical copy.
	// Attention: "rawPacketSize" is in bytes !!!
	int rawPacketSize = HEADER_SIZE; 
	// Note that body data are not sent if the packet is a READ request !!!
	if((isWrite()) && getDataLengthDw() != 0) {
		rawPacketSize += getFullDataLengthInBytes();
	}
	_rawPacket = new BYTE[rawPacketSize];
	
	// Fill the buffer with the header
	memcpy(_rawPacket, _hdr, HEADER_SIZE);
	
	// Fill the body only if is a not empty WRITE
	if(isWrite() && getDataLengthDw() != 0) {
		if (OpCodes::isSame(getOpCode()))  
			memcpy(_rawPacket + HEADER_SIZE, _data, getFullDataLengthInBytes());
		else if(OpCodes::isSeq(getOpCode())) {
			int dspFactor = _requestInfo->getBcuRequest()->getDspFactor();
			int requestFirstDsp = _requestInfo->getBcuRequest()->getFirstDsp();
			int requestStride = _requestInfo->getBcuRequest()->getDataSizeBytes();
			for(int dsp=getFirstDSP(); dsp<=getLastDSP(); dsp+=dspFactor) {
				memcpy(_rawPacket + HEADER_SIZE + getDataLengthBytes()*((dsp-getFirstDSP())/dspFactor),	// Dest
			 		   _data + requestStride*((dsp-requestFirstDsp)/dspFactor),  						// Source
			 		   getDataLengthBytes());   														// Length
			}
		}
		else {
			throw UdpFatalException("BcuPacket::send: invalid Opcode (neither SAMEnor SEQ)");
		}
	}
	
	// Send the packet (can throw an UdpFatalException)
	conn->send(_rawPacket, rawPacketSize);
	
	Logger::get()->log(Logger::LOG_LEV_TRACE, "\tPacket sent: ");
	printStatus(true);
	
	// Clear the internal buffer
	delete [] _rawPacket;
	_rawPacket = NULL;
}


void BcuPacket::receive(UdpConnection* conn, bool checkId) throw (UdpFatalException, UdpTimeoutException, UnexpectedBcuPacketException) {
	
	uint8 expectedMsgId = getMsgId();	
	
	// Receive a packet (can throw an UdpFatalException or an UdpTimeoutException)
	_rawPacket = new BYTE[HEADER_SIZE + MAX_BODY_SIZE];
	try {
		conn->receive(_rawPacket, HEADER_SIZE + MAX_BODY_SIZE);
	}
	catch(UdpTimeoutException& e) {
		delete [] _rawPacket;
		throw;
	}
	
	// This is the received header, wich saves only the msgId of the request header,
	// and contains the reply status (success or failed)
	UdpHeader* replyHeader = (UdpHeader*)(_rawPacket);
	
	// If requested, check the msgId
	// If is an unexpected packet...
	if(checkId && (replyHeader->msgId != expectedMsgId)) {
		BYTE* unexpectedRawPacket = _rawPacket;	// Only for safety
		_rawPacket = NULL;						// Only for safety
		//Logger::get()->log(Logger::LOG_LEV_WARNING, "Received packet %d instead of %d", receivedHeader->msgId, expectedMsgId);
		throw UnexpectedBcuPacketException("Received packet " + Utils::itoa(replyHeader->msgId) + " instead of " + Utils::itoa(expectedMsgId), 
											replyHeader->msgId, 
											unexpectedRawPacket);
	}

	// ...else set the packet using the internal buffer
	else {
		// Note that body data are received ONLY if the packet WAS a read request and
		// the reply IS a success !!!
		if(OpCodes::isRead(getOpCode()) && OpCodes::isSuccess(replyHeader->opCode) && getDataLengthDw() != 0) {
			if (OpCodes::isSeq(getOpCode())) {
				int dspFactor = _requestInfo->getBcuRequest()->getDspFactor();
				int requestFirstDsp = _requestInfo->getBcuRequest()->getFirstDsp();
				int requestStride = _requestInfo->getBcuRequest()->getDataSizeBytes();
				for(int dsp=getFirstDSP(); dsp<=getLastDSP(); dsp+=dspFactor) {
					//-->//Logger::get()->log(Logger::LOG_LEV_TRACE, "!!! --- memcopy to shm address = %x --- !!!", _data + requestStride*(dsp-requestFirstDsp));
					memcpy(_data + requestStride*((dsp-requestFirstDsp)/dspFactor),  										// Dest
						   _rawPacket + HEADER_SIZE + getDataLengthBytes()*((dsp-getFirstDSP())/dspFactor), 	// Source
				 		   getDataLengthBytes());   															// Length
				} 												
			}
			else {
				throw UdpFatalException("BcuPacket::receive: Opcode is not Same nor Seq. Don't know how to handle it!");
			}
		}
			
		Logger::get()->log(Logger::LOG_LEV_TRACE, "\tPacket received: ");
		printStatus(false, replyHeader->opCode);
			
		// Save the current request header, except for the "opCode" field (reply status), 
		// later used to notify the client (see notifyReply method)
		_hdr->opCode = replyHeader->opCode;
	}
	
	// Clear the internal buffer
	delete [] _rawPacket;
	_rawPacket = NULL;
}

void BcuPacket::receive(BYTE* rawPacket) {
	
	_rawPacket = rawPacket;
	
	UdpHeader* replyHeader = (UdpHeader*)(_rawPacket);
	
	// Note that body data are received ONLY if the packet WAS a read request and
	// the reply IS a success !!!
	if(OpCodes::isRead(_hdr->opCode) && OpCodes::isSuccess(replyHeader->opCode) && getDataLengthDw() != 0) {
		if (OpCodes::isSame(getOpCode()))  
			memcpy((unsigned char*)_data, _rawPacket + HEADER_SIZE, getFullDataLengthInBytes());
		else if (OpCodes::isSeq(getOpCode())) {
			int dspFactor = _requestInfo->getBcuRequest()->getDspFactor();
			int requestFirstDsp = _requestInfo->getBcuRequest()->getFirstDsp();
			int requestStride = _requestInfo->getBcuRequest()->getDataSizeBytes();
			for(int dsp=getFirstDSP(); dsp<=getLastDSP(); dsp+=dspFactor) {
				//-->//Logger::get()->log(Logger::LOG_LEV_TRACE, "!!! --- memcopy to shm address = %x --- !!!", _data + requestStride*(dsp-requestFirstDsp));
				memcpy(_data + requestStride*((dsp-requestFirstDsp)/dspFactor),  										// Dest
					   _rawPacket + HEADER_SIZE + getDataLengthBytes()*	((dsp-getFirstDSP())/dspFactor), 	// Source
			 		   getDataLengthBytes());    															// Length
			}												
		}
		else {
			throw UdpFatalException("BcuPacket::receive: Opcode is not Same nor Seq. Don't know how to handle it!");
		}
	}
		
	Logger::get()->log(Logger::LOG_LEV_TRACE, "\tPacket received: ");
	printStatus(false, replyHeader->opCode);
		
	// Save the request header, except for the "opCode" field (reply status), 
	// later used to notify the client (see notifyReply method)
	_hdr->opCode = replyHeader->opCode;
	
	// Clear the internal buffer
	delete _rawPacket;
	_rawPacket = NULL;
}

void BcuPacket::notifyReply(int bcuId) { 
	// Not all packets reference a RequestInfo (i.e. WatchdogPacket doesn't)
	if(_requestInfo) {
		// Set the request failed only if it isn't already failed !!!
		if(!_requestInfo->isFailed() && !isSuccess()) {
			_requestInfo->setFailed();
			
		}
		// Extra debug info to know which BCU has failed
		if (!isSuccess()) {
			Logger* logger = Logger::get();
         if (isFault())
			   logger->log(Logger::LOG_LEV_ERROR, "******** FAILED PACKET FROM BCU %d - CMD FAULT *******", bcuId);
         else if (isCATimeout())
			   logger->log(Logger::LOG_LEV_ERROR, "******** FAILED PACKET FROM BCU %d - CMD TIMEOUT CA *******", bcuId);
         else 
			   logger->log(Logger::LOG_LEV_ERROR, "******** FAILED PACKET FROM BCU %d *******, opcode=%d", bcuId, getOpCode());
		}
		_requestInfo->decreaseReplyCounter(bcuId);
	}
}

void BcuPacket::printStatus(bool isSend, int replyOpCode) {
	Logger* logger = Logger::get();
	logger->log(Logger::LOG_LEV_TRACE, "\t> MsgId:       %d", getMsgId());
	logger->log(Logger::LOG_LEV_TRACE, "\t> Opcode:      %d", getOpCode());
	if(!isSend) {
		logger->log(Logger::LOG_LEV_TRACE, "\t> Reply Opcode: %d", replyOpCode);
	}
	logger->log(Logger::LOG_LEV_TRACE, "\t> Flags:       %d", getFlags());
	logger->log(Logger::LOG_LEV_TRACE, "\t> First Dsp:   %d", getFirstDSP());
	logger->log(Logger::LOG_LEV_TRACE, "\t> Last Dsp:    %d", getLastDSP());
	logger->log(Logger::LOG_LEV_TRACE, "\t> Address:     %d", getMemoryAddress());
	logger->log(Logger::LOG_LEV_TRACE, "\t> Data length: %d dword, %d bytes", getDataLengthDw(), getDataLengthBytes());
	logger->log(Logger::LOG_LEV_TRACE, "\t> Packet size: %d bytes", getFullDataLengthInBytes());
	if(logger->getLevel() >= Logger::LOG_LEV_TRACE) {
		if(((isSend && isWrite()) || (!isSend && isRead())) && getDataLengthDw() != 0) {
			Logger::get()->log(Logger::LOG_LEV_TRACE, "\t> Packet body: ");
			printf("\t");
			for(int i=0; i<getFullDataLength(); i++) {
					printf("%08X ", ((unsigned int*)(_rawPacket+HEADER_SIZE))[i]);
			}
			printf("\n");
		}
	}
	logger->log(Logger::LOG_LEV_TRACE, "\n");
}



        
