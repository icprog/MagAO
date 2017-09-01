// @File: msgtxt.c
//
// Message codes explanations
//
// This file contains a small library to be used by applications
// which want to show explanation of message codes or error codes
//@

#include <string.h>

#include "base/msglib.h"
#include "base/msgtxt.h"

#include "base/msgcodes.c"

//@Function{API}: msgtxt_c
//
// Return explanation given item code from a DbTable
//
// This routine allows exploration of a DbTable list (used for message
// codes and for error codes).  It returns the long item explanation,
// given the item code.
// @

DbTable*  msgtxt_c(int code) //@P{code}: Message code to explain
                            //@R: DbTable item (NULL if not found)
{
    DbTable* pt = NULL;
    int i;

    for(i=0;i<msgcodesTabLen;i++) {
       if(msgcodesTable[i].code == code) {
          pt=msgcodesTable+i;
          break;
       }
    }
    return pt;
}

//@Function{API}: msgtxt_i
//
// Return explanation given item index from a DbTable
//
// This routine allows exploration of a DbTable list (used for message
// codes and for error codes).  It returns the item explanation,
// given the item index. Indexes start at 0.
//@

DbTable*  msgtxt_i(int id) //@P{id}: Message index 
                           //@R: pointer to DbTable (NULL for invalid index)
{
    return lao_scan(id,msgcodesTable,msgcodesTabLen);
}


//@Function{API}: msgtxt_s
//
// Returns sequence of DbTable list matching given string
//
// This routine allows exploration of a DbTable list (used for message
// codes and for error codes).  It returns a list of item explanation,
// matching a given string (case unsensitive)
//
// The routine must be called many times with a string specification
// until it returns a NULL pointer, indicating that no other matches 
// are availablee
//@

DbTable*  msgtxt_s(char *match) //@P{id}: String to match.
                                // If NULL will try another  match with the previously 
                                // defined string
                                // Note: only the first 31 characters are considered for matching.
                                //@R: pointer to DbTable (NULL for no match)
{
    return lao_strsrc(match,msgcodesTable,msgcodesTabLen);
}
