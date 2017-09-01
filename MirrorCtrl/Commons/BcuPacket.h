#ifndef BCUPACKET_H_INCLUDE
#define BCUPACKET_H_INCLUDE

#include <string> 
using namespace std;

#include "Commons/BcuRequestInfo.h"

// Lib
#include "aotypes.h"
#include "BcuLib/BcuCommon.h"
#include "UdpConnection.h"

#include "Commons/MirrorCtrlExceptions.h"
#include "Utils.h"

using namespace Arcetri;





/*
 * Define the format of a BCU packet and the available
 * operations on it.
 * 
 * A Bcu packet is composed by an header and a body.
 * The header is always allocated by constructor, and deallocated by destructor.
 * The body is used to send or receive data, and is simply a reference to a memory 
 * location allocated by someone else, i.e. a MirrorCtrl client. 
 * 
 * 
 */
class BcuPacket {
	
	public:
		
		// Udp Header and body max size in bytes
		static const int HEADER_SIZE = Bcu::Constants::PACKET_HEADER_SIZE;
		static const int MAX_BODY_SIZE = Bcu::Constants::MAX_PACKET_SIZE;
		
	
		// Flags
		static const uint8 WANTREPLY_FLAG 	= Bcu::Constants::WANTREPLY_FLAG;
		static const uint8 ASQUADWORD_FLAG 	= Bcu::Constants::ASQUADWORD_FLAG;
	
		/*
		 * Create an UdpPacket with an empty header and null data.
		 * Call set(...) to correctly initialize the packet.
		 */
		BcuPacket();
		
		/*
		 * Create an UdpPacket with initialized header and data.
		 * The field "data" must be inizialized whith the address of a 
		 * previously allocate memory, both for sending and receiving.
		 */
		BcuPacket(uint8 msgId,
				  uint8 firstDsp, 
				  uint8 lastDsp, 
				  uint8 opCode, 
				  int memoryAddress,
				  uint8 flags,
				  uint16 dataLengthDw,
				  BYTE* data,					// Can be NULL if both those conditions are true:
				  								//	- Flags doesn't include WANTREPLY_FLAG 
				  								//  - dataLength = 0
				  BcuRequestInfo* reqInfo);		// Can be NULL if flags doesn't include WANTREPLY_FLAG
		
		/*
		 * Correctly in-depth destroy the object.
		 * Obviously doesn't destroy body data !
		 */
		virtual ~BcuPacket();
		
		/*
		 * Completely set the object
		 */
		void set(uint8 msgId,
        		 uint8 firstDsp, 
        		 uint8 lastDsp, 
        		 uint8 opCode,  
        		 int memoryAddress,
        		 uint8 flags, 
                 uint16 dataLengthDw,
                 BYTE* data,
                 BcuRequestInfo* reqInfo);
       
                 
        // --- Packet checkers --- //
        // Delegate to BcuCommon library
        
        bool isWrite() 	 { return Bcu::OpCodes::isWrite(getOpCode()); }						 
		bool isRead() 	 { return Bcu::OpCodes::isRead(getOpCode()); }					
		bool isSuccess() { return Bcu::OpCodes::isSuccess(getOpCode()); }
		bool isFault() { return Bcu::OpCodes::isFault(getOpCode()); }
		bool isCATimeout() { return Bcu::OpCodes::isCATimeout(getOpCode()); }
				
		// --- Packet fields getters --- //
		
		uint8  getFirstDSP()        { return (_hdr->DSPs[0]>>4) | (_hdr->DSPs[1] & 0x0F)<<4; }
		uint8  getLastDSP()         { return _hdr->DSPs[2]; }
		uint8  getOpCode()          { return _hdr->opCode; }
		uint16 getDataLengthDw()    { return _hdr->dataLength; }
		int    getDataLengthBytes() { return _hdr->dataLength*Bcu::Constants::DWORD_SIZE; }
		int    getMemoryAddress()   { return _hdr->memoryAddress[0] |
			                                   _hdr->memoryAddress[1]<<8 |
			                                   _hdr->memoryAddress[2]<<16 |
			                                   _hdr->memoryAddress[3]<<24; }
		uint8  getMsgId()           { return _hdr->msgId; }
		uint8  getFlags()			{ return _hdr->flags; }
		uint8  getFlagWantreply()   { return _hdr->flags & WANTREPLY_FLAG; }
		uint8  getFlagAsquadword()	{ return _hdr->flags & ASQUADWORD_FLAG; }
		
		// --- Special getters doing computation --- //
		
		/*
		 * Takes care about the kind of opCode: if is a *READ_SEQ*
		 * the packet data length must be multiplied for the DSP number
		 */
		int getFullDataLength();
		int getFullDataLengthInBytes();
		
		// --- Packet fields setters --- //
		
		void setFirstDSP(uint8 v)		{ _hdr->DSPs[0] = (v&0x0F)<<4; _hdr->DSPs[1] = (v&0xF0)>>4; }
		void setLastDSP(uint8 v)        { _hdr->DSPs[2] = (v&0xFF); }
		void setOpCode(uint8 v)        	{ _hdr->opCode = v; }
		void setDataLengthDw(uint16 v)  { _hdr->dataLength = v; }	// in DWORD !!!
		void setMemoryAddress(int v)	{ _hdr->memoryAddress[0] = v&0xFF; 
										  _hdr->memoryAddress[1] = (v>>8)&0xFF; 
										  _hdr->memoryAddress[2] = (v>>16)&0xFF; 
										  _hdr->memoryAddress[3] = (v>>24)&0xFF; }
		void setMsgID(uint8 v)          { _hdr->msgId = v; }
		void setFlags(uint8 flags)		{ _hdr->flags = flags; }
		void setFlagWantreply(uint8 v) 	{ (_hdr->flags |= (v) ? WANTREPLY_FLAG : 0x00); 
										  (_hdr->flags &= (v) ? 0xFF : ~WANTREPLY_FLAG); }
		void setFlagAsquadword(uint8 v)	{ (_hdr->flags |= (v) ? ASQUADWORD_FLAG : 0x00); 
										  (_hdr->flags &= (v) ? 0xFF : ~ASQUADWORD_FLAG); }
				
				
		/*
		 * Set the info about the request to wich the packet belongs
		 */
		void setRequestInfo(BcuRequestInfo* requestInfo) { _requestInfo = requestInfo; }	
						  
					
		/*
		 * Set the reference to body data. Note that dataLen is in DWORD !!!
		 */				   
		void setData(BYTE* data, int dataLen) { _data = data; setDataLengthDw(dataLen); }	
		
		/*
		 * Return the reference to body data
		 */
		BYTE* getData() { return _data; }	   						
										   
		/*
		 * Send an UDP packet using a given UdpConnection
		 */
        void send(UdpConnection* conn) throw (UdpFatalException);
        
        /*
         * Receive an UDP packet, blocking for a fixed timeout.
         * 
         * Attention: a call to setData(data, datalen) with a not-NULL "data" reference
         * to a memory buffer of size "datalen" must be done before !
         */
        void receive(UdpConnection* conn, bool checkId = false) throw (UdpFatalException, UdpTimeoutException, UnexpectedBcuPacketException); 
        
        /*
 		 * Receive an UDP packet from a given buffer: in this case, used to receive
 		 * an unexpected packet, we don't need to check for the correct msg id (have
 		 * been implicitely done by RoundQueue::recoverUnexpected(...) method calling 
 		 * this)
		 */
		void receive(BYTE* rawPacket);
        
      	/*
      	 * Notify when the reply is got
      	 */
      	 void notifyReply(int bcuId);
      	 
        /*
         * Facility methods to print the full status of the object
         */
        void printStatus(bool isSend, int replyOpCode = -1); 
        
		
	private:
		
		// Reference to a contiguous memory location used as send/receive buffer.
		// Is allocated  and deallocate by send/receive methods (eventually
		// by the receive(BYTE* rawPacket) which handle tehe receiving of an 
		// unexpectedPacket)
		BYTE*		_rawPacket;
		
		// Meaningfull parts of the _rawPacket
		UdpHeader* 	_hdr;
		BYTE*		_data;	// The size of this buffer is defined in _hdr->dataLength
		
		// A reference to the corrisponding BcuRequestInfo (used to notify when reply got)
		BcuRequestInfo* _requestInfo;
		
	private:	// DISABLE DEFAULT COPY CONSTRUCTOR AND ASSIGNMENT, because
				// they won't work. Please redefine them if you need them !
	
		BcuPacket(const BcuPacket& packet);
		BcuPacket operator=(const BcuPacket& packet);
};


#endif /*UDPPACKET__H_INCLUDE*/
