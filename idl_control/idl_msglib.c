//+File: idl_msglib.c
//
// Msglib wrapper for IDL
//-

#include <stdio.h>

#include "idl_export.h"

#include "msglib.h"
#include "commlib.h"
#include "BCUmsg.h"
#include "bcucommand.h"
#include "errlib.h"

// Error codes
#ifndef NO_ERROR
#define		NO_ERROR		(0L)
#endif /* NO_ERROR */

#define		PAR_NUM_ERR		(-1L)

// IDL function prototypes.

int test( int lArgc, void **lpvArgv);
int idl_function( int lArgc, void **lpvArgv);

int idl_setup( int lArgc, void **lpvArgv);
int idl_SendMessage(  int lArgc, void **lpvArgv);
int idl_ReceiveMessage(  int lArgc, void **lpvArgv);
IDL_LONG idl_SendBCUCommand( int lArgc, void **lpvArgv);

// Global data

unsigned char *idl_receivebuffer = NULL;
unsigned char idl_seqnum=0;
int idl_setup_ok=0;

SharedMem *shared = NULL;

// Names and #defines for the server environment

char *server_ip    = "127.0.0.1";
char MyName[ PROC_NAME_LEN+1];
char *BCUCTRL_name = "BCUCTRL";
char *BCUCTRL_msg  = "BCU";

//+Function: idl_setup
//
// Setup the Supervisor link
//-

int idl_setup( int lArgc, void **lpvArgv)
{
	int stat;
	char *clientname;
	long **lplpArgv;

	// lArgc is the number of arguments
	if (lArgc !=1 )
		return(PAR_NUM_ERR);

	lplpArgv = (long **)lpvArgv;
	clientname =  ((IDL_STRING *)lplpArgv[0])->s;

	memset( MyName, 0, PROC_NAME_LEN+1);
	strncpy( MyName, clientname, PROC_NAME_LEN);

	// Connnect to  server
	stat = ConnectToServer( server_ip, MyName, 1);
	if (stat >=0)
		{
		idl_setup_ok=1;
		return NO_ERROR;
		}
	else
		return PLAIN_ERROR(stat);
}


//+Function: idl_function
//
// Sample function to show how arguments are handled
//-

int idl_function( int lArgc, void **lpvArgv)
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

int test( int lArgc, void **lpvArgv)
{
	return 0L;
}

 


//+Function: idl_SendMessage
//
// Wrapper around the SendMessage() msglib function
//
// Calling syntax from IDL:
//
// result = CALL_EXTERNAL("idl_msglib.dll", "idl_SendMessage", 
//                         destination, (string)
//                         message code, (string)
//                         message data, (BYTARR)
//                         data length, (int)
//                         seqence number (int)
//
//-

int idl_SendMessage( int lArgc, void **lpvArgv)
{
	int err, msg_len, seqnum;
	char *msg_dest, *msg_code;
	UCHAR *msg_data;
	MsgBuf msgb;
	long **lplpArgv;

	// lArgc is the number of arguments
	if (lArgc !=5 )
		return(PAR_NUM_ERR);

	// Check if we are connected with the Supervisor
	if (! idl_setup_ok)
		return MISSING_CLIENTNAME_ERROR;

	lplpArgv = (long **)lpvArgv;

	msg_dest =  ((IDL_STRING *)lplpArgv[0])->s;
	msg_code =  ((IDL_STRING *)lplpArgv[1])->s;
	msg_data =   (UCHAR *)     (lplpArgv[2]);
	msg_len =    (int)       (*lplpArgv[3]);
	seqnum =     (int)       (*lplpArgv[4]);


	InitMsgBuf(&msgb, 0);
	err = FillMessage( msg_len, seqnum, MyName, msg_dest, msg_code, msg_data, &msgb);
	if (err != NO_ERROR)
		return PLAIN_ERROR(err);

	err = SendToServer(&msgb);
	if (err != NO_ERROR)
		return PLAIN_ERROR(err);

	FreeMsgBuf(&msgb);

	return NO_ERROR;
}


//+Function: idl_ReceiveMessage
//
// Wrapper around the WaitMessage() msglib function
//
// Calling syntax from IDL:
// result = CALL_EXTERNAL("idl_msglib.dll", "idl_ReceiveMessage", 
//                         from, (string already >= 10 chars)
//                         code  (string already >= 10 chars)
//                         data  (BYTARR that will be filled with data)
//                         datalen (length of data BYTARR)
//-

int idl_ReceiveMessage( int lArgc, void **lpvArgv)
{
	long	**lplpArgv;
	int err;
	int msg_len, len;
	char *msg_from, *msg_code, *msg_data;
	MsgBuf msgb;

	// lArgc is the number of arguments
	if (lArgc !=4 )
		return(PAR_NUM_ERR);

	// Check if we are connected with the Supervisor
	if (! idl_setup_ok)
		return MISSING_CLIENTNAME_ERROR;

	lplpArgv = (long **)lpvArgv;

	msg_from =  ((IDL_STRING *)lplpArgv[0])->s;
	msg_code =  ((IDL_STRING *)lplpArgv[1])->s;
	msg_data =   (UCHAR *)     (lplpArgv[2]);
	msg_len =    (int)       (*lplpArgv[3]);


	InitMsgBuf(&msgb, 0);
	err = WaitMsgFromServer(&msgb);
	if (err == NO_ERROR)
		{
		strcpy( msg_from, msgb.bufmem->Hdr.From);
		strcpy( msg_code, msgb.bufmem->Hdr.Code);
		len = msgb.bufmem->Hdr.Len;
		if (len > msg_len)
			len = msg_len;
		memcpy( msg_data, MSG_BODY(msgb), len);
		}

	return PLAIN_ERROR(err);
}

//+Function: idl_SendBCUCommand
//
// Wrapper to send a BCU command
//
// Calling syntax from IDL:
//
// result = CALL_EXTERNAL("idl_msglib.dll", "idl_SendBCUcommand",
//                         BCU number, (int)
//                         firstDSP, (int)
//                         lastDSP, (int)
//                         opcode, (int)
//                         address, (long int)
//                         data, (BYTARR)
//                         data length, (int, bytes)
//                         timeout, (int, milliseconds, <=0 is infinite)
//
//-

IDL_LONG idl_SendBCUCommand( int lArgc, void **argv)
{
	BCUmessage cmd, *answerCmd;

	int stat,datalen;
	short int BCU_num, firstDSP, lastDSP, opcode, timeout;
	int address;
	char *data;
	MsgBuf msgb;

	// lArgc is the number of arguments
	if (lArgc !=8 )
		return(PAR_NUM_ERR);

	// Check if we are connected with the Supervisor
	if (! idl_setup_ok)
		return MISSING_CLIENTNAME_ERROR;

	// Extract IDL parameters

	BCU_num  =  *( (IDL_INT *)     (argv[0]));
	firstDSP =  *( (IDL_INT *)     (argv[1]));
	lastDSP  =  *( (IDL_INT *)     (argv[2]));
	opcode   =  *( (IDL_INT *)     (argv[3]));
	address  =  *( (IDL_LONG *) (argv[4]));
	data     =  (UCHAR *)		 (argv[5]);
	datalen  =  *( (IDL_INT *)	 (argv[6]));
	timeout  =  *( (IDL_INT *)	 (argv[7]));

	printf("BCU_num : %d\n", BCU_num);
	printf("firstDSP: %d\n", firstDSP);
	printf("lastDSP : %d\n", lastDSP);
	printf("opcode  : %d\n", opcode);
	printf("address : %d\n", address);
	printf("datalen : %d\n", datalen);
	printf("timeout : %d\n", timeout);

	stat = sendBCUCommand( MyName, BCU_num, firstDSP, lastDSP, opcode, address, datalen, timeout, idl_seqnum);

	// Increment the seqnum - to distinguish between different messages

	idl_seqnum++;

	return PLAIN_ERROR(stat);
}


