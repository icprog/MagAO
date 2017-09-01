//@File: logerr.h
//
//Definitions and prototypes for the module logerr.c
//@

#ifndef LOGERR_H
#define LOGERR_H

#include "types.h"
#include "rtdb.h"


#define BYTE_FMT     1
#define WORD_FMT     2
#define INT_FMT      3

#define SHORT_TIME_FMT 0      // Select differential time mode
#define LONG_TIME_FMT  1      // Select absoulte time mode
#define DIFF_TIME_FMT  2      // Select differential time mode

#define LOG_START   0
#define LOG_REOPEN  1

                              // Var dump mode flags
#define ON_REQUEST  0
#define ON_TRACE    1
                                // Program terminate reasons

#define TERM_REASON_0_MSG "unknown (illegal reason code)"

#define TERM_CLNTREQ      1
#define TERM_REASON_1_MSG "Client request"

#define TERM_FATALERR     2
#define TERM_REASON_2_MSG "Fatal error"

#define TERM_KBINTERR     3
#define TERM_REASON_3_MSG "Keyboard interrupt"

#define TERM_MAX_REASON   3


int SetDebugLevel(int level);
int GetDebugLevel(void);
int DecrDebugLevel(void);
int IncrDebugLevel(void);

void SetTimeMode(int mode);

int SetLogfile(int);
void SetMaxlines(int nl);

int GetMaxlines(void);
int GetLoglines(void);

// int LAO_Write( int lev, char * fmt, ...);

int LAO_Info(Client *Source, char * fmt, ...);
int LAO_Trace(Client *Source, char * fmt, ...);
int LAO_VTrace(Client *Source, char * fmt, ...);

int LAO_Info(Client *Source, char * fmt, ...);
int LAO_Debug(Client *Source, char * fmt, ...);
int LAO_remLog(Client *Source, int lev, int seq, char * txt);

int LAO_NoMsgRou(MsgBuf *, int);
int LAO_LogMsgRou(MsgBuf *);
int LAO_LogMsgRec(MsgBuf *);
int LAO_LogMsgSnt(MsgBuf *, Client *cl);

int LAO_Error(Client *Source, int lao_error, char * msg, ...);

int LAO_Fatal(Client *Source, int lao_error, char * msg, ...);

int LAO_DumpVar(Client *src,DB_Var *vv, int lock);
int LAO_DumpVarList(Client *src,char *pref);
int LAO_DumpBstring(ShmListscan* listX, char* cbuf, int lng);
int LAO_DumpBufsList(void);
int LAO_DumpCLtable(void);
int LAO_DumpInfo(int);
int LAO_DumpNotifTable(int both);
int LAO_DumpVstat(void);
int LAO_clientstr(Client *client, int nconn,char *chbuf, int lng);
void CleanExit(int why);

int LAO_InfoLine(int line, char *buf, int buflen);
int LAO_InfoString(char *buf, int buflen);

int LAO_Freeze(int lock);



#endif
