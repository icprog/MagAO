//@File: logerr.c

// Error and log management routines
//@

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <pthread.h>

#include <arpa/inet.h>  // inet_ntoa()

#include <sys/stat.h>
#include <sys/timeb.h>
#include <sys/time.h>  // gettimeofday()

#include "base/errlib.h"
#include "base/msglib.h"

#include "msgdrtdb.h"

#include "shmlib.h"

#include "client.h"
#include "table.h"

#include "logerr.h"

static int maxLogRecs=LOGLINES;
static int rCounter=0;
static int DebugLevel=0;
static FILE *LogFile=NULL;

extern int errCount;

extern Client *MySelf;
extern struct PGINFO progInfo;
extern CLtable ClientTable;

//@Function{API}: SetMaxlines

// Set the maximum number of lin es in log file
//@

void SetMaxlines(int mx)
{
    if(mx<=1000) mx=LOGLINES;
    maxLogRecs=mx;
}




//@Function{API}: SetDebugLevel

// Set the debug verbosity level 
// 0: Error, Warning, Info
// 1: Debug
// 2: Trace
// 3: Verbose Trace
//@

int SetDebugLevel(int level)
{
    if(level<0) level=0;
    if(level>3) level=3;
    DebugLevel=level;
    return DebugLevel;
}

//@Function{API}: IncrDebugLevel

// Increase the debug verbosity level (0,1,2)
//@

int IncrDebugLevel(void)
{
    DebugLevel += 1;
    if(DebugLevel>2) DebugLevel=2;
    return DebugLevel;
}

//@Function{API}: DecrDebugLevel

// Decrease the debug verbosity level (0,1,2)
//@

int DecrDebugLevel(void)
{
    DebugLevel -= 1;
    if(DebugLevel<0) DebugLevel=0;
    return DebugLevel;
}


extern pthread_mutex_t LogMutex;

static void nlk_write(char *name, int lev, char * fmt, ...)
{
    va_list argp;
    char loghdr[MASTER_LOG_HEADER_LEN];

    logString(name,lev,NULL,loghdr);
    fprintf(LogFile,"%s",loghdr);
    va_start(argp, fmt);
    vfprintf(LogFile, fmt, argp);
    va_end(argp);
    fprintf(LogFile,"\n");
    rCounter++;
}


static char save_file[MAX_FNAME+15];

static char * newnameLog(char log_file[MAX_FNAME])
{

     snprintf(save_file,MAX_FNAME+15,"%s/%s.%14.14ld.log",progInfo.logPath,progInfo.MyName,time(NULL));
     snprintf(log_file,MAX_FNAME,"%s/%s.log",progInfo.logPath,progInfo.MyName);
     return save_file;
}

static void rotateLogs(void)
{
    char log_file[MAX_FNAME];
    char * new_name;
    int i;

    if(LogFile==stdout) return;

    new_name=newnameLog(log_file);

    nlk_write(progInfo.MyName,MASTER_LOG_LEV_INFO,"Closing logfile as %s for reopen",new_name);
    fclose(LogFile);
    rename(log_file,new_name);

    rCounter=0;

    if((LogFile=fopen(log_file,"w"))==NULL) {
       LogFile=stdout;
       nlk_write(progInfo.MyName,MASTER_LOG_LEV_ERROR,"Cannot open file %s",log_file);
    }

    nlk_write(progInfo.MyName,MASTER_LOG_LEV_INFO,"Logfile reopened (max lines exceeded)");
    for(i=0;;i++) {
        char buff[1024];
        int info=LAO_InfoLine(i,buff,1024);
        if(info==0) break;
        nlk_write(progInfo.MyName,MASTER_LOG_LEV_INFO,buff);
    }
    fflush(LogFile);
}

   

//@Procedure{API}: SetLogfile

// Set the log file name (default is stdout)
//
// If an actual file is openend the previous version is renamed
//
//@


int SetLogfile(int why)
{
    char log_file[MAX_FNAME];
    int i;
    char *new_name;

    MUTEX_LOCK(&LogMutex,"SetLogfile");

    new_name=newnameLog(log_file);
    if(LogFile && LogFile!=stdout) {
       nlk_write(progInfo.MyName,MASTER_LOG_LEV_INFO,"Closing logfile as %s for reopen", new_name);
       fclose(LogFile);
    }
    rename(log_file,new_name);

    if((LogFile=fopen(log_file,"w"))==NULL) {
        LogFile=stdout;
        nlk_write(progInfo.MyName,MASTER_LOG_LEV_ERROR,"Cannot open log file %s", log_file);
    }
    rCounter=0;

    if(why==LOG_REOPEN) {
        nlk_write(progInfo.MyName,MASTER_LOG_LEV_INFO,"Logfile reopend on client's request");
        for(i=0;;i++) {
            char buff[1024];
            int info=LAO_InfoLine(i,buff,1024);
            if(info==0) break;
            nlk_write(progInfo.MyName,MASTER_LOG_LEV_INFO,buff);
        }
        fflush(LogFile);
    }
    MUTEX_UNLOCK(&LogMutex,"SetLogfile");

    return NO_ERROR;
}


//@Procedure{API}: LAO_NoMsgRou

// Print a log record for a message which could not be routed

// The log is recorded unconditionally
//@

int LAO_NoMsgRou(MsgBuf *msgb, int errcod)
{
    char aux[200];

    StrHeader(msgb,aux,200);

    LAO_Error(0,errcod,aux);

    return NO_ERROR;
}


//@Procedure{API}: LAO_Freeze

// Locks/Unlocks the log mutex to simulate a freeze condition

//@

int LAO_Freeze(int lock)
{
   if(lock) {
      MUTEX_LOCK(&LogMutex,"LAO_Freeze");
   } else {
      MUTEX_UNLOCK(&LogMutex,"LAO_Freeze");
   }

   return NO_ERROR;
}


//@Procedure{API}: LAO_LogMsgRou

// Print a log record for a message to be routed

// The log is recorded if current debug level is greather than or equal
// to 2 (Trace)
//@

int LAO_LogMsgRou(MsgBuf *msgb)
{
    char loghdr[MASTER_LOG_HEADER_LEN];
    char aux[200];

    if(DebugLevel<2) return NO_ERROR;
    if(LogFile==NULL) LogFile=stdout;

    StrHeader(msgb,aux,200);

    MUTEX_LOCK(&LogMutex,"LAO_MsgRou");

    logString(progInfo.MyName,MASTER_LOG_LEV_TRACE,NULL,loghdr);
    fprintf(LogFile,"%sRoute - %s\n" , loghdr, aux);
    if(rCounter++>maxLogRecs) rotateLogs(); else fflush(LogFile);
    MUTEX_UNLOCK(&LogMutex,"LAO_MsgRou");

    return NO_ERROR;
}


//@Procedure{API}: LAO_LogMsgRec

// Print a log record for a message received

// The log is recorded if current debug level is greather than or equal
// to 2 (Trace)
//@

int LAO_LogMsgRec(MsgBuf *msgb)
{
    char loghdr[MASTER_LOG_HEADER_LEN];
    char aux[200];

    if(DebugLevel<2) return NO_ERROR;
    if(LogFile==NULL) LogFile=stdout;

    StrHeader(msgb,aux,200);

    MUTEX_LOCK(&LogMutex,"LAO_LogMsgRec");

    logString(progInfo.MyName,MASTER_LOG_LEV_TRACE,NULL,loghdr);
    fprintf(LogFile,"%sRecvd - %s\n" , loghdr, aux);
    if(rCounter++>maxLogRecs) rotateLogs(); else fflush(LogFile);

    MUTEX_UNLOCK(&LogMutex,"LAO_LogMsgRec");

    return NO_ERROR;
}


//@Procedure{API}: LAO_LogMsgSnt

// Print a log record for a message sent

// The log is recorded if current debug level is greather than or equal
// to 2 (Trace)
//@

int LAO_LogMsgSnt(MsgBuf *msgb, Client *cldest)
{
    char loghdr[MASTER_LOG_HEADER_LEN];
    char aux[200];

    if(DebugLevel<2) return NO_ERROR;
    if(LogFile==NULL) LogFile=stdout;

    if(cldest && cldest->clType==PEER_MSGD)
       StrHeaderE(msgb,"",cldest->Name,aux,200);
    else
       StrHeader(msgb,aux,200);

    MUTEX_LOCK(&LogMutex,"LAO_LogMsgSnt");

    logString(progInfo.MyName,MASTER_LOG_LEV_TRACE,NULL,loghdr);
    fprintf(LogFile,"%sSend  - %s\n" , loghdr, aux);
    if(rCounter++>maxLogRecs) rotateLogs(); else fflush(LogFile);

    MUTEX_UNLOCK(&LogMutex,"LAO_LogMsgSnt");

    return NO_ERROR;
}


//@Procedure{API}: LAO_remLog

// Print a log record sent by client to current log file

// The log is recorded unconditionally
// 
//@

int LAO_remLog(Client *src, int level, int seqn, char * msg)
{
    char loghdr[MASTER_LOG_HEADER_LEN];
    char *pt;

    if(!src->toLog) return NO_ERROR;
    if(LogFile==NULL) LogFile=stdout;

    MUTEX_LOCK(&LogMutex,"LAO_remLog");

    nlogString(progInfo.MyName,level,seqn,NULL,loghdr);
    pt=strchr(msg,'\n');
    if(pt) *pt++='\0';
    fprintf(LogFile,"%s[%s] %s\n",loghdr,src->Name,msg);
    while(pt) {
       char *str=pt;
       char *end=strchr(pt,'\n');
       if(end) {
           *end='\0';
           pt=end+1;
       } else pt=NULL;
       fprintf(LogFile,"%s[%s]  . %s\n",loghdr,src->Name,str);
       rCounter++;
    }
    if(rCounter++>maxLogRecs) rotateLogs(); else fflush(LogFile);

    MUTEX_UNLOCK(&LogMutex,"LAO_remLog");

    return NO_ERROR;
}

//@Procedure{API}: LAO_Info

// Print a Info log record to current log file

// The log is recorded unconditionally
//@

int LAO_Info(Client *src, char * fmt, ...)
{
    va_list argp;
    char loghdr[MASTER_LOG_HEADER_LEN];
    Client *cl;

    if(!src) {
        cl=MySelf;
    } else {
        cl=src;
    }

    if(LogFile==NULL) LogFile=stdout;

    MUTEX_LOCK(&LogMutex,"LAO_Info");

    logString(progInfo.MyName,MASTER_LOG_LEV_INFO,NULL,loghdr);
    if(src)
        fprintf(LogFile,"%s%s: ",loghdr,src->Name);
    else
        fprintf(LogFile,"%s",loghdr);
    va_start(argp, fmt);
    vfprintf(LogFile, fmt, argp);
    va_end(argp);
    fprintf(LogFile,"\n");
    if(rCounter++>maxLogRecs) rotateLogs(); else fflush(LogFile);

    MUTEX_UNLOCK(&LogMutex,"LAO_Info");

    return NO_ERROR;
}

//@Procedure{API}: LAO_Debug

// Print a log record to current log file

// The log is recorded if current debug level is greather than or equal
// to 2 (Verbose)
// The record is cretead as level LOG_LEV_DEBUG
//@

int LAO_Debug(Client *src, char * fmt, ...)
{
    va_list argp;
    char loghdr[MASTER_LOG_HEADER_LEN];
    Client *cl;

    if(!src) {
        cl=MySelf;
    } else {
        cl=src;
    }

    if(DebugLevel<1 || (!cl->toLog)) return NO_ERROR;
    if(LogFile==NULL) LogFile=stdout;

    MUTEX_LOCK(&LogMutex,"LAO_Debug");

    logString(progInfo.MyName,MASTER_LOG_LEV_DEBUG,NULL,loghdr);
    if(src)
        fprintf(LogFile,"%s%s: ",loghdr,src->Name);
    else
        fprintf(LogFile,"%s",loghdr);
    va_start(argp, fmt);
    vfprintf(LogFile, fmt, argp);
    va_end(argp);
    fprintf(LogFile,"\n");
    if(rCounter++>maxLogRecs) rotateLogs(); else fflush(LogFile);

    MUTEX_UNLOCK(&LogMutex,"LAO_Debug");

    return NO_ERROR;
}


//@Procedure{API}: LAO_Trace

// Print a log record to current log file

// The log is recorded if current debug level is greather than or equal
// to 2 (Trace)
//@

int LAO_Trace(Client *src, char * fmt, ...)
{
    va_list argp;
    char loghdr[MASTER_LOG_HEADER_LEN];
    Client *cl;

    if(src) {
        cl=src;
    } else {
        cl=MySelf;
    }

    if(DebugLevel<2 || !cl->toLog) return NO_ERROR;
    if(LogFile==NULL) LogFile=stdout;

    MUTEX_LOCK(&LogMutex,"LAO_Trace");

    logString(progInfo.MyName,MASTER_LOG_LEV_TRACE,NULL,loghdr);
    fprintf(LogFile,"%s",loghdr);
    va_start(argp, fmt);
    vfprintf(LogFile, fmt, argp);
    va_end(argp);
    fprintf(LogFile,"\n");
    if(rCounter++>maxLogRecs) rotateLogs(); else fflush(LogFile);

    MUTEX_UNLOCK(&LogMutex,"LAO_Trace");

    return NO_ERROR;
}

//@Procedure{API}: LAO_VTrace

// Print a log record to current log file

// The log is recorded if current debug level is greather than or equal
// to 3 (VTrace)
//@

int LAO_VTrace(Client *src, char * fmt, ...)
{
    va_list argp;
    char loghdr[MASTER_LOG_HEADER_LEN];
    Client *cl;

    if(src) {
        cl=src;
    } else {
        cl=MySelf;
    }

    if(DebugLevel<3 || !cl->toLog) return NO_ERROR;
    if(LogFile==NULL) LogFile=stdout;

    MUTEX_LOCK(&LogMutex,"LAO_Trace");

    logString(progInfo.MyName,MASTER_LOG_LEV_VTRACE,NULL,loghdr);
    fprintf(LogFile,"%s",loghdr);
    va_start(argp, fmt);
    vfprintf(LogFile, fmt, argp);
    va_end(argp);
    fprintf(LogFile,"\n");
    if(rCounter++>maxLogRecs) rotateLogs(); else fflush(LogFile);

    MUTEX_UNLOCK(&LogMutex,"LAO_Trace");

    return NO_ERROR;
}

static char *error_Format1 = "%s%s [%d]: %s";
static char *error_Format2 = "%s%s: %s [%d]: %s";

//@Procedure{API}: LAO_Error

// Print an error record to current log file

// The absolute value of the error code is printed (or stored in log file).
// If the SYSTEM_ERROR flag is set the system error message is also recorded.
//@

int LAO_Error(Client *src, int errcode, char * fmt, ...)
{
    va_list argp;
    DbTable *pt=lao_errinfo(errcode);
    char loghdr[MASTER_LOG_HEADER_LEN];

    if(LogFile==NULL) LogFile=stdout;


    MUTEX_LOCK(&LogMutex,"LAO_Error");

    logString(progInfo.MyName,MASTER_LOG_LEV_ERROR,NULL,loghdr);
    errCount++;
    if(src)
        fprintf(LogFile,error_Format2, loghdr,src->Name,pt->name,pt->code,pt->descr);
    else
        fprintf(LogFile,error_Format1, loghdr,pt->name,pt->code,pt->descr);
    if(fmt != NULL) {
        if(*fmt!='\0') {
            fprintf(LogFile," - ");
            va_start(argp, fmt);
            vfprintf(LogFile, fmt, argp);
            va_end(argp);
        }
    }
    if(IS_SYSTEM_ERROR(errcode))
        fprintf(LogFile," -- SYSERROR: %s",strerror(errno));
    fprintf(LogFile,"\n");
    if(rCounter++>maxLogRecs) rotateLogs(); else fflush(LogFile);

    MUTEX_UNLOCK(&LogMutex,"LAO_Error");

    return NO_ERROR;
}



//@Procedure{API}: LAO_Fatal

// Print an error record to current log file

// The absolute value of the error code is printed (or stored in log file).
// If the SYSTEM_ERROR flag is set the system error message is also recorded.
//@

int LAO_Fatal(Client *src, int errcode, char * fmt, ...)
{
    va_list argp;
    DbTable *pt=lao_errinfo(errcode);
    char loghdr[MASTER_LOG_HEADER_LEN];

    if(LogFile==NULL) LogFile=stdout;

    MUTEX_LOCK(&LogMutex,"LAO_Error");

    logString(progInfo.MyName,MASTER_LOG_LEV_FATAL,NULL,loghdr);
    errCount++;
    if(src)
        fprintf(LogFile,error_Format2, loghdr,src->Name,pt->name,pt->code,pt->descr);
    else
        fprintf(LogFile,error_Format1, loghdr,pt->name,pt->code,pt->descr);
    if(fmt != NULL) {
        if(*fmt!='\0') {
            fprintf(LogFile," - ");
            va_start(argp, fmt);
            vfprintf(LogFile, fmt, argp);
            va_end(argp);
        }
    }
    if(IS_SYSTEM_ERROR(errcode))
        fprintf(LogFile," -- SYSERROR: %s",strerror(errno));
    fprintf(LogFile,"\n");
    if(rCounter++>maxLogRecs) rotateLogs(); else fflush(LogFile);

    MUTEX_UNLOCK(&LogMutex,"LAO_Error");

    CleanExit(TERM_FATALERR);
    return NO_ERROR;
}



//@Function{API}: GetMaxlines

// Returns max number of log lines

//@
int GetMaxlines(void)
{
    return maxLogRecs ;
}


//@Function{API}: GetLoglines

// Returns current number of log lines

//@
int GetLoglines(void)
{
    return rCounter;
}


//@Function{API}: GetDebugLevel

// Returns current error level

//@
int GetDebugLevel(void)
{
    return DebugLevel;
}

//@Procedure{API}: CleanExit

// Cleans up everything end exits

// This routine is to be called when exiting from MsgdRTDB to perform
// clean up as required. It will be invoked also by the SIGKIL signal handler.
//@

static char *reasons[] = { TERM_REASON_0_MSG,
                           TERM_REASON_1_MSG,
                           TERM_REASON_2_MSG,
                           TERM_REASON_3_MSG };

void CleanExit(int why)
{

if(why<1 || why>TERM_MAX_REASON) why=0;

LAO_Info(NULL,"Exiting due to: %s",reasons[why]);

// Put here cleanup code, if needed

TerminateAllClientThreads();

if(LogFile != stdout) {
   LAO_Info(NULL,"Closing log file.");
   fclose(LogFile);
}
exit(0);
}




//@Function LAO_DumpCLtable

// Dumps the list of clients into log file.
// The output to log file is unconditional (i.e.: not affected by current
// log level)
//@

int LAO_DumpCLtable()
{
int i;
Client *aClient;
char chbuf[300];
char loghdr[MASTER_LOG_HEADER_LEN];

    if(LogFile==NULL) LogFile=stdout;

    MUTEX_LOCK(&(ClientTable.mutex),"LAO_DumpCLtable");
    MUTEX_LOCK(&LogMutex,"LAO_DumpCLtable");

    logString(progInfo.MyName,MASTER_LOG_LEV_INFO,NULL,loghdr);
    fprintf(LogFile,"%sClient list -- Max Clients: %d, Currently: %d\n",
                         loghdr,ClientTable.maxItem,ClientTable.nItems);
    rCounter++;
    for(i=1;i<=ClientTable.maxCurItem;i++) {
	if((aClient=ClientTable.Items[i])!=NULL) {
             LAO_clientstr(aClient,ClientTable.nConn[i],chbuf,300);
             fprintf(LogFile,"%s  %s\n",loghdr,chbuf);
             rCounter++;
        } else {
            if(*(ClientTable.Names[i])!='\0') {
                fprintf(LogFile,"%s  %s [Id=%d N.Conn=%d]  DISCONNECTED\n",loghdr,ClientTable.Names[i],i,ClientTable.nConn[i]);
                rCounter++;
            }
        }
    }
    if(rCounter++>maxLogRecs) rotateLogs(); else fflush(LogFile);

    MUTEX_UNLOCK(&LogMutex,"LAO_DumpCLtable");
    MUTEX_UNLOCK(&(ClientTable.mutex),"LAO_DumpCLtable");

    return NO_ERROR;
}



int LAO_DumpBstring(ShmListscan* listX,
                       char*        cbuf,
                       int          lng)
{
int alng;

alng=snprintf(cbuf,lng,"%2d - %s:%s len:%d NCons:%d MaxCons:%d [shmkey:0x%8.8x  semkey:0x%8.8x]",
                              listX->next, listX->bInfo->producer,
                              listX->bInfo->name, listX->bInfo->lng,
                              listX->cInfo->nCons, listX->bInfo->maxC,
                              listX->bInfo->shm_key, listX->bInfo->sem_key );
if(++alng>lng) alng=lng;
return alng;
}



//@Function LAO_DumpBufsList

// Dumps the list of clients into log file.
// The output to log file is unconditional (i.e.: not affected by current
// log level)
//@
int LAO_DumpBufsList()
{
extern pthread_mutex_t shmMgmtMutex;

ShmListscan listX;
char cbuf[200];
int vv=1;
char loghdr[MASTER_LOG_HEADER_LEN];

    if(LogFile==NULL) LogFile=stdout;

    MUTEX_LOCK(&shmMgmtMutex,"LAO_DumpBufsList");
    MUTEX_LOCK(&LogMutex,"LAO_DumpBufsList");

    logString(progInfo.MyName,MASTER_LOG_LEV_INFO,NULL,loghdr);
    fprintf(LogFile,"%sShared buffer list -- Max Buffers: %d, Currently: %d\n",
                       loghdr,MAX_SHM_BLOCKS,nBuffers());
    rCounter++;
    InitShmList(&listX);     // Initialize Shared Buffer list retrieval
    for(;;) {                // Scan shared buffer list
	vv=NextShmList(&listX);
        if(vv==0) break;
        LAO_DumpBstring(&listX,cbuf,200);
        fprintf(LogFile,"%s %s\n",loghdr,cbuf);
        rCounter++;
        for(vv=0;vv<MAX_CONSUMERS;vv++)  {
            Client *pt=listX.cInfo->cList[vv];
            if(pt) {
                 fprintf(LogFile,"%s    Client %2d  %s\n",loghdr,vv,pt->Name);
                 rCounter++;
            }
        }
    }
    if(rCounter++>maxLogRecs) rotateLogs(); else fflush(LogFile);

    MUTEX_UNLOCK(&LogMutex,"LAO_DumpBufsList");
    MUTEX_UNLOCK(&shmMgmtMutex,"LAO_DumpBufsList");

    return NO_ERROR;
}


//@Function LAO_DumpInfo

// Dumps MsgD-RTDB info onto log file

// The output to log file is unconditional (i.e.: not affected by current
// log level)
//@

int LAO_DumpInfo(int isstart)
{
int i;
char loghdr[MASTER_LOG_HEADER_LEN];

    if(LogFile==NULL) LogFile=stdout;

    MUTEX_LOCK(&LogMutex,"LAO_DumpInfo");

    logString(progInfo.MyName,MASTER_LOG_LEV_INFO,NULL,loghdr);
    if(isstart)
        fprintf(LogFile,"%sMsgD-RTDB start\n",loghdr);
    else
        fprintf(LogFile,"%sMsgD-RTDB info\n",loghdr);
    rCounter++;
    for(i=0;;i++) {
        char buff[1024];
        int info=LAO_InfoLine(i,buff,1024);
        if(info==0) break;
        fprintf(LogFile,"%s  %s\n",loghdr,buff);
        rCounter++;
    }
    if(rCounter++>maxLogRecs) rotateLogs(); else fflush(LogFile);

    MUTEX_UNLOCK(&LogMutex,"LAO_DumpInfo");

    return NO_ERROR;
}




//@Function LAO_DumpVstat

// Dumps variable table status onto log file

// The output to log file is unconditional (i.e.: not affected by current
// log level)
//@

#define ITEMS_X_LINE 100

int LAO_DumpVstat()
{
char loghdr[MASTER_LOG_HEADER_LEN];
extern VarTable variableTB;
char dbuff[300];

    if(LogFile==NULL) LogFile=stdout;

    MUTEX_LOCK(&(variableTB.mutex),"LAO_DumpVstat");

    snprintf(dbuff,300,"Hash table -- nItems: %d, hashSize: %d, ovflSize: %d, nColls: %d, nIns: %d, nRem: %d",
                                         variableTB.htable.nItms,
                                                    variableTB.htable.size,
                                                                  variableTB.htable.ovfl,
                                                                                variableTB.htable.nColls,
                                                                                         variableTB.htable.nInsert,
                                                                                                   variableTB.htable.nRemove);

    MUTEX_UNLOCK(&(variableTB.mutex),"LAO_DumpVstat");

    MUTEX_LOCK(&LogMutex,"LAO_DumpVstat");

    logString(progInfo.MyName,MASTER_LOG_LEV_INFO,NULL,loghdr);
    fprintf(LogFile,"%s%s\n",loghdr,dbuff);
    if(rCounter++>maxLogRecs) rotateLogs(); else fflush(LogFile);

    MUTEX_UNLOCK(&LogMutex,"LAO_DumpVstat");

    return NO_ERROR;
}




//@Procedure{API}: LAO_DumpVarList

// Print list of variables to Log file

// Printout a list of variables matching given prefix
//
// The request is not affected by current log level
//@


int LAO_DumpVarList(Client *cl,char *prefix)
{
   Listscan listX;
   char loghdr[MASTER_LOG_HEADER_LEN];
   char vnbuf[VAR_NAME_LEN+1];

   rmVDomain(prefix,vnbuf);

   if(LogFile==NULL) LogFile=stdout;

   MUTEX_LOCK(&LogMutex,"LAO_DumpVarList");

   logString(progInfo.MyName,MASTER_LOG_LEV_INFO,NULL,loghdr);
   fprintf(LogFile,"%sVariable list dump: %s\n",loghdr,vnbuf);

   InitVarScan(vnbuf,&listX,LOCKMUTEX);  // Initialize Variable list
                                       // retrieval (whith list mutex lock)
   for(;;) {                              // Scan variable list
       DB_Var *vpt=NextVarScan(&listX);
       if(vpt==NULL) break;        // List is finished
       LAO_DumpVar(cl,vpt,DONTLOCKMUTEX);
   }
   EndVarScan(&listX);

   MUTEX_UNLOCK(&LogMutex,"LAO_DumpVarList");

   return NO_ERROR;
}



static void DumpCList(CList *cl)
{
   int i;

   for(i=0;i<cl->nItms;i++) {
      fprintf(LogFile, " %s",(cl->body[i].fname));
   }
   fprintf(LogFile,"\n");
}

//@Procedure{API}: LAO_DumpNotifTable

// Print a notify table item to Log file

// Printout a Notify list item
// The request is not affected by current log level
//@

int LAO_DumpNotifTable(int both)
{
extern SortedTable notifyTB,mirrorTB;

char *fmth="%s  Prefix \"%s\":";
char loghdr[MASTER_LOG_HEADER_LEN];
Listscan listX;

if(LogFile==NULL) LogFile=stdout;

MUTEX_LOCK(&LogMutex,"LAO_DumpNotifTable");

logString(progInfo.MyName,MASTER_LOG_LEV_INFO,NULL,loghdr);
if(both) {
    InitTableScan(&notifyTB,&listX);
    fprintf(LogFile,"%sNotify table dump -- N.Items: %d\n",loghdr,notifyTB.stable.nItms);
    rCounter++;
    for(;;) {
        s_item *vv=NextTableScan(&listX);
        if(!vv) break;
        logString(progInfo.MyName,MASTER_LOG_LEV_INFO,NULL,loghdr);
        fprintf(LogFile, fmth,loghdr, vv->key);
        DumpCList(vv->value);
        rCounter++;
    }
    EndTableScan(&notifyTB);
    if(rCounter>maxLogRecs) rotateLogs(); else fflush(LogFile);
}

/*
InitTableScan(&mirrorTB,&listX);
fprintf(LogFile,"%sMirrored variable table dump -- N.Items: %d\n",loghdr,mirrorTB.stable.nItms);
rCounter++;
for(;;) {
    s_item *vv=NextTableScan(&listX);
    if(!vv) break;
    logString(progInfo.MyName,MASTER_LOG_LEV_INFO,NULL,loghdr);
    fprintf(LogFile, "%s %s\n",loghdr, vv->key);
    rCounter++;
}
EndTableScan(&mirrorTB);
if(rCounter>maxLogRecs) rotateLogs(); else fflush(LogFile);
*/

MUTEX_UNLOCK(&LogMutex,"LAO_DumpNotifTable");

return NO_ERROR;
}


//@Procedure{API}: LAO_DumpVar

// Print an RTDB variable to Log file

// Printout a Variable from RTDB onto the log file.
// The request is not affected by current log level
//@

#define VARSPERLINE  20

int LAO_DumpVar(Client *src,
                 DB_Var *vpt, 
                 int lock)
{
   char *fmth="%sName=%s Type=%s Size=%d LastModif=%s %s %s %s\n";
   char *type;
   char *locked,*trace,*global;
   int i,nit,ntype;
   char loghdr[MASTER_LOG_HEADER_LEN];
   char txt[30];
   localVar *vv;

   if(LogFile==NULL) LogFile=stdout;

   vv=vpt->Var;
   nit=vv->H.NItems;
   ntype=vv->H.Type;
   switch(ntype) {
      case INT_VARIABLE: type="Integer";  break;
      case REAL_VARIABLE: type="Real";    break;
      case CHAR_VARIABLE: type="String";  break;
      case BIT8_VARIABLE: type="Bit8";  break;
      case BIT16_VARIABLE: type="Bit16";  break;
      case BIT32_VARIABLE: type="Bit32";  break;
      case BIT64_VARIABLE: type="Bit64";  break;
   
      default:
         LAO_Error(src, VAR_TYPE_ERROR, "In LAO_DumpVar()");
         return NO_ERROR;
   }


   if(lock) {
      MUTEX_LOCK(&LogMutex,"LAO_DumpVar");
   }

   logString(progInfo.MyName,MASTER_LOG_LEV_INFO,NULL,loghdr);
   locked=(vpt->Lock==PUBLIC_VAR)?"unlocked":"locked";
   trace=(vpt->trace)?"trace":"notrace";
   global=(vpt->Mirrored)?"global":"local";
   timeString(&(vv->H.MTime),txt);
   fprintf(LogFile, fmth,loghdr, vpt->Name,type,nit,txt,locked,trace,global);
   fprintf(LogFile, "%s Notify to:",loghdr);
   for(i=0;i<(vpt->NList)->nItms;i++) 
      fprintf(LogFile," %s",(vpt->NList->body[i]).fname);
    
   if(ntype==CHAR_VARIABLE) {
      fprintf(LogFile,"\n%s ",loghdr);
      for(i=0;i<nit;i++) fprintf(LogFile,"%c",vv->Value.Sv[i]);
   } else {
      if(nit==0) {
         fprintf(LogFile,"\n%s UNDEFINED",loghdr);
      } else {
         for(i=0;i<nit;i++) {
            if(i%VARSPERLINE == 0) fprintf(LogFile,"\n%s ",loghdr);
            switch(ntype) {
               case INT_VARIABLE: fprintf(LogFile," %ld",vv->Value.Lv[i]); break;
               case REAL_VARIABLE: fprintf(LogFile," %f",vv->Value.Dv[i]); break;
               case BIT8_VARIABLE: fprintf(LogFile," %02x",vv->Value.B8[i]); break;
               case BIT16_VARIABLE: fprintf(LogFile," %04hx",vv->Value.B16[i]); break;
               case BIT32_VARIABLE: fprintf(LogFile," %08lx",vv->Value.B32[i]); break;
               case BIT64_VARIABLE: fprintf(LogFile," %016llx",vv->Value.B64[i]); break;
            }
         }
      }
   }
   fprintf(LogFile,"\n");
   if(rCounter++>maxLogRecs) rotateLogs(); else fflush(LogFile);

   if(lock) {
      MUTEX_UNLOCK(&LogMutex,"LAO_DumpVar");
   }
   return NO_ERROR;
}

int LAO_clientstr(Client* aClient,
                  int     nConn,
                  char*   chbuf,
                  int     lng)
{
    char *isready,*ctype;
    char aux[60];
    char tmstr[30];
    int alng;

    if(aClient->ClientReady)
        isready="R ";
    else
        isready="NR";

    if(aClient->clType==STD_CLIENT)
       ctype="Clnt";
    else
       ctype="Peer";

    aux[0]='\0';
    if(aClient->Waiting[0]) 
        snprintf(aux,60,"W:%s",aClient->Waiting);
    else
        snprintf(aux,60,"NW");

    timeString(&(aClient->startTm),tmstr);

    alng=snprintf(chbuf,lng,"%s (%s) [Id=%d N.Conn=%d] %s %s %s  @=%s Start:%s",
             aClient->Name,ctype,aClient->Id,nConn,aClient->version,
             isready,aux,aClient->conn.ip,tmstr);

    if(++alng>lng) alng=lng;
    return alng;
}



//@Function: LAO_InfoString
//
// Stores all program information in a single string with '\n' separators.
// a '\0' terminator is always appended.
//
// Returns the total length of the string includeing the trailing '\0'. 
// If buffer space is not enough it is filled up and the value -1 is returned.
//@

int LAO_InfoString(char *buf, int buflng)
{
int i,rest=buflng,tot=0;
char *pt;

for(i=0,pt=buf;rest>0;i++) {
   int ret=LAO_InfoLine(i,pt,rest);
   if(ret==0) break;
   if(ret>=rest) {
      tot=(-1);
      break;
   }
   rest -= ret;
   pt+=ret;
   tot+=ret;
   if(rest>0) {
      *(pt++) ='\n';
      rest--;
      tot++;
   }
}
return tot;
}



//@Function: LAO_InfoLine
//
// Writes into buffer a line of Program information, propely formatted
//@
int LAO_InfoLine(int line, char *buf, int buflng)
{
    struct timeval now;
    long nsecs;
    int hours,minutes;
    char tmstr[30];
    int len=0;

    switch(line) {
    case 0:
       len = snprintf(buf,buflng,"MsgD-RTDB [%s]  %s",progInfo.MyName,GetVersionID());
       break;
    case 1:
       gettimeofday(&now, NULL);   
       nsecs=(now.tv_sec-progInfo.startTime.tv_sec);
       hours=nsecs/3600;
       nsecs -= hours*3600;
       minutes=nsecs/60;
       nsecs -= minutes*60;
       timeString(&progInfo.startTime,tmstr);
       len = snprintf(buf,buflng,"Started on: %s - uptime (hours): %d:%2.2d:%2.2d",tmstr,hours,minutes,(int)nsecs);
       break;
    case 2:
       len = snprintf(buf,buflng,"Debug Flags: %s %s %s %s",PEER_DEBUG_INFO,MEM_DEBUG_INFO,MUTEX_DEBUG_INFO,MUTEX_ERROR_INFO);
       break;
    case 3:
       len = snprintf(buf,buflng,"Main PID: %d",progInfo.MainPID);
       break;
    case 4:
       len = snprintf(buf,buflng,"Listening on port: %d",progInfo.lPort);
       break;
    case 5:
       len = snprintf(buf,buflng,"Msg Header Id: %d",progInfo.msgMagic);
       break;
    case 6:
       len = snprintf(buf,buflng,"Max num of returned msgs: %d",progInfo.maxRetMsgs);
       break;
    case 7:
       len = snprintf(buf,buflng,"NetIO Timeout: %d (microsec)",progInfo.netIoTmout);
       break;
    case 8:
       len = snprintf(buf,buflng,"Config file: %s",progInfo.cfgName);
       break;
    case 9:
       if(LogFile && LogFile!=stdout)
           len = snprintf(buf,buflng,"Log file: %s/%s.log",progInfo.logPath,progInfo.MyName);
       else
           len = snprintf(buf,buflng,"Log file: stdout");
       break;
    case 10:
       len = snprintf(buf,buflng,"Log lines: %d/%d",GetLoglines(),GetMaxlines());
       break;
    case 11:
       len = snprintf(buf,buflng,"Debug level: %d.  Error count: %d",GetDebugLevel(),errCount);
       break;
    case 12:
       len = snprintf(buf,buflng,"N.Clients: %d/%d. N.Peers: %d/%d",ClientTable.nItems,ClientTable.maxItem,
                                                                    ClientTable.nPeers,ClientTable.maxPeer);
       break;
    case 13:
       len = snprintf(buf,buflng,"N.Buffers: %d/%d",nBuffers(),MAX_SHM_BLOCKS);
       break;
    case 14:
       len = snprintf(buf,buflng,"N.Variables: %d",nVars());
       break;
    default:
       len=0;
    }
    return len;
}


