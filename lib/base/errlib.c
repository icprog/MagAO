//@File: errlib.c
//
// Error management library
//
//
// History:
//
// 1 mar 2002:   First creation. (L.F.)
// 
// 1 apr 2005:   Improved error list generation (L.F.)
//
// Notes on LBT-AdOpt software error management.
//
// To ensure a constant behaviour of the LBT-AdOpt software the error 
// management of all modules must follow the guidelines described here.
//
// Error codes to be used MUST be defined in the include file errordb.txt
//
// The symbol NO_ERROR (defined as zero value) must be used to signal
// successful completion.
//
// All error codes are negative integers and are divided into classes.
//
// Any LBT-AdOpt software routine can use one out of two possible
// mechanisms to report error conditions.
//
// The preferred one, to be used whenever possible, is an integer code 
// returned as function value. Then it is up to the calling code to 
// take appropriate actions depending on the code value.
//
// On occasions (e.g. when the function must return a pointer or the like)
// the  function value will have some "faulty" return (e.g.: a NULL pointer)
// and the corresponding error code will be next available by calling
// lao_errno(). This function has thus a functionality which is similar
// to the global variable errno used in standard C programs.
//@

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "errlib.h"

#include "base/errordb.c"   // includes definition for 
                       // static DbTable errordbTable[] = {
                       // The file errordb.c is generated automatically
                       // from the central error definition file: errordb.txt

static int laoerrno=NO_ERROR;


static DbTable Dummy = { (-1), "Undefined", "Undefined" };

// Local implementation of nonstandard function strcasestr()
//
static char *STRCASESTR(char *str, char *sub)
{
int lng=strlen(sub);

while(*str) {
  if(strncasecmp(sub,str,lng)==0) return str;
  str++;
}
return NULL;
}

//@Function: lao_logsrc
//
//  Search into a DbTable. Sorted table (decreasing)
//
// This routine searches a message string labeled with given code
// into a table. A log search is performed so elements in the
// table must be sorted in decreasing order.
//@

static
DbTable *lao_logsrc(int code, DbTable *table, int tbLen)
                                      // Returns: pointer to constant string
                                      // If the code is not defined 
                                      // a pointer to a dummy element is returned
{
    DbTable *pt = &Dummy;
    int i,i0,i1,cd;

    i0=0;
    i1=tbLen-1;

    while(i0<=i1) {
       i=(i0+i1)/2;
       cd=table[i].code;
       if(cd == code) {
          pt=table+i;
          break;
       } else if( cd<code)
          i1=i-1;
       else
          i0=i+1;
    }
    return pt;
}

//@Function{API}: lao_scan
//
// Scan an entire DbTable
//
// This routine scans a DbTable.
//@

DbTable *lao_scan(int ix,          // @P{ix}: Table index
                                   // 0: first item. 
                  DbTable *table,  // @P{table}: table
                  int tbLen)       // @P{tbLen}: table length
                                   // @R: pointer to table item
                                   // If the index is outside table, return NULL
                                   // a pointer to the string ":?" is returned
{
   if(ix<0 || ix>=tbLen) return NULL;
   return table+ix;
}


//@Function{API}: lao_strtable
//
// Search into a DbTable. Generic
//
// This routine searches a message string labeled with given code
// into a table. A linear search is performes, so no assumption
// is made about code ordering.
//@

const char *lao_strtable(int code,        // @P{code}: item to search
                         DbTable *table,  // @P{table}: table
                         int tbLen)       // @P{tbLen}: table length
                                          // @R:: pointer to constant string
                                          // If the code is not defined 
                                          // a pointer to the string ":?" is returned
{
    char *pt = "?";
    int i;

    for(i=0;i<tbLen;i++) {
	   if(table[i].code == code) {
	    pt=table[i].descr;
	    break;
	   }
    }
    return pt;
}



//@Function{API}: lao_errno
//
// Returns The last error code
//
// This routine returns the last error code.
//@

int lao_errno(void) 
{
    return laoerrno;
}


//@Procedure{API}: lao_seterr

// Sets the error code
//
// This routine must be called by library routines to set the value of 
// the error code.
//
// Error codes must be derived by the list supplied in errlib.h
//@

void lao_seterr(int errc)      // @P{errc}: Error code. Zero or positive values
                                  // are considered equivalent.
{
    if(errc>NO_ERROR) errc = NO_ERROR;
    laoerrno=errc;
}


//@Function{API}: lao_strerror
//
//  Returns a string describing an error code
//
// This routine must be called to get a string description corresponding
// to an error code as returned by any LBT-AdOpt routine
//@

const char *lao_strerror(int errc) // @P{errc}:Error code. Positive values are
                                   // equivalent to NO_ERROR (0)
                                   // @R: pointer to constant string
                                   // If the error code is not defined 
                                   // a pointer to the string "?" is returned
{
    char *pt="?";
    DbTable* tmp;

    errc=PLAIN_ERROR(errc);   // Restore flags
    errc=(errc>0)?0:errc;

    if((tmp=lao_logsrc( errc, errordbTable, errordbTabLen))) pt=tmp->descr;
    return pt;
}



//@Function{API}: lao_errinfo
//
//  Returns the info structure for an error code
//
// This routine must be called to get the full descriptio of
// an error code.
//@

DbTable* lao_errinfo(int errc) // @P{errc}: Error code. Positive values are
                               // equivalent to NO_ERROR (0)
                               // @R: pointer to constant string. (NULL if not found)
{
    errc=PLAIN_ERROR(errc);   // Restore flags
    errc=(errc>0)?0:errc;

    return lao_logsrc( errc, errordbTable, errordbTabLen);
}

//@Function{API}: lao_strsrc
//
// Search into error DbTable for error description matching string
//
// This routine allows exploration of a DbTable list (used for message
// codes and for error codes).  It returns a list of item explanation,
// matching a given string (case unsensitive)
//
// The routine must be called many times with a string specification
// until it returns a NULL pointer, indicating that no other matches 
// are available
//@

static char mtch[32];
static int  curidx=(-1);

DbTable*  lao_strsrc(char *match,     //@P{match}: String to match.
                                      // Note: only the first 31 characters are considered for matching.
                      DbTable *table, // @P{table}: table
                      int tbLen)      // @P{tbLen}: table length
                                      //@R: pointer to DbTable item (NULL for no match)
{
    DbTable *pt = NULL;
    int i;

    if(strncmp(mtch,match,31)!=0) {
        strncpy(mtch,match,31);
        mtch[31]='\0';
        curidx=0;
    }

    if(curidx==(-1)) return NULL;

    for(i=curidx;i<tbLen;i++) {
        DbTable *cpt=table+i;
        if(STRCASESTR(cpt->name,mtch) || STRCASESTR(cpt->descr,mtch)) {
            pt=cpt;
            curidx=i+1;
            break;
        }
    }
    if(!pt) 
        curidx=(-1);

    return pt;
}



//@Function{API}: lao_strsrc
//
// Search into error table for error description matching string
//
// This routine allows exploration of the error table. 
// It returns a list of error explanations, matching a given string 
// (case unsensitive)
//
// The routine must be called many times with a string specification
// until it returns a NULL pointer, indicating that no other matches 
// are available
//@
//

DbTable * lao_errsrc(char * match)
{
    return lao_strsrc( match, errordbTable, errordbTabLen);
}

//@Function{API}: lao_errscan
//
// Scan the error table
//
//@

DbTable *lao_errscan(int ix)          // @P{ix}: Table index
                                   // 0: first item. 
                                   // @R: pointer to table item
                                   // If the index is outside table, return NULL
                                   // a pointer to the string ":?" is returned
{
   return lao_scan(ix,errordbTable,errordbTabLen);
}


