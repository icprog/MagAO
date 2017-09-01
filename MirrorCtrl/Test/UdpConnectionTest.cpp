#include <stdio.h>
#include <stdlib.h>
#include <string>
using namespace std;

#include "UdpConnection.h"
#include "aotypes.h"

int main() {
	
	printf("--- TESTING UDP CONNECTION ---\n");
	
	UdpConnection::setVerbosity(Logger::LOG_LEV_TRACE);
	UdpConnection conn(10010, "127.0.0.1", 10010, 10);
	conn.send((BYTE*)"A PACKET", 9);
	
	
	BYTE bufferIn[9];
	conn.receive(bufferIn, 9);
	printf("Data received: %s\n", bufferIn);

}
