#ifndef UDPCONNECTION_H_INCLUDE
#define UDPCONNECTION_H_INCLUDE

#include <string>
#include <stdio.h>
using namespace std;

#include "AOExcept.h"
#include "aotypes.h"
#include "Logger.h"
using namespace Arcetri;

namespace Arcetri {
	
	
/*
 * Generic exception for the UdpConnection
 */
class UdpException: public AOException {
	
	public:
		UdpException(string m): AOException(m) { exception_id = "UdpException"; }
		virtual ~UdpException() {}
};

/*
 * Fatal exception for the UdpConnection.
 */
class UdpFatalException: public UdpException {
	
	public:
		explicit UdpFatalException(string m): UdpException(m) { exception_id = "UdpFatalException"; }
		virtual ~UdpFatalException() throw() {}
};

/*
 * Thrown when a receive timeout happends.
 * Usually this is not a fatal exception
 */
class UdpTimeoutException: public UdpException {
	
	public:
		explicit UdpTimeoutException(string m): UdpException(m) { exception_id = "UdpTimeoutException"; }
		virtual ~UdpTimeoutException() throw() {}
};

/*
 * A funny class to create an istogram af receiving delay
 */ 
class DelayIstogram {
	
	public:
		// Please set step_ms as a submultiple of timeout_ms
		DelayIstogram(int timeout_ms, int step_ms);
		~DelayIstogram() {};
		
		void setValue(float value_us);
		
		void print();
	
	private:
		
		int _timeout_ms;
		int _size;
		int* _data;
		int _extraDelay;
		
		Logger* _logger;
	
};


/*
 * A connection to send and receive data using UDP protocol
 * This is the thread-safe version of the commlib low-level part.
 * 
 * An UDP connection is defined by a local port, a remote ip address and
 * a remote port.
 */
class UdpConnection {
	
	public:
	
		/*
		 * Create an UDP connection beetwen a local port and a remote address:port 
		 * The remoteHost can be: IPV4-dot, IPV6-dot, host name.
		 */
		UdpConnection(int localPort, 
					  string remoteHost, 
					  int remotePort, 
					  int receiveTimeout_ms,
					  int logLevel = Logger::LOG_LEV_WARNING)  // Zero means receive will wait indefinitely
					  throw (UdpFatalException);
		
		virtual ~UdpConnection();
		
		/*
		 * Get the receive timeout
		 */
		 int getReceiveTimeout_ms() { return _receiveTimeout_ms; }
		
		/*
		 * Change the receive timeout
		 */
		 void setReceiveTimeout_ms(int receiveTimeout_ms) { _receiveTimeout_ms = receiveTimeout_ms; }
		
		/*
		 * Some facility getters
		 */
		 int 	getLocalPort() { return _localPort; }
		 string getRemoteIpAsString() { return _remoteIpAsString; }
		 int	getRemotePort() { return _remotePort; }
		 
		
		/*
		 * Send a buffer of data to remote address:port.
		 * 
		 * NOTE: The client has the full responsibility of creating/destropying 
		 * the buffer !!!
		 */
		void send(BYTE* buffer,			// Pointer to buffer of data to send
	       		  int bufferlen)		// Buffer length in BYTES
	       		  throw (UdpFatalException);
		
		/*
		 * Read buffer of data from remote address:port.
		 * 
		 * [to check] This implementation with EAGAIN should be checked.
		 */
		void receive(BYTE* receiveBuffer, 	// Allocated buffer
					 int bufferlen)			// Buffer length in BYTES
	       		 	 throw (UdpFatalException, UdpTimeoutException);
	       		 	
	    /*
	     * Return the verbosity of the library (the level of its logger)
	     */
		static int getVerbosity() { return _logger->getLevel(); } ;
		
		/*
		 * Set the verbosity of the library (the level of its logger)
		 */
		static void setVerbosity(int level) { _logger->setLevel(level); }
		
	private:
	
		// These 3 field completely define the connection
		int 			_socket;
		unsigned int 	_remoteIp;
		int				_remotePort;
		
		int				_receiveTimeout_ms;
		
		// Fields saved only to print info
		string			_remoteIpAsString;	
		int 			_localPort;			
		
		//Repetitions of recv(...) in case of EAGAIN error
		static const int EAGAIN_LOOP = 5;
		
		static Logger* _logger;
		
		
		//*** TEST TO HAVE A DELAY ISTOGRAM - REMOVE ***//
		struct timeval _startReceive, _endReceive;
		DelayIstogram* _delayIstogram;
		//**********************************************//
	
};


} // End namespace Arcetri

#endif /*UDPCONNECTION_H_INCLUDE*/
