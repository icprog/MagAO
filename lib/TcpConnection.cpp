#include "TcpConnection.h"
#include "Utils.h"

#include <sys/time.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <fcntl.h>
#include <netdb.h>

using namespace Arcetri;

TcpConnection::TcpConnection(string remoteHost, int remotePort, int logLevel) throw (TcpCreationException) {

	_logger = Logger::get("TCPCONNECTION", logLevel);

	// Retrieve 'hostent' struct from hostname (IPV4-dot, IPV6-dot or hostname)
	struct hostent* hostInfo = gethostbyname(remoteHost.c_str());
	if(hostInfo == NULL) {
		throw TcpCreationException("Impossible to resolve ip address for host " + remoteHost + " (1)", __FILE__, __LINE__);
	}
	struct in_addr* addressStruct = (struct in_addr*)hostInfo->h_addr;
	if(addressStruct->s_addr == INADDR_NONE) {
		throw TcpCreationException("Impossible to resolve ip address) for host" + remoteHost + " (2)", __FILE__, __LINE__);
	}

	_logger->log(Logger::LOG_LEV_INFO, "Creating TcpConnection to %s:%d", (string(inet_ntoa(*addressStruct))).c_str(), remotePort);

	_tcpSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	//fcntl(_tcpSocket, F_SETFL, O_NONBLOCK);

   	struct sockaddr_in serverAddr;
   	serverAddr.sin_family      = AF_INET;             	 /* Internet address family */
	serverAddr.sin_addr.s_addr = addressStruct->s_addr;  /* Server IP address or hostname */
	serverAddr.sin_port        = htons(remotePort); 	 /* Server port */

  	if(connect(_tcpSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
  		_logger->log(Logger::LOG_LEV_ERROR, "TcpConnection creation failed");
  		close(_tcpSocket);
  		throw TcpCreationException("Impossible to connect to " + string(inet_ntoa(*addressStruct)) + ":" + Utils::itoa(remotePort), __FILE__, __LINE__);
  	}
  	else {
  		_logger->log(Logger::LOG_LEV_INFO, "TcpConnection succesfully created!");
  	}

//
//   	send() // Urgent data: MSG_OOB

}

TcpConnection::~TcpConnection() {
	close(_tcpSocket);
	_logger->log(Logger::LOG_LEV_DEBUG, "TcpConnection: socket closed and object destroyed");
}

void TcpConnection::sendTcp(BYTE* buffer,int bufferlen) throw (TcpSendException) {

	// Check preconditions
	if(buffer == NULL) {
		throw TcpSendException("Error sending data (NULL buffer)", __FILE__, __LINE__);
	}
	if(bufferlen == 0) {
		throw TcpSendException("Error sending data (0 buffer lenght)", __FILE__, __LINE__);
	}

	_logger->log(Logger::LOG_LEV_DEBUG, "TcpConnection: send %d bytes", bufferlen);
	if(send(_tcpSocket, buffer, bufferlen, 0) != bufferlen) {
		_logger->log(Logger::LOG_LEV_DEBUG, "TcpConnection: error in send(...)");
		throw TcpSendException("Error sending data (NETWORK_ERROR)", __FILE__, __LINE__);
	}
	else {
		// Extra logging
		if(_logger->getLevel() >= Logger::LOG_LEV_TRACE) {
			_logger->log(Logger::LOG_LEV_TRACE, "Sent data (%d bytes):", bufferlen);
			//printf("TCP Send: ");
			//for(int b=0; b<bufferlen; b++) {
			//	printf("%2X ", buffer[b]);
			//}
			//printf("\n");
		}
	}
}

int TcpConnection::receiveTcp(BYTE* receivebuffer, int bufferlen, int timeout_ms) throw (TcpReceiveException, TcpTimeoutException) {

	// Check preconditions
	if(receivebuffer == NULL) {
		throw TcpReceiveException("Error receiving data (NULL buffer)", __FILE__, __LINE__);
	}

	// Use a select() call to have a reliable timeout
	fd_set set;
	FD_ZERO(&set);
	FD_SET((unsigned int)_tcpSocket, &set);

	struct timeval tv;
	tv.tv_sec = 0; 	//_receiveTimeout_ms/1000;
	tv.tv_usec = timeout_ms*1000;


	int ret = select(_tcpSocket+1, &set, NULL, NULL, (timeout_ms==0) ? NULL : &tv);

	if (!ret) {
		_logger->log(Logger::LOG_LEV_WARNING, "TcpConnection: timeout %d ms expired", timeout_ms);
		throw TcpTimeoutException(__FILE__, __LINE__);
	}
	else if (ret < 0) {
		_logger->log(Logger::LOG_LEV_WARNING, "TcpConnection: error in select(...)");
		throw TcpReceiveException("Error receiving data", __FILE__, __LINE__);
	}

	int recvdBytes;
	if ((recvdBytes = recv(_tcpSocket, receivebuffer, bufferlen, 0)) > 0) {
		// Extra logging
		if(_logger->getLevel() >= Logger::LOG_LEV_TRACE) {
			_logger->log(Logger::LOG_LEV_TRACE, "TcpConnection: received data (%d bytes):", recvdBytes);
			//printf("TCP Recv: ");
			//for(int b=0; b<recvdBytes; b++) {
			//	printf("%2X ", receivebuffer[b]);
			//}
			//printf("\n");
		}
	}
	// If we got an error
	else {
      close(_tcpSocket);
		throw TcpReceiveException("Error receiving data (errno=" + Utils::itoa(errno)+ " ), socket closed.", __FILE__, __LINE__);
	}

	return recvdBytes;	// Receiving OK
}

