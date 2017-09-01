//@File: idllib.h
//
// Header file for the IDL client/server library
//@

#ifndef LIBIDL_H_INCLUDED
#define LIBIDL_H_INCLUDED

#include "base/msglib.h"

// idl_answer structure.
//
// Pointer fields must be initialized to NULL or to malloc()ed buffers with their length in
// the first two variables. It's best to use the InitIDLAnswer() function
//
// When done, remember to use the FreeIDLAnswer() function to free the memory used.

typedef struct
{
    int output_len;
    int error_len;
    int error_code;   // error code of IDL functions
    char *output;
    char *error;
} idl_answer;

// Functions for clients

int PrepareIDLAnswer( idl_answer *answer);
int GetIDLAnswer( MsgBuf *msgb, idl_answer *answer);
int FreeIDLAnswer( idl_answer *answer);

// Functions for servers

int SendIDLAnswer( MsgBuf *msgb, char *output, char *error);


#endif // LIBIDL_H_INCLUDED



