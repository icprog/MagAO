#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>

#include "UdpConnection.h"


extern "C" {
#include "base/timelib.h"
}

using namespace Arcetri;


Logger* UdpConnection::_logger = NULL;

UdpConnection::UdpConnection(int localPort, string remoteHost, int remotePort, int receiveTimeout_ms, int logLevel) throw (UdpFatalException) {

	//*** TEST TO HAVE A DELAY ISTOGRAM - REMOVE ***//
	// int step_ms = 1;
	// _delayIstogram = new DelayIstogram(receiveTimeout_ms, step_ms);
	//**********************************************//

	_logger = Logger::get("UDPCONNECTION", logLevel);

	// Retrieve 'hostent' struct from hostname (IPV4-dot, IPV6-dot or hostname)
	struct hostent* hostInfo = gethostbyname(remoteHost.c_str());
	if(hostInfo == NULL) {
		throw UdpFatalException("Impossible to resolve IPV4 address for host " + remoteHost);
	}
	struct in_addr* addressStruct = (struct in_addr*)hostInfo->h_addr;
	
	// Save connection info for send/receive
	_localPort = localPort;
	_remoteIpAsString = string(inet_ntoa(*addressStruct));
	_remoteIp = addressStruct->s_addr;
	
	if(_remoteIp == INADDR_NONE) {
		throw UdpFatalException("Impossible to resolve ip (int) for host" + remoteHost);
	}
	_remotePort = remotePort;
	_receiveTimeout_ms = receiveTimeout_ms;

	// Create the socket
	_logger->log(Logger::LOG_LEV_DEBUG, "UdpConnection: creating the socket for remote Ip %s...", _remoteIpAsString.c_str());
	_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (_socket == -1) {
		_logger->log(Logger::LOG_LEV_WARNING, "UdpConnection: error in socket(...)");
		if(errno == EACCES) {
			_logger->log(Logger::LOG_LEV_WARNING, " ->Error detail: EACCES error (%d)", errno);
		}
		else {
			_logger->log(Logger::LOG_LEV_WARNING, " ->Error detail: OTHER error (%d)", errno);
		}
		throw UdpFatalException("Error creating UdpConnection (NETWORK_ERROR)");
	}
	
	// Setting up the local binding for the socket
	socklen_t addrLen;
	struct sockaddr_in localAddr;
	localAddr.sin_family = AF_INET;
	localAddr.sin_port = htons((unsigned short)localPort);
	localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	addrLen = sizeof(localAddr);
	_logger->log(Logger::LOG_LEV_DEBUG, "UdpConnection: setting up socket on localhost, port %d", localPort);

	// Bind the socket
	if(bind(_socket, (struct sockaddr*)&localAddr, addrLen) == -1) {
		_logger->log(Logger::LOG_LEV_WARNING, "UdpConnection: error in bind(...)");
		throw UdpFatalException("Error creating UdpConnection (NETWORK_ERROR)");
	}

	// ???
	fcntl(_socket, F_SETFL, O_NONBLOCK);
	_logger->log(Logger::LOG_LEV_DEBUG, "UdpConnection: fnctl O_NONBLOCK");
	_logger->log(Logger::LOG_LEV_DEBUG, "UdpConnection: socket created");
	
	_logger->log(Logger::LOG_LEV_DEBUG, "UdpConnection: object created");
}


UdpConnection::~UdpConnection() {
	close(_socket);
	//*** TEST TO HAVE A DELAY ISTOGRAM - REMOVE ***//
	//_delayIstogram->print();
	//***********************************************//
	_logger->log(Logger::LOG_LEV_DEBUG, "UdpConnection: socket closed and object destroyed");
}


void UdpConnection::send(BYTE* buffer,int bufferlen) throw (UdpFatalException) {

	// Setting up the remote binding for the socket
	struct sockaddr_in remoteAddr;
	remoteAddr.sin_family=AF_INET;
	remoteAddr.sin_port=htons( (unsigned short)_remotePort);
	remoteAddr.sin_addr.s_addr = _remoteIp;

	//_logger->log(Logger::LOG_LEV_DEBUG, "Sending packet to %s:%d, %d bytes", inet_ntoa(remoteAddr.sin_addr), _remotePort, bufferlen);

	// Send
   errno = 0;
	int rc = sendto(_socket, buffer, bufferlen, 0, (struct sockaddr*)&remoteAddr, sizeof(remoteAddr));
  	if (rc < 0) {
		_logger->log(Logger::LOG_LEV_DEBUG, "UdpConnection: error in sendto(...)");
      _logger->log(Logger::LOG_LEV_DEBUG, "UdpConnection: errno = %s", strerror(errno));
		throw UdpFatalException("Error sending data (NETWORK_ERROR)");
	}

	//_logger->log(Logger::LOG_LEV_DEBUG, "Packet sent !!!");
}
	
		
void UdpConnection::receive(BYTE* receivebuffer, int bufferlen) throw (UdpFatalException, UdpTimeoutException) {
	
	// Use a select() call to have a reliable timeout
	fd_set set;
	FD_ZERO(&set);
	FD_SET( (unsigned int)_socket, &set);

	struct timeval tv;
	tv.tv_sec = 0; 	//_receiveTimeout_ms/1000;
	tv.tv_usec = _receiveTimeout_ms*1000;

	//_logger->log(Logger::LOG_LEV_DEBUG, "Receive timeout = %d", _receiveTimeout_ms);

	// Do select
	//*** TEST TO HAVE A DELAY ISTOGRAM - REMOVE ***//
//	gettimeofday(&_startReceive, NULL);
	//**********************************************//
	
	int ret = select(_socket+1, &set, NULL, NULL, (_receiveTimeout_ms==0) ? NULL : &tv);
	
	//*** TEST TO HAVE A DELAY ISTOGRAM - REMOVE ***//
//	gettimeofday(&_endReceive, NULL);
//	_delayIstogram->setValue(((_endReceive.tv_usec-_startReceive.tv_usec) + (_endReceive.tv_sec - _startReceive.tv_sec)*1000000));
	//**********************************************//
	
	if (!ret) {
		//_logger->log(Logger::LOG_LEV_INFO, "Receive TIMEOUT: %d ms", ((_endReceive.tv_usec-_startReceive.tv_usec) + (_endReceive.tv_sec - _startReceive.tv_sec)*1000000)/1000);
		throw UdpTimeoutException("UdpConnection: timeout receiving data (TIMEOUT_ERROR)");
	}
	else if (ret < 0) {
		_logger->log(Logger::LOG_LEV_WARNING, "UdpConnection: error in select(...)");
		throw UdpFatalException("Error receiving data");
	}

	// Loop trapping EAGAIN errors
	for (int i=0; i<EAGAIN_LOOP; i++) {
		if ((ret = recv(_socket, receivebuffer, bufferlen, 0)) >= 0) {
			//_logger->log(Logger::LOG_LEV_DEBUG, "Packet received, %d bytes", err);
			return;	// Receiving OK
		}
		// If we got an error, loop again in case of an EAGAIN
		else {
			if(errno == EAGAIN) {
                _logger->log(Logger::LOG_LEV_WARNING, "UdpConnection: received %d bytes instead of %d", ret, bufferlen);
                _logger->log(Logger::LOG_LEV_WARNING, "UdpConnection: recovering from EAGAIN error in recvfrom() - iteration %d", i);
                nusleep(20*1000); 
			}
		}
	}

	_logger->log(Logger::LOG_LEV_WARNING, "UdpConnection: EAGAIN repetead %d times, exiting", EAGAIN_LOOP);
	throw UdpFatalException("Error receiving data (NETWORK_ERROR)");
}




/*** DELAY-ISTOGRAM CLASS ***/

DelayIstogram::DelayIstogram(int timeout_ms, int step_ms)
{
	_logger = Logger::get("UDPCONNECTION");
	
	_extraDelay = 0;
	
	_timeout_ms = timeout_ms;
	_size = (timeout_ms/step_ms)+1; // The last position host all timouts > timeout_ms
	_data = new int[_size];
	for(int i=0; i<_size; i++) {
		_data[i] = 0;
	}
}

void DelayIstogram::setValue(float value_us) {

	int delay_ms = (int)(value_us/1000.); // Convert to ms and round down
	if(delay_ms>=(_timeout_ms+1)) {
		_extraDelay++;
	}
	else {
		_data[delay_ms]++;
	}
}

void DelayIstogram::print() {
	_logger->log(Logger::LOG_LEV_INFO, "- Packets delay distribution -");
	_logger->log(Logger::LOG_LEV_INFO, "------------------------------");
	_logger->log(Logger::LOG_LEV_INFO, "    ms           packets      ");
	_logger->log(Logger::LOG_LEV_INFO, "------------------------------");
	int total = 0;
	for(int i=0; i<_size; i++) {
		if(_data[i] != 0) {
			_logger->log(Logger::LOG_LEV_INFO, "%.5d            %d", i, _data[i]);
			total += _data[i];
		}
	}
	_logger->log(Logger::LOG_LEV_INFO, "Timed-out        %d", _extraDelay);
	total += _extraDelay;
	_logger->log(Logger::LOG_LEV_INFO, "------------------------------");
	_logger->log(Logger::LOG_LEV_INFO, "Total		%d", total);
}
