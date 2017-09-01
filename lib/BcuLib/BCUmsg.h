//+File: BCUmsg.h
//
// Header file for communications with the BCU control program
//
// This file contains  some structures of interest to all programs 
// that need to communicate with the BCU control program.
//
// A correctly filled  BCUmsg structure (typedef'ed to BCUmessage) must be
// sent to BCUCtrl as a message to start a BCU command. Special attention
// should be paid to the Client field: it must match exactly the client
// name as specified in the message.
//
// If the Client field is not valid (Cient[0] == 0), the structured is assumed
// to be internally generated.
//
// buffer_id can be a valid shared memory ID (positive), or one of a few
// special values.
//-

#ifndef BCU_MSG_INCLUDED
#define BCU_MSG_INCLUDED

#define BCU_WFS		(0)

#include "base/common.h"

struct BCUmsg
{
	int BCUnumber;		// Number of the requested BCU
	char Client[PROC_NAME_LEN+1];  // Client name 
	int firstDSP;		// first DSP to be addressed
	int lastDSP;		// last DSP to be addressed
	int command;		// Command (WR_SAME, WR_SEQ, RD_SEQ)
	int status;		// Status answer (CMD_SUCCESS, CMD_FAIL, RD_REPLY)
	int address;		// BCU memory address to read/write (in 32 bit words)
	int flags;		// Flags to add to BCU command (es, FLAG_ASQUADWORD)
	int len;		// Length of buffer (in BYTES)
	int buffer_id;          // Shared memory buffer id
};

typedef struct BCUmsg BCUmessage;

#define BUFFERID_LOCALBUFFER	(-1)
#define BUFFERID_APPENDTOMSG	(-2)
#define BUFFERID_SINGLEWORD	(-3)

#endif // BCU_MSG_INCLUDED

