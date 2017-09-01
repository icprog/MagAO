#ifndef DIAGNOSTICMANAGERESCEPTIONS_H_
#define DIAGNOSTICMANAGERESCEPTIONS_H_

#include "AOExcept.h"
#include "Utils.h"
using namespace Arcetri;

/*
 * Root exception
 */
class MasterDiagnosticException: public AOException {

	public:
		explicit MasterDiagnosticException(string id, string message): AOException(message) {
			exception_id = id;
		}
		
		virtual ~MasterDiagnosticException() {}
};

/*
 * Base exception for the FramesDownloader
 */
class FramesDownloaderException: public MasterDiagnosticException {

	public:
		explicit FramesDownloaderException(string id, string message): MasterDiagnosticException(id, message) {}
		virtual ~FramesDownloaderException() {}
};

class InvalidDiagnosticPacketException: public FramesDownloaderException {
	public:
		explicit InvalidDiagnosticPacketException(int expectedFrameId, 
		                      					  int currentFrameId, 
		                      					  int expectedPacketId, 
		                      					  int currentPacketId):
		         FramesDownloaderException("InvalidDiagnosticPacketException", 
		                                   "Unexpected packetId " + Utils::itoa(currentPacketId) + " for frameId " + Utils::itoa(currentFrameId) + " (instead of packetId " + Utils::itoa(expectedPacketId) + " for frameId " + Utils::itoa(expectedFrameId) + ")") {}
		virtual ~InvalidDiagnosticPacketException() {}
};

class InvalidDiagnosticFrameException: public FramesDownloaderException {
	public:
		explicit InvalidDiagnosticFrameException():FramesDownloaderException("InvalidDiagnosticFrameException", "Header and footer don't match") {}
		virtual ~InvalidDiagnosticFrameException() {}
};

class BufferSlotsOutOfBoundsException: public FramesDownloaderException {
	public: 
		explicit BufferSlotsOutOfBoundsException(string bcuName): FramesDownloaderException("BufferSlotsOutOfBoundsException", "FramesDownloader " + bcuName + ": frame slot exceeding buffer capacity") {};
		virtual ~BufferSlotsOutOfBoundsException() {};
};

class NotInizializedBufferException: public FramesDownloaderException {
	public: 
		explicit NotInizializedBufferException(string bcuName): FramesDownloaderException("NotInizializedBufferException", "FramesDownloader " + bcuName + ": Buffer isn't initialized (0 slots)") {};
		virtual ~NotInizializedBufferException() {};
};


/*
 * Base exception for the FramesFunnel
 */
class FramesFunnelException: public MasterDiagnosticException {

	public:
		explicit FramesFunnelException(string id, string message): MasterDiagnosticException(id, message) {}
		virtual ~FramesFunnelException() {}
};

/*
 * Base exception for the FramesBouncer
 */
class FramesBouncerException: public MasterDiagnosticException {

	public:
		explicit FramesBouncerException(string id, string message): MasterDiagnosticException(id, message) {}
		virtual ~FramesBouncerException() {}
};


#endif /*DIAGNOSTICMANAGERESCEPTIONS_H_*/
