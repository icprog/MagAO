//@File: rtdblib.h

// Definitions for the rtdblib library

//@

#ifndef RTDBLIB_H
#define RTDBLIB_H

#include <sys/time.h>         // (for struct timeval)

#include <limits.h>

#include "base/common.h"


// Special code for variable ownership
#define PUBLIC_VAR   (-999)

#include "base/vartypes.h"


#if LONG_MAX > 2147483647
#define LONG_IS_64
//This means we're on a 64 bit long machine (e.g. x86_64)
//But msglib assumes 32 bit longs if it is on a 32 bit machine (e.g. i386)
//We must deal with timeval, long, and unsigned long
#endif

#ifdef LONG_IS_64
//This code is used for temporary conversion, but we convert back
//to native long size so functions that use this stuff don't notice
struct timeval32{
	int tv_sec;
	int tv_usec;
};

struct timeval tv_fm_tv32(struct timeval32 tv32);

struct timeval32 tv32_fm_tv(struct timeval tv);
#else
#define timeval32 timeval
#define tv_fm_tv32(tv32) tv32
#define tv32_fm_tv(tv)  tv
#endif

typedef struct {
    char           Name[VAR_NAME_LEN+1];    // Variable Name
    char           Owner[PROC_NAME_LEN+1];  // Variable owner (if public='\0')
    int            Dummy;        // Dummy field to be removed (will change header size)
    int            Type;         // See defines above
    int            NItems;       // Variable length
    struct timeval32 MTime;        // Last modification time
} VHeader;

typedef struct {
    VHeader    H;
    union  {
	double Dv[0];
#ifdef LONG_IS_64
	int Lv[0];
#else
	long   Lv[0];
#endif
	char   Sv[0];
	unsigned char   B8[0];
	unsigned short  B16[0];
#ifdef LONG_IS_64
	unsigned int B32[0];
#else
	unsigned long   B32[0];
#endif
	unsigned long long B64[0];
    }            Value;
} Variable;








#ifndef ONLY_VARCODES

#ifdef LIBRARY_DEVELOPER


int VarCmd(const char *Client, int MsgCode, const char *VarName, int VarType,
           int  NElements, const void *Value, MsgBuf *imsgb);
#endif

// Prototypes


char *rmVDomain(const char *name, char *vbuf);

int VarSize(int VarType, int NElem);

                   // When using the thrdlib utilities, the following functions
                   // MUST not be used. Use corresponding thrdlib functions
#ifndef USER_THRDLIB
int SetVar(char *Client,char *VarName, int VarType, 
	    int NElem, void *Value, MsgBuf *msg);
int VarNotif(char *Client, char *VarName);
int VarNoNotif(char *Client, char *VarName);
int TakeVar(char *Client, char *vname, MsgBuf *imsgb);
int GiveVar(char *Client, char *vname, MsgBuf *imsgb);
void VarDebug(int);

int DumpVars(char *Client,char *VarName, MsgBuf *msg);
int CreaVar(char *Client,char *VarName, int VarType, char *VarMode, 
	    int NElem, MsgBuf *msg);
int VarList(char *Client, char *prefix, int seqnum, MsgBuf *msg);
int GetVar(char *Client, char *VarName, int seqnum, MsgBuf *msg);
int DelVar(char *Client,char *VarName, int tmout, MsgProc callbk, MsgBuf *msg);
int ReadVar(char *Client, char *VarName, void *varBuf, int nItms, 
            int tmout, MsgProc callbk, MsgBuf *imsgb);
int LockVars(char *Client,char *prefix,int tmout,MsgProc callbk,MsgBuf *imsgb);
int UnlockVars(char *Client,char *prefix,int tmout,MsgProc callbk,MsgBuf *imsgb);
int WriteVar(char *Client, char *VarName, int   VarType, 
             int  NElements, void *Value, int tmout, MsgProc callbk, MsgBuf *imsgb) ;
#endif

#endif  // ONLY_VARCODES

#endif
