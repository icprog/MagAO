//+File: idl_commlib.c
//
// Windows DLL interface for BCU routines
//-

#include <windows.h>
#include <stdio.h>
#include <memory.h>

#include "commlib.h"
#include "errlib.h"
#include "export.h"

// Error codes
#ifndef NO_ERROR
#define		NO_ERROR		(0L)
#endif /* NO_ERROR */

#define		PAR_NUM_ERR		(-1L)

// IDL function prototypes.

BOOL WINAPI DllMain(HINSTANCE hInst, ULONG ulReason, LPVOID lpReserved);
LONG WINAPI test(LONG lArgc, LPVOID lpvArgv);
DWORD WINAPI receiveThreadFunc( LPVOID parameter);

LONG WINAPI idl_function( LONG lArgc, LPVOID lpvArgv);
LONG WINAPI idl_setup(LONG lArgc, LPVOID lpvArgv);
LONG WINAPI idl_WriteToBCU( LONG lArgc, LPVOID lpvArgv);
LONG WINAPI idl_ReadFromBCU( LONG lArgc, LPVOID lpvArgv);
LONG WINAPI idl_GetPacketNum( LONG lArgc, LPVOID lpvArgv);

void DumpBCUcommand( char *dump, BCUcommand *cmd);

// Global data

unsigned char *idl_receivebuffer = NULL;
unsigned char idl_firstID = 0;
unsigned char idl_counter=0;
int idl_threadstatus = NO_ERROR;

HANDLE threadHandle;


//+Function: DllMain
//
// Windows DLL entry point.
//
// Automatically called when the DLL is first loaded
//-

BOOL WINAPI DllMain(HINSTANCE hInst, ULONG ulReason, LPVOID lpReserved)
{
	if (ulReason == DLL_PROCESS_ATTACH)
		idl_setup(0, NULL);

	return(TRUE);
}

//+Function: setup
//
// Setup windows sockets
//-

LONG WINAPI idl_setup(LONG lArgc, LPVOID lpvArgv)
{
	WORD wVersionRequested;
	WSADATA wsaData;	
	int err;
	int stack_size = 4000;
	int flags=0;
	int threadID =0;
 
	wVersionRequested = MAKEWORD( 1, 0 );
 
	err = WSAStartup( wVersionRequested, &wsaData );
	if ( err != 0)
		return NETWORK_ERROR;


	threadHandle = CreateThread(
		NULL,	        		// Security attributes
		stack_size,
		receiveThreadFunc,
		(LPVOID)BCU_PORT,
		flags,
		&threadID);

	if (threadHandle == NULL)
		return THREAD_CREATION_ERROR;
	else
		return NO_ERROR;
}
	
	
	
//+Function: receiveThreadFunc
//
// Function to receive messages from the BCU
//
// Separate thread waiting for messages from the BCU
//-
DWORD WINAPI receiveThreadFunc( LPVOID parameter)
{
	int err, socket = 0;
	char *packetbuffer;
	unsigned char number, msgID;
	BCUcommand *BCUcmd;

	int port = (int)parameter;

	packetbuffer = (char *)malloc(MAX_UDP_PACKET_LENGTH);
	if (!packetbuffer)
		return MEMORY_ALLOC_ERROR;

	// Infinite loop waiting...

	while(1)
		{
		err = WaitUDPpacket( BCU_LOCAL_PORT, packetbuffer, MAX_UDP_PACKET_LENGTH, &socket, 0, NULL);

		// Even if there are errors, there's not much that we can do
		if (err <0)
			{
			idl_threadstatus = err;
			Sleep(100);
			continue;
			}

		idl_threadstatus = NO_ERROR;

		BCUcmd = (BCUcommand *) packetbuffer;
		msgID= GetMsgID(*BCUcmd);
		number = msgID - idl_firstID;

		idl_counter--;
		if (idl_receivebuffer)
			{
			memcpy( idl_receivebuffer + number*MAX_DATA_TRANSFER*4, packetbuffer + sizeof(BCUcommand), GetDataLength(*BCUcmd)*4);
			}
		}

	free(packetbuffer);

}


//+Function: idl_function
//
// Sample function to show how arguments are handled
//-

LONG WINAPI idl_function(LONG lArgc, LPVOID lpvArgv)
{
	long			**lplpArgv;
	int				foo,bar;
	unsigned char   *buffer;

	// lArgc is the number of arguments
	if (lArgc !=3 ) return(PAR_NUM_ERR);

	lplpArgv = (long **)lpvArgv;

	//lplpArgv is now an array of pointers
	foo = (int)(*lplpArgv[0]);
	bar = (int)(*lplpArgv[1]);
	buffer = (unsigned char *)(lplpArgv[2]);


	*lplpArgv[0] = (long)foo;
	*lplpArgv[1] = (long)bar;
	*lplpArgv[2] = (long)buffer;
	
	return(NO_ERROR);
}

//+Function: test
//
// Sample function to test the correct DLL loading
//-

LONG WINAPI test(LONG lArgc, LPVOID lpvArgv)
{
	return 0L;
}

 


//+Function: idl_ReadFromBCU
//
// Wrapper around the idl_ReadFromBCU commlib function
//
// Calling syntax from IDL:
//
// result = CALL_EXTERNAL("idl_commlib.dll", "idl_ReadFromBCU", 
//                         remote_IP_addr, (string)
//                         firstDSP,       (integer)
//                         lastDSP,        (integer)
//                         opcode,         (integer)
//                         addressInBCU    (unsigned integer)
//                         datalen,        (integer)
//                         firstID,        (integer - return value)
//                         lastID,         (integer - return value)
//                         buffer          (buffer to place data in)
//                        )
//
//                                          
//-

LONG WINAPI idl_ReadFromBCU(LONG lArgc, LPVOID lpvArgv)
{
	long **lplpArgv;
	char *remote_addr;
	int firstDSP, lastDSP, opcode, datalen, err;
	unsigned int address;
	unsigned char firstID, lastID;
	BYTE *receivebuffer;
	int wait_counter;

	// lArgc is the number of arguments
	if (lArgc !=9 )
		return(PAR_NUM_ERR);

	lplpArgv = (long **)lpvArgv;

	remote_addr =  ((IDL_STRING *)lplpArgv[0])->s;
	firstDSP =    (int)         (*lplpArgv[1]);
	lastDSP =     (int)         (*lplpArgv[2]);
	opcode =      (int)         (*lplpArgv[3]);
	address =     (unsigned int)(*lplpArgv[4]);
	datalen =     (int)         (*lplpArgv[5]);

	receivebuffer = (BYTE *)    (lplpArgv[8]); // last parameter


	// Prepare parameters for the receiving thread

	idl_receivebuffer = receivebuffer;
	idl_counter = GetPacketNum( firstDSP, lastDSP, datalen, &idl_firstID);

	// Send the read request
	err = ReadFromBCU( remote_addr, firstDSP, lastDSP, opcode, address, datalen, &firstID, &lastID);

	*lplpArgv[6] = (long)firstID;
	*lplpArgv[7] = (long)lastID;

	if (err != NO_ERROR)
		return err;

	// Now wait for the answer (coming from the other thread)
	wait_counter=0;
	while( idl_counter>0)
		{
		if (idl_threadstatus != NO_ERROR)
			return (long)idl_threadstatus;

		Sleep(1);
		wait_counter++;

		if (wait_counter>5000)
			return (long) TIMEOUT_ERROR;
		}
	
	return (long)err;
}


//+Function: idl_WriteToBCU
//
// Wrapper around the idl_WriteToBCU commlib function
//
// Calling syntax from IDL:
// result = CALL_EXTERNAL("idl_commlib.dll", "idl_WriteToBCU", 
//                         remote_IP_addr, (string)
//                         firstDSP,       (integer)
//                         lastDSP,        (integer)
//                         opcode,         (integer)
//                         addressInBCU    (unsigned integer)
//                         data,           (BYTARR of data)
//                         datalen,        (integer)
//                         firstID,        (integer - return value)
//                         lastID,         (integer - return value)
//-

LONG WINAPI idl_WriteToBCU(LONG lArgc, LPVOID lpvArgv)
{
	long	**lplpArgv;
	char *remote_addr;
	BYTE *data;
	int firstDSP, lastDSP, opcode, datalen, err;
	unsigned int address;
	unsigned char firstID, lastID;
	int wait_counter,socket=0;

	// lArgc is the number of arguments
	if (lArgc !=9 )
		return(PAR_NUM_ERR);

	lplpArgv = (long **)lpvArgv;

	remote_addr =  ((IDL_STRING *)lplpArgv[0])->s;
	firstDSP =    (int)         (*lplpArgv[1]);
	lastDSP =     (int)         (*lplpArgv[2]);
	opcode =      (int)         (*lplpArgv[3]);
	address =     (unsigned int)(*lplpArgv[4]);
	data =        (BYTE *)      (lplpArgv[5]);
	datalen =     (int)         (*lplpArgv[6]);

	idl_receivebuffer = NULL;
	idl_counter = GetPacketNum( firstDSP, lastDSP, datalen, &idl_firstID);

	err = WriteToBCU( remote_addr, firstDSP, lastDSP, opcode, address, data, datalen, &firstID, &lastID);

	*lplpArgv[7] = (long)firstID;
	*lplpArgv[8] = (long)lastID;

	if (err != NO_ERROR)
		return (long)err;

	//Now wait for the answer (from the other thread)
	wait_counter=0;
	while( idl_counter>0)
		{
		if (idl_threadstatus != NO_ERROR)
			return (long)idl_threadstatus;

		Sleep(1);
		wait_counter++;

		if (wait_counter>5000)
			return (long) TIMEOUT_ERROR;
		}
	
	return (long)err;
}


//+Function: idl_GetPacketNum
//
// Wrapper around the idl_GetPacketNum commlib function
//-

LONG WINAPI idl_GetPacketNum(LONG lArgc, LPVOID lpvArgv)
{
	long	**lplpArgv;
	int firstDSP, lastDSP, len, err;
	unsigned char firstID;

	// lArgc is the number of arguments
	if (lArgc !=4 )
		return(PAR_NUM_ERR);

	lplpArgv = (long **)lpvArgv;

	firstDSP = (int)(*lplpArgv[0]);
	lastDSP = (int)(*lplpArgv[1]);
	len = (int)(*lplpArgv[2]);

	err = GetPacketNum( firstDSP, lastDSP, len, &firstID);

	*lplpArgv[3] = (long)firstID;

	return (long)err;
}




void DumpBCUcommand( char *dump, BCUcommand *cmd)
{
	sprintf( dump, "dummy: %02X \n DSPs: %d %d %d \n opcode: %d \n Length: %d \n flags: %d  \n msgID: %d \n  addr: %02X %20X %02x %02x",
		cmd->dummy_word, cmd->DSPs[0], cmd->DSPs[1], cmd->DSPs[2],
		cmd->opcode, cmd->length, cmd->flags, cmd->msgID, 
		cmd->address[0], cmd->address[1], cmd->address[2], cmd->address[3]);

}

