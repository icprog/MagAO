//@File: idllib.c
//
// Library for IDL server/clients
//@

#include <string.h>		// memset()
#include <stdlib.h>		// malloc(), free()

#include "base/thrdlib.h"
#include "base/errordb.h"

#include "idllib/idllib.h"
#include <stdio.h>
#include <errno.h>


//+Function: PrepareIDLAnswer
//
// Prepares the IDLanswer struct

int PrepareIDLAnswer( idl_answer *answer)
{
	memset( answer, 0, sizeof(idl_answer));
	return NO_ERROR;
}

//+Function: FreeIDLAnswer
//
// Frees any memory allocated in the answer structure

int FreeIDLAnswer( idl_answer *answer)
{
	if (answer->output)
		free(answer->output);
	if (answer->error)
		free(answer->error);

	answer->output = NULL;
	answer->error  = NULL;
	answer->output_len = 0;
	answer->error_len = 0;
    answer->error_code = 0;

	return NO_ERROR;
}


//+Function: GetIDLAnswer
//
// Given a msg buffer, copies IDL output and error in the
// pointed answer structure. Pointers in the structure are
// automatically allocated or enlarged as needed.
//
// Fields remain valid after the overwriting of the msg buffer.

int GetIDLAnswer( MsgBuf *msgb, idl_answer *answer)
{
	int alloc_output =0;
	int alloc_error  =0;
    char * ptr, *end_ptr;
    long int lret;

    // The signed/unsigned char casting is to avoid compiler warnings
	unsigned char *idl_output = MSG_BODY(msgb);
	int idl_output_len = strlen((char *)idl_output)+1;
	unsigned char *idl_error = MSG_BODY(msgb) + idl_output_len;
	int idl_error_len = strlen((char *)idl_error)+1;

	// The next 30 lines are memory management for the two strings in the answer structure
	if (! answer->output)
		alloc_output = 1;
	else if ( idl_output_len > answer->output_len)
		{
		free(answer->output);
		alloc_output = 1;
		}

	if (! answer->error)
		alloc_error = 1;
	else if ( idl_error_len > answer->error_len)
		{
		free(answer->error);
		alloc_error = 1;
		}
	if (alloc_output)
		{
		answer->output_len = idl_output_len; 
		if ((answer->output = (char *)malloc( answer->output_len)) == NULL)
			return MEMORY_ALLOC_ERROR;
		}

	if (alloc_error)
		{
		answer->error_len = idl_error_len;
		if ((answer->error = (char *)malloc( answer->error_len)) == NULL)
			return MEMORY_ALLOC_ERROR;
		}

	// Copy answer strings in the answer structure
	strcpy( answer->output, (char *)idl_output);
	strcpy( answer->error, (char *)idl_error);

    // WARNING: this bit of code is based on the assumption
    // that IDL stdout has always a \n in last position
    // IDL stdout last char is \n: search the next-to-last
    if ((ptr = strrchr((char*)idl_output, '\n')) == NULL){
        answer->error_code = IDL_NOT_AVAILABLE_ERROR;
    } else {
        *ptr='\0'; // overwrite last \n with \0
        if ((ptr = strrchr((char*)idl_output, '\n')) == NULL){ 
            // there was only a '\n' in stdout
            // point ptr to begin of the string 
            ptr = (char*)idl_output;
        } 
        lret = strtol(ptr,&end_ptr,0);
        if (ptr==end_ptr && lret==0){
            // no digits at all
            answer->error_code = IDL_NOT_AVAILABLE_ERROR;
        }
        else if (errno == ERANGE || errno == EINVAL){
            // conversion overflow or no conversion performed
            // Note that an IDL function returning -10001
            // will be interpreted as an error.
            answer->error_code = IDL_NOT_AVAILABLE_ERROR;
        }
        else 
            answer->error_code = (int)lret;
    }
	return NO_ERROR;
}


// Sends a properly formatted IDL answer message. The msgb must
// already been initialized with a proper sender/receiver header.

int SendIDLAnswer( MsgBuf *msgb, char *output, char *error)
{
	char *buffer;
	int output_len, error_len;

	output_len = strlen(output)+1;
	error_len  = strlen(error)+1;

	buffer = (char *)malloc( output_len+error_len);
	if (!buffer)
		return MEMORY_ALLOC_ERROR;

	strcpy( buffer, output);
	strcpy( buffer + output_len, error);

   thReplyMsg(IDLREPLY, output_len+error_len, buffer, msgb);
	free(buffer);
   return NO_ERROR;
}

