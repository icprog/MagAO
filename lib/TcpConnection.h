#ifndef TCPCONNECTION_H_INCLUDE
#define TCPCONNECTION_H_INCLUDE

#include "Logger.h"
#include "aotypes.h"

#include <string>


namespace Arcetri {
	
/*
 * Generic exception for the TcpConnection
 */
class TcpException: public AOException {
	
	public:
		TcpException(string msg, int errCode = TCP_GENERIC_ERROR, string file = "", int line = 0): AOException(msg, errCode, file, line) { exception_id = "TcpException"; }
		virtual ~TcpException() throw() {}
};

/*
 * Fatal exception for the TcpConnection
 */
class TcpCreationException: public TcpException {
	
	public:
		explicit TcpCreationException(string msg, string file = "", int line = 0): TcpException(msg, TCP_CREATION_ERROR, file, line) { exception_id = "TcpCreationException"; }
		virtual ~TcpCreationException() throw() {}
};

/*
 * Send exception for the TcpConnection
 */
class TcpSendException: public TcpException {
	
	public:
		explicit TcpSendException(string msg, string file = "", int line = 0): TcpException(msg, TCP_SEND_ERROR, file, line) { exception_id = "TcpSendException"; }
		virtual ~TcpSendException() throw() {}
};

/*
 * Receive error exception for the TcpConnection. 
 */
class TcpReceiveException: public TcpException {
	
	public:
		explicit TcpReceiveException(string m, string file = "", int line = 0): TcpException(m, TCP_RECEIVE_ERROR, file, line) { exception_id = "TcpReceiveException"; }
		virtual ~TcpReceiveException() throw() {}
};

/*
 * Receive timeout exception for the TcpConnection. 
 */
class TcpTimeoutException: public TcpException {
	
	public:
		explicit TcpTimeoutException(string file = "", int line = 0): TcpException("Timeout expired", TCP_RECEIVE_TIMEOUT, file, line) { exception_id = "TcpTimeoutException"; }
		virtual ~TcpTimeoutException() throw() {}
};

	
	
/*
 * This class provides a Tcp connection with a remote host
 */
class TcpConnection {
	
	public:
	
		/*
		 * Creates a Tcp connection
		 * 
		 * Preconditions:
		 * 	- 'remoteHost' must be an host name defined in /etc/hosts, or a valid IPV4/IPV6 address
		 * 	- 'remoteHost' must refer to a reachable host
		 *  - 'remotePort' must be a valid TCP port on the remote host
		 * 
		 * otherwise a TcpFatalException is thrown
		 * 
		 * Postconditions:
		 * 	- a valid TcpConnection is created, available to call sendTcp and receiveTcp
		 */
		TcpConnection(string remoteHost, 
					  int remotePort,
					  int logLevel = Logger::LOG_LEV_WARNING) throw(TcpCreationException);
		
		virtual ~TcpConnection();
		
		/*
		 * Send data using the TCP connection
		 * 
		 * Preconditions:
		 * 	- The buffer is not NULL
		 * 	- The buffer len is not zero
		 * 
		 * otherwise a TcpSendException is thrown
		 * 
		 * Postconditions:
		 * 	- 'bufferlen' data has been succesfully sent
		 */
		void sendTcp(BYTE* buffer, int bufferlen) throw (TcpSendException);
		
		/*
		 * Receive data from TCP connection, and returns the
		 * number of bytes received.
		 * 
		 * If timeout_ms is zero, waits forever ??????
		 * 
		 * Preconditions:
		 * 	- The buffer is not NULL
		 *  
		 * otherwise a TcpReceiveException is thrown
		 * 
		 * Postconditions:
		 * 	- The receivebuffer contains the received data (size returned by function)
		 */
		int receiveTcp(BYTE* receivebuffer, int bufferlen, int  timeout_ms) throw (TcpReceiveException, TcpTimeoutException);
		
	private:
	
		int _tcpSocket;
	
		Logger* _logger;
};

}

#endif /*TCPCONNECTION_H_INCLUDE*/
